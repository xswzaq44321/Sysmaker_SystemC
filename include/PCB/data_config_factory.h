// factory.h

#pragma once

#include <systemc>
#include <unordered_map>
#include <string>
#include <memory>

#include "PCB/pcb_payload.h"
#include "json.hpp"

class Data_Config_Factory {
public:
    using data_fact_t = std::function<std::unique_ptr<pcb::pcb_data_if>(const nlohmann::json &)>;
    using inte_conf_t = std::function<std::unique_ptr<pcb::pcb_interface_config_if>(const nlohmann::json &)>;

    /**
     * @brief Registers data pack's type and it's factory functions
     * @param type data pack's type
     * @param data_fact_func factory function for data section
     * @param interface_config_fact_func factory function for interface config section
     */
    explicit Data_Config_Factory(const std::string& type, data_fact_t data_fact_func, inte_conf_t interface_config_fact_func);

    /**
     * @brief Factory function for generating data pack's data section
     * @param type data pack's type
     * @param json_obj nlohmann::json representing data section
     * @return pointer to data section's base type (pcb::pcb_data_if*)
     */
    static std::unique_ptr<pcb::pcb_data_if> produce_data(const std::string& type, const nlohmann::json &json_obj);
    /**
     * @brief Factory function for generating data pack's interface config section
     * @param type data pack's type
     * @param json_obj nlohmann::json representing interface config section
     * @return pointer to interface config's base type (pcb::pcb_interface_config_if*)
     */
    static std::unique_ptr<pcb::pcb_interface_config_if> produce_interface_config(const std::string& type, const nlohmann::json &json_obj);

private:
    static std::unordered_map<std::string, data_fact_t> *data_factory;
    static std::unordered_map<std::string, inte_conf_t> *interface_config_factory;
};

/**
 * @brief Registers data pack's type and it's factory functions. You should only use this in source file
 * @param TYPE token: data pack's type
 * @param data_fact_func factory function for data section
 * @param interface_config_fact_func factory function for interface config section
 */
#define register_type_factory(TYPE, data_fact_func, interface_config_fact_func) \
    static Data_Config_Factory TYPE##_factory(#TYPE, data_fact_func, interface_config_fact_func);
