#ifndef GHEMS_function_H
#define GHEMS_function_H

#include <vector>
#include <string>
using namespace std;

// common parameter
extern int time_block, variable, divide, sample_time, point_num, piecewise_num;
extern float delta_T;
extern float Pgrid_max, Psell_max, Delta_battery, Pfc_max;

// flag
extern bool SOC_change_flag;

typedef struct 
{
	bool flag;
	int number;
	vector<int> start;
	vector<int> end;
	vector<int> operation_time;
	vector<int> remain_operation_time;
	vector<float> power;
	string str_publicLoad = "publicLoad";
} PUBLICLOAD;

typedef struct 
{
	bool flag;
	bool generate_result_flag;
	bool can_discharge;
	int total_charging_pole;
	int normal_charging_pole;
	int fast_charging_pole;
	int super_fast_charging_pole;
	int can_charge_amount;
	float normal_charging_power;
	float MAX_SOC;
	float MIN_SOC;
	float threshold_SOC;
	vector<int> Pole_ID, start_timeblock, departure_timeblock, number;
	vector<float> now_SOC, start_SOC, battery_capacity;
    string str_charging = "EM_charging";
	string str_discharging = "EM_discharging";
	string str_mu = "EM_mu";
} ELECTRICMOTOR;

// typedef struct 
// {
// 	bool flag;
// 	int total_charging_pole;
// 	int can_charge_amount;
// 	float charging_power;
// 	float MAX_SOC;
// 	float MIN_SOC;
// 	float threshold_SOC;
// 	string str_charging = "EV_charging";
// 	string str_discharging = "EV_discharging";
// 	string str_mu = "EV_mu";
// 	string str_Pcharge = "EV_Pcharge";
// 	string str_Pdischarge = "EV_Pdischarge";
// 	string str_SOC = "EV_SOC";
// } ELECTRICVEHICLE;

int determine_realTimeOrOneDayMode_andGetSOC(ENERGYSTORAGESYSTEM &ess, ELECTRICMOTOR em, int real_time, vector<string> variable_name);
float *getOrUpdate_SolarInfo_ThroughSampleTime(const char *weather);
void updateTableCost(float *totalLoad, float *totalLoad_price, float *real_grid_pirce, float *publicLoad, float *publicLoad_price, float *fuelCell_kW_price, float *Hydrogen_g_consumption, float *real_sell_pirce, float *demandResponse_feedback, float totalLoad_sum, float totalLoad_priceSum, float real_grid_pirceSum, float publicLoad_sum, float publicLoad_priceSum, float fuelCell_kW_priceSum, float Hydrogen_g_consumptionSum, float real_sell_pirceSum, float totalLoad_taipowerPriceSum, float demandResponse_feedbackSum);
void optimization(ENERGYSTORAGESYSTEM ess, PUBLICLOAD pl, ELECTRICMOTOR em, vector<string> variable_name, vector<float> Pgrid_max_array, float *load_model, float *price);
void setting_GLPK_columnBoundary(ENERGYSTORAGESYSTEM ess, PUBLICLOAD pl, ELECTRICMOTOR em, vector<string> variable_name, vector<float> Pgrid_max_array, glp_prob *mip);
void calculateCostInfo(PUBLICLOAD pl, float *price, bool Pgrid_flag, bool Psell_flag, bool Pess_flag, bool Pfc_flag);
void updateSingleHouseholdCost();
void insert_GHEMS_variable(ENERGYSTORAGESYSTEM ess);
float getPrevious_battery_dischargeSOC(int sample_time, string target_equip_name);
float *get_allDay_price(string col_name);
float *get_totalLoad_power(bool uncontrollable_load_flag);
int *countPublicLoads_AlreadyOpenedTimes(int publicLoad_num);
vector<int> count_publicLoads_RemainOperateTime(int public_num, vector<int> public_ot, int *buff);
void update_fullSOC_or_overtime_EM_inPole(ELECTRICMOTOR em, int sample_time);
void record_vehicle_result(string table, float SOC, int sample_time, int number);
void empty_charging_pole(int pole_id);
int enter_newEMInfo_inPole(ELECTRICMOTOR em, int sample_time);
void generate_vehicle_result(ELECTRICMOTOR em, string table, int EM_amount, int time_mean, int time_variance, int soc_mean, int soc_variance, int wait_mean, int wait_variance, int start_number, vector<int> user, vector<float> capacity, vector<int> type_id, vector<int> empty_pole, int sample_time);
void fetch_vehicle_result(string table, int EM_amount, int start_number, vector<int> empty_pole, int sample_time);
void enter_charging_pole(int number, int wait, float SOC, float BAT_capacity, int start_timeblock, int departure_timeblock, int pole_id);
void insert_vehicle_result(string table, int Pole_ID, int number, int type, float BAT_CAP, int start_timeblock, float start_SOC, int original_departure_timeblock, int wait);
#endif
