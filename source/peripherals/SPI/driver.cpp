#include "peripherals/SPI/driver.h"
#include "json.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;
using json = nlohmann::json;
using namespace SPI;

SPI_Driver::SPI_Driver(sc_module_name name, const pcb::pin_config_t &pin_config, const pcb::pcb_interface_config &ic)
    : PCB_Target_IF(name, pin_config)
    , spi_peq("spi_peq")
{
    std::string cs_pin   = pin_config.func_to_pin("CS");
    std::string sclk_pin = pin_config.func_to_pin("SCLK");
    std::string mosi_pin = pin_config.func_to_pin("MOSI");
    std::string miso_pin = pin_config.func_to_pin("MISO");
    if (!pin_value.count(cs_pin)) {
        SC_REPORT_ERROR("SPI_Driver", "pin_value for CS not found!");
    }
    if (!pin_value.count(sclk_pin)) {
        SC_REPORT_ERROR("SPI_Driver", "pin_value for SCLK not found!");
    }
    if (!pin_value.count(mosi_pin)) {
        SC_REPORT_ERROR("SPI_Driver", "pin_value for MOSI not found!");
    }
    if (!pin_value.count(miso_pin)) {
        SC_REPORT_ERROR("SPI_Driver", "pin_value for MISO not found!");
    }
    cs   = pin_value[cs_pin].get();
    sclk = pin_value[sclk_pin].get();
    mosi = pin_value[mosi_pin].get();
    miso = pin_value[miso_pin].get();
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to my CS", cs_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to my SCLK", sclk_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to my MOSI", mosi_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to my MISO", miso_pin.c_str());

    hardware = std::make_unique<SPI_Hardware>("hardware", ic);
    hardware->cs.bind(*cs);
    hardware->sclk.bind(*sclk);
    hardware->mosi.bind(*mosi);
    hardware->miso.bind(*miso);
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to hardware CS", cs_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to hardware SCLK", sclk_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to hardware MOSI", mosi_pin.c_str());
    Report_Info(SC_DEBUG, this->name(), "bind trace %s to hardware MISO", miso_pin.c_str());

    SC_THREAD(run);
    // sensitive << spi_peq.get_event();

    sc_trace(tf, *cs, std::string(this->name()) + ".CS");
    sc_trace(tf, *sclk, std::string(this->name()) + ".SCLK");
    sc_trace(tf, *mosi, std::string(this->name()) + ".MOSI");
    sc_trace(tf, *miso, std::string(this->name()) + ".MISO");
}

void SPI_Driver::hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
{
    spi_peq.notify(trans);
    // Report_Info(SC_DEBUG, name(), "Received transation data: \nIC: %s\nData: %s", json(*IC).dump().c_str(), json(*data).dump().c_str());
}

static inline bool is_high(const sc_dt::sc_logic &v)
{
    return v == SC_LOGIC_1;
}

static inline void write_pin(sc_inout_resolved *pin, bool val)
{
    pin->write(val ? SC_LOGIC_1 : SC_LOGIC_0);
}

void SPI_Driver::run()
{
    cs->write(sc_dt::sc_logic_1);
    sclk->write(sc_dt::sc_logic_Z);
    mosi->write(sc_dt::sc_logic_Z);
    miso->write(sc_dt::sc_logic_Z);
    wait(SC_ZERO_TIME);

    Report_Info(SC_DEBUG, name(), "pin initialized");

    while (true) {
        wait(spi_peq.get_event());
        pcb::pcb_payload *trans = spi_peq.get_next_transaction();

        if (trans->get_type() == "PCB") {
            pcb_behavior(trans);
        } else if (trans->get_type() == "SPI") {
            spi_behavior(trans);
        }

        hw_access_done_peq.notify(*trans, tlm::tlm_phase_enum::BEGIN_RESP);
    }
}

void SPI_Driver::pcb_behavior(pcb::pcb_payload *trans)
{
    auto *data = trans->get_data();
    for (const auto &[pin, value] : data->pin_states) {
        if (!pin_value.count(pin)) {
            continue;
        }
        if (value == "5V") {
            Report_Info(SC_DEBUG, name(), "set pin %s to 5V", pin.c_str());
            pin_value[pin]->write(SC_LOGIC_1);
        } else if (value == "0V") {
            Report_Info(SC_DEBUG, name(), "set pin %s to 0V", pin.c_str());
            pin_value[pin]->write(SC_LOGIC_0);
        }
    }
}

void SPI_Driver::spi_behavior(pcb::pcb_payload *trans)
{
    auto *ic   = dynamic_cast<SPI_interface_config *>(trans->get_interface_config());
    auto *data = dynamic_cast<SPI_data *>(trans->get_data());
    Report_Info(SC_DEBUG, name(), "Received transation data: \nInterface Config: %s\nData: %s", json(*ic).dump().c_str(), json(*data).dump().c_str());

    // checking interface configuration
    std::string sclk_pin = this->pin_config.func_to_pin("SCLK");
    std::string cs_pin   = this->pin_config.func_to_pin("CS");
    std::string mosi_pin = this->pin_config.func_to_pin("MOSI");
    std::string miso_pin = this->pin_config.func_to_pin("MISO");
    if (!sclk_pin.empty() && ic->pin_config[sclk_pin] != "SCLK") {
        SC_REPORT_WARNING(name(), "SCLK misconfigured!");
    }
    if (!cs_pin.empty() && ic->pin_config[cs_pin] != "CS") {
        SC_REPORT_WARNING(name(), "CS misconfigured!");
    }
    if (!mosi_pin.empty() && ic->pin_config[mosi_pin] != "MOSI") {
        SC_REPORT_WARNING(name(), "MOSI misconfigured!");
    }
    if (!miso_pin.empty() && ic->pin_config[miso_pin] != "MISO") {
        SC_REPORT_WARNING(name(), "MISO misconfigured!");
    }

    int       freq = ic->clock_freq;
    SPI::CPOL cpol = ic->cpol;
    SPI::CPHA cpha = ic->cpha;
    // SPI::Bit_Order bit_order = ic->bit_order;
    // SPI::Data_Size data_size = ic->data_size;
    sc_time   period(1.0 / freq, SC_SEC);
    sc_time   half_period(period / 2);
    sc_time   DOPD_delay(period / 8); // Propagation delay time from shifting edge of SCLK to data appearing on DOUT
    sc_time   CSDOZ_delay(period / 8); // Propagation delay time from rising edge of CS to DOUT becoming Hi-Z

    // write_pin(cs, false);
    write_pin(sclk, cpol);
    wait(SC_ZERO_TIME);
    if (cpha == 1) {
        wait(half_period);
    }

    // begin transfer every bites of data
    sc_time next_sample = sc_time_stamp();
    for (uint8_t tx : data->mosi) {
        uint32_t rx = 0;

        Report_Info(SC_DEBUG, name(), "Start XFER, TX=0x%x (CPOL=%d, CPHA=%d)", tx & 0xFF, cpol, cpha);
        if (cpha == 0) {
            for (int bit = 7; bit >= 0; --bit) {
                bool bit_val = ((tx >> bit) & 0b1);
                // shifting data out
                next_sample += half_period;
                Report_Info(SC_DEBUG, name(), "bit %d - SCLK returned to idle (%d)", bit, cpol);
                write_pin(sclk, cpol);
                wait(DOPD_delay);
                Report_Info(SC_DEBUG, name(), "bit %d - MOSI set to %d", bit, bit_val);
                write_pin(mosi, bit_val);
                wait(next_sample - sc_time_stamp());

                // sampling data in
                next_sample += half_period;
                Report_Info(SC_DEBUG, name(), "bit %d - SCLK toggled to %d (sampling edge)", bit, !cpol);
                write_pin(sclk, !cpol);
                // wait(DOPD_delay);
                bool miso_bit = is_high(miso->read());
                Report_Info(SC_DEBUG, name(), "bit %d - Sampled MISO = %d", bit, miso_bit);
                rx |= (miso_bit << bit);
                wait(next_sample - sc_time_stamp());
            }
        } else {
            for (int bit = 7; bit >= 0; --bit) {
                bool bit_val = ((tx >> bit) & 0b1);
                // shifting data out
                next_sample += half_period;
                Report_Info(SC_DEBUG, name(), "bit %d - SCLK returned to idle (%d)", bit, cpol);
                write_pin(sclk, !cpol);
                wait(DOPD_delay);
                Report_Info(SC_DEBUG, name(), "bit %d - MOSI set to %d", bit, bit_val);
                write_pin(mosi, bit_val);
                wait(next_sample - sc_time_stamp());

                // sampling data in
                next_sample += half_period;
                Report_Info(SC_DEBUG, name(), "bit %d - SCLK toggled to %d (sampling edge)", bit, !cpol);
                write_pin(sclk, cpol);
                // wait(DOPD_delay);
                bool miso_bit = is_high(miso->read());
                Report_Info(SC_DEBUG, name(), "bit %d - Sampled MISO = %d", bit, miso_bit);
                rx |= (miso_bit << bit);
                wait(next_sample - sc_time_stamp());
            }
        }
        Report_Info(SC_DEBUG, name(), "pushed rx data: ") << rx;
        data->miso.push_back(rx);
    }
    if (cpha == 0) {
        next_sample += half_period;
        write_pin(sclk, cpol);
        wait(CSDOZ_delay);
        mosi->write(SC_LOGIC_Z);
        wait(next_sample - sc_time_stamp());

        next_sample += half_period;
        // write_pin(cs, true);
        wait(next_sample - sc_time_stamp());
    } else {
        next_sample += half_period;
        // write_pin(cs, true);
        wait(CSDOZ_delay);
        mosi->write(SC_LOGIC_Z);
        wait(next_sample - sc_time_stamp());
    }

    trans->set_end_time(sc_time_stamp());
}