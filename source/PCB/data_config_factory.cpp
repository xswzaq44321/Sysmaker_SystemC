#include "PCB/data_config_factory.h"

#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>

#include "json.hpp"

std::unordered_map<std::string, Data_Config_Factory::data_fact_t>      *Data_Config_Factory::data_factory               = nullptr;
std::unordered_map<std::string, Data_Config_Factory::inte_conf_fact_t> *Data_Config_Factory::interface_config_factory   = nullptr;
std::unordered_map<std::string, Data_Config_Factory::data_json_t>      *Data_Config_Factory::data_converter             = nullptr;
std::unordered_map<std::string, Data_Config_Factory::inte_conf_json_t> *Data_Config_Factory::interface_config_converter = nullptr;

Data_Config_Factory::Data_Config_Factory(const std::string &type, data_fact_t data_fact_func, inte_conf_fact_t interface_config_fact_func, data_json_t data_json, inte_conf_json_t inte_conf_json)
{
    if (!data_factory) {
        static std::unordered_map<std::string, data_fact_t> data_factory_local;
        data_factory = &data_factory_local;
    }
    if (!interface_config_factory) {
        static std::unordered_map<std::string, inte_conf_fact_t> interface_config_factory_local;
        interface_config_factory = &interface_config_factory_local;
    }
    if (!data_converter) {
        static std::unordered_map<std::string, Data_Config_Factory::data_json_t> data_converter_local;
        data_converter = &data_converter_local;
    }
    if (!interface_config_converter) {
        static std::unordered_map<std::string, Data_Config_Factory::inte_conf_json_t> interface_config_converter_local;
        interface_config_converter = &interface_config_converter_local;
    }
    if (data_factory->count(type) || interface_config_factory->count(type)) {
        throw std::invalid_argument("Fatal error: Factory function for type: \"" + type + "\" already exists!");
    }
    if (data_converter->count(type) || interface_config_converter->count(type)) {
        throw std::invalid_argument("Fatal error: Converter function for type: \"" + type + "\" already exists!");
    }
    data_factory->insert_or_assign(type, data_fact_func);
    interface_config_factory->insert_or_assign(type, interface_config_fact_func);
    data_converter->insert_or_assign(type, data_json);
    interface_config_converter->insert_or_assign(type, inte_conf_json);
}

std::unique_ptr<pcb::pcb_data> Data_Config_Factory::produce_data(const std::string &type, const nlohmann::json &json_obj)
{
    if (!data_factory || !data_factory->count(type)) {
        throw std::out_of_range("Factory function for type: \"" + type + "\" does not exists!");
    }
    return data_factory->at(type)(json_obj);
}
std::unique_ptr<pcb::pcb_interface_config> Data_Config_Factory::produce_interface_config(const std::string &type, const nlohmann::json &json_obj)
{
    if (!interface_config_factory || !interface_config_factory->count(type)) {
        throw std::out_of_range("Factory function for type: \"" + type + "\" does not exists!");
    }
    return interface_config_factory->at(type)(json_obj);
}
nlohmann::json Data_Config_Factory::data_to_json(const std::string &type, const pcb::pcb_data *obj)
{
    if (!data_converter || !data_converter->count(type)) {
        throw std::out_of_range("Converter function for type: \"" + type + "\" does not exists!");
    }
    return data_converter->at(type)(obj);
}

nlohmann::json Data_Config_Factory::interface_config_to_json(const std::string &type, const pcb::pcb_interface_config *obj)
{
    if (!interface_config_converter || !interface_config_converter->count(type)) {
        throw std::out_of_range("Converter function for type: \"" + type + "\" does not exists!");
    }
    return interface_config_converter->at(type)(obj);
}
