// UART/hardware.h
#pragma once

#include <systemc>

#include "peripherals/UART/data_pack.h"
#include "report.h"

#include <memory>

class UART_Hardware : public sc_core::sc_module {
    UART_interface_config ic;

public:
    UART_Hardware(sc_core::sc_module_name name, const pcb::pcb_interface_config &ic)
        : sc_module(name)
        , ic(dynamic_cast<const UART_interface_config &>(ic))
    {
        SC_THREAD(run);
    }
    virtual ~UART_Hardware()
    {
    }

    sc_core::sc_in_resolved  rxd;
    sc_core::sc_out_resolved txd;

    sc_core::sc_signal_resolved debugt_trace_clk;

private:
    void run();
};