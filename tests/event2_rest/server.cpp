#include <zsi/base/trace.h>
#include <zsi/mem/memory.h>
#include <zsi/app/trace2console.h>
#include <zsi/app/waitloop.h>

#include <zpp/event2/rest.h>
#include <zpp/zpp.h>

int main(int argc, char **argv){
    zmem_init();

    ztrace_register(ztrace2console, NULL);
    zmsg("rest server you...");

    z::ev2::RestServer svr;
    svr.Run(8088);

    ztimer_and_signal(NULL, NULL);

    svr.Stop();
    zmem_fini();
    ztrace_flush();
    return 0;
}