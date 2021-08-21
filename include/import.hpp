#ifndef IMPORT_H
#define IMPORT_H
#include "SQL.hpp"
#include <sstream>

class import
{
private:
    SQL sql;
    
    // int find_variableName_position(vector<string> variableNameArray, string target);
    // void messagePrint(int lineNum, const char *message, char contentSize = 'S', float content = 0,  char tabInHeader = 'N');
    bool determine_distributedGroup_status(string condition);
    vector<string> split_array(string timearray);

    // typedef struct
    // {
    //     string cems = "cems";
    //     string hems = "hems";
    // }ENERGYMANAGESYSTEM;
    
    typedef struct
    {
        int startTime;
        int endTime;
        int minDecrease_power;
        int feedback_price;
        int customer_baseLine;
        vector<int> demand_info;
        vector<float> Pgrid_max_array;
    }DRINFO;

    typedef struct
    {
        int time_block;
        int householdAmount;
        int householdDistributed;
        float divide;
        float delta_T;
        int variable_num;
        // hems only 
        int distributed_householdAmount;
        int real_household_id;          // in range 1~5, 6~10 etc...
        int distributed_household_id;   // in range 1~distributed_householdAmount
        int app_count;
        int next_simulate_timeblock;
        // battery
        int Vsys;
        float Cbat;
        float SOC_min;
        float SOC_max;
        float SOC_threshold;
        float SOC_ini;
        float Pbat_min;
        float Pbat_max;
        // buy or sell taipower electric
        float Pgrid_max;
        float Psell_max;
        // fuel cell
        float Pfc_max;
        float point_num = 6;
        float piecewise_num = point_num - 1;
        // weather
        string simulate_weather;
        vector<float> weather;
        // price
        string simulate_price;
        vector<float> price;
        // cems only
        vector<float> load_model;
        int Global_next_simulate_timeblock;
    }BASEPARAMETER;

    typedef struct
    {
        // hems only
        bool interrupt;
        bool uninterrupt;
        bool varying;
        bool comfortLevel;

        // common flag
        bool Pgrid;
        bool Pess;
        bool uncontrollable_load;
        bool real_time;
        bool Global_real_time;
        int dr_mode;

        // cems only flag
        bool publicLoad;
        bool mu_grid;
        bool Psell;
        bool SOCchange;
        bool Pfc;
    }FLAG;
    
    typedef struct
    {
        

    }COMFORTLEVEL;

    typedef struct
    {
        int load_num;
        vector<vector<int>> time_info;
        vector<float> power;
    }PUBLICLOAD;

    typedef struct
    {
        int load_num;
        vector<vector<int>> time_info;
        vector<float> power;
    }INTERRUPT;

    typedef struct
    {
        int load_num;
        vector<vector<int>> time_info;
        vector<float> power;
    }UNINTERRUPT;

    typedef struct
    {
        int load_num;
        vector<vector<int>> time_info;
        vector<vector<float>> power;
        vector<vector<int>> block;
    }VARYING;

public:
    
    import(std::string iP, std::string name, std::string passwd, std::string database);
    // ENERGYMANAGESYSTEM sys;
    DRINFO dr;
    void get_dr_mode();
    void get_demand_response();
    void get_Pgrid_max_array();

    BASEPARAMETER bp;
    // common
    void get_experimental_parameters(string ems_name);
    void get_allDay_price();
    // cems
    void determine_GHEMS_realTimeOrOneDayMode_andGetSOC();
    void getOrUpdate_SolarInfo_ThroughSampleTime();
    void get_totalLoad_power();
    // hems
    void get_distributedGroup_householdAndSampleTime(int group_num);
    void determine_LHEMS_realTimeOrOneDayMode_andGetSOC(int group_num);
    void init_totalLoad_tableAndFlag(int group_num);

    FLAG fg;
    vector<string> variable_name;
    void get_flag(string ems_name);
    void create_variable_name(string ems_name);

    PUBLICLOAD pl;
    int get_publicLoad_num();
    void get_publicLoad_info();
    INTERRUPT irl;
    int get_interrupt_num();
    void get_interrupt_info();
    UNINTERRUPT uirl;
    int get_uninterrupt_num();
    void get_uninterrupt_info();
    VARYING varl;
    int get_varying_num();
    void get_varying_info();
};

#endif