#include "peripherals/UART/driver.h"
#include "json.hpp"

using namespace sc_core;
using namespace sc_dt;
using json = nlohmann::json;

UART_Driver::UART_Driver(sc_module_name name, const pcb::pin_config_t &pin_config, const pcb::pcb_interface_config &ic)
    : PCB_Target_IF(name, pin_config)
    , uart_peq("uart_peq")
{
    std::string tx_pin = pin_config.func_to_pin("TX");
    std::string rx_pin = pin_config.func_to_pin("RX");
    if (!pin_value.count(tx_pin)) {
        SC_REPORT_ERROR("UART_Driver", "pin_value for TX not found!");
    }
    if (!pin_value.count(rx_pin)) {
        SC_REPORT_ERROR("UART_Driver", "pin_value for RX not found!");
    }
    txd = pin_value[tx_pin].get();
    rxd = pin_value[rx_pin].get();
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to my tx", tx_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to my rx", rx_pin.c_str());

    hardware = std::make_unique<UART_Hardware>("hardware", ic);
    hardware->rxd.bind(*txd);
    hardware->txd.bind(*rxd);
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to hardware rx", tx_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to hardware tx", rx_pin.c_str());

    SC_THREAD(run);
    // sensitive << uart_peq.get_event();

    sc_trace(tf, *rxd, std::string(this->name()) + ".RX");
    sc_trace(tf, *txd, std::string(this->name()) + ".TX");
    sc_trace(tf, debug_trace_clk, std::string(this->name()) + ".debug_trace_clk");
    sc_trace(tf, hardware->debugt_trace_clk, std::string(this->name()) + ".hw_trace_clk");
}

void UART_Driver::hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
{
    uart_peq.notify(trans);
    // Report_Info(SC_DEBUG, name(), "Received transation data: \nIC: %s\nData: %s", json(*IC).dump().c_str(), json(*data).dump().c_str());
}

static inline bool is_high(const sc_dt::sc_logic &v)
{
    return v == sc_logic_1;
}

void UART_Driver::run()
{
    rxd->write(sc_dt::sc_logic_Z);
    txd->write(sc_dt::sc_logic_1);
    debug_trace_clk.write(sc_logic_1);
    wait(SC_ZERO_TIME);

    Report_Info(SC_DEBUG, name(), "pin initialized");

    while (true) {
        wait(uart_peq.get_event());
        pcb::pcb_payload *trans = uart_peq.get_next_transaction();
        auto             *ic    = dynamic_cast<UART_interface_config *>(trans->get_interface_config());
        auto             *data  = dynamic_cast<UART_data *>(trans->get_data());
        Report_Info(SC_DEBUG, name(), "Received transation data: \nInterface Config: %s\nData: %s", json(*ic).dump().c_str(), json(*data).dump().c_str());

        // checking interface configuration
        std::string tx_pin = pin_config.func_to_pin("TX");
        std::string rx_pin = pin_config.func_to_pin("RX");
        if (ic->pin_config[tx_pin] != "TX" || ic->pin_config[rx_pin] != "RX") {
            SC_REPORT_WARNING(name(), "TX and RX pin misconfigured!");
            // hw_access_done_peq.notify(*trans, tlm::tlm_phase_enum::BEGIN_RESP);
            // continue;
        }

        int     baud_rate = ic->baud_rate;
        int     data_bits = ic->data_bits;
        int     stop_bits = ic->stop_bits;
        sc_time baud_period(1.0 / baud_rate, SC_SEC);
        for (uint8_t datum : data->tx) {
            sc_time next_sample = sc_time_stamp() + baud_period;
            // start bit, always low
            txd->write(sc_logic_0);
            debug_trace_clk.write(sc_logic_0);
            // Report_Info(SC_DEBUG, name(), "[%s]: UART start-bit write %c", sc_time_stamp().to_string().c_str(), '0');
            wait(next_sample - sc_time_stamp());

            // Data bit
            for (int i = 0; i < std::max<int>(data_bits, 8); ++i) {
                txd->write(((datum >> i) & 1) ? sc_logic_1 : sc_logic_0);
                debug_trace_clk.write(debug_trace_clk.read() == sc_logic_Z ? sc_logic_X : sc_logic_Z);
                // Report_Info(SC_DEBUG, name(), "[%s]: UART write %c", sc_time_stamp().to_string().c_str(), ((datum >> i) & 1) ? '1' : '0');
                next_sample += baud_period;
                wait(next_sample - sc_time_stamp());
            }

            /* parity bit */
            if (ic->parity_bit != UART::PARITYBIT_0) {
                sc_logic p;
                int      bit_cnt = __builtin_popcount(datum);
                if (ic->parity_bit == UART::PARITYBIT_1) {
                    p = (bit_cnt % 2) ? sc_logic_0 : sc_logic_1;
                } else if (ic->parity_bit == UART::PARITYBIT_2) {
                    p = (bit_cnt % 2) ? sc_logic_1 : sc_logic_0;
                }
                txd->write(p);
                next_sample += baud_period;
                wait(next_sample - sc_time_stamp());
            } else if (data_bits == 9) {
                // mark/space parity
                next_sample += baud_period;
                wait(next_sample - sc_time_stamp());
            }

            // stop bit
            txd->write(sc_logic_1);
            debug_trace_clk.write(sc_logic_1);
            // Report_Info(SC_DEBUG, name(), "[%s]: UART stop-bit write %c", sc_time_stamp().to_string().c_str(), '1');
            next_sample += stop_bits * baud_period;
            wait(next_sample - sc_time_stamp());
        }

        hw_access_done_peq.notify(*trans, tlm::tlm_phase_enum::BEGIN_RESP);
    }
}