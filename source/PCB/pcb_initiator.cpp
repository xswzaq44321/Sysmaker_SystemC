#include <systemc>

#include "PCB/pcb_initiator.hpp"
#include "report.h"
#include "json.hpp"

#include <map>
#include <memory>

using namespace sc_core;
using json = nlohmann::json;

tlm::tlm_sync_enum PCB_Initiator::nb_bw(pcb::pcb_payload &trans,
                                        tlm::tlm_phase   &phase,
                                        sc_core::sc_time &delay)
{
    Report_Info(SC_DEBUG, name()) << phase;
    Report_Info(SC_DEBUG, name(), "putting trans into peq with delay: %s", delay.to_string().c_str());

    peq.notify(trans, phase, delay);
    // if (phase == tlm::BEGIN_RESP) {
    //     phase = tlm::END_RESP;
    // }
    return tlm::TLM_ACCEPTED;
}

void PCB_Initiator::peq_cb(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
{
    if (phase == tlm::BEGIN_RESP) {
        // check with transaction data ...

        tlm::tlm_phase fw_phase = tlm::END_RESP;
        sc_time        delay    = SC_ZERO_TIME;
        socket->nb_transport_fw(trans, fw_phase, delay);
        trans.set_end_time(sc_time_stamp());
        bridge_obj.out_reqs.write(json(trans).dump(4));
    }
}

/// 主控流程：示範對兩個 slave 各傳 1 byte
void PCB_Initiator::main_thread()
{
    wait(SC_ZERO_TIME);
    while (true) {
        std::string json_str = bridge_obj.in_reqs.read(); // blocking read
        Report_Info(SC_DEBUG, name(), "Send data packet: ") << json_str;
        trans_to_slave(json_str);
    }

    // std::ifstream file("datapack.json");
    // std::stringstream ss;
    // ss << file.rdbuf();
    // std::string json_str = ss.str();
    // Report_Info(SC_DEBUG, name(), "Send data packet: ") << json_str;
    // trans_to_slave(json_str);

    // wait(sc_time("1 us"));
    // Report_Info(SC_DEBUG, name(), "Send data packet: ") << json_str;
    // trans_to_slave(json_str);

    // wait(sc_time("1 us"));
    // uint8_t     dataB = 0x22;
    // pcb::pins_t pinB  = { "PE2", "PE5", "PE6", "PG14" };
    // Report_Info(SC_DEBUG, name(), "Send write request to %s with 0x%x", pinB.to_string().c_str(), dataB);
    // trans_to_slave(pinB, /*write*/ true, &dataB);

    // wait(sc_time("1 us"));
    // uint8_t rdA = 0;
    // Report_Info(SC_DEBUG, name(), "Send read request to %s", pinA.to_string().c_str());
    // trans_to_slave(pinA, /*write*/ false, &rdA);

    // sc_core::sc_stop();
}

/// 發送一筆 non-blocking 事務並等待完成
void PCB_Initiator::trans_to_slave(std::string json_str)
{
    // transaction objects are always originate from initiator, so it's initiator's responsibility to memory manage it

    auto [it, res] = trans_keeper.insert(std::make_unique<pcb::pcb_payload>(json::parse(json_str).get<pcb::pcb_payload>()));
    if (!res) {
        SC_REPORT_ERROR(name(), "Cannot create new transaction object!");
        return;
    }
    auto *trans = it->get();

    tlm::tlm_phase   phase  = tlm::BEGIN_REQ;
    sc_core::sc_time delay  = trans->get_begin_time() ? *trans->get_begin_time() - sc_time_stamp() : SC_ZERO_TIME;
    auto             status = socket->nb_transport_fw(*trans, phase, delay);

    Report_Info(SC_DEBUG, name()) << phase;
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