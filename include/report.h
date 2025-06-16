// report.h
#pragma once

#include <systemc>

#include <string>
#include <sstream>
#include <cstdio>
#include <cstdarg>

// template <typename MSG_TYPE, typename... Ts>
// void report_info_verb(int verbosity, MSG_TYPE msg_type, Ts &&...ts)
// {
//     std::stringstream ss;
//     ((ss << std::forward<Ts>(ts)), ...);
//     SC_REPORT_INFO_VERB(msg_type, ss.str().c_str(), verbosity);
// }

template <typename MSG_TYPE>
class Report_Info {
protected:
    int               verbosity;
    MSG_TYPE          msg_id;
    std::stringstream ss;

public:
    Report_Info(int verbosity, MSG_TYPE msg_id)
        : verbosity(verbosity)
        , msg_id(msg_id)
    {
    }
    Report_Info(int verbosity, MSG_TYPE msg_id, const char *format, ...)
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

        ss << buf.data();
    }
    Report_Info(int verbosity, MSG_TYPE msg_id, const char *format, va_list args)
        : verbosity(verbosity)
        , msg_id(msg_id)
    {
        va_list args2;

        va_copy(args2, args);
        int sz = vsnprintf(nullptr, 0, format, args2);
        va_end(args2);

        std::vector<char> buf(sz + 1);
        vsprintf(buf.data(), format, args);

        ss << buf.data();
    }
    ~Report_Info()
    {
        flush();
    }

    virtual void flush()
    {
        SC_REPORT_INFO_VERB(msg_id, ss.str().c_str(), verbosity);
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

template <typename MSG_TYPE>
class Report_Info_TStamped : public Report_Info<MSG_TYPE> {
public:
    Report_Info_TStamped(int verbosity, MSG_TYPE msg_id, const char *format, ...)
        : Report_Info<MSG_TYPE>(verbosity, msg_id, format)
    {
    }

    virtual void flush() override
    {
        std::string msg = "[" + sc_core::sc_time_stamp().to_string() + "]: " + this->ss.str();
        SC_REPORT_INFO_VERB(this->msg_id, msg.c_str(), this->verbosity);
    }
};