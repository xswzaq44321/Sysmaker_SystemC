#include "peripherals/UART/driver.h"
#include "json.hpp"

using namespace sc_core;
using namespace sc_dt;
using json = nlohmann::json;

void UART_Driver::hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
{
    peq.notify(trans);
    // Report_Info(SC_DEBUG, name(), "[%s]: Received transation data: \nIC: %s\nData: %s", sc_core::sc_time_stamp().to_string().c_str(), json(*IC).dump().c_str(), json(*data).dump().c_str());
}

static inline bool is_high(const sc_dt::sc_logic &v)
{
    return v == SC_LOGIC_1 || v == SC_LOGIC_Z;
}

void UART_Driver::run()
{
    while (true) {
        wait();
        pcb::pcb_payload *trans = peq.get_next_transaction();
        auto             *ic    = dynamic_cast<UART_interface_config *>(trans->get_interface_config());
        auto             *data  = dynamic_cast<UART_data *>(trans->get_data());
        Report_Info(SC_DEBUG, name(), "[%s]: Received transation data: \nInterface Config: %s\nData: %s", sc_core::sc_time_stamp().to_string().c_str(), json(*ic).dump().c_str(), json(*data).dump().c_str());

        int     baud_rate = ic->baud_rate;
        int     data_bits = ic->data_bits;
        int     stop_bits = ic->stop_bits;
        sc_time baud_period(1.0 / baud_rate, SC_SEC);
        for (uint8_t datum : data->tx) {
            // start bit, always low
            txd->write(sc_logic_0);
            // Report_Info(SC_DEBUG, name(), "[%s]: UART start-bit write %c", sc_time_stamp().to_string().c_str(), '0');
            wait(baud_period);

            // Data bit
            for (int i = 0; i < data_bits; ++i) {
                txd->write(((datum >> i) & 1) ? sc_logic_1 : sc_logic_0);
                // Report_Info(SC_DEBUG, name(), "[%s]: UART write %c", sc_time_stamp().to_string().c_str(), ((datum >> i) & 1) ? '1' : '0');
                wait(baud_period);
            }

            // stop bit
            txd->write(sc_logic_1);
            // Report_Info(SC_DEBUG, name(), "[%s]: UART stop-bit write %c", sc_time_stamp().to_string().c_str(), '1');
            wait(baud_period * stop_bits);
        }

        hw_access_done_peq.notify(*trans, tlm::tlm_phase_enum::BEGIN_RESP);
    }
}