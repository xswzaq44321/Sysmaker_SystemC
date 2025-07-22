#include "peripherals/UART/hardware.h"

#include <systemc>

#include "report.h"

using namespace sc_core;
using namespace sc_dt;

static inline bool is_high(const sc_dt::sc_logic &v)
{
    return v == sc_logic_1;
}

void UART_Hardware::run()
{
    txd.write(sc_dt::sc_logic_1);
    debugt_trace_clk.write(sc_logic_1);
    wait(SC_ZERO_TIME);

    Report_Info(SC_DEBUG, name(), "pin initialized");

    while (true) {
        /* 0) 讀當前設定 */
        sc_time baud_period(1.0 / ic.baud_rate, SC_SEC);

        /* 1) 等 start-bit 的 1→0 */
        while (is_high(rxd->read()))
            wait(rxd->value_changed_event());

        sc_time next_sample = sc_time_stamp() + 1.5 * baud_period;

        debugt_trace_clk.write(sc_logic_0);
        txd.write(sc_logic_0);
        /* 2) 對準 bit#0 中點（+1.5 bit */
        wait(next_sample - sc_time_stamp());

        /* 3) 收 8 data bits */
        uint8_t rx_byte = 0;
        for (int i = 0; i < std::max<int>(ic.data_bits, 8); ++i) {
            /* 讀此位 */
            bool val = is_high(rxd->read());
            rx_byte |= (val << i);
            txd.write(val ? sc_logic_1 : sc_logic_0);
            debugt_trace_clk.write(debugt_trace_clk.read() == sc_logic_Z ? sc_logic_X : sc_logic_Z);

            /* 下一取樣點 */
            next_sample += baud_period;
            wait(next_sample - sc_time_stamp());
        }

        /* 4) stop-bit */
        if (rxd->read() != sc_logic_1) {
            SC_REPORT_WARNING("UART", "framing error: stop bit not found!");
        }
        debugt_trace_clk.write(sc_logic_1);
        txd.write(sc_logic_1);

        Report_Info(SC_FULL, name(), "UART read 0x%x, ascii: %c", rx_byte, rx_byte);
    }
}