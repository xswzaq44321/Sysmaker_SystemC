// pcb_target.h
#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/passthrough_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#include "PCB/pcb_payload.h"
#include "report.h"

class PCB_Target_IF : protected sc_core::sc_module {
public:
    using pin_value_t = sc_core::sc_inout_resolved;
    tlm_utils::passthrough_target_socket<PCB_Target_IF, 32, pcb::pcb_protocol_types> socket;
    // each pin_value is declared as pointer, so as for derived class to be able to reference to them
    std::unordered_map<std::string, std::unique_ptr<pin_value_t>>                    pin_value;

    PCB_Target_IF(sc_core::sc_module_name name, const pcb::pin_config_t &pin_config)
        : sc_module(name)
        , socket("socket")
        , pin_config(pin_config)
        , peq(this, &PCB_Target_IF::peq_cb)
        , hw_access_done_peq(this, &PCB_Target_IF::done_peq_cb)
    {
        socket.register_nb_transport_fw(this, &PCB_Target_IF::nb_fw);
        for (const std::string &p : pin_config.to_pins()) {
            pin_value[p] = std::make_unique<pin_value_t>(p.c_str());
        }
    }

    virtual ~PCB_Target_IF() = default;

public:
    pcb::pins_t getPins() const
    {
        return pin_config.to_pins();
    }

protected:
    const pcb::pin_config_t                                                  pin_config;
    tlm_utils::peq_with_cb_and_phase<PCB_Target_IF, pcb::pcb_protocol_types> peq;
    tlm_utils::peq_with_cb_and_phase<PCB_Target_IF, pcb::pcb_protocol_types> hw_access_done_peq;

    // this is where you emulates hw pin wave form,
    // by the end of emulation, you MUST notify `hw_access_done_peq`
    virtual void hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase) = 0;

private:
    /// Forward path：master 送 BEGIN_REQ 進來
    tlm::tlm_sync_enum nb_fw(pcb::pcb_payload &trans,
                             tlm::tlm_phase   &phase,
                             sc_core::sc_time &delay)
    {
        Report_Info(sc_core::SC_DEBUG, name()) << phase;
        if (phase == tlm::BEGIN_REQ) {
            // 把事務丟進佇列 (queue)，稍後處理
            Report_Info(sc_core::SC_DEBUG, name(), "putting trans into peq with delay: %s", delay.to_string().c_str());
            peq.notify(trans, phase, delay);
            phase = tlm::END_REQ; // 馬上回 END_REQ 允許 pipeline
            delay = sc_core::SC_ZERO_TIME; // 此回覆本身無額外延遲
            return tlm::TLM_UPDATED;
        } else if (phase == tlm::END_RESP) {
            Report_Info(sc_core::SC_DEBUG, name(), "putting trans into peq with delay: %s", delay.to_string().c_str());
            peq.notify(trans, phase, delay);
            return tlm::TLM_ACCEPTED;
        }
        return tlm::TLM_ACCEPTED; // 其他 phase 不處理
    }

    void peq_cb(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
    {
        if (phase == tlm::BEGIN_REQ) {
            hw_access(trans, phase);
        } else if (phase == tlm::END_RESP) {
            /* TODO: remove this */
            // Report_Info(sc_core::SC_MEDIUM, name(), "ending simulation");
            // sc_core::sc_stop();
        }
    }
    void done_peq_cb(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
    {
        Report_Info(sc_core::SC_DEBUG, name(), "hw_access finished.");

        // 回應 BEGIN_RESP
        tlm::tlm_phase     bw_phase = tlm::BEGIN_RESP;
        sc_core::sc_time   delay    = sc_core::SC_ZERO_TIME;
        tlm::tlm_sync_enum status   = socket->nb_transport_bw(trans, bw_phase, delay);

        if (status == tlm::TLM_ACCEPTED) {
            // 等對方回 END_RESP
        }
        // 完成後由 initiator 釋放 trans
    }
};
