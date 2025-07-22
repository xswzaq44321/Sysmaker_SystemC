#include "peripherals/SPI/hardware.h"

#include <systemc>

#include "report.h"

using namespace sc_core;
using namespace sc_dt;

static inline bool is_high(const sc_dt::sc_logic &v)
{
    return v == sc_logic_1;
}

static inline void write_pin(sc_out_resolved *pin, bool val)
{
    pin->write(val ? SC_LOGIC_1 : SC_LOGIC_0);
}

static uint8_t process_rx(uint8_t rx, bool is_first_byte)
{
    static uint8_t last_addr = 0;
    static uint8_t regs[256] = {};
    regs[59]                 = 10;
    regs[60]                 = 20;
    regs[61]                 = 30;
    static bool read_mode    = false;
    if (is_first_byte) {
        if (rx & 0x80) { // Read
            read_mode = true;
        } else { // Write
            read_mode = false;
            return 0;
        }
        last_addr = rx & 0x7F;
    }
    if (read_mode) {
        return regs[last_addr++];
    } else {
        regs[last_addr] = rx;
        return 0;
    }
}

void SPI_Hardware::spi_behavior_thread()
{
    // miso->write(sc_dt::sc_logic_Z);
    Report_Info(SC_DEBUG, name(), "pin initialized");
    // wait(SC_ZERO_TIME);

    uint8_t   tx = 0, rx = 0;
    int       freq = ic.clock_freq;
    SPI::CPOL cpol = ic.cpol;
    SPI::CPHA cpha = ic.cpha;
    // SPI::Bit_Order bit_order = ic.bit_order;
    // SPI::Data_Size data_size = ic.data_size;
    sc_time   period(1.0 / freq, SC_SEC);
    sc_time   DOPD_delay(period / 8); // Propagation delay time from shifting edge of SCLK to data appearing on DOUT
    sc_time   CSDOZ_delay(period / 8); // Propagation delay time from rising edge of CS to DOUT becoming Hi-Z

    while (is_high(cs.read())) {
        wait(cs.value_changed_event());
    }

    Report_Info(SC_DEBUG, name(), "CS pulled low, hardware begin working");

    tx = rx         = 0;
    bool first_byte = true;

    while (true) {
        if (cpha == 0) {
            for (int bit = 7; bit >= 0; --bit) {
                bool bit_val = ((tx >> bit) & 0b1);

                // shifting data out
                wait(DOPD_delay);
                write_pin(&miso, bit_val);
                Report_Info(SC_DEBUG, name(), "bit %d - MISO set to %d", bit, bit_val);
                if (cpol) {
                    wait(sclk.negedge_event());
                } else {
                    wait(sclk.posedge_event());
                }

                // sampling data in
                bool mosi_bit = is_high(mosi->read());
                Report_Info(SC_DEBUG, name(), "bit %d - Sampled MOSI = %d", bit, mosi_bit);
                rx |= (mosi_bit << bit);
                if (!cpol) {
                    wait(sclk.negedge_event());
                } else {
                    wait(sclk.posedge_event());
                }
            }
        } else {
            for (int bit = 7; bit >= 0; --bit) {
                bool bit_val = ((tx >> bit) & 0b1);

                // shifting data out
                if (cpol) {
                    wait(sclk.negedge_event());
                } else {
                    wait(sclk.posedge_event());
                }
                wait(DOPD_delay);
                write_pin(&miso, bit_val);
                Report_Info(SC_DEBUG, name(), "bit %d - MISO set to %d", bit, bit_val);

                // sampling data in
                if (!cpol) {
                    wait(sclk.negedge_event());
                } else {
                    wait(sclk.posedge_event());
                }
                bool mosi_bit = is_high(mosi->read());
                Report_Info(SC_DEBUG, name(), "bit %d - Sampled MOSI = %d", bit, mosi_bit);
                rx |= (mosi_bit << bit);
            }
        }
        tx         = process_rx(rx, first_byte);
        first_byte = false;
    }
    miso.write(SC_LOGIC_Z);
}