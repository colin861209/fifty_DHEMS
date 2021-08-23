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
    
    bool determine_distributedGroup_status(string condition);
    vector<int> split_array(string timearray);
    bool get_continuityLoad_flag(string load_type, int offset_num);
    int get_already_operate_time(string load_type, int offset_num);
    int get_remain_ot_time(int ot_time, int already_ot_time);
    int get_remain_ot_time(int ot_time, int already_ot_time, int flag);
    int determine_change_end_time(int ot, int already, int remain_time, int flag);

public:
    
    SQLACTION(std::string iP, std::string name, std::string passwd, std::string database);

    IMPORT ipt;
    EXPORT ept;

    // HEMS & CEMS import
    void get_dr_mode();
    void get_demand_response();
    void get_Pgrid_max_array();
    // common
    void get_experimental_parameters(ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);
    void get_allDay_price();
    // cems
    void determine_GHEMS_realTimeOrOneDayMode_andGetSOC();
    void getOrUpdate_SolarInfo_ThroughSampleTime();
    void get_totalLoad_power();
    // hems
    void get_distributedGroup_householdAndSampleTime(int group_num);
    void determine_LHEMS_realTimeOrOneDayMode_andGetSOC(int group_num);
    void init_totalLoad_tableAndFlag(int group_num);

    void get_flag(ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);
    void create_variable_name(ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::HEMS);

    int get_publicLoad_num();
    void get_publicLoad_info();
    int get_interrupt_num();
    void get_interrupt_info();
    int get_uninterrupt_num();
    void get_uninterrupt_info();
    int get_varying_num();
    void get_varying_info();

    // // HEMS & CEMS export
    void update_new_load_model();
    // void calculate_table_cost_info();
    // void update_table_cost_info();
    // void calculate_table_BaseParameter_total_cost_info();
};

#endif