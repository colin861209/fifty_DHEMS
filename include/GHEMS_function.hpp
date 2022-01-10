#ifndef GHEMS_function_H
#define GHEMS_function_H

#include <vector>
#include <string>
using namespace std;

// common parameter
extern int time_block, variable, divide, sample_time, point_num, piecewise_num, publicLoad_num;
extern float delta_T;
extern float Cbat, Vsys, SOC_ini, SOC_min, SOC_max, SOC_thres, Pbat_min, Pbat_max, Pgrid_max, Psell_max, Delta_battery, Pfc_max;

// EM parameter
extern int total_charging_pole, normal_charging_pole, fast_charging_pole, super_fast_charging_pole, EM_can_charge_amount;
extern float EM_MAX_SOC, EM_MIN_SOC, EM_threshold_SOC;

// flag
extern bool SOC_change_flag, publicLoad_flag, EM_flag, EM_generate_result_flag, EM_can_discharge;

int determine_realTimeOrOneDayMode_andGetSOC(int real_time, vector<string> variable_name);
float *getOrUpdate_SolarInfo_ThroughSampleTime(const char *weather);
void updateTableCost(float *totalLoad, float *totalLoad_price, float *real_grid_pirce, float *publicLoad, float *publicLoad_price, float *fuelCell_kW_price, float *Hydrogen_g_consumption, float *real_sell_pirce, float *demandResponse_feedback, float totalLoad_sum, float totalLoad_priceSum, float real_grid_pirceSum, float publicLoad_sum, float publicLoad_priceSum, float fuelCell_kW_priceSum, float Hydrogen_g_consumptionSum, float real_sell_pirceSum, float totalLoad_taipowerPriceSum, float demandResponse_feedbackSum);
void optimization(vector<string> variable_name, vector<float> Pgrid_max_array, float *load_model, float *price);
void setting_GLPK_columnBoundary(vector<string> variable_name, vector<float> Pgrid_max_array, glp_prob *mip);
void calculateCostInfo(float *price, bool publicLoad_flag, bool Pgrid_flag, bool Psell_flag, bool Pess_flag, bool Pfc_flag);
void updateSingleHouseholdCost();
void insert_GHEMS_variable();
float getPrevious_battery_dischargeSOC(int sample_time, string target_equip_name);
float *get_allDay_price(string col_name);
float *get_totalLoad_power(bool uncontrollable_load_flag);
int *countPublicLoads_AlreadyOpenedTimes(int publicLoad_num);
int *count_publicLoads_RemainOperateTime(int public_num, int *public_ot, int *buff);
void update_fullSOC_or_overtime_EM_inPole(int sample_time);
void record_vehicle_result(string table, float SOC, int sample_time, int number);
void empty_charging_pole(int pole_id);
int enter_newEMInfo_inPole(int sample_time);
void generate_vehicle_result(string table, int EM_amount, int time_mean, int time_variance, int soc_mean, int soc_variance, int wait_mean, int wait_variance, int start_number, vector<int> user, vector<float> capacity, vector<int> type_id, vector<int> empty_pole, int sample_time);
void fetch_vehicle_result(string table, int EM_amount, int start_number, vector<int> empty_pole, int sample_time);
void enter_charging_pole(int number, int wait, float SOC, float BAT_capacity, int start_timeblock, int departure_timeblock, int pole_id);
void insert_vehicle_result(string table, int Pole_ID, int number, int type, float BAT_CAP, int start_timeblock, float start_SOC, int original_departure_timeblock, int wait);
#endif
