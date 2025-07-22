// UART/data_pack.h
#pragma once

#include <systemc>

#include "PCB/pcb_payload.h"
#include "json.hpp"

namespace UART {
enum Data_Bits {
    DATABITS_8 = 8,
    DATABITS_9 = 9
};
enum Parity_Bit {
    PARITYBIT_0 = 0,
    PARITYBIT_1 = 1,
    PARITYBIT_2 = 2
};
enum Stop_Bits {
    STOPBITS_1 = 1,
    STOPBITS_2 = 2
};
}

class UART_interface_config : public pcb::pcb_interface_config {
public:
    UART_interface_config() = default;
    UART_interface_config(int              baud_rate,
                          UART::Data_Bits  data_bits,
                          UART::Parity_Bit parity_bit,
                          UART::Stop_Bits  stop_bits)
        : baud_rate(baud_rate)
        , data_bits(data_bits)
        , parity_bit(parity_bit)
        , stop_bits(stop_bits)
    {
    }
    ~UART_interface_config() = default;

    int              baud_rate;
    UART::Data_Bits  data_bits;
    UART::Parity_Bit parity_bit;
    UART::Stop_Bits  stop_bits;
};
void from_json(const nlohmann::json &j, UART_interface_config &o);
void to_json(nlohmann::json &j, const UART_interface_config &o);

class UART_data : public pcb::pcb_data {
public:
    UART_data()  = default;
    ~UART_data() = default;

    std::vector<uint8_t> tx;
    std::vector<uint8_t> rx;
};
void from_json(const nlohmann::json &j, UART_data &o);
void to_json(nlohmann::json &j, const UART_data &o);
