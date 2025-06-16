// UART/driver.hpp

#pragma once

#include <systemc>
#include <tlm_utils/peq_with_get.h>

#include "peripherals/UART/data_pack.h"
#include "PCB/pcb_target.hpp"
#include "PCB/pcb_payload.h"
#include "hardware.h"

class UART_Driver : public PCB_Target_IF {
public:
    UART_Driver(sc_core::sc_module_name name, const pcb::pin_config_t &pin_config, std::unique_ptr<UART_Hardware> &&hw)
        : PCB_Target_IF(name, pin_config)
        , peq("peq")
    {
        std::string tx_pin = pin_config.func_to_pin("TX");
        std::string rx_pin = pin_config.func_to_pin("RX");
        if (!pin_value.count(tx_pin)) {
            SC_REPORT_ERROR("UART_Driver", "pin_value for TX not found!");
        }
        if (!pin_value.count(rx_pin)) {
            SC_REPORT_ERROR("UART_Driver", "pin_value for RX not found!");
        }
        txd = pin_value[tx_pin].get();
        rxd = pin_value[rx_pin].get();

        hardware = std::move(hw);
        hardware->rxd.bind(*txd);
        hardware->txd.bind(*rxd);

        SC_THREAD(run);
        sensitive << peq.get_event();
    }
    virtual ~UART_Driver() = default;

public:
    sc_core::sc_inout_resolved *rxd;
    sc_core::sc_inout_resolved *txd;

    virtual void hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase);

protected:
    std::unique_ptr<UART_Hardware> hardware;

private:
    tlm_utils::peq_with_get<pcb::pcb_payload> peq;

    void run();
};