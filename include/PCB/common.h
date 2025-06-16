// common.h

#pragma once

#include <string>

#include "json.hpp"

namespace pcb {

using json = nlohmann::json;

// descriptive class for what function this pin haves
// such as input or ourput? data format?
class pin_func_t {
    std::string name;

public:
    pin_func_t() = default;
    pin_func_t(const std::string &s)
        : name(s)
    {
    }
    pin_func_t(const char *s)
        : name(s)
    {
    }
    friend bool operator==(const pin_func_t &, const pin_func_t &);

    std::string to_string() const
    {
        return name;
    }
};
inline void from_json(const json &j, pin_func_t &p)
{
    p = j.get<std::string>();
}
inline void to_json(json &j, const pin_func_t &p)
{
    j = json { p.to_string() };
}
inline bool operator==(const pin_func_t &lhs, const pin_func_t &rhs)
{
    return lhs.to_string() == rhs.to_string();
}

}
