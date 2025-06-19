// report.h
#pragma once

#include <systemc>

#include <string>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <variant>

template <bool timestamp = true>
class Report_Info {
protected:
    sc_core::sc_verbosity           verbosity;
    std::variant<int, const char *> msg_id;
    std::stringstream               ss;

public:
    Report_Info(sc_core::sc_verbosity verbosity, std::variant<int, const char *> msg_id)
        : verbosity(verbosity)
        , msg_id(msg_id)
    {
        if constexpr (timestamp) {
            ss << "[" << sc_core::sc_time_stamp() << "]: ";
        }
    }
    Report_Info(sc_core::sc_verbosity verbosity, std::variant<int, const char *> msg_id, const char *format, ...) __attribute__((format(printf, 4, 5)))
        : verbosity(verbosity)
        , msg_id(msg_id)
    {
        va_list args, args2;
        va_start(args, format);

        va_copy(args2, args);
        int sz = vsnprintf(nullptr, 0, format, args2);
        va_end(args2);

        std::vector<char> buf(sz + 1);
        vsprintf(buf.data(), format, args);
        va_end(args);

        if constexpr (timestamp) {
            ss << "[" << sc_core::sc_time_stamp() << "]: ";
        }
        ss << buf.data();
    }
    ~Report_Info()
    {
        flush();
    }

    virtual void flush()
    {
        if (std::holds_alternative<int>(msg_id)) {
            SC_REPORT_INFO_VERB(std::get<int>(msg_id), ss.str().c_str(), verbosity);
        } else if (std::holds_alternative<const char *>(msg_id)) {
            SC_REPORT_INFO_VERB(std::get<const char *>(msg_id), ss.str().c_str(), verbosity);
        }
    }

    Report_Info &printf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        int sz = vsnprintf(nullptr, 0, format, args);

        std::vector<char> buf(sz + 1);
        vsprintf(buf.data(), format, args);

        va_end(args);

        ss << buf.data();

        return *this;
    }

    template <typename T>
    Report_Info &operator<<(T &&t)
    {
        ss << std::forward<T>(t);
        return *this;
    }

    // This is to deal with std::endl
    using Stdendl = std::ostream &(*)(std::ostream &);
    Report_Info &operator<<(Stdendl manip)
    {
        manip(ss);
        return *this;
    }
};
