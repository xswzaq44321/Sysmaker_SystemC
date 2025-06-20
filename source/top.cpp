#include "top.h"
#include <set>
#include <string>

#include "peripherals/UART/driver.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

sc_core::sc_trace_file *tf;

top::top(sc_module_name name, std::string trace_file)
    : sc_module(name)
    , clk("clk_4", 250.0, SC_NS, 125.0, 0, SC_MS, true)
// , bridge_obj("qemu_bridge", true, "/tmp/fake_qemu.sock")
{
    tf = sc_create_vcd_trace_file(trace_file.c_str());

    // bridge_obj.clk(clk);

    initiator = std::make_unique<PCB_Initiator>("initiator");
    virt_pcb  = std::make_unique<PCB_Interconnect>("virt_pcb");

    read_net_list();
    // auto uart_hw = std::make_unique<UART_Hardware>(
    //     "UART HW simulation module",
    //     UART_interface_config(9600, UART::DATABITS_8, UART::PARITYBIT_0, UART::STOPBITS_1));
    // targets = std::make_unique<UART_Driver>(
    //     "UART Driver",
    //     pcb::pin_config_t { { "PA9", "TX" }, { "PA10", "RX" } },
    //     std::move(uart_hw));

    initiator->socket.bind(virt_pcb->targ_socket);
    for (const auto &target : targets) {
        virt_pcb->bind_target(*target, target->socket);
    }
}

void top::read_net_list(std::string file)
{
    std::ifstream     f(file);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string json_str = ss.str();
    json        j        = json::parse(json_str);

    for (const auto &[device, obj] : j.items()) {
        auto pin_to_trace = obj.at("pin_to_trace").get<std::unordered_map<std::string, std::string>>();
        for (const auto &comp : obj.at("components")) {
            std::string type = comp.at("type");
            if (type == "UART") {
                auto              hw_ic = comp.at("Interface Configuration").get<UART_interface_config>();
                pcb::pin_config_t pin_config;
                for (const auto &[pin_num, func] : hw_ic.pin_config) {
                    if (!pin_to_trace.count(pin_num)) {
                        char buf[1001];
                        snprintf(buf, 1000, "%s: pin %s not found in pin_to_trace", device.c_str(), pin_num.c_str());
                        SC_REPORT_WARNING(name(), buf);
                    }
                    pin_config.insert(std::make_pair(pin_to_trace[pin_num], func));
                }
                auto target = std::make_unique<UART_Driver>(
                    device.c_str(),
                    pin_config, hw_ic);
                targets.push_back(std::move(target));
            }
        }
    }
}
