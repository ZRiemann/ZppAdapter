/**@file tests/basereq_rep.c
 * @brief 基准测试程序
 */
#include <stdio.h>
#include <zmq.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <zsi/base/time.h>
#include <zsi/base/trace.h>
#include <zsi/zpp/trace2console.h>

void svr_rep(int argc, char **argv);
void cli_req(int argc, char **argv);
void cli_req_percon(int argc, char **argv);
void svr_rep_udp(int argc, char **argv);


void svr_pub(int argc, char **argv);
void cli_sub(int argc, char **argv);

static void *g_ctx;

int main(int argc, char **argv){
  ztrace_reg(ztrace_console, NULL);
  
  printf("useage: basezmq svr_rep <port>\n"
	 "useage: basezmq (not support)svr_rep_udp <port>\n"
	 "useage: basezmq cli_req <request-num> <endpoints>\n"
	 "useage: basezmq cli_req_percon <request-num> <end1> <endp2>\n\n"
	 "nuseage: basezmq svr_pub <port> <interval> [<begin> <end>]\n"
	 "useage: basezmq cli_sub <id/0> <endps> <end1> ... <endn> [<subs...>]\n"
	 );
  g_ctx = zmq_ctx_new();
  if(argc > 2 && strcmp("svr_rep", argv[1]) == 0){
    svr_rep(argc, argv);
  }else if(argc > 2 && strcmp("cli_req", argv[1]) == 0){
    cli_req(argc, argv);
  }else if(argc > 2 && strcmp("svr_pub", argv[1]) == 0){
    svr_pub(argc, argv);
  }else if(argc > 2 && strcmp("cli_sub", argv[1]) == 0){
    cli_sub(argc, argv);
  }else if(argc > 2 && strcmp("cli_req_percon", argv[1])==0){
    cli_req_percon(argc, argv);
  }else if(argc > 2 && strcmp("svr_rep_udp", argv[1])==0){
    svr_rep_udp(argc, argv);
  }else{
    printf("not support argvs, exit now.\n");
  }
  
  zmq_ctx_term(g_ctx);
}

void svr_rep(int argc, char **argv){
  printf("run rep server, just echo back req\n");
  char endpoint[32];
  void *rep = zmq_socket(g_ctx, ZMQ_REP);
  sprintf(endpoint, "tcp://*:%s", argv[2]);
  printf("bind %s\n", endpoint);
  zmq_bind(rep, endpoint);

  while(1){
    zmq_msg_t msg_recv;
    zmq_msg_t msg_send;

    zmq_msg_init(&msg_recv);
    zmq_msg_init(&msg_send);

    zmq_msg_recv(&msg_recv, rep, 0);
    printf("msg<size:%lu>\n", zmq_msg_size(&msg_recv));
    zmq_msg_copy(&msg_send, &msg_recv);
    zmq_msg_send(&msg_send, rep, 0);

    zmq_msg_close(&msg_recv);
    zmq_msg_close(&msg_send);
  }
  zmq_close(rep);
}

void svr_rep_udp(int argc, char **argv){
  printf("ZMQ_REP not support udp, only ZMQ_REDIO/DISH support udp\n");
}

void cli_req(int argc, char **argv){
  int total = atoi(argv[2]);
  int i;
  time_t old;
  printf("run req client, with %d reqs\n", total);
  void *req = zmq_socket(g_ctx, ZMQ_REQ);
  for(i = 3; i<argc; ++i){
    printf("connect to %s\n", argv[i]);
    zmq_connect(req, argv[i]);
  }

  //sleep(2);
  time(&old);
  char buf[32];
  for(i=0; i<total; ++i){
    zmq_msg_t msg_recv;
    zmq_msg_t msg_send;

    zmq_msg_init(&msg_recv);
    sprintf(buf,"%d",i);
    zmq_msg_init_data(&msg_send, buf, strlen(buf), NULL, NULL);
    
    zmq_msg_send(&msg_send, req, 0);

    zmq_msg_recv(&msg_recv, req, 0);
    //printf("reply ok.\n");
    zmq_msg_close(&msg_recv);
    zmq_msg_close(&msg_send);
  }
  old = time(NULL) - old;
  printf("totoal use %d seconds\n", (int)old);

  zmq_close(req);
}

void cli_req_percon(int argc, char **argv){
  int total = atoi(argv[2]);
  int i;
  time_t old;
  printf("run req client, with %d reqs\n", total);
  void *req = zmq_socket(g_ctx, ZMQ_REQ);
  //sleep(2);
  time(&old);
  char buf[32];
  int j = 3;
  for(i=0; i<total; ++i){
    //for(i = 3; i<argc; ++i){
      //printf("connect to %s\n", argv[i]);
    if(j == 3){
      j = 4;
    }else{
      j = 3;
    }
    zmq_connect(req, argv[j]);
      //}

    zmq_msg_t msg_recv;
    zmq_msg_t msg_send;

    zmq_msg_init(&msg_recv);
    sprintf(buf,"%d",i);
    zmq_msg_init_data(&msg_send, buf, strlen(buf), NULL, NULL);
    
    zmq_msg_send(&msg_send, req, 0);

    zmq_msg_recv(&msg_recv, req, 0);
    //if(i%100 ==  0){
    //printf("recv[%d] %d ok...\n",j, i);
    //}

    zmq_msg_close(&msg_recv);
    zmq_msg_close(&msg_send);
    zmq_disconnect(req, argv[j]);
  }
  zmq_close(req);
  old = time(NULL) - old;
  printf("totoal use %d seconds\n", (int)old);
}

void svr_pub(int argc, char **argv){
  char endpoint[32];
  char buf[256];
  int begin;
  int end;
  int interval;
  
  void *pub;
  
  pub = zmq_socket(g_ctx, ZMQ_PUB);
  sprintf(endpoint, "tcp://*:%s", argv[2]);
  printf("bind %s\n", endpoint);
  zmq_bind(pub, endpoint);
  interval = atoi(argv[3]);
  if(interval < 0 || interval > 10000){
    interval = 1000; // 1sec
  }
  if(argc == 6){
    begin = atoi(argv[4]);
    end = atoi(argv[5]);
    if(begin < 0){
      begin = 10;
    }
    if(end < begin){
      end = begin + 10;
    }
  }else{
    begin = -1;
    end = -1;
  }
  if(begin == -1){
    // no filter
    while(1){
      zmq_msg_t msg_send;
      sprintf(buf, "pub: %d", zrandin(100));
      zmq_msg_init_data(&msg_send, buf, strlen(buf), NULL, NULL);
      zdbg("%s", buf);      
      zmq_msg_send(&msg_send, pub, 0);
    
      zmq_msg_close(&msg_send);
      zsleepms(interval);
    }

  }else{
    while(1){
      zmq_msg_t msg_data;
      zmq_msg_t msg_send;

      sprintf(endpoint, "%d", zrandat(begin, end));
      sprintf(buf, "pub: %d", zrandin(100));

      zmq_msg_init_data(&msg_send, endpoint, strlen(endpoint), NULL, NULL);
      zmq_msg_init_data(&msg_data, buf, strlen(buf), NULL, NULL);

      zdbg("id:%s data:%s", endpoint, buf);
      zmq_msg_send(&msg_send, pub, ZMQ_SNDMORE);
      zmq_msg_send(&msg_data, pub, 0);
      
      zmq_msg_close(&msg_send);
      zmq_msg_close(&msg_data);
      zsleepms(interval);
    }
  }
  
  zmq_close(pub);
}

void cli_sub(int argc, char **argv){
  void *sub;
  char buf[256];
  int endps;
  int subids;
  char *param;
  int more;
  size_t ln;
  
  zdbg("start sub client ...");
  sub = zmq_socket(g_ctx, ZMQ_SUB);
  if(argv[2][0] != '0'){
    zdbg("Set id: %s", argv[2]);
    zmq_setsockopt(sub, ZMQ_IDENTITY, argv[2], strlen(argv[2]));
  }
  endps = atoi(argv[3]);
  if(endps <= 0){
    zdbg("no endpoints");
    zmq_close(sub);
    return;
  }

  while(endps){
    zdbg("connect to %s", argv[endps+3]);
    zmq_connect(sub, argv[endps+3]);
    --endps;
  }
  
  endps = atoi(argv[3]);
  if(argc > endps+4){
    // sub filter
    subids = argc - 4 - endps;
    while(subids){
      param = argv[endps+3+subids];
      zdbg("Subscribe msg: %s", param);
      zmq_setsockopt(sub, ZMQ_SUBSCRIBE, param, strlen(param));
      --subids;
    }
  }else{
    // sub all
    zdbg("Subscribe ALL msg");
    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);
  }

  
  while(1){
    zmq_msg_t msg_recv;
    zmq_msg_init(&msg_recv);

    zmq_msg_recv(&msg_recv, sub, 0);
    endps = zmq_msg_size(&msg_recv);

    zdump_mix(buf, 256, (const unsigned char*)zmq_msg_data(&msg_recv), &endps);
    zdbg("%s",buf);
    zmq_msg_close(&msg_recv);
    
    ln = sizeof(more);
    zmq_getsockopt(sub, ZMQ_RCVMORE, &more, &ln);
    if(more){
      zmq_msg_t msg_recv1;
      zmq_msg_init(&msg_recv1);
      
      zmq_msg_recv(&msg_recv1, sub, 0);
      endps = zmq_msg_size(&msg_recv1);
      
      zdump_mix(buf, 256, (const unsigned char*)zmq_msg_data(&msg_recv1), &endps);
      zdbg("recvmore %s",buf);
      zmq_msg_close(&msg_recv1);      
    }
  }
  zmq_close(sub);
}
