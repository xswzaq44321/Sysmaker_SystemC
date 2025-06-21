#include <systemc>

#include <csignal>
#include <string>

#include "socket_api.h"

class bridge : public sc_core::sc_module {
public:
    bridge(sc_core::sc_module_name name, bool isUnix, std::string pathOrIp, int port = 0);
    ~bridge();

    void run();

    sc_core::sc_in_clk            clk;
    sc_core::sc_fifo<std::string> in_reqs;
    sc_core::sc_fifo<std::string> out_reqs;

protected:
    struct client client_obj;
    char          buf[100'000];
    int           buf_size = 100'000;
};