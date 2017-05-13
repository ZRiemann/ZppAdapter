#include "tstzmq.h"
#include <zpp/zppzmq.h>
#include <zpp/zppmysql.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/thread/thread.h>
#include <string.h>
#include <stdlib.h>

#include <zpp/zrpdjson.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

using namespace z;
using namespace rapidjson;

void mysql_base(int argc, char **argv);

void rjson_filewr(int argc, char **argv);
void rjson_base(int argc, char **argv);

void svr_rep(int argc, char **argv);
void cli_req(int argc, char **argv);
void cli_req_percon(int argc, char **argv);
void cli_lazy_pirate(int argc, char **argv);
void svr_pub(int argc, char **argv);
void cli_sub(int argc, char **argv);

void tstzmq(int argc, char **argv){
  z::ZmqSocket::ZmqInit();
  printf("useage: basezmq svr_rep <port>\n"
	 "useage: basezmq (notsupport)svr_rep_udp <port>\n"
	 "useage: basezmq cli_req <request-num> <endpoints>\n"
	 "useage: basezmq cli_req_percon <request-num> <endp1> <endp2>\n"
	 "useage: basezmq cli_lazy_pirate <request-num> <endpoints...>\n"
	 "useage: basezmq svr_pub <port> <interval> [<begin> <end>]\n"
	 "useage: basezmq cli_sub <id/0> <endps> <end1> ... <endn> [<subs...>]\n"
	 "\nusage: basezmq rjson_base\n"
	 "useage: basezmq rjson_filewr\n"
	 "\nusage: basezmq mysql_base\n"
	 );
  
  if(argc > 2 && strcmp("svr_rep", argv[1]) == 0){
    svr_rep(argc, argv);
  }else if(argc > 2 && strcmp("cli_req", argv[1]) == 0){
    cli_req(argc, argv);
  }else if(argc > 2 && strcmp("cli_lazy_pirate", argv[1]) == 0){
    cli_lazy_pirate(argc, argv);
  }else if(argc > 2 && strcmp("svr_pub", argv[1]) == 0){
    svr_pub(argc, argv);
  }else if(argc > 2 && strcmp("cli_sub", argv[1]) == 0){
    cli_sub(argc, argv);
  }else if(argc > 2 && strcmp("svr_rep_udp", argv[1]) == 0){
    printf("ZMQ_REP not support udp, only ZMQ_REDIO/DISH support udp\n");
  }else if(argc > 2 && strcmp("cli_req_percon", argv[1]) == 0){
    cli_req_percon(argc, argv);
  }else if(argc >= 2 && strcmp("rjson_base", argv[1]) == 0){
    rjson_base(argc, argv);
  }else if(argc >= 2 && strcmp("rjson_filewr", argv[1]) == 0){
    rjson_filewr(argc, argv);
  }else if(argc >= 2 && strcmp("mysql_base", argv[1]) == 0){
    mysql_base(argc, argv);
  }else{
    printf("not support argvs, exit now.\n");
  }
  
  z::ZmqSocket::ZmqFini();
}

void svr_rep(int argc, char **argv){
  printf("run rep server, just echo back req\n");
  char endpoint[32];
  ZmqSocket rep(ZMQ_REP);
  sprintf(endpoint, "tcp://*:%s", argv[2]);
  printf("bind %s\n", endpoint);
  rep.Bind(endpoint);

  while(1){
    ZmqMsg msgRecv;
    ZmqMsg msgSend;
    
    rep.MsgRecv(&msgRecv, 0);

    msgSend.Copy(&msgRecv);
    rep.MsgSend(&msgSend, 0);
  }
}

void cli_req(int argc, char **argv){
  int total = atoi(argv[2]);
  int i;

  int initOk;
  void *tick;
  printf("run req client, with %d reqs\n", total);
  ZmqSocket req(ZMQ_REQ);
  for(i = 3; i<argc; ++i){
    req.Connect(argv[i]);
  }
  //sleep(2);
  tick = ztick();
  char buf[32];
  for(i=0; i<total; ++i){
    ZmqMsg msgRecv;
    sprintf(buf,"%d",i);
    ZmqMsg msgSend(&initOk, buf, strlen(buf), NULL,  NULL);

    req.MsgSend(&msgSend, 0);
    req.MsgRecv(&msgRecv, 0);
  }
  ztock(tick, NULL,NULL);

}

void cli_req_percon(int argc, char **argv){
  int total = atoi(argv[2]);
  int i;
  int j = 3;
  int initOk;
  void *tick;
  printf("run req client, with %d reqs\n", total);
  //ztrace_ctl(0x0c);
  //sleep(2);

  char buf[32];
  tick = ztick();
  for(i=0; i<total; ++i){
    ZmqSocket req(ZMQ_REQ);
    j = 3 == j ? 4 : 3;
    req.Connect(argv[j]);
    ZmqMsg msgRecv;
    sprintf(buf,"%d",i);
    ZmqMsg msgSend(&initOk, buf, strlen(buf), NULL,  NULL);

    req.MsgSend(&msgSend, 0);
    req.MsgRecv(&msgRecv, 0);
  }
  ztock(tick, NULL, NULL);
}

void cli_lazy_pirate(int argc, char **argv){
  int total = atoi(argv[2]);
  int i;
  int initOk;
  void *tick;
  printf("run req client, with %d reqs\n", total);
  ZmqSocket req(ZMQ_REQ);
  for(i = 3; i<argc; ++i){
    req.Connect(argv[i]);
  }
  //sleep(2);

  char buf[32];
  tick = ztick();
  for(i=0; i<total; ++i){
    ZmqMsg msgRecv;
    sprintf(buf,"%d",i);
    ZmqMsg msgSend(&initOk, buf, strlen(buf), NULL,  NULL);

    req.LazyPirateReq(&msgSend, &msgRecv, 2,2);
  }
  ztock(tick, NULL, NULL);
}

void svr_pub(int argc, char **argv){
  ZmqSocket pub(ZMQ_PUB);

  char endpoint[32];
  sprintf(endpoint, "tcp://*:%s", argv[2]);
  pub.Bind(endpoint);

  int interval = atoi(argv[3]);
  if(interval < 0 || interval > 10000){
    interval = 1000; //1sec
  }

  int begin, end; // filter id begin~end
  if(argc == 6){
    begin = atoi(argv[4]);
    end = atoi(argv[5]);
    if(begin < 10){
      begin = 1;
    }
    if(end <= begin){
      end = begin + 10;
    }
  }else{
    begin = end = -1;
  }

  int initOk;
  char buf[128]; // pub data
  if(begin == -1){
    // no filter
    while(1){
      sprintf(buf, "pub: %d", zrandin(100));
      ZmqMsg msg(&initOk, buf, strlen(buf), NULL, NULL);
      pub.MsgSend(&msg, 0);
      zsleepms(interval);
    }
  }else{
    // filter id
    char id[32];
    while(1){
      sprintf(buf, "pub: %d", zrandin(100));
      sprintf(id, "%d", zrandat(begin, end));

      ZmqMsg msgId(&initOk, id, strlen(id), NULL, NULL);
      ZmqMsg msgData(&initOk, buf, strlen(buf), NULL, NULL);

      pub.MsgSend(&msgId, 0);
      pub.MsgSend(&msgData, 0);
      
      zsleepms(interval);
    }
  }
}

void cli_sub(int argc, char **argv){
  ZmqSocket *sub = NULL;
  
  if(argv[2][0] != '0'){
    sub = new ZmqSocket(ZMQ_SUB, argv[2]);
  }else{
    sub = new ZmqSocket(ZMQ_SUB);
  }
  
  int endps = atoi(argv[3]);

  while(endps){
    sub->Connect(argv[endps+3]);
    --endps;
  }
  
  endps = atoi(argv[3]);
  
  if(argc > (endps + 4)){
    // sub filter
    int subids = argc - 4 - endps;
    while(subids){
      sub->Subscribe(argv[endps+3+subids], strlen(argv[endps+3+subids]));
      --subids;
    }
  }else{
      sub->SubAll();
    }
  
  while(1){
    do{
      ZmqMsg msg;
      sub->MsgRecv(&msg, 0);      
    }while(1 == sub->IsMore());
  }
  
  if(sub){
    delete sub;
  }
}

void rjson_base(int argc, char **argv){
  Document *doc;
  Value v;//(kObjectType);
  FILE* fp;
  
  doc = new Document;
  printf("test json base\n");

  doc->Parse("{\"aaa\":11}");
  //doc->SetObject();
  doc->SetNull();
  doc->SetObject();
  //doc->setInt(10);
  //v = 10;
  //(*doc)["first"] = 10;
  doc->AddMember("contacts", "abc", doc->GetAllocator());
  doc->AddMember("contacts1", 10, doc->GetAllocator());

  v.SetObject();
  v.AddMember("v1", 11,  doc->GetAllocator());
  doc->AddMember("contacts3", v, doc->GetAllocator());
  printf("v isnull<%d>\n", v.IsNull());
  doc->AddMember("contacts2", v, doc->GetAllocator());
  printf("v isnull<%d>\n", v.IsNull());

  doc->EraseMember("contacts2");
  printf("-----------------------\n");
  printf("doc[contaccts1] = %d\n", (*doc)["contacts1"].GetInt());
  //Document d;
  //d.Parse(json);
  // ...
  fp = fopen("output.json", "w"); // 非 Windows 平台使用 "w"
  char writeBuffer[4906];
  FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  Writer< FileWriteStream > writer(os);
  //d.Accept(writer);
  doc->Accept(writer);
  fclose(fp);
  delete doc;
}

void rjson_filewr(int argc, char **argv){
  zdbg("RJson file read write");

  RJson doc;
  

  doc.LoadFile("output.json");
  
  doc.SaveFile("output1.json");

  
}

void mysql_base(int argc, char **argv){
  MySql::traceFlag = 0xff;
  MySql mysql;
}
