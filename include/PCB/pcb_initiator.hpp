// pcb_initiator.h
#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

#include "PCB/pcb_payload.h"
#include "PCB/data_config_factory.h"
#include "sysmaker_mux/bridge.h"

#include <unordered_set>

class PCB_Initiator : protected sc_core::sc_module {
public:
    tlm_utils::simple_initiator_socket<PCB_Initiator, 32, pcb::pcb_protocol_types> socket;
    tlm_utils::peq_with_cb_and_phase<PCB_Initiator, pcb::pcb_protocol_types>       peq;
    std::unordered_set<std::unique_ptr<pcb::pcb_payload>>                          trans_keeper;

    PCB_Initiator(sc_core::sc_module_name name, std::string unix_socket_path)
        : socket("socket")
        , peq(this, &PCB_Initiator::peq_cb)
        , clk("clk_4", 250.0, sc_core::SC_NS, 125.0, 0, sc_core::SC_MS, true)
        , bridge_obj("qemu_bridge", true, unix_socket_path)
    {
        socket.register_nb_transport_bw(this, &PCB_Initiator::nb_bw);
        SC_THREAD(main_thread);

        bridge_obj.clk(clk);
    }

    virtual ~PCB_Initiator() = default;

private:
    sc_core::sc_clock clk;
    bridge            bridge_obj;

    /// Back-ward path：Slave 回 BEGIN/END_RESP
    tlm::tlm_sync_enum nb_bw(pcb::pcb_payload &trans,
                             tlm::tlm_phase   &phase,
                             sc_core::sc_time &delay);

    void peq_cb(pcb::pcb_payload &trans, const tlm::tlm_phase &phase);

    void main_thread();

    void trans_to_slave(std::string json_str);
};
