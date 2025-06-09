// pcb_initiator.h
#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

#include <map>
#include <memory>
#include <unordered_set>

#include "PCB/pcb_payload.h"
#include "report.h"

class PCB_Initiator : sc_core::sc_module {
public:
    tlm_utils::simple_initiator_socket<PCB_Initiator, 32, pcb::pcb_protocol_types> socket;
    tlm_utils::peq_with_cb_and_phase<PCB_Initiator, pcb::pcb_protocol_types>       peq;
    std::unordered_set<std::unique_ptr<pcb::pcb_payload>>                          trans_keeper;

    SC_CTOR(PCB_Initiator)
        : socket("socket")
        , peq(this, &PCB_Initiator::peq_cb)
    {
        SC_THREAD(main_thread);
    }

    virtual ~PCB_Initiator() = default;

    virtual void before_end_of_elaboration()
    {
        socket.register_nb_transport_bw(this, &PCB_Initiator::nb_bw);
    }

private:
    /// Back-ward path：Slave 回 BEGIN/END_RESP
    tlm::tlm_sync_enum nb_bw(pcb::pcb_payload &trans,
                             tlm::tlm_phase   &phase,
                             sc_core::sc_time &delay)
    {
        Report_Info(SC_DEBUG, name(), "[%s]: ", sc_core::sc_time_stamp().to_string().c_str()) << phase;
        Report_Info(SC_DEBUG, name(), "[%s]: putting trans into peq with delay: %s", sc_core::sc_time_stamp().to_string().c_str(), delay.to_string().c_str());

        peq.notify(trans, phase, delay);
        // if (phase == tlm::BEGIN_RESP) {
        //     phase = tlm::END_RESP;
        // }
        return tlm::TLM_ACCEPTED;
    }

    void peq_cb(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
    {
        if (phase == tlm::BEGIN_RESP) {
            // check with transaction data ...

            tlm::tlm_phase   fw_phase = tlm::END_RESP;
            sc_core::sc_time delay("125 us");
            socket->nb_transport_fw(trans, fw_phase, delay);
        }
    }

    /// 主控流程：示範對兩個 slave 各傳 1 byte
    void main_thread()
    {
        wait(sc_time("1 us"));
        uint8_t     dataA = 0x11;
        pcb::pins_t pinA  = { "PE2", "PE5", "PE6", "PG13" };
        Report_Info(SC_DEBUG, name(), "[%s]: Send write request to %s with 0x%x", sc_core::sc_time_stamp().to_string().c_str(), pinA.to_string().c_str(), dataA);
        trans_to_slave(pinA, /*write*/ true, &dataA);

        wait(sc_time("1 us"));
        uint8_t     dataB = 0x22;
        pcb::pins_t pinB  = { "PE2", "PE5", "PE6", "PG14" };
        Report_Info(SC_DEBUG, name(), "[%s]: Send write request to %s with 0x%x", sc_core::sc_time_stamp().to_string().c_str(), pinB.to_string().c_str(), dataB);
        trans_to_slave(pinB, /*write*/ true, &dataB);

        wait(sc_time("1 us"));
        uint8_t rdA = 0;
        Report_Info(SC_DEBUG, name(), "[%s]: Send read request to %s", sc_core::sc_time_stamp().to_string().c_str(), pinA.to_string().c_str());
        trans_to_slave(pinA, /*write*/ false, &rdA);

        // sc_core::sc_stop();
    }

    /// 發送一筆 non-blocking 事務並等待完成
    void trans_to_slave(pcb::pins_t pins, bool is_wr, uint8_t *data)
    {
        // transaction objects are always originate from initiator, so it's initiator's responsibility to memory manage it
        auto [it, res] = trans_keeper.insert(std::make_unique<pcb::pcb_payload>());
        if (!res) {
            SC_REPORT_ERROR(name(), "Cannot create new transaction object!");
            return;
        }
        auto *trans = it->get();

        if (is_wr) {
            trans->set_write();
        } else {
            trans->set_read();
        }
        trans->set_pins(pins);

        tlm::tlm_phase   phase  = tlm::BEGIN_REQ;
        sc_core::sc_time delay  = SC_ZERO_TIME;
        auto             status = socket->nb_transport_fw(*trans, phase, delay);

        Report_Info(SC_DEBUG, name(), "[%s]: ", sc_core::sc_time_stamp().to_string().c_str()) << phase;
        if (status == tlm::TLM_UPDATED && phase == tlm::END_REQ)
            ; // 正常情況，等待 resp
        else if (status != tlm::TLM_ACCEPTED)
            SC_REPORT_WARNING("PCB_Initiator", "非法返回狀態");

        // if (willWait) {
        //     // 等待 BEGIN_RESP → END_RESP 完成
        //     sc_core::wait(state.done);
        // }

        // trans->release(); // 釋放事務
        // tracker.erase(trans);
    }
};
