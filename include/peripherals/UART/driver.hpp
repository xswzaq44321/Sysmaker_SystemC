// UART/driver.hpp

#pragma once

#include <systemc>

#include "peripherals/UART/data_pack.hpp"
#include "PCB/pcb_target.hpp"
#include "PCB/pcb_payload.h"

class UART_Driver : public PCB_Target_IF {
public:
    UART_Driver(sc_core::sc_module_name name, const pcb::pin_config_t &pin_config)
        : PCB_Target_IF(name, pin_config)
    {
    }
    virtual ~UART_Driver() = default;

    virtual void hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase);
};