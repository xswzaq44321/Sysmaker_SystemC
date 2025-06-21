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
    using data_fact_t      = std::function<std::unique_ptr<pcb::pcb_data_if>(const nlohmann::json &)>;
    using inte_conf_fact_t = std::function<std::unique_ptr<pcb::pcb_interface_config_if>(const nlohmann::json &)>;
    using data_json_t      = std::function<nlohmann::json(const pcb::pcb_data_if *)>;
    using inte_conf_json_t = std::function<nlohmann::json(const pcb::pcb_interface_config_if *)>;

    /**
     * @brief Registers data pack's type and it's factory functions
     * @param type data pack's type
     * @param data_fact_func factory function for data section
     * @param interface_config_fact_func factory function for interface config section
     * @param data_json converter function for converting data section into json
     * @param inte_conf_json converter function for converting interface config section into json
     */
    explicit Data_Config_Factory(const std::string &type, data_fact_t data_fact_func, inte_conf_fact_t interface_config_fact_func, data_json_t data_json, inte_conf_json_t inte_conf_json);

    /**
     * @brief Factory function for generating data pack's data section
     * @param type data pack's type
     * @param json_obj nlohmann::json representing data section
     * @return pointer to data section's base type (pcb::pcb_data_if*)
     */
    static std::unique_ptr<pcb::pcb_data_if>             produce_data(const std::string &type, const nlohmann::json &json_obj);
    /**
     * @brief Factory function for generating data pack's interface config section
     * @param type data pack's type
     * @param json_obj nlohmann::json representing interface config section
     * @return pointer to interface config's base type (pcb::pcb_interface_config_if*)
     */
    static std::unique_ptr<pcb::pcb_interface_config_if> produce_interface_config(const std::string &type, const nlohmann::json &json_obj);
    /**
     * @brief Conversion function for converting data pack's data section into json
     * @param type data pack's type
     * @param obj pointer to data section
     * @return nlohmann::json obj
     */
    static nlohmann::json                                data_to_json(const std::string &type, const pcb::pcb_data_if *obj);
    /**
     * @brief Conversion function for converting data pack's interface config section into json
     * @param type data pack's type
     * @param obj pointer to interface config section
     * @return nlohmann::json obj
     */
    static nlohmann::json                                interface_config_to_json(const std::string &type, const pcb::pcb_interface_config_if *obj);

private:
    static std::unordered_map<std::string, data_fact_t>      *data_factory;
    static std::unordered_map<std::string, inte_conf_fact_t> *interface_config_factory;
    static std::unordered_map<std::string, data_json_t>      *data_converter;
    static std::unordered_map<std::string, inte_conf_json_t> *interface_config_converter;
};

/**
 * @brief Registers data pack's type and it's factory functions. You should only use this in source file
 * @param TYPE token: data pack's type
 * @param data_fact_func factory function for data section
 * @param interface_config_fact_func factory function for interface config section
 * @param data_json converter function for converting data section into json
 * @param inte_conf_json converter function for converting interface config section into json
 */
#define register_type_factory(TYPE, data_fact_func, interface_config_fact_func, data_json, inte_conf_json) \
    static Data_Config_Factory TYPE##_factory(#TYPE, data_fact_func, interface_config_fact_func, data_json, inte_conf_json);
