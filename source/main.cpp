#include <signal.h>
#include <systemc>
#include "top.h"

using namespace std;

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

    top hw_top("hw_top");
    printf("Start Simulating.......\n");
    sc_start();
    return 0;
}