#include "peripherals/UART/hardware.h"

#include <systemc>

#include "report.h"

using namespace sc_core;
using namespace sc_dt;

static inline bool is_high(const sc_dt::sc_logic &v)
{
    return v == SC_LOGIC_1 || v == SC_LOGIC_Z;
}

void UART_Hardware::run()
{
    while (true) {
        /* 0) 讀當前設定 ------------------------------------------ */
        sc_time baud_period(1.0 / ic.baud_rate, SC_SEC);

        /* 1) 等 start-bit 的 1→0 -------------------------------- */
        while (is_high(rxd->read()))
            wait(rxd->value_changed_event());

        // Report_Info(SC_DEBUG, name(), "[%s]: UART start-bit read %c", sc_time_stamp().to_string().c_str(), is_high(rxd->read()) ? '1' : '0');

        /* 2) 對準 bit#0 中點（+1.5 bit） ------------------------- */
        wait(baud_period * 1.5);

        /* 3) 收 8 data bits ------------------------------------- */
        uint8_t rx_byte = 0;
        for (int i = 0; i < ic.data_bits; ++i) {
            /* --- 讀此位 --- */
            bool val = is_high(rxd->read());
            rx_byte |= (val << i);
            // Report_Info(SC_DEBUG, name(), "[%s]: UART read %c", sc_time_stamp().to_string().c_str(), is_high(rxd->read()) ? '1' : '0');

            /* 下一取樣點 ------------------------------------- */
            if (i < ic.data_bits - 1) {
                wait(baud_period);
            } else {
                wait(0.5 * baud_period);
            }
        }

        Report_Info(SC_FULL, name(), "[%s]: UART read 0x%x, ascii: %c", sc_time_stamp().to_string().c_str(), rx_byte, rx_byte);

        /* 4) stop-bit ------------------------------ */
        wait(baud_period * ic.stop_bits);
        // Report_Info(SC_DEBUG, name(), "[%s]: UART stop-bit read %c", sc_time_stamp().to_string().c_str(), is_high(rxd->read()) ? '1' : '0');
    }
}