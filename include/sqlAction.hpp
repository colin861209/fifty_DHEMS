#ifndef IMPORT_H
#define IMPORT_H
#include "SQL.hpp"
#include "variable.hpp"
#include <sstream>
#include <numeric>

class SQLACTION
{
private:
    
    SQL sql;
public:
    
    SQLACTION(std::string iP, std::string name, std::string passwd, std::string database);
    ~SQLACTION();

    IMPORT ipt;
    EXPORT ept;

    // HEMS & CEMS import
    // common
    void get_experimental_parameters(ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);
    void get_allDay_price();
    void get_flag(ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);
    void create_variable_name(ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);
    void get_dr_mode();
    void get_demand_response();
    // cems
    void determine_GHEMS_realTimeOrOneDayMode_andGetSOC();
    void getOrUpdate_SolarInfo_ThroughSampleTime();
    void get_totalLoad_power();
    void get_Pgrid_max_array();
    int get_publicLoad_num();
    void get_publicLoad_info();
    // hems
    void get_distributedGroup_householdAndSampleTime(int group_num);
    void determine_LHEMS_realTimeOrOneDayMode_andGetSOC(int group_num);
    void init_totalLoad_tableAndFlag(int group_num);
    int get_interrupt_num();
    void get_interrupt_info();
    int get_uninterrupt_num();
    void get_uninterrupt_info();
    int get_varying_num();
    void get_varying_info();

    // HEMS & CEMS export
    // cems
    void calculate_table_cost_info();
    void update_table_cost_info();
    void calculate_table_BaseParameter_total_cost_info();
    void update_table_BaseParameter_total_cost_info();
    void update_new_SOC();
    void update_Global_next_simulate_timeblock();
    // hems
    void update_new_load_model(int group_num);
    void update_next_simulate_timeblock(int group_num);
    void update_household_id(int group_num);
private:
    // common
    vector<int> split_array(string timearray);
    int get_already_operate_time(string load_type, int offset_num, ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);
    int get_remain_ot_time(int ot_time, int already_ot_time);
    // hems
    bool determine_distributedGroup_status(string condition);
    bool get_continuityLoad_flag(string load_type, int offset_num);
    int get_remain_ot_time(int ot_time, int already_ot_time, int flag);
    int determine_change_end_time(int ot, int already, int remain_time, int flag);
    vector<float> convert_real_power_array(vector<float> power_tmp, vector<int> block_tmp);
    vector<int> convert_real_block_array(int start, int end);
    float find_varyingLoad_max_power(vector<float> power);
    // cems
    void insert_table_cost(string cost_name, vector<float> costinfo);
    void update_table_cost(string cost_name, vector<float> costinfo);
    int calculate_publicLoad_decrease_operate_time(int start, int end);
};

#endif