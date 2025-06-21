#include <signal.h>
#include <systemc>
#include "top.h"

using namespace std;
using namespace sc_core;

static void intHandler(int intid)
{
    cout << "Interrupted\n";
    sc_stop();
}

int sc_main(int argc, char **argv)
{
    signal(SIGINT, intHandler);

    sc_report_handler::set_log_file_name("report.log");
    sc_report_handler::set_verbosity_level(SC_DEBUG);

    if (argc <= 1) {
        printf("usage: %s <unix-socket-path>\n", argv[0]);
        exit(1);
    }

    top hw_top("hw_top", argv[1]);
    cout << "Start Simulating......." << endl;
    sc_start();
    return 0;
}