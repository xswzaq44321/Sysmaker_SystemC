// UART/data_pack.h
#pragma once

#include <systemc>

#include "PCB/pcb_payload.h"
#include "json.hpp"

class UART_interface_config : public pcb::pcb_interface_config_if {
public:
    UART_interface_config()  = default;
    ~UART_interface_config() = default;

    int baud_rate;
    enum Data_Bits {
        DATABITS_8 = 8,
        DATABITS_9 = 9
    } data_bits;
    enum Parity_Bit {
        PARITYBIT_0 = 0,
        PARITYBIT_1 = 1,
        PARITYBIT_2 = 2
    } parity_bit;
    enum Stop_Bits {
        STOPBITS_8 = 8,
        STOPBITS_9 = 9
    } stop_bits;
};
void from_json(const nlohmann::json &j, UART_interface_config &o);
void to_json(nlohmann::json &j, const UART_interface_config &o);

class UART_data : public pcb::pcb_data_if {
public:
    UART_data()  = default;
    ~UART_data() = default;

    std::vector<uint8_t> tx;
    std::vector<uint8_t> rx;
};
void from_json(const nlohmann::json &j, UART_data &o);
void to_json(nlohmann::json &j, const UART_data &o);
