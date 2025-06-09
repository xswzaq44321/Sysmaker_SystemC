#include <signal.h>
#include <systemc>
#include "top.h"

using namespace std;

int sc_main(int argc, char **argv)
{
    sc_report_handler::set_verbosity_level(SC_DEBUG);
    top hw_top("hw_top");
    printf("Start Simulating.......\n");
    sc_start();
    return 0;
}