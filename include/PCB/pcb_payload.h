// pcb_payload.h
#pragma once

#include <systemc>
#include <tlm>

#include <set>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "PCB/common.h"
#include "json.hpp"

namespace pcb {

using json = nlohmann::json;

#define USING_FUNC(member, func)                                                \
    template <typename... Ts>                                                   \
    auto func(Ts &&...args) -> decltype(member.func(std::forward<Ts>(args)...)) \
    {                                                                           \
        return member.func(std::forward<Ts>(args)...);                          \
    }
#define USING_FUNC_CONST(member, func)                                                \
    template <typename... Ts>                                                         \
    auto func(Ts &&...args) const -> decltype(member.func(std::forward<Ts>(args)...)) \
    {                                                                                 \
        return member.func(std::forward<Ts>(args)...);                                \
    }

class pin_config_t {
public:
    using map_type = std::unordered_map<std::string, pin_func_t>;
    map_type m;

    pin_config_t() = default;
    pin_config_t(std::initializer_list<map_type::value_type> init)
        : m(init)
    {
    }

    USING_FUNC(m, insert)
    USING_FUNC(m, find)
    USING_FUNC(m, operator[])
    USING_FUNC(m, count)
    USING_FUNC(m, begin)
    USING_FUNC_CONST(m, begin)
    USING_FUNC_CONST(m, cbegin)
    USING_FUNC(m, end)
    USING_FUNC_CONST(m, end)
    USING_FUNC_CONST(m, cend)

    std::string to_string() const
    {
        std::string str("{ ");
        bool        needComma = false;
        for (const auto &[pin, pin_func] : m) {
            if (needComma) {
                str += ", ";
            }
            needComma = true;
            str += "{" + pin + ", " + pin_func.to_string() + "}";
        }
        str += " }";
        return str;
    }

    std::string func_to_pin(const pin_func_t &target) const
    {
        for (const auto &[pin, pin_func] : m) {
            if (pin_func == target)
                return pin;
        }
        return "";
    }

    pins_t to_pins() const
    {
        pins_t res;
        for (const auto &[pin, pin_func] : m) {
            res.insert(pin);
        }
        return res;
    }
};
void to_json(json &j, const pin_config_t &p);
void from_json(const json &j, pin_config_t &p);

#undef USING_FUNC
#undef USING_FUNC_CONST

class pcb_interface_config {
public:
    virtual ~pcb_interface_config() = default;

    pin_config_t pin_config;

public:
    /**
     * @brief updates internal member vairables based on passed in json obj
     * this is intended to be called when implementing derived class's `from_json`
     * @param j json object containing "Pin Configuration"
     */
    void from_json(const nlohmann::json &j);
    /**
     * @brief dumps internal member variables to passed in json
     * this is intended to be called when implementing derived class's `to_json`
     * @param j json object to store the result
     */
    void to_json(nlohmann::json &j) const;
};
void to_json(nlohmann::json &j, const pcb_interface_config &o);
void from_json(const nlohmann::json &j, pcb_interface_config &o);

class pcb_data {
public:
    virtual ~pcb_data() = default;

    std::unordered_map<std::string, std::string> pin_states;

public:
    /**
     * @brief updates internal member vairables based on passed in json obj
     * this is intended to be called when implementing derived class's `from_json`
     * @param j json object containing "PinStates"
     */
    void from_json(const nlohmann::json &j);
    /**
     * @brief dumps internal member variables to passed in json
     * this is intended to be called when implementing derived class's `to_json`
     * @param j json object to store the result
     */
    void to_json(nlohmann::json &j) const;
};
void to_json(nlohmann::json &j, const pcb_data &o);
void from_json(const nlohmann::json &j, pcb_data &o);

class pcb_payload {
public:
    pcb_payload()                                  = default;
    pcb_payload(pcb_payload &&rhs)                 = default;
    pcb_payload(const pcb_payload &rhs)            = delete;
    pcb_payload &operator=(const pcb_payload &rhs) = delete;

    virtual ~pcb_payload() = default;

public:
    /* API */
    pins_t                   get_pins() const { return m_pins; }
    void                     set_pins(const pins_t &pins) { m_pins = pins; }
    sc_core::sc_time        *get_begin_time() const { return m_begin_time.get(); }
    void                     set_begin_time(sc_core::sc_time time) { m_begin_time = std::make_unique<sc_core::sc_time>(time); }
    sc_core::sc_time        *get_end_time() const { return m_end_time.get(); }
    void                     set_end_time(sc_core::sc_time time) { m_end_time = std::make_unique<sc_core::sc_time>(time); }
    std::string              get_type() const { return m_type; }
    void                     set_type(std::string type) { m_type = type; }
    pcb_interface_config *get_interface_config() const { return m_interface_config.get(); }
    void                     set_interface_config(std::shared_ptr<pcb_interface_config> pic) { m_interface_config = pic; }
    pcb_data             *get_data() const { return m_data.get(); }
    void                     set_data(std::shared_ptr<pcb_data> data) { m_data = data; }

private:
    /* attributes */
    pins_t                                   m_pins;
    std::unique_ptr<sc_core::sc_time>        m_begin_time;
    std::unique_ptr<sc_core::sc_time>        m_end_time;
    std::string                              m_type;
    std::shared_ptr<pcb_interface_config> m_interface_config;
    std::shared_ptr<pcb_data>             m_data;

    friend void to_json(json &j, const pcb_payload &p);
    friend void from_json(const json &j, pcb_payload &p);

public:
    struct {
        int init;
        int targ;
    } tlm_route;
};
void to_json(json &j, const pcb_payload &p);
void from_json(const json &j, pcb_payload &p);

struct pcb_protocol_types {
    typedef pcb_payload    tlm_payload_type;
    typedef tlm::tlm_phase tlm_phase_type;
};

}

std::ostream &operator<<(std::ostream &os, const pcb::pins_t &rhs);