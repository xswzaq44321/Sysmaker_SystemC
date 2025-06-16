#include <systemc>

#include <string>
#include <memory>
#include "json.hpp"

#include "PCB/pcb_payload.h"
#include "peripherals/UART/data_pack.h"
#include "PCB/data_config_factory.h"

using json = nlohmann::json;

/*
{
    "Pin Configuration": {
        "PB2": "TX",
        "PB3": "RX"
    },
    "Baud Rate": 9600,
    "Data Bits": 8,
    "Parity Bit": 0,
    "Stop Bits": 1
}
*/
void from_json(const nlohmann::json &j, UART_interface_config &o)
{
    o.pin_config = j.at("Pin Configuration").get<pcb::pin_config_t>();
    o.baud_rate  = j.at("Baud Rate").get<int>();
    o.data_bits  = static_cast<UART::Data_Bits>(j.at("Data Bits").get<int>());
    o.parity_bit = static_cast<UART::Parity_Bit>(j.at("Parity Bit").get<int>());
    o.stop_bits  = static_cast<UART::Stop_Bits>(j.at("Stop Bits").get<int>());
}
void to_json(nlohmann::json &j, const UART_interface_config &o)
{
    j = json::object({
        { "Pin Configuration", o.pin_config },
        { "Baud Rate", o.baud_rate },
        { "Data Bits", o.data_bits },
        { "Parity Bit", o.parity_bit },
        { "Stop Bits", o.stop_bits },
    });
}

/*
    {
        "TX": [104, 101, 108, 108, 111, 44, 32, 119, 111, 114, 108, 100],
        "RX": []
    }
*/
void from_json(const nlohmann::json &j, UART_data &o)
{
    o.tx = j.at("TX").get<std::vector<uint8_t>>();
    o.rx = j.at("RX").get<std::vector<uint8_t>>();
}
void to_json(nlohmann::json &j, const UART_data &o)
{
    j = json::object({
        { "TX", o.tx },
        { "RX", o.rx },
    });
}

static std::unique_ptr<pcb::pcb_data_if> uart_data_factory(const nlohmann::json &raw_data)
{
    auto obj = std::make_unique<UART_data>(raw_data.get<UART_data>());
    // Here you can do some content checking

    return obj;
}

static std::unique_ptr<pcb::pcb_interface_config_if> uart_interface_config_factory(const nlohmann::json &raw_data)
{
    auto obj = std::make_unique<UART_interface_config>(raw_data.get<UART_interface_config>());
    // Here you can do some content checking

    return obj;
}

register_type_factory(UART, uart_data_factory, uart_interface_config_factory);
