#ifndef _ZPP_PB2JSON_H_
#define _ZPP_PB2JSON_H_
/**@file zpp/json2pb2json.h
 * @brief translate json to protobuf or translate protobuf to json
 * @ The JSON representation of an `Any` value uses the regular
 *  representation of the deserialized, embedded message, with an
 *  additional field `@type` which contains the type URL. Example:
 *
 *      package google.profile;
 *      message Person {
 *        string first_name = 1;
 *        string last_name = 2;
 *      }
 *
 *      {
 *        "@type": "type.googleapis.com/google.profile.Person",
 *        "firstName": <string>,
 *        "lastName": <string>
 *      }
 *      // personal "@Any":"packet.name"
 *      // short name "@any":"name"
 */
#include "json_rapid.hpp"
#include <google/protobuf/message.h>

namespace z{
    namespace json{
        int Json2pb(RJson *json, google::protobuf::Message **pb);
        int Pb2json(RJson *json, const google::protobuf::Message *pb);
    }
}

#endif