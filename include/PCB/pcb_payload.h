// pcb_payload.h
#pragma once

#include <systemc>
#include <tlm>

#include <set>
#include <string>
#include <vector>

namespace pcb {

class pin_t : public std::string {
public:
    using std::string::string;

    const std::string &to_string() const
    {
        return *this;
    }
};

// pins_t should provide a total order over pins, to prevent dead lock
class pins_t : public std::set<pin_t> {
public:
    using std::set<pin_t>::set;

    std::string to_string() const
    {
        std::string str("{");
        bool        needComma = false;
        for (const auto &pinStr : *this) {
            if (needComma) {
                str += ", ";
            }
            str += pinStr;
            needComma = true;
        }
        str += "}";
        return str;
    }
};

class pcb_interface_config {
    virtual ~pcb_interface_config() = default;
};

class pcb_data {
    virtual ~pcb_data() = default;
};

class pcb_payload {
public:
    pcb_payload()
        : m_read(false)
        , m_begin_time(0)
        , m_end_time(0)
    {
    }
    pcb_payload(const pcb_payload &rhs)            = delete;
    pcb_payload &operator=(const pcb_payload &rhs) = delete;

    virtual ~pcb_payload() = default;

public:
    /* API */
    bool                                  is_read() const { return m_read; }
    void                                  set_read() { m_read = true; }
    bool                                  is_write() const { return !m_read; }
    void                                  set_write() { m_read = false; }
    pins_t                                get_pins() const { return m_pins; }
    void                                  set_pins(const pins_t &pins) { m_pins = pins; }
    sc_dt::uint64                         get_begin_time() const { return m_begin_time; }
    void                                  set_begin_time(sc_dt::uint64 time) { m_begin_time = time; }
    sc_dt::uint64                         get_end_time() const { return m_end_time; }
    void                                  set_end_time(sc_dt::uint64 time) { m_end_time = time; }
    std::string                           get_type() const { return m_type; }
    void                                  set_type(std::string type) { m_type = type; }
    std::shared_ptr<pcb_interface_config> get_interface_config() const { return m_interface_config; }
    void                                  set_interface_config(std::shared_ptr<pcb_interface_config> pic) { m_interface_config = pic; }
    std::shared_ptr<pcb_data>             get_data() const { return m_data; }
    void                                  set_data(std::shared_ptr<pcb_data> data) { m_data = data; }

private:
    /* attributes */
    bool                                  m_read;
    pins_t                                m_pins;
    sc_dt::uint64                         m_begin_time;
    sc_dt::uint64                         m_end_time;
    std::string                           m_type;
    std::shared_ptr<pcb_interface_config> m_interface_config;
    std::shared_ptr<pcb_data>             m_data;

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

inline std::ostream &operator<<(std::ostream &os, const pcb::pins_t &rhs)
{
    os << rhs.to_string();
    return os;
}

template <>
struct std::hash<pcb::pin_t> {
    std::size_t operator()(const pcb::pin_t &s) const noexcept
    {
        return std::hash<std::string> {}(s);
    }
};