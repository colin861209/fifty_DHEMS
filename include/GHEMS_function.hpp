#ifndef GHEMS_function_H
#define GHEMS_function_H

#include <vector>
#include <string>
using namespace std;

// common parameter
extern int time_block, variable, divide, sample_time, point_num, piecewise_num, publicLoad_num;
extern float delta_T;
extern float Cbat, Vsys, SOC_ini, SOC_min, SOC_max, SOC_thres, Pbat_min, Pbat_max, Pgrid_max, Psell_max, Delta_battery, Pfc_max;

// flag
extern bool SOC_change_flag, publicLoad_flag;

int determine_realTimeOrOneDayMode_andGetSOC(int real_time, vector<string> variable_name);
float *getOrUpdate_SolarInfo_ThroughSampleTime(const char *weather);
void updateTableCost(float *totalLoad, float *totalLoad_price, float *real_grid_pirce, float *fuelCell_kW_price, float *Hydrogen_g_consumption, float *real_sell_price, float *demandResponse_feedback, float totalLoad_sum, float totalLoad_priceSum, float real_grid_pirceSum, float fuelCell_kW_priceSum, float Hydrogen_g_consumptionSum, float real_sell_priceSum, float totalLoad_taipowerPriceSum, float demandResponse_feedbackSum);
void optimization(vector<string> variable_name, vector<float> Pgrid_max_array, float *load_model, float *price);
void setting_GLPK_columnBoundary(vector<string> variable_name, vector<float> Pgrid_max_array, glp_prob *mip);
void calculateCostInfo(float *price, bool publicLoad_flag, bool Pgrid_flag, bool Psell_flag, bool Pess_flag, bool Pfc_flag);
void insert_GHEMS_variable();
float getPrevious_battery_dischargeSOC(int sample_time, string target_equip_name);
float *get_allDay_price(string col_name);
float *get_totalLoad_power(bool uncontrollable_load_flag);
float **getPublicLoad(int, int);
int *countPublicLoads_AlreadyOpenedTimes(int publicLoad_num);
int *count_publicLoads_RemainOperateTime(int public_num, int *public_ot, int *buff);

#endif
