// pcb_target.h
#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/passthrough_target_socket.h>
#include <tlm_utils/peq_with_get.h>

#include <set>
#include <unordered_map>
#include <string>

#include "PCB/pcb_payload.h"
#include "report.h"

class PCB_Target_IF : sc_core::sc_module {
public:
    using pin_value_t = sc_inout_resolved;

    tlm_utils::passthrough_target_socket<PCB_Target_IF, 32, pcb::pcb_protocol_types> socket;
    tlm_utils::peq_with_get<pcb::pcb_payload>                                        peq;
    std::unordered_map<pcb::pin_t, std::unique_ptr<pin_value_t>>                     pin_value;

    PCB_Target_IF(sc_core::sc_module_name name, const pcb::pins_t &pins)
        : sc_module(name)
        , socket("socket")
        , peq("peq")
        , pins(pins)
    {
        for (const pcb::pin_t &p : pins) {
            pin_value[p] = std::make_unique<pin_value_t>(p.to_string().c_str());
        }
        SC_THREAD(run);
    }

    virtual ~PCB_Target_IF() = default;

    virtual void before_end_of_elaboration()
    {
        socket.register_nb_transport_fw(this, &PCB_Target_IF::nb_fw);
    }

    virtual void hw_access(pcb::pcb_payload &trans)
    {
        Report_Info(SC_MEDIUM, name(), "[%s]: hw_access", sc_core::sc_time_stamp().to_string().c_str());

        // This is where you should be driving hw pins
        // for (const pcb::pin_t &p : pins) {
        //     pin_value[p]->write(sc_logic_1);
        //     wait(sc_time("1 ns"));
        //     cout << pin_value[p]->read() << endl;
        // }
    };

public:
    const pcb::pins_t &getPins() const
    {
        return pins;
    }

protected:
    pcb::pins_t pins;

private:
    /// Forward path：master 送 BEGIN_REQ 進來
    tlm::tlm_sync_enum nb_fw(pcb::pcb_payload &trans,
                             tlm::tlm_phase   &phase,
                             sc_core::sc_time &delay)
    {
        Report_Info(SC_DEBUG, name(), "[%s]: ", sc_core::sc_time_stamp().to_string().c_str()) << phase;
        if (phase == tlm::BEGIN_REQ) {
            // 把事務丟進佇列 (queue)，稍後處理
            Report_Info(SC_DEBUG, name(), "[%s]: putting trans into peq with delay: %s", sc_core::sc_time_stamp().to_string().c_str(), delay.to_string().c_str());
            peq.notify(trans, delay);
            phase = tlm::END_REQ; // 馬上回 END_REQ 允許 pipeline
            delay = sc_core::SC_ZERO_TIME; // 此回覆本身無額外延遲
            return tlm::TLM_UPDATED;
        }
        return tlm::TLM_ACCEPTED; // 其他 phase 不處理
    }

    void run()
    {
        while (true) {
            auto *trans = peq.get_next_transaction();
            if (!trans) {
                wait(peq.get_event());
                continue;
            }

            hw_access(*trans);

            // 回應 BEGIN_RESP
            tlm::tlm_phase     phase  = tlm::BEGIN_RESP;
            sc_core::sc_time   delay  = sc_core::SC_ZERO_TIME;
            tlm::tlm_sync_enum status = socket->nb_transport_bw(*trans, phase, delay);

            Report_Info(SC_DEBUG, name(), "[%s]: ", sc_core::sc_time_stamp().to_string().c_str()) << phase;
            if (status == tlm::TLM_ACCEPTED) {
                // 等對方回 END_RESP
            }
            // 完成後由 initiator 釋放 trans
        }
    }
};
