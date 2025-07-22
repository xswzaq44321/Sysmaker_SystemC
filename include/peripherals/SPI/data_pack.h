// SPI/data_pack.h
#pragma once

#include <systemc>

#include "PCB/pcb_payload.h"
#include "json.hpp"

namespace SPI {
enum CPOL {
    LOW  = 0,
    HIGH = 1
};
enum CPHA {
    EDGE_1 = 0,
    EDGE_2 = 1
};
enum Bit_Order {
    LSB,
    MSB
};
enum Data_Size {
    DATASIZE_8  = 8,
    DATASIZE_16 = 16
};
}

class SPI_interface_config : public pcb::pcb_interface_config {
public:
    SPI_interface_config() = default;
    SPI_interface_config(int            clock_freq,
                         SPI::CPOL      cpol,
                         SPI::CPHA      cpha,
                         SPI::Bit_Order bit_order,
                         SPI::Data_Size data_size)
        : clock_freq(clock_freq)
        , cpol(cpol)
        , cpha(cpha)
        , bit_order(bit_order)
        , data_size(data_size)
    {
    }
    ~SPI_interface_config() = default;

    int            clock_freq;
    SPI::CPOL      cpol;
    SPI::CPHA      cpha;
    SPI::Bit_Order bit_order;
    SPI::Data_Size data_size;
};
void from_json(const nlohmann::json &j, SPI_interface_config &o);
void to_json(nlohmann::json &j, const SPI_interface_config &o);

class SPI_data : public pcb::pcb_data {
public:
    SPI_data()  = default;
    ~SPI_data() = default;

    std::vector<uint8_t> mosi;
    std::vector<uint8_t> miso;
};
void from_json(const nlohmann::json &j, SPI_data &o);
void to_json(nlohmann::json &j, const SPI_data &o);