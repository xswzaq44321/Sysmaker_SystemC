#include <systemc>

#include "sysmaker_mux/bridge.h"

#include "PCB/pcb_initiator.hpp"
#include "PCB/pcb_interconnect.hpp"
#include "PCB/pcb_target.hpp"

using namespace sc_core;

class top : public sc_module {
public:
    top(sc_module_name name);
    ~top() = default;

    sc_clock clk; // declares a clock
    // bridge bridge_obj;

    std::unique_ptr<PCB_Initiator>    initiator;
    std::unique_ptr<PCB_Interconnect> virt_pcb;
    std::unique_ptr<PCB_Target_IF>    targetA;
    std::unique_ptr<PCB_Target_IF>    targetB;
};
