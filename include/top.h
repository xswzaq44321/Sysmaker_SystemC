#include <systemc>
#include "sysmaker_mux/bridge.h"

using namespace sc_core;

class top : public sc_module
{
public:
    SC_HAS_PROCESS(top);
    top(sc_module_name name);
    ~top() = default;

    sc_clock clk; // declares a clock
    bridge bridge_obj;
};
