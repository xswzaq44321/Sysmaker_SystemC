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

// pins_t should provide a total order over pins, to prevent dead lock
class pins_t : public std::set<std::string> {
public:
    using std::set<std::string>::set;

    std::string to_string() const
    {
        std::string str("{");
        bool        needComma = false;
        for (const std::string &pinStr : *this) {
            if (needComma) {
                str += ", ";
            }
            str += pinStr;
            needComma = true;
        }
        str += "}";
        return str;
    }

    bool intersect(const pins_t& rhs) const {
        auto it = this->cbegin();
        auto jt = rhs.cbegin();
        while(it != this->cend() && jt != rhs.cend()){
            if(*it < *jt)
                ++it;
            else if(*it > *jt)
                ++jt;
            else
                return true;
        }
        return false;
    }
};
void to_json(json &j, const pins_t &p);
void from_json(const json &j, pins_t &p);

class pin_config_t {
public:
    using map_type = std::unordered_map<std::string, pin_func_t>;
    map_type m;

    pin_config_t() = default;
    pin_config_t(std::initializer_list<map_type::value_type> init)
        : m(init)
    {
    }

    template <typename... Ts>
    auto insert(Ts &&...args) -> decltype(m.insert(std::forward<Ts>(args)...))
    {
        return m.insert(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto find(Ts &&...args) -> decltype(m.find(std::forward<Ts>(args)...))
    {
        return m.find(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto operator[](Ts &&...args) -> decltype(m.operator[](std::forward<Ts>(args)...))
    {
        return m.operator[](std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto count(Ts &&...args) -> decltype(m.count(std::forward<Ts>(args)...))
    {
        return m.count(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto begin(Ts &&...args) -> decltype(m.begin(std::forward<Ts>(args)...))
    {
        return m.begin(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto begin(Ts &&...args) const -> decltype(m.begin(std::forward<Ts>(args)...))
    {
        return m.begin(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto cbegin(Ts &&...args) const -> decltype(m.cbegin(std::forward<Ts>(args)...))
    {
        return m.cbegin(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto end(Ts &&...args) -> decltype(m.end(std::forward<Ts>(args)...))
    {
        return m.end(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto end(Ts &&...args) const -> decltype(m.end(std::forward<Ts>(args)...))
    {
        return m.end(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    auto cend(Ts &&...args) const -> decltype(m.cend(std::forward<Ts>(args)...))
    {
        return m.cend(std::forward<Ts>(args)...);
    }

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
        return "NOT_FOUND";
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

class pcb_interface_config_if {
public:
    virtual ~pcb_interface_config_if() = 0;

    pin_config_t pin_config;
};

class pcb_data_if {
public:
    virtual ~pcb_data_if() = 0;
};

class pcb_payload {
public:
    pcb_payload()                                  = default;
    pcb_payload(const pcb_payload &rhs)            = delete;
    pcb_payload &operator=(const pcb_payload &rhs) = delete;
    pcb_payload(const std::string &json_str);

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
    pcb_interface_config_if *get_interface_config() const { return m_interface_config.get(); }
    void                     set_interface_config(std::shared_ptr<pcb_interface_config_if> pic) { m_interface_config = pic; }
    pcb_data_if             *get_data() const { return m_data.get(); }
    void                     set_data(std::shared_ptr<pcb_data_if> data) { m_data = data; }

private:
    /* attributes */
    pins_t                                   m_pins;
    std::unique_ptr<sc_core::sc_time>        m_begin_time;
    std::unique_ptr<sc_core::sc_time>        m_end_time;
    std::string                              m_type;
    std::shared_ptr<pcb_interface_config_if> m_interface_config;
    std::shared_ptr<pcb_data_if>             m_data;

public:
    struct {
        int init;
        int targ;
    } tlm_route;
};

struct pcb_protocol_types {
    typedef pcb_payload    tlm_payload_type;
    typedef tlm::tlm_phase tlm_phase_type;
};

}

std::ostream &operator<<(std::ostream &os, const pcb::pins_t &rhs);