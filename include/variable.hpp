#ifndef VARIABLE_H
#define VARIABLE_H

#include <vector>
#include <string.h>
#include <string>
using namespace std;

enum ENERGYMANAGESYSTEM
{   
    CEMS,
    HEMS
};

enum OBJECTIVETARGET{ MAXIMUM, MINIMUM };

class IMPORT {

private:

    typedef struct
    {
        int startTime;
        int endTime;
        int minDecrease_power;
        int feedback_price;
        int customer_baseLine;
        vector<int> demand_info;
        vector<float> Pgrid_max_array;
        vector<bool> participate_status;
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
        float hydro_price;
        float hydro_cons = 0.04;
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
        vector<int> start, end, ot, reot;
        vector<int> alreadyot;
        vector<float> power;
    }PUBLICLOAD;

    typedef struct
    {
        int load_num;
        vector<int> start, end, ot, reot;
        vector<int> alreadyot;
        vector<float> power;
    }INTERRUPT;

    typedef struct
    {
        int load_num;
        vector<int> start, end, ot, reot;
        vector<int> alreadyot;
        vector<bool> continue_flag;
        vector<float> power;
    }UNINTERRUPT;

    typedef struct
    {
        int load_num;
        vector<int> start, end, ot, reot;
        vector<int> alreadyot;
        vector<bool> continue_flag;
        vector<vector<float>> power;
        vector<vector<int>> block;
        vector<float> max_power;
    }VARYING;

public:

    vector<string> variable_name;
    DRINFO dr;
    BASEPARAMETER bp;
    FLAG fg;
    COMFORTLEVEL comf;
    PUBLICLOAD pl;
    INTERRUPT irl;
    UNINTERRUPT uirl;
    VARYING varl;
};

class EXPORT {

private:
    // IMPORT ipt;
    typedef struct
    {
        vector<float> new_load_model;
    }LOADMODEL;

    typedef struct
    {
        // all load comsuption & cost price
        vector<float> totalLoad;
        vector<float> cost_of_totalLoad;
        float summation_totalLoad;
        float summation_cost_of_totalLoad;
        // real use grid power & cost price
        vector<float> cost_of_gridOnly;
        float summation_cost_of_gridOnly;
        float summation_taipowercost_of_totalLoad;
        // fuel cell comsume grams of H2 & cost price
        vector<float> cost_of_fuelCell;
        vector<float> hydrogen_comsuption;
        float summation_cost_of_fuelCell;
        float summation_hydrogen_comsuption;
        // feedback price
        vector<float> feedback_of_sellGrid;
        vector<float> feedback_of_dr;
        float summation_feedback_of_realSell;
        float summation_feedback_of_dr;
        // table cost column cost_name
        string name_totalLoad = "total_load_power";
        string name_cost_of_totalLoad = "total_load_price";
        string name_cost_of_gridOnly = "real_buy_grid_price";
        string name_cost_of_fuelCell = "FC_price";
        string name_hydrogen_comsuption = "hydrogen_comsuption";
        string name_feedback_of_sellGrid = "real_sell_grid_price";
        string name_feedback_of_dr = "demand_response_feedback";
    }COSTINFORMATION;

public:
    LOADMODEL lm;
    COSTINFORMATION ci;
};

#endif