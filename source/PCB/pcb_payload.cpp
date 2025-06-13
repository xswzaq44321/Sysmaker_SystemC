#include <systemc>
#include <tlm>

#include <set>
#include <string>
#include <vector>
#include <memory>

#include "PCB/common.h"
#include "PCB/pcb_payload.h"
#include "json.hpp"
#include "PCB/pcb_payload.h"
#include "PCB/data_config_factory.h"

namespace pcb {

using json = nlohmann::json;
using namespace sc_core;

void to_json(json &j, const pins_t &p)
{
    j = json::array();
    for (const std::string &v : p) {
        j.push_back(v);
    }
}
void from_json(const json &j, pins_t &p)
{
    for (const auto &v : j) {
        p.insert(v.get<std::string>());
    }
}

void to_json(json &j, const pin_config_t &p)
{
    j = json::object();
    for (const auto &[k, v] : p) {
        j[k] = json(v);
    }
}
void from_json(const json &j, pin_config_t &p)
{
    for (const auto &[key, val] : j.items()) {
        p.insert(std::make_pair(key, val.get<pin_func_t>()));
    }
}

pcb_interface_config_if::~pcb_interface_config_if() { }
pcb_data_if::~pcb_data_if() { }

/*
{
    "pins": [
        "PB2",
        "PB3"
    ],
    "beginTime": "10ns",
    "endTime": "30ns",
    "type": "UART",
    "Interface Configuration": {...},
    "Data": {...}
}
*/
pcb_payload::pcb_payload(const std::string &json_str)
{
    json j = json::parse(json_str);
    m_pins = j.at("pins").get<pins_t>();
    if (j.count("beginTime")) {
        m_begin_time = std::make_unique<sc_time>(j.at("beginTime").get<std::string>());
    }
    if (j.count("endTime")) {
        m_end_time = std::make_unique<sc_time>(j.at("endTime").get<std::string>());
    }
    std::string type   = j.at("type");
    m_interface_config = Data_Config_Factory::produce_interface_config(type, j.at("Interface Configuration"));
    m_data             = Data_Config_Factory::produce_data(type, j.at("Data"));
}

}
std::ostream &operator<<(std::ostream &os, const pcb::pins_t &rhs)
{
    os << rhs.to_string();
    return os;
}