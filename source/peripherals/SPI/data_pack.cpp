#include <systemc>

#include <string>
#include <memory>
#include "json.hpp"

#include "PCB/pcb_payload.h"
#include "peripherals/SPI/data_pack.h"
#include "PCB/data_config_factory.h"

using namespace std;
using json = nlohmann::json;

/*
{
    ...
    "Clock Frequency": 4000000,
    "SPI Mode": 3,
    "Bit Order": "MSB",
    "Data Size": 8
}
*/
void from_json(const nlohmann::json &j, SPI_interface_config &o)
{
    o.from_json(j);
    o.clock_freq = j.at("Clock Frequency").get<int>();
    o.cpol       = static_cast<SPI::CPOL>((j.at("SPI Mode").get<int>() & 0b10) >> 1);
    o.cpha       = static_cast<SPI::CPHA>((j.at("SPI Mode").get<int>() & 0b01) >> 0);
    if (string bit_order = j.at("Bit Order").get<string>(); bit_order == "MSB") {
        o.bit_order = SPI::MSB;
    } else if (bit_order == "LSB") {
        o.bit_order = SPI::LSB;
    }
    o.data_size = static_cast<SPI::Data_Size>(j.at("Data Size").get<int>());
}
void to_json(nlohmann::json &j, const SPI_interface_config &o)
{
    o.to_json(j);
    nlohmann::json tmp;
    tmp["Clock Frequency"] = o.clock_freq;
    tmp["SPI Mode"]        = (o.cpol << 1) | (o.cpha << 0);
    if (o.bit_order == SPI::MSB) {
        tmp["Bit Order"] = string("MSB");
    } else if (o.bit_order == SPI::LSB) {
        tmp["Bit Order"] = string("LSB");
    }
    tmp["Data Size"] = o.bit_order;
    j.update(tmp, true);
}

/*
{
    ...
    "MOSI": [
        104, 101, 108, 108, 111, 44, 32, 119, 111, 114, 108, 100
    ]
    "MISO": [
        0
    ]
}
*/
void from_json(const nlohmann::json &j, SPI_data &o)
{
    o.from_json(j);
    o.mosi = j.at("MOSI").get<vector<uint8_t>>();
    if (j.contains("MISO")) {
        o.miso = j.at("MISO").get<vector<uint8_t>>();
    }
}
void to_json(nlohmann::json &j, const SPI_data &o)
{
    o.to_json(j);
    auto tmp = json::object({
        { "MOSI", o.mosi },
        { "MISO", o.miso },
    });
    j.update(tmp, true);
}

static std::unique_ptr<pcb::pcb_data> spi_data_factory(const nlohmann::json &raw_data)
{
    auto obj = std::make_unique<SPI_data>(raw_data.get<SPI_data>());
    // Here you can do some content checking

    return obj;
}
static std::unique_ptr<pcb::pcb_interface_config> spi_interface_config_factory(const nlohmann::json &raw_data)
{
    auto obj = std::make_unique<SPI_interface_config>(raw_data.get<SPI_interface_config>());
    // Here you can do some content checking

    return obj;
}
static nlohmann::json spi_data_to_json(const pcb::pcb_data *obj)
{
    const SPI_data *o = dynamic_cast<const SPI_data *>(obj);
    if (!o) {
        throw std::invalid_argument("Fatal error: `obj` type is not SPI_data type!");
    }
    return json(*o);
}
static nlohmann::json spi_interface_config_to_json(const pcb::pcb_interface_config *obj)
{
    const SPI_interface_config *o = dynamic_cast<const SPI_interface_config *>(obj);
    if (!o) {
        throw std::invalid_argument("Fatal error: `obj` type is not SPI_interface_config type!");
    }
    return json(*o);
}

register_type_factory(SPI, spi_data_factory, spi_interface_config_factory, spi_data_to_json, spi_interface_config_to_json);
