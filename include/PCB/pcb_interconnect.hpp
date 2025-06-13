// pcb_interconnect.h
#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>

#include <set>
#include <map>
#include <string>

#include "PCB/pcb_payload.h"
#include "PCB/pcb_target.hpp"

class PCB_Interconnect : sc_core::sc_module {
private:
    tlm_utils::multi_passthrough_initiator_socket<PCB_Interconnect, 32, pcb::pcb_protocol_types> init_socket;

public:
    tlm_utils::multi_passthrough_target_socket<PCB_Interconnect, 32, pcb::pcb_protocol_types> targ_socket;

    SC_CTOR(PCB_Interconnect)
        : init_socket("init_socket")
        , targ_socket("targ_socket")
    {
        targ_socket.register_nb_transport_fw(this, &PCB_Interconnect::nb_fw);
        init_socket.register_nb_transport_bw(this, &PCB_Interconnect::nb_bw);
    }

    virtual void end_of_elaboration()
    {
        n_targs = targ_socket.size();
        sc_assert(n_inits == init_socket.size());
    }

public:
    template <typename SOCK_TYPE>
    void bind_target(PCB_Target_IF &target, SOCK_TYPE &s)
    {
        const pcb::pins_t &pins = target.getPins();
        for (const std::string &p : pins) {
            // connect each target's pin to this pcb's trace
            if (!trace_value.count(p)) {
                trace_value[p] = std::make_unique<trace_t>(p.c_str());
            }
            PCB_Target_IF::pin_value_t &target_pin = (*target.pin_value[p]);
            target_pin.bind(*trace_value[p]);
            Report_Info(SC_DEBUG, name(), "bind target pin %s to trace %s", target.pin_value[p]->name(), trace_value[p]->name());
        }
        target_id[pins] = n_inits;
        n_inits++;
        init_socket.bind(s);
    }

private:
    tlm::tlm_sync_enum nb_bw(int id, pcb::pcb_payload &tx, tlm::tlm_phase &ph, sc_core::sc_time &delay)
    {
        int init = tx.tlm_route.init;
        sc_assert(id == tx.tlm_route.targ);
        return targ_socket[init]->nb_transport_bw(tx, ph, delay);
    }

    tlm::tlm_sync_enum nb_fw(int id, pcb::pcb_payload &tx, tlm::tlm_phase &ph, sc_core::sc_time &delay)
    {
        const int vid     = decode_pins(tx.get_pins());
        tx.tlm_route.init = id;
        tx.tlm_route.targ = vid;
        if (vid == -1) {
            std::string buf = "No such target with pins(" + tx.get_pins().to_string() + ") found!";
            SC_REPORT_WARNING("SPI_Interconnect", buf.c_str());
            return tlm::tlm_sync_enum::TLM_COMPLETED;
        }
        return init_socket[vid]->nb_transport_fw(tx, ph, delay);
    }

    int decode_pins(const pcb::pins_t &pins)
    {
        if (!target_id.count(pins))
            return -1;
        return target_id[pins];
    }

protected:
    using trace_t = sc_signal_resolved;

    unsigned int                                             n_targs = 0;
    unsigned int                                             n_inits = 0;
    std::map<pcb::pins_t, int>                               target_id;
    std::unordered_map<std::string, std::unique_ptr<trace_t>> trace_value;
};