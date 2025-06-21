// UART/driver.hpp

#pragma once

#include <systemc>
#include <tlm_utils/peq_with_get.h>

#include "peripherals/UART/data_pack.h"
#include "PCB/pcb_target.hpp"
#include "PCB/pcb_payload.h"
#include "hardware.h"

extern sc_core::sc_trace_file *tf;

class UART_Driver : public PCB_Target_IF {
public:
    UART_Driver(sc_core::sc_module_name name, const pcb::pin_config_t &pin_config, const UART_interface_config &ic);
    virtual ~UART_Driver() = default;

public:
    sc_core::sc_inout_resolved *rxd;
    sc_core::sc_inout_resolved *txd;

    sc_core::sc_signal_resolved trace_clk;

    virtual void hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase);

protected:
    std::unique_ptr<UART_Hardware> hardware;

private:
    tlm_utils::peq_with_get<pcb::pcb_payload> uart_peq;

    void run();
};