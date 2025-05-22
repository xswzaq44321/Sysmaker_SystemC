#include <signal.h>
#include <systemc.h>
#include "top.h"

using namespace std;

int sc_main(int argc, char **argv)
{
    top hw_top("hw_top");
    printf("Start Simulating.......\n");
    sc_start();
    return 0;
}