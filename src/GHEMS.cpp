#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
#include "SQLFunction.hpp"
// use function 'find_variableName_position' needs
#include "scheduling_parameter.hpp"
#include "GHEMS_function.hpp"
#include <string>
#include <vector>
#include <algorithm>

// #define FC_START_POWER 0.35 // Pfc start power
using namespace std;

vector<string> variable_name;
// common parameter
int time_block = 0, variable = 0, divide = 0, sample_time = 0, point_num = 0, piecewise_num;
float delta_T = 0.0;
float Cbat = 0.0, Vsys = 0.0, SOC_ini = 0.0, SOC_min = 0.0, SOC_max = 0.0, SOC_thres = 0.0, Pbat_min = 0.0, Pbat_max = 0.0, Pgrid_max = 0.0, Psell_max = 0.0, Delta_battery = 0.0, Pfc_max = 0.0;
// dr
int dr_mode, dr_startTime, dr_endTime, dr_minDecrease_power, dr_feedback_price, dr_customer_baseLine;
// flag
bool Pgrid_flag, mu_grid_flag, Psell_flag, Pess_flag, Pfc_flag, SOC_change_flag;
vector<float> Pgrid_max_array;

int main(int argc, const char **argv)
{

	time_t t = time(NULL);
	struct tm now_time = *localtime(&t);
	int real_time = 0;
	
	PUBLICLOAD pl;
	ELECTRICMOTOR em;
	// ELECTRICVEHICLE ev;

	if (!connect_mysql("DHEMS_fiftyHousehold"))
		messagePrint(__LINE__, "Failed to Connect MySQL");
	else
		messagePrint(__LINE__, "Success to Connect MySQL");

	// =-=-=-=-=-=-=- get parameter values from BaseParameter in need -=-=-=-=-=-=-= //
	vector<float> parameter_tmp;
	parameter_tmp.push_back(value_receive("BaseParameter", "parameter_name", "time_block", 'F'));
	parameter_tmp.push_back(value_receive("BaseParameter", "parameter_name", "householdAmount", 'F'));
	for (int i = 9; i <= 18; i++)
		parameter_tmp.push_back(value_receive("BaseParameter", "parameter_id", i, 'F'));
	parameter_tmp.push_back(value_receive("BaseParameter", "parameter_name", "Global_real_time"));

	// =-=-=-=-=-=-=- we suppose that enerage appliance in community should same as the single appliance times household amount -=-=-=-=-=-=-= //
	time_block = parameter_tmp[0];
	Vsys = parameter_tmp[2] * parameter_tmp[1];
	Cbat = parameter_tmp[3];
	SOC_min = parameter_tmp[4];
	SOC_max = parameter_tmp[5];
	SOC_thres = parameter_tmp[6];
	Pbat_min = parameter_tmp[7] * parameter_tmp[1];
	Pbat_max = parameter_tmp[8] * parameter_tmp[1];
	Pgrid_max = parameter_tmp[9] * parameter_tmp[1];
	Psell_max = parameter_tmp[10] * parameter_tmp[1];
	Pfc_max = parameter_tmp[11] * parameter_tmp[1];
	real_time = (int)parameter_tmp[12];

	divide = (time_block / 24);
	delta_T = 1.0 / (float)divide;
	point_num = 6;
	piecewise_num = point_num - 1;

	// =-=-=-=-=-=-=- get demand response -=-=-=-=-=-=-= //
	dr_mode = value_receive("BaseParameter", "parameter_name", "dr_mode");
	messagePrint(__LINE__, "dr mode: ", 'I', dr_mode);
	if (dr_mode != 0)
	{
		int *dr_info = demand_response_info(dr_mode);
		dr_startTime = dr_info[0];
		dr_endTime = dr_info[1];
		dr_minDecrease_power = dr_info[2];
		dr_feedback_price = dr_info[3];
		dr_customer_baseLine = dr_info[4];
	}

	// Choose resource be use in GHEMS
	pl.flag = flag_receive("GHEMS_flag", pl.str_publicLoad);
	Pgrid_flag = flag_receive("GHEMS_flag", "Pgrid");
	mu_grid_flag = flag_receive("GHEMS_flag", "mu_grid");
	Psell_flag = flag_receive("GHEMS_flag", "Psell");
	Pess_flag = flag_receive("GHEMS_flag", "Pess");
	Pfc_flag = flag_receive("GHEMS_flag", "Pfc");
	SOC_change_flag = flag_receive("GHEMS_flag", "SOC_change");
	
	// =-=-=-=-=-=-=- get parameter values from EM_parameter in need -=-=-=-=-=-=-= //
	// NOTE: 2022/01/03 Discuss with professor comfirm not using fast/super fast charging users, so not fully complete all the process
	em.flag = value_receive("BaseParameter", "parameter_name", "ElectricMotor");
	if (em.flag)
	{
		em.total_charging_pole = value_receive("EM_Parameter", "parameter_name", "Total_Charging_Pole");
		em.normal_charging_pole = value_receive("EM_Parameter", "parameter_name", "Normal_Charging_Pole");
		em.fast_charging_pole = value_receive("EM_Parameter", "parameter_name", "Fast_Charging_Pole");
		em.super_fast_charging_pole = value_receive("EM_Parameter", "parameter_name", "Super_Fast_Charging_Pole");
		em.normal_charging_power = value_receive("EM_Parameter", "parameter_name", "Normal_Charging_power", 'F');
		em.MAX_SOC = value_receive("EM_Parameter", "parameter_name", "EM_Upper_SOC", 'F');
		em.threshold_SOC = value_receive("EM_Parameter", "parameter_name", "EM_threshold_SOC", 'F');
		em.MIN_SOC = value_receive("EM_Parameter", "parameter_name", "EM_Lower_SOC", 'F');
		em.can_discharge = value_receive("EM_Parameter", "parameter_name", "Motor_can_discharge");
		em.generate_result_flag = value_receive("BaseParameter", "parameter_name", "EM_generate_random_user_result");
	}

	// determine realtime mode should after getting above related parameter
	sample_time = value_receive("BaseParameter", "parameter_name", "Global_next_simulate_timeblock");
	// =-=-=-=-=-=-=- return 1 after determine mode and get SOC -=-=-=-=-=-=-= //
	real_time = determine_realTimeOrOneDayMode_andGetSOC(em, real_time, variable_name);
	if ((sample_time + 1) == 97)
	{
		messagePrint(__LINE__, "Time block to the end !!");
		exit(0);
	}
	sample_time = value_receive("BaseParameter", "parameter_name", "Global_next_simulate_timeblock");
	messagePrint(__LINE__, "sample time from database = ", 'I', sample_time);

	// =-=-=-=-=-=-=- create EM users -=-=-=-=-=-=-= //
	if (em.flag)
	{
		// return how many motors can charge (means flag 'sure' = 1)
		em.can_charge_amount = enter_newEMInfo_inPole(em, sample_time);
	}

	if (pl.flag == 1)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE group_id = 5");
		pl.number = turn_value_to_int(0);
		for (int i = 0; i < pl.number; i++)
			variable_name.push_back(pl.str_publicLoad + to_string(i + 1));
	}
	if (Pgrid_flag == 1)
		variable_name.push_back("Pgrid");
	if (mu_grid_flag == 1)
		variable_name.push_back("mu_grid");
	if (Psell_flag == 1)
		variable_name.push_back("Psell");
	if (Pess_flag == 1)
	{
		variable_name.push_back("Pess");
		variable_name.push_back("Pcharge");
		variable_name.push_back("Pdischarge");
		variable_name.push_back("SOC");
		variable_name.push_back("Z");
		if (SOC_change_flag)
		{
			variable_name.push_back("SOC_change");
			variable_name.push_back("SOC_increase");
			variable_name.push_back("SOC_decrease");
			variable_name.push_back("SOC_Z");
		}
	}
	if (Pfc_flag == 1)
	{
		variable_name.push_back("Pfc");
		variable_name.push_back("Pfct");
		variable_name.push_back("PfcON");
		variable_name.push_back("PfcOFF");
		variable_name.push_back("muFC");
		for (int i = 0; i < piecewise_num; i++)
			variable_name.push_back("zPfc" + to_string(i + 1));
		for (int i = 0; i < piecewise_num; i++)
			variable_name.push_back("lambda_Pfc" + to_string(i + 1));
	}
	if (em.flag)
	{
		for (int i = 0; i < em.can_charge_amount; i++)
			variable_name.push_back(em.str_charging + to_string(i + 1));
		if (em.can_discharge)
		{
			for (int i = 0; i < em.can_charge_amount; i++)
				variable_name.push_back(em.str_discharging + to_string(i + 1));
			for (int i = 0; i < em.can_charge_amount; i++)
				variable_name.push_back(em.str_mu + to_string(i + 1));
		}
	}
	variable = variable_name.size();

	// =-=-=-=-=-=-=- get electric price data -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price' ");
	string simulate_price = turn_value_to_string(0);
	float *price = get_allDay_price(simulate_price);

	// =-=-=-=-=-=-=- get households' loads consumption from table 'totalLoad_model' & uncontrollable load from table 'LHEMS_uncontrollable_load' -=-=-=-=-=-=-= //
	bool uncontrollable_load_flag = value_receive("BaseParameter", "parameter_name", "uncontrollable_load_flag");
	float *load_model = get_totalLoad_power(uncontrollable_load_flag);

	if (sample_time == 0)
		insert_GHEMS_variable();

	// =-=-=-=-=-=-=- get total weighting from dr_alpha -=-=-=-=-=-=-= //
	if (dr_mode != 0)
	{
		for (int i = 0; i < time_block - sample_time; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(A%d) FROM `LHEMS_control_status` WHERE equip_name = 'dr_alpha' ", i + sample_time);
			float dr_weighting_sumOfAlpha = turn_value_to_float(0);
			Pgrid_max_array.push_back(Pgrid_max / parameter_tmp[1] * dr_weighting_sumOfAlpha);
		}
	}

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = '%d-%02d-%02d' WHERE parameter_name = 'lastTime_execute' ", now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday);
	sent_query();

	optimization(pl, em, variable_name, Pgrid_max_array, load_model, price);
	calculateCostInfo(pl, price, Pgrid_flag, Psell_flag, Pess_flag, Pfc_flag);
	updateSingleHouseholdCost();
	
	if (em.flag)
	{
		update_fullSOC_or_overtime_EM_inPole(em, sample_time);
	}
	
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET value = (SELECT A%d FROM GHEMS_control_status where equip_name = 'SOC') WHERE parameter_name = 'now_SOC'", sample_time);
	sent_query();

	printf("LINE %d: sample_time = %d\n", __LINE__, sample_time);
	printf("LINE %d: next sample_time = %d\n\n", __LINE__, sample_time + 1);

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = '%d' WHERE  parameter_name = 'Global_next_simulate_timeblock' ", sample_time + 1);
	sent_query();

	if (sample_time == 95)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET `value` = '0' WHERE `parameter_name` = 'EM_generate_random_user_result'");
		sent_query();
	}

	mysql_close(mysql_con);
	return 0;
}