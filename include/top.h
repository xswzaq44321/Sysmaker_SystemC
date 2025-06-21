#include <systemc>

#include "PCB/pcb_initiator.hpp"
#include "PCB/pcb_interconnect.hpp"
#include "PCB/pcb_target.hpp"

extern sc_core::sc_trace_file *tf;

class top : public sc_core::sc_module {
public:
    top(sc_core::sc_module_name name, std::string unix_socket_path, std::string trace_file = "wave");
    ~top() = default;

    std::unique_ptr<PCB_Initiator>              initiator;
    std::unique_ptr<PCB_Interconnect>           virt_pcb;
    std::vector<std::unique_ptr<PCB_Target_IF>> targets;

    void read_net_list(std::string file = "netlist.json");

    void end_of_simulation()
    {
        sc_close_vcd_trace_file(tf);
    }
};
