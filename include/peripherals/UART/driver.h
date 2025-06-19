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
    UART_Driver(sc_core::sc_module_name name, const pcb::pin_config_t &pin_config, const UART_interface_config &ic)
        : PCB_Target_IF(name, pin_config)
        , uart_peq("uart_peq")
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
        Report_Info(sc_core::SC_DEBUG, this->name(), "bind trace %s to my tx", tx_pin.c_str());
        Report_Info(sc_core::SC_DEBUG, this->name(), "bind trace %s to my rx", rx_pin.c_str());

        hardware = std::make_unique<UART_Hardware>("hardware", ic);
        hardware->rxd.bind(*txd);
        hardware->txd.bind(*rxd);
        Report_Info(sc_core::SC_DEBUG, this->name(), "bind trace %s to hardware rx", tx_pin.c_str());
        Report_Info(sc_core::SC_DEBUG, this->name(), "bind trace %s to hardware tx", rx_pin.c_str());

        SC_THREAD(run);
        sensitive << uart_peq.get_event();

        sc_trace(tf, *rxd, std::string(this->name()) + ".RX");
        sc_trace(tf, *txd, std::string(this->name()) + ".TX");
        sc_trace(tf, trace_clk, std::string(this->name()) + ".trace_clk");
        sc_trace(tf, hardware->trace_clk, std::string(this->name()) + ".hw_trace_clk");
    }
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