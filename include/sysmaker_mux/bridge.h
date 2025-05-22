#include <systemc.h>

#include <csignal>

#include "socket_api.h"

class bridge : public sc_module
{
public:
    SC_HAS_PROCESS(bridge);
    bridge(sc_module_name name, bool isUnix, const char *pathOrIp, int port = 0);
    ~bridge();

    void run();

    sc_in_clk clk;

protected:
    struct client client_obj;
    char buf[100'000];
    int buf_size = 100'000;
};