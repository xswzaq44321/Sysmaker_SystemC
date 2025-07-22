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
    for (const auto &[k, v] : p) {
        j[k] = json(v);
    }
}
void from_json(const json &j, pin_config_t &p)
{
    for (const auto &[key, val] : j.items()) {
        if (val.type() == json::value_t::string) {
            p.insert(std::make_pair(key, val.get<pin_func_t>()));
        } else {
            p.insert(std::make_pair(key, pin_func_t()));
        }
    }
}

/*
{
    "Pin Configuration": {
        "PB2": "TX",
        "PB3": "RX"
    },
}
*/
void pcb_interface_config::from_json(const nlohmann::json &j)
{
    this->pin_config = j.at("Pin Configuration").get<pcb::pin_config_t>();
}
void pcb_interface_config::to_json(nlohmann::json &j) const
{
    nlohmann::json tmp;
    tmp["Pin Configuration"] = json(this->pin_config);
    j.update(tmp, true);
}
void from_json(const nlohmann::json &j, pcb_interface_config &o)
{
    o.from_json(j);
}
void to_json(nlohmann::json &j, const pcb_interface_config &o)
{
    o.to_json(j);
}
/*
{
    "PinStates":{
        "PG13": "0V",
        "PG14": "5V"
    }
}
*/
void pcb_data::from_json(const nlohmann::json &j)
{
    for (const auto &[key, val] : j.at("PinStates").items()) {
        this->pin_states[key] = val;
    }
}
void pcb_data::to_json(nlohmann::json &j) const
{
    nlohmann::json tmp;
    for (const auto &[key, val] : this->pin_states) {
        tmp["PinStates"][key] = val;
    }
    j.update(tmp, true);
}
void from_json(const nlohmann::json &j, pcb_data &o)
{
    o.from_json(j);
}
void to_json(nlohmann::json &j, const pcb_data &o)
{
    o.to_json(j);
}

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
void from_json(const json &j, pcb_payload &p)
{
    // std::cout << j.dump(4) << std::endl;
    p.m_pins = j.at("pins").get<pins_t>();
    if (j.count("beginTime")) {
        p.m_begin_time = std::make_unique<sc_time>(j.at("beginTime").get<std::string>());
    }
    if (j.count("endTime")) {
        p.m_end_time = std::make_unique<sc_time>(j.at("endTime").get<std::string>());
    }
    p.m_type             = j.at("type");
    p.m_interface_config = Data_Config_Factory::produce_interface_config(p.m_type, j.at("Interface Configuration"));
    p.m_data             = Data_Config_Factory::produce_data(p.m_type, j.at("Data"));
}
void to_json(json &j, const pcb_payload &p)
{
    j["pins"] = json(p.m_pins);
    if (p.m_begin_time)
        j["beginTime"] = json(p.m_begin_time->to_string());
    if (p.m_end_time)
        j["endTime"] = json(p.m_end_time->to_string());
    j["type"]                    = p.m_type;
    j["Interface Configuration"] = Data_Config_Factory::interface_config_to_json(p.m_type, p.m_interface_config.get());
    j["Data"]                    = Data_Config_Factory::data_to_json(p.m_type, p.m_data.get());
}

static std::unique_ptr<pcb::pcb_data> pcb_data_factory(const nlohmann::json &raw_data)
{
    auto obj = std::make_unique<pcb_data>(raw_data.get<pcb_data>());
    // Here you can do some content checking

    return obj;
}
static std::unique_ptr<pcb::pcb_interface_config> pcb_interface_config_factory(const nlohmann::json &raw_data)
{
    auto obj = std::make_unique<pcb_interface_config>(raw_data.get<pcb_interface_config>());
    // Here you can do some content checking

    return obj;
}
static nlohmann::json pcb_data_to_json(const pcb::pcb_data *obj)
{
    const pcb_data *o = dynamic_cast<const pcb_data *>(obj);
    if (!o) {
        throw std::invalid_argument("Fatal error: `obj` type is not pcb_data type!");
    }
    return json(*o);
}
static nlohmann::json pcb_interface_config_to_json(const pcb::pcb_interface_config *obj)
{
    const pcb_interface_config *o = dynamic_cast<const pcb_interface_config *>(obj);
    if (!o) {
        throw std::invalid_argument("Fatal error: `obj` type is not pcb_interface_config type!");
    }
    return json(*o);
}

register_type_factory(PCB, pcb_data_factory, pcb_interface_config_factory, pcb_data_to_json, pcb_interface_config_to_json);

}

std::ostream &operator<<(std::ostream &os, const pcb::pins_t &rhs)
{
    os << rhs.to_string();
    return os;
}
