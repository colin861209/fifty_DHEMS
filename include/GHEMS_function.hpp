#ifndef GHEMS_function_H
#define GHEMS_function_H

#include <vector>
#include <string>
using namespace std;

typedef struct 
{
	// flag
	bool flag;
	string str_publicLoad = "publicLoad";
	// Type of public load
	int stoppable_group_id = 5;
	int stoppable_number;
	vector<int> stoppable_start;
	vector<int> stoppable_end;
	vector<int> stoppable_operation_time;
	vector<int> stoppable_remain_operation_time;
	vector<float> stoppable_power;
	string str_stoppable_publicLoad = "stoppable_publicLoad";
	int deferrable_group_id = 6;
	int deferrable_number;
	vector<int> deferrable_start;
	vector<int> deferrable_end;
	vector<int> deferrable_operation_time;
	vector<int> deferrable_remain_operation_time;
	vector<float> deferrable_power;
	string str_deferrable_publicLoad = "deferrable_publicLoad";
} PUBLICLOAD;

typedef struct 
{
	bool flag;
	bool generate_result_flag;
	bool can_discharge;
	int total_charging_pole;
	int normal_charging_pole;
	int can_charge_amount;
	float normal_charging_power;
	float MAX_SOC;
	float MIN_SOC;
	float threshold_SOC;
	vector<int> Pole_ID, start_timeblock, departure_timeblock, number;
	vector<float> now_SOC, start_SOC, battery_capacity;
    string str_charging = "EM_charging";
	string str_discharging = "EM_discharging";
} ELECTRICMOTOR;

typedef struct 
{
	bool flag;
	bool generate_result_flag;
	bool can_discharge;
	int total_charging_pole;
	int can_charge_amount;
	float charging_power;
	float MAX_SOC;
	float MIN_SOC;
	float threshold_SOC;
	vector<int> Pole_ID, start_timeblock, departure_timeblock, number;
	vector<float> now_SOC, start_SOC, battery_capacity;
	string str_charging = "EV_charging";
	string str_discharging = "EV_discharging";
} ELECTRICVEHICLE;

int determine_realTimeOrOneDayMode_andGetSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM &ess, ELECTRICMOTOR em, ELECTRICVEHICLE ev, int real_time);
float *getOrUpdate_SolarInfo_ThroughSampleTime(BASEPARAMETER bp, const char *weather);
void updateTableCost(BASEPARAMETER bp, float *totalLoad, float *totalLoad_price, float *real_grid_pirce, float *publicLoad, float *publicLoad_price, float *fuelCell_kW_price, float *Hydrogen_g_consumption, float *real_sell_pirce, float *demandResponse_feedback, float totalLoad_sum, float totalLoad_priceSum, float real_grid_pirceSum, float publicLoad_sum, float publicLoad_priceSum, float fuelCell_kW_priceSum, float Hydrogen_g_consumptionSum, float real_sell_pirceSum, float totalLoad_taipowerPriceSum, float demandResponse_feedbackSum);
void optimization(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, PUBLICLOAD pl, UNCONTROLLABLELOAD ucl, ELECTRICMOTOR em, ELECTRICVEHICLE ev);
void setting_GLPK_columnBoundary(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, PUBLICLOAD pl, ELECTRICMOTOR em, ELECTRICVEHICLE ev, glp_prob *mip);
void calculateCostInfo(BASEPARAMETER bp, DEMANDRESPONSE dr, PUBLICLOAD pl, ELECTRICMOTOR em, ELECTRICVEHICLE ev, UNCONTROLLABLELOAD ucl);
void updateSingleHouseholdCost(BASEPARAMETER bp, DEMANDRESPONSE dr);
void insert_GHEMS_variable(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess);
float getPrevious_battery_dischargeSOC(int time_block, int sample_time, string target_equip_name);
float *get_totalLoad_power(int time_block, bool uncontrollable_load_flag);
int *countPublicLoads_AlreadyOpenedTimes(BASEPARAMETER bp, int publicLoad_num, string publicLoad_name);
vector<int> count_publicLoads_RemainOperateTime(int public_num, vector<int> public_ot, int *buff);
void Global_UCload_rand_operationTime(BASEPARAMETER bp, UNCONTROLLABLELOAD &ucl);

// electric vehicle & motor 
// major
void update_fullSOC_or_overtime_EM_inPole(ELECTRICMOTOR em, int sample_time);
void update_fullSOC_or_overtime_EV_inPole(ELECTRICVEHICLE ev, int sample_time);
int enter_newEMInfo_inPole(ELECTRICMOTOR em, int sample_time);
int enter_newEVInfo_inPole(ELECTRICVEHICLE ev, int sample_time);
// minor
void record_vehicle_result(string table, float SOC, int sample_time, int number);
void empty_charging_pole(string table, int pole_id);
void generate_vehicle_result(ELECTRICMOTOR em, string table, int EM_amount, int time_mean, int time_variance, int soc_mean, int soc_variance, int wait_mean, int wait_variance, int start_number, vector<int> user, vector<float> capacity, vector<int> type_id, vector<int> empty_pole, int sample_time);
void generate_vehicle_result(ELECTRICVEHICLE ev, string table, int EV_amount, int time_mean, int time_variance, int soc_mean, int soc_variance, int wait_mean, int wait_variance, int start_number, vector<int> user, vector<float> capacity, vector<int> type_id, vector<int> empty_pole, int sample_time);
void fetch_vehicle_result(string result_table, string pole_table, string cdstatus_table, int EM_amount, int start_number, vector<int> empty_pole, int sample_time);
void enter_charging_pole(string pole_table, string cdstatus_table, int number, int wait, float SOC, float BAT_capacity, int start_timeblock, int departure_timeblock, int pole_id);
void insert_vehicle_result(string table, int Pole_ID, int number, int type, float BAT_CAP, int start_timeblock, float start_SOC, int original_departure_timeblock, int wait);
#endif
