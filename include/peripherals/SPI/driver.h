// SPI/driver.hpp

#pragma once

#include <systemc>
#include <tlm_utils/peq_with_get.h>

#include "peripherals/SPI/data_pack.h"
#include "PCB/pcb_target.hpp"
#include "PCB/pcb_payload.h"
#include "hardware.h"

extern sc_core::sc_trace_file *tf;

class SPI_Driver : public PCB_Target_IF {
public:
    SPI_Driver(sc_core::sc_module_name name, const pcb::pin_config_t &pin_config, const pcb::pcb_interface_config &ic);
    virtual ~SPI_Driver() = default;

public:
    sc_core::sc_inout_resolved *cs;
    sc_core::sc_inout_resolved *sclk;
    sc_core::sc_inout_resolved *mosi;
    sc_core::sc_inout_resolved *miso;

    virtual void hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase);

protected:
    std::unique_ptr<SPI_Hardware> hardware;

private:
    tlm_utils::peq_with_get<pcb::pcb_payload> spi_peq;

    void run();

    void pcb_behavior(pcb::pcb_payload *trans);
    void spi_behavior(pcb::pcb_payload *trans);
};