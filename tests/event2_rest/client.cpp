#include <zsi/base/trace.h>
#include <zsi/mem/memory.h>
#include <zsi/app/trace2console.h>
//#include <zsi/app/waitloop.h>

#include <zpp/event2/rest.h>
#include <zpp/zpp.h>

int main(int argc, char **argv){
    zmem_init();

    ztrace_register(ztrace2console, NULL);
    zmsg("rest server you...");

    z::ev2::RestClient cli;
    z::ev2::Request req;
    if (argc > 1) {
        //http://192.168.1.112:8280/framework-admin/api/v1/test
        cli.Connect("192.168.1.112", 8280);
        req.uri = "/framework-admin/api/v1/test";
    }else{
        cli.Connect("127.0.0.1", 8088);
        req.uri = "/api/v1/test";
    }
    req.request = "{\"hello\":\"world!\"}";
    req.response = "";
    cli.Add(req);
    /*
    req.response = "";
    cli.Get(req);
    req.response = "";
    cli.Update(req);
    req.response = "";
    cli.Del(req);
    req.response = "";

    req.uri = "/api";
    cli.Del(req);
    */
    //ztimer_and_signal(NULL, NULL);
    cli.Disconnect();

    zmem_fini();
    ztrace_flush();
    return 0;
}
