#include <google/protobuf/descriptor.h>
#include <google/protobuf/any.pb.h>
#include <vector>
#include "../json_rapid.hpp" // for no install pass make
using google::protobuf::Message;
using google::protobuf::MessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::UnknownFieldSet;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::Reflection;
using google::protobuf::Any;

namespace z{
    namespace json{
        const char *NAME_ANY = "google.protobuf.Any";
        int Pb2json(RJson *json, const Message *pb);
        static int Field2json(RJson *json, const Message *pb, const FieldDescriptor *field, int idx){
            int ret = ZOK;
            const Reflection *ref = pb->GetReflection();
            const bool repeated = field->is_repeated();
            //const std::string &key = field->json_name(); // foo_bar => fooBar
            const std::string &key = field->name(); // foo_bar => foo_bar
            switch (field->cpp_type())
            {
            case FieldDescriptor::CPPTYPE_DOUBLE:
            {
                double val = repeated ? ref->GetRepeatedDouble(*pb, field, idx) : ref->GetDouble(*pb, field);
                repeated ? json->PushBack(val) : json->SetDouble(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_FLOAT:
            {
                double val = repeated ? ref->GetRepeatedFloat(*pb, field, idx) : ref->GetFloat(*pb, field);
                repeated ? json->PushBack(val) : json->SetDouble(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_INT64:
            {
                int64_t val = repeated ? ref->GetRepeatedInt64(*pb, field, idx) : ref->GetInt64(*pb, field);
                repeated ? json->PushBack(val) : json->SetInt64(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_UINT64:
            {
                uint64_t val = repeated ? ref->GetRepeatedUInt64(*pb, field, idx) : ref->GetUInt64(*pb, field);
                repeated ? json->PushBack(val) : json->SetUint64(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_INT32:
            {
                int32_t val = repeated ? ref->GetRepeatedInt32(*pb, field, idx) : ref->GetInt32(*pb, field);
                repeated ? json->PushBack(val) : json->SetInt(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_UINT32:
            {
                uint32_t val = repeated ? ref->GetRepeatedUInt32(*pb, field, idx) : ref->GetUInt32(*pb, field);
                repeated ? json->PushBack(val) : json->SetUint(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_BOOL:
            {
                bool val = repeated ? ref->GetRepeatedBool(*pb, field, idx) : ref->GetBool(*pb, field);
                repeated ? json->PushBack(val) : json->SetBool(key.c_str(), val);
            }
            break;
            case FieldDescriptor::CPPTYPE_STRING:
            {
                std::string scratch;
                const std::string &val = repeated ? ref->GetRepeatedStringReference(*pb, field, idx, &scratch) : ref->GetStringReference(*pb, field, &scratch);
                if(field->type() == FieldDescriptor::TYPE_BYTES){
                    ZERRC(ZNOT_SUPPORT); // Any value;
                }else{
                    repeated ? json->PushBack(rapidjson::Value(val.c_str(), *json->GetAllocator()).Move()) : json->SetString(key.c_str(), val.c_str());
                }
            }
            break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
            {
                const Message &val = repeated ? ref->GetRepeatedMessage(*pb, field, idx) : ref->GetMessage(*pb, field);
                RJson js(json->GetAllocator());
                js.SetObject();
                Pb2json(&js, &val);
                repeated ? json->PushBack(js.val->Move()) : json->AddMember(key.c_str(), &js);
            }
            break;
            case FieldDescriptor::CPPTYPE_ENUM:
            {
                uint32_t val = repeated ? ref->GetRepeatedEnum(*pb, field, idx)->number() : ref->GetEnum(*pb, field)->number();
                repeated ? json->PushBack(val) : json->SetUint(key.c_str(), val);
            }
            break;
            default:
                ZERRC(ZNOT_SUPPORT);
            }
            return ret;
        }

        typedef struct message_t{
            char flag; // 0-msg 1-array
            Message *msg; // msg
            const char *key; // key or array name
        }msg_t;

        typedef std::vector<msg_t> Msgs;

        static int CreateMessage(const char *name, Msgs &msgs){
            const Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(name);
            if(!descriptor){return ZCAST_FAIL;}
            const Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
            //if(!prototype){return ZCAST_FAIL;}
            Message* new_obj = prototype->New();
            if(!new_obj){return ZMEM_INSUFFICIENT;}
            msg_t &msg = msgs.back();
            msg.msg = new_obj;
            if(NULL == msg.key){
                // get parent array name
                if(msgs.size() >= 2){
                    msg.key = msgs[msgs.size()-2].key; // array parent message
                }else{
                    msg.key = name;
                }
            }
            msg.flag = 0;
            return ZOK;
        }

        static int SetMessage(RJson::ValueType status, const char *key, RJson &val, Msgs &msgs){
            int ret = ZPARAM_INVALID;
            Message *msg = msgs.back().msg;
            if(!key){
                key = msgs.back().key; // array name
                msg = msgs[msgs.size()-2].msg; // array parent message
            }
            if(!msg && key && '@' != *key){
                // can not cover to Message
                ZERRC(ret);
                return ret;
            }
            switch(status){
            case RJson::valString:
                if(key && '@' == *key){
                    ret = CreateMessage(val.GetCString(), msgs);
                }else{
                    const Reflection *ref = msg->GetReflection();
                    const Descriptor *d = msg->GetDescriptor();
                    if(!d || !ref){
                        ZERRC(ret);
                        break;}
                    const FieldDescriptor *field = d->FindFieldByName(key);
                    if(!field || (field->cpp_type() != FieldDescriptor::CPPTYPE_STRING)){
                        ZERRC(ret);
                        break;
                    }
                    field->is_repeated() ? ref->AddString(msg, field, val.GetCString()) : ref->SetString(msg, field, val.GetCString());
                    ret = ZOK;
                }
                break;
            case RJson::valNumber:{
                const Reflection *ref = msg->GetReflection();
                const Descriptor *d = msg->GetDescriptor();
                if(!d || !ref){
                    ZERRC(ret);
                    break;
                }
                const FieldDescriptor *field = d->FindFieldByName(key);
                if(!field){
                    ZERRC(ret);
                    break;
                }
                switch(field->cpp_type()){
                case FieldDescriptor::CPPTYPE_DOUBLE:
                    if(val.IsDouble()){
                        field->is_repeated() ? ref->AddDouble(msg, field, val.val->GetDouble()) : ref->SetDouble(msg, field, val.val->GetDouble());
                    }
                    break;
                case FieldDescriptor::CPPTYPE_FLOAT:
                    if(val.IsDouble()){
                        field->is_repeated() ? ref->AddFloat(msg, field, (float)val.val->GetDouble()) : ref->SetFloat(msg, field, (float)val.val->GetDouble());
                    }
                    break;
                case FieldDescriptor::CPPTYPE_INT32:
                    if(val.IsInt()){
                        field->is_repeated() ? ref->AddInt32(msg, field, val.val->GetInt()) : ref->SetInt32(msg, field, val.val->GetInt());
                    }
                    break;
                case FieldDescriptor::CPPTYPE_UINT32:
                    if(val.IsUint()){
                        field->is_repeated() ? ref->AddUInt32(msg, field, val.val->GetUint()) : ref->SetUInt32(msg, field, val.val->GetUint());
                    }
                    break;
                case FieldDescriptor::CPPTYPE_UINT64:
                    if(val.IsUint64()){
                        field->is_repeated() ? ref->AddUInt64(msg, field, val.val->GetUint64()) : ref->SetUInt64(msg, field, val.val->GetUint64());
                    }
                    break;
                case FieldDescriptor::CPPTYPE_INT64:
                    if(val.IsInt64()){
                        field->is_repeated() ? ref->AddInt64(msg, field, val.val->GetInt64()) : ref->SetInt64(msg, field, val.val->GetInt64());
                    }
                    break;
                default:
                    ret = ZCAST_FAIL;
                    break;
                }
                if(ret != ZCAST_FAIL){
                    ret = ZOK;
                }
            }
                break;
            case RJson::valFalse:
            case RJson::valTrue:{
                const Reflection *ref = msg->GetReflection();
                const Descriptor *d = msg->GetDescriptor();
                if(!d || !ref){break;}
                const FieldDescriptor *field = d->FindFieldByName(key);
                if(!field || (field->cpp_type() != FieldDescriptor::CPPTYPE_BOOL)){break;}
                bool bVal = (status == RJson::valTrue) ? true : false;
                field->is_repeated() ? ref->AddBool(msg, field, bVal) : ref->SetBool(msg, field, bVal);
                ret = ZOK;
            }
                break;
            default:
                ret = ZNOT_SUPPORT;
            }
            ZERRCX(ret);
            return ret;
        }

        static int MergeMessage(const char *key, Message *parent, Message *child){
            int ret = ZPARAM_INVALID;
            const Reflection *ref = parent->GetReflection();
            const Descriptor *d = parent->GetDescriptor();
            if(!d || !ref){
                ZERRC(ret);
                return ret;
            }
            const FieldDescriptor *field = d->FindFieldByName(key);
            if(!field){
                ZERRC(ret);
                return ret;
            }

            Message *refMsg = field->is_repeated() ? ref->AddMessage(parent, field) : ref->MutableMessage(parent, field);
            if(refMsg->GetDescriptor()->full_name() == NAME_ANY){
                Any *any = dynamic_cast<Any*>(refMsg);
                if(any){
                    any->PackFrom(*child);
                }
            }else{
                child->GetReflection()->Swap(refMsg, child);
            }
            return ret;
        }
        static int Json2field(RJson::ValueType status, RJson &name, RJson &value, void *hint){
            int ret = ZPARAM_INVALID;
            Msgs *msgs = (Msgs*)hint;
            msg_t msg;

            switch(status){
            case RJson::valFalse:
            case RJson::valTrue:
            case RJson::valString:
            case RJson::valNumber:
                ret = SetMessage(status, name.val ? name.GetCString() : NULL, value, *msgs);
                break;
            case RJson::statArrayBegin:
                msg.flag = 1;
                name.val ? msg.key = name.GetCString() : msg.key = NULL;
                msgs->push_back(msg);
                ret = ZOK;
                break;
            case RJson::statArrayEnd:
                msgs->pop_back();
                ret = ZOK;
                break;
            case RJson::statObjectBegin:
                msg.flag = 0;
                msg.msg = NULL;
                msg.key = name.val ? name.GetCString() : NULL;
                msgs->push_back(msg);
                ret = ZOK;
                break;
            case RJson::statObjectEnd:
                if(msgs->size() > 1){
                    msg = msgs->back();
                    msgs->pop_back();
                    if(msg.flag != 0){
                        break; // not a message; impossible.
                    }
                }else{
                    ret = ZOK; // this is the root message;
                    break;
                }

                if(msgs->size()){
                    msg_t &parent = msgs->back();
                    if(parent.flag != 0){
                        // It's Array or other get parent again
                        if(msgs->size() < 2){
                            break; // array has no parent, impossible
                        }
                        msg_t &father = msgs->at(msgs->size() - 2);
                        if(father.flag != 0){
                            break; // father is not message; impossible.
                        }
                        ret = MergeMessage(msg.key, father.msg, msg.msg);
                    }else{
                        // It's Message
                        ret = MergeMessage(msg.key, parent.msg, msg.msg);
                    }
                    delete msg.msg;
                }
                ret = ZOK;
                break;
            case RJson::valNull:
            case RJson::valObject:
            case RJson::valArray:
                ret = ZOK; // dummy
                break;
            default:
                break;
            }
            ZERRCX(ret);
            return ret;
        }

        int Json2pb(RJson *json, Message **pb){
            int ret = ZOK;
            if(!json || !pb){
                return ZPARAM_INVALID;
            }
            *pb = NULL;
            Msgs msgs;
            ret = json->ForEach(Json2field, &msgs);
            if(ZOK == ret){
                if(1 == msgs.size()){
                    *pb = msgs.back().msg;
                }else{
                    ret = ZPARAM_INVALID;
                }
            }
            ZERRCX(ret);
            return ret;
        }
        int Json2pb(RJson *json, google::protobuf::Message &pb){
            int ret = ZOK;
            if(!json){
                return ZPARAM_INVALID;
            }
            Msgs msgs;
            ret = json->ForEach(Json2field, &msgs);
            if(ZOK == ret){
                if(1 == msgs.size()){
                    pb.GetReflection()->Swap(&pb, msgs.back().msg);
                }else{
                    ret = ZPARAM_INVALID;
                }
            }
            ZERRCX(ret);
            return ret;
        }
        int Pb2json(RJson *json, const Message *pb){
            int ret = ZOK;
            const Descriptor *d = pb->GetDescriptor();
            const Reflection *ref = pb->GetReflection();
            const FieldDescriptor *field = NULL;
            if (!d || !ref) return(ZPARAM_INVALID);

            // Any protobuf
            if(d->full_name() == NAME_ANY){
                const Any* any = dynamic_cast<const Any*>(pb);
                std::string scratch;
                std::string full_type_name;
                const FieldDescriptor* name_field;
                const FieldDescriptor* value_field;
                google::protobuf::internal::GetAnyFieldDescriptors(*pb, &name_field, &value_field);
                const std::string &type_name = ref->GetStringReference(*pb, name_field, &scratch);
                google::protobuf::internal::ParseAnyTypeUrl(type_name, &full_type_name);
                const Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(full_type_name);
                if(!descriptor){return ZCAST_FAIL;}
                const Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
                //if(!prototype){return ZCAST_FAIL;}
                Message* new_obj = prototype->New();
                if(!new_obj){return ZMEM_INSUFFICIENT;}

                ret = any->UnpackTo(new_obj) ? ZOK : ZCAST_FAIL;
                if(ret == ZOK){
                    Pb2json(json, new_obj);
                }
                delete new_obj;
                ZERRCX(ret);
                return ret;
            }


            // normal protobuf
            std::vector<const FieldDescriptor *> fields;
            ref->ListFields(*pb, &fields);
            json->SetObject();
            json->SetString("@type", d->full_name().c_str());
            for (size_t i = 0; i != fields.size(); i++)
            {
                field = fields[i];
                if(field->is_repeated()){
                    size_t count = ref->FieldSize(*pb, field);
                    if(!count)continue;
                    RJson jsArr(json->GetAllocator());
                    jsArr.SetArray();
                    for(size_t j=0; j<count; j++){
                        if(ZOK != (ret = Field2json(&jsArr, pb, field, j)))break;
                    }
                    json->AddMember(field->name().c_str(), &jsArr);
                }else if(ref->HasField(*pb, field)){
                    if(ZOK != (ret = Field2json(json, pb, field,0)))break;
                }else{
                    ret = ZNOT_EXIST;
                    break;
                }
            }
            ZERRCX(ret);
            return(ret);
        }
    }
}
