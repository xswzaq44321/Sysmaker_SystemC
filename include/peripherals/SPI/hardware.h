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
    {
        SC_THREAD(spi_behavior_thread);
        async_reset_signal_is(cs_internal_bool, true);

        SC_METHOD(convert_cs_to_bool);
        sensitive << cs;
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

    void spi_behavior_thread();
    void convert_cs_to_bool()
    {
        bool reset_active = (cs.read() != sc_dt::SC_LOGIC_0);
        cs_internal_bool.write(reset_active);
    }
};