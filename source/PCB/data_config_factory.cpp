#include "PCB/data_config_factory.h"

#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>

#include "json.hpp"

std::unordered_map<std::string, Data_Config_Factory::data_fact_t> *Data_Config_Factory::data_factory             = nullptr;
std::unordered_map<std::string, Data_Config_Factory::inte_conf_t> *Data_Config_Factory::interface_config_factory = nullptr;

Data_Config_Factory::Data_Config_Factory(const std::string &type, data_fact_t data_fact_func, inte_conf_t interface_config_fact_func)
{
    if (!data_factory) {
        static std::unordered_map<std::string, data_fact_t> data_factory_local;
        data_factory = &data_factory_local;
    }
    if (!interface_config_factory) {
        static std::unordered_map<std::string, inte_conf_t> interface_config_factory_local;
        interface_config_factory = &interface_config_factory_local;
    }
    if (data_factory->count(type) || interface_config_factory->count(type)) {
        throw std::invalid_argument("Fatal error: Factory function for type: " + type + " already exists!");
    }
    data_factory->insert_or_assign(type, data_fact_func);
    interface_config_factory->insert_or_assign(type, interface_config_fact_func);
}

std::unique_ptr<pcb::pcb_data_if> Data_Config_Factory::produce_data(const std::string &type, const nlohmann::json &json_obj)
{
    if (!data_factory || !data_factory->count(type)) {
        throw std::out_of_range("Factory function for type:" + type + " does not exists!");
    }
    return data_factory->at(type)(json_obj);
}
std::unique_ptr<pcb::pcb_interface_config_if> Data_Config_Factory::produce_interface_config(const std::string &type, const nlohmann::json &json_obj)
{
    if (!interface_config_factory || !interface_config_factory->count(type)) {
        throw std::out_of_range("Factory function for type:" + type + " does not exists!");
    }
    return interface_config_factory->at(type)(json_obj);
}