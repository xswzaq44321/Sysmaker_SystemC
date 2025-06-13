#include "peripherals/UART/driver.hpp"
#include "json.hpp"

using namespace sc_core;
using json = nlohmann::json;

void UART_Driver::hw_access(pcb::pcb_payload &trans, const tlm::tlm_phase &phase)
{
    auto IC   = dynamic_cast<UART_interface_config *>(trans.get_interface_config());
    auto data = dynamic_cast<UART_data *>(trans.get_data());
    Report_Info(SC_DEBUG, name(), "[%s]: Received transation data: \nIC: %s\nData: %s", sc_core::sc_time_stamp().to_string().c_str(), json(*IC).dump().c_str(), json(*data).dump().c_str());
    hw_access_done_peq.notify(trans, phase, sc_time("1us"));
}