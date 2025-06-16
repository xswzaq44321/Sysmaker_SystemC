#include "top.h"
#include <set>
#include <string>

#include "peripherals/UART/driver.h"

using namespace std;

top::top(sc_module_name name)
    : sc_module(name)
    , clk("clk_4", 250.0, SC_NS, 125.0, 0, SC_MS, true)
// , bridge_obj("qemu_bridge", true, "/tmp/fake_qemu.sock")
{
    // bridge_obj.clk(clk);

    initiator    = std::make_unique<PCB_Initiator>("initiator");
    virt_pcb     = std::make_unique<PCB_Interconnect>("virt_pcb");
    auto uart_hw = std::make_unique<UART_Hardware>(
        "UART HW simulation module",
        UART_interface_config(9600, UART::DATABITS_8, UART::PARITYBIT_0, UART::STOPBITS_1));
    uart_driver = std::make_unique<UART_Driver>(
        "UART Driver",
        pcb::pin_config_t { { "PB2", "TX" }, { "PB3", "RX" } },
        std::move(uart_hw));

    initiator->socket.bind(virt_pcb->targ_socket);
    virt_pcb->bind_target(*uart_driver, uart_driver->socket);
}