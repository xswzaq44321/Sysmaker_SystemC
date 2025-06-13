#include "top.h"
#include <set>
#include <string>

#include "peripherals/UART/driver.hpp"

using namespace std;

top::top(sc_module_name name)
    : sc_module(name)
    , clk("clk_4", 250.0, SC_NS, 125.0, 0, SC_MS, true)
// , bridge_obj("qemu_bridge", true, "/tmp/fake_qemu.sock")
{
    // bridge_obj.clk(clk);

    initiator = std::make_unique<PCB_Initiator>("initiator");
    virt_pcb  = std::make_unique<PCB_Interconnect>("virt_pcb");
    targetA   = std::make_unique<UART_Driver>("targetA", pcb::pin_config_t { { "PB2", "TX" }, { "PB3", "RX" } });
    targetB   = std::make_unique<UART_Driver>("targetB", pcb::pin_config_t { { "PB2", "RX" }, { "PB3", "TX" } });

    initiator->socket.bind(virt_pcb->targ_socket);
    virt_pcb->bind_target(*targetA, targetA->socket);
    virt_pcb->bind_target(*targetB, targetB->socket);
}