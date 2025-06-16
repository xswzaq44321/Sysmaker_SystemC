// UART/hardware.h
#pragma once

#include <systemc>

#include "peripherals/UART/data_pack.h"

class UART_Hardware : public sc_core::sc_module {
    UART_interface_config ic;

public:
    UART_Hardware(sc_core::sc_module_name name, const UART_interface_config &ic)
        : sc_module(name)
        , ic(ic)
    {
        SC_THREAD(run);
    }
    virtual ~UART_Hardware()
    {
    }

    sc_core::sc_inout_resolved rxd;
    sc_core::sc_inout_resolved txd;

private:
    void run();
};