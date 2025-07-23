// SPI/hardware.h
#pragma once

#include <systemc>

#include "peripherals/SPI/data_pack.h"
#include "report.h"

#include <memory>

class SPI_Hardware : public sc_core::sc_module {
public:
    SPI_Hardware(sc_core::sc_module_name name, const pcb::pcb_interface_config &ic)
        : sc_module(name)
        , ic(dynamic_cast<const SPI_interface_config &>(ic))
        , sample_clk("sensor_sample_clock", 1, sc_core::SC_SEC)
    {
        SC_THREAD(spi_behavior_thread);
        async_reset_signal_is(cs_internal_bool, true);

        SC_METHOD(convert_cs_to_bool);
        sensitive << cs;

        SC_THREAD(sensor_thread);
    }
    virtual ~SPI_Hardware()
    {
    }

    sc_core::sc_in_resolved  cs;
    sc_core::sc_in_resolved  sclk;
    sc_core::sc_in_resolved  mosi;
    sc_core::sc_out_resolved miso;

private:
    SPI_interface_config ic;

    sc_core::sc_signal<bool> cs_internal_bool;

    uint8_t           last_addr = 0;
    uint8_t           regs[256] = {};
    uint8_t           ACCEL_FS; // Accel Full Scale
    sc_core::sc_clock sample_clk;
    void              sensor_thread();
    uint8_t           process_rx(uint8_t rx, bool is_first_byte);

    void spi_behavior_thread();
    void convert_cs_to_bool()
    {
        bool reset_active = (cs.read() != sc_dt::SC_LOGIC_0);
        if (reset_active) {
            Report_Info(sc_core::SC_DEBUG, name(), "CS set to high, expect HW to be off");
        }
        cs_internal_bool.write(reset_active);
    }
};