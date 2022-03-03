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

int main(int argc, const char **argv)
{

	time_t t = time(NULL);
	struct tm now_time = *localtime(&t);
	
	BASEPARAMETER bp;
	ENERGYSTORAGESYSTEM ess;
	PUBLICLOAD pl;
	DEMANDRESPONSE dr;
	ELECTRICMOTOR em;
	ELECTRICVEHICLE ev;

	if (!connect_mysql("DHEMS_fiftyHousehold"))
		messagePrint(__LINE__, "Failed to Connect MySQL");
	else
		messagePrint(__LINE__, "Success to Connect MySQL");

	// =-=-=-=-=-=-=- get parameter values from BaseParameter in need -=-=-=-=-=-=-= //
	// we suppose that enerage appliance in community should same as the single appliance times household amount
	int householdAmount = value_receive("BaseParameter", "parameter_name", "householdAmount");
	int real_time = value_receive("BaseParameter", "parameter_name", "Global_real_time");
	bp.time_block = value_receive("BaseParameter", "parameter_name", "time_block");
	bp.sample_time = value_receive("BaseParameter", "parameter_name", "Global_next_simulate_timeblock");
	bp.Pgrid_max = value_receive("BaseParameter", "parameter_name", "Pgridmax", 'F') * householdAmount;
	bp.Psell_max = value_receive("BaseParameter", "parameter_name", "Psellmax", 'F') * householdAmount;
	bp.Pfc_max = value_receive("BaseParameter", "parameter_name", "Pfcmax", 'F') * householdAmount;
	bp.divide = bp.time_block / 24;
	bp.delta_T = 1.0 / float(bp.divide);
	bp.point_num = 6;
	bp.piecewise_num = bp.point_num - 1;
	ess.capacity = value_receive("BaseParameter", "parameter_name", "Cbat", 'F') * householdAmount;
	ess.battery_rate = value_receive("BaseParameter", "parameter_name", "battery_rate", 'F');
	ess.MIN_SOC = value_receive("BaseParameter", "parameter_name", "SOCmin", 'F');
	ess.MAX_SOC = value_receive("BaseParameter", "parameter_name", "SOCmax", 'F');
	ess.threshold_SOC = value_receive("BaseParameter", "parameter_name", "SOCthres", 'F');
	ess.MIN_power = ess.capacity * ess.battery_rate;
	ess.MAX_power = ess.capacity * ess.battery_rate;

	// =-=-=-=-=-=-=- get demand response -=-=-=-=-=-=-= //
	dr.mode = value_receive("BaseParameter", "parameter_name", "dr_mode");
	messagePrint(__LINE__, "dr mode: ", 'I', dr.mode);
	if (dr.mode != 0)
	{
		int *dr_info = demand_response_info(dr.mode);
		dr.startTime = dr_info[0];
		dr.endTime = dr_info[1];
		dr.minDecrease_power = dr_info[2];
		dr.feedback_price = dr_info[3];
		dr.customer_baseLine = dr_info[4];
	}

	// Choose resource be use in GHEMS
	pl.flag = flag_receive("GHEMS_flag", pl.str_publicLoad);
	bp.Pgrid_flag = flag_receive("GHEMS_flag", bp.str_Pgrid);
	bp.mu_grid_flag = flag_receive("GHEMS_flag", bp.str_mu_grid);
	bp.Psell_flag = flag_receive("GHEMS_flag", bp.str_Psell);
	ess.flag = flag_receive("GHEMS_flag", ess.str_Pess);
	bp.Pfc_flag = flag_receive("GHEMS_flag", bp.str_Pfc);
	bp.SOC_change_flag = flag_receive("GHEMS_flag", bp.str_SOC_change);
	
	// =-=-=-=-=-=-=- get parameter values from EM_parameter in need -=-=-=-=-=-=-= //
	// NOTE: 2022/01/03 Discuss with professor comfirm not using fast/super fast charging users, so not fully complete all the process
	// 2022/01/27 DELETE fast/super fast info
	em.flag = value_receive("BaseParameter", "parameter_name", "ElectricMotor");
	if (em.flag)
	{
		em.total_charging_pole = value_receive("EM_Parameter", "parameter_name", "Total_Charging_Pole");
		em.normal_charging_pole = value_receive("EM_Parameter", "parameter_name", "Normal_Charging_Pole");
		em.normal_charging_power = value_receive("EM_Parameter", "parameter_name", "Normal_Charging_power", 'F');
		em.MAX_SOC = value_receive("EM_Parameter", "parameter_name", "EM_Upper_SOC", 'F');
		em.threshold_SOC = value_receive("EM_Parameter", "parameter_name", "EM_threshold_SOC", 'F');
		em.MIN_SOC = value_receive("EM_Parameter", "parameter_name", "EM_Lower_SOC", 'F');
		em.can_discharge = value_receive("EM_Parameter", "parameter_name", "Motor_can_discharge");
		em.generate_result_flag = value_receive("BaseParameter", "parameter_name", "EM_generate_random_user_result");
	}

	ev.flag = value_receive("BaseParameter", "parameter_name", "ElectricVehicle");
	if (ev.flag)
	{
		ev.total_charging_pole = value_receive("EV_Parameter", "parameter_name", "Total_Charging_Pole");
		ev.charging_power = value_receive("EV_Parameter", "parameter_name", "Charging_power", 'F');
		ev.MAX_SOC = value_receive("EV_Parameter", "parameter_name", "EM_Upper_SOC", 'F');
		ev.threshold_SOC = value_receive("EV_Parameter", "parameter_name", "EM_threshold_SOC", 'F');
		ev.MIN_SOC = value_receive("EV_Parameter", "parameter_name", "EM_Lower_SOC", 'F');
		ev.can_discharge = value_receive("EV_Parameter", "parameter_name", "Vehicle_can_discharge");
		ev.generate_result_flag = value_receive("BaseParameter", "parameter_name", "EV_generate_random_user_result");
	}
	// determine realtime mode should after getting above related parameter
	// =-=-=-=-=-=-=- return 1 after determine mode and get SOC -=-=-=-=-=-=-= //
	real_time = determine_realTimeOrOneDayMode_andGetSOC(bp, ess, em, ev, real_time);
	if ((bp.sample_time + 1) == 97)
	{
		messagePrint(__LINE__, "Time block to the end !!");
		exit(0);
	}
	bp.remain_timeblock = bp.time_block - bp.sample_time;
	messagePrint(__LINE__, "sample time from database = ", 'I', bp.sample_time);

	// =-=-=-=-=-=-=- create EM users -=-=-=-=-=-=-= //
	if (em.flag)
	{
		// return how many motors can charge (means flag 'sure' = 1)
		em.can_charge_amount = enter_newEMInfo_inPole(em, bp.sample_time);
	}
	// =-=-=-=-=-=-=- create EV users -=-=-=-=-=-=-= //
	if (ev.flag)
	{
		// return how many cars can charge (means flag 'sure' = 1)
		ev.can_charge_amount = enter_newEVInfo_inPole(ev, bp.sample_time);
	}

	if (pl.flag == 1)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE group_id = '5'");
		pl.forceToStop_number = turn_value_to_int(0);
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE group_id = '6' ");
		pl.interrupt_number = turn_value_to_int(0);
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE group_id = '7' ");
		pl.periodic_number = turn_value_to_int(0);
		for (int i = 0; i < pl.forceToStop_number; i++)
			bp.variable_name.push_back(pl.str_forceToStop_publicLoad + to_string(i + 1));
		for (int i = 0; i < pl.interrupt_number; i++)
			bp.variable_name.push_back(pl.str_interrupt_publicLoad + to_string(i + 1));
		for (int i = 0; i < pl.periodic_number; i++)
			bp.variable_name.push_back(pl.str_periodic_publicLoad + to_string(i + 1));
	}
	if (bp.Pgrid_flag == 1)
		bp.variable_name.push_back(bp.str_Pgrid);
	if (bp.mu_grid_flag == 1)
		bp.variable_name.push_back(bp.str_mu_grid);
	if (bp.Psell_flag == 1)
		bp.variable_name.push_back(bp.str_Psell);
	if (ess.flag == 1)
	{
		bp.variable_name.push_back(ess.str_Pess);
		bp.variable_name.push_back(ess.str_Pcharge);
		bp.variable_name.push_back(ess.str_Pdischarge);
		bp.variable_name.push_back(ess.str_SOC);
		bp.variable_name.push_back(ess.str_Z);
		if (bp.SOC_change_flag)
		{
			bp.variable_name.push_back(bp.str_SOC_change);
			bp.variable_name.push_back(bp.str_SOC_increase);
			bp.variable_name.push_back(bp.str_SOC_decrease);
			bp.variable_name.push_back(bp.str_SOC_Z);
		}
	}
	if (bp.Pfc_flag == 1)
	{
		bp.variable_name.push_back(bp.str_Pfc);
		bp.variable_name.push_back(bp.str_Pfct);
		bp.variable_name.push_back(bp.str_PfcON);
		bp.variable_name.push_back(bp.str_PfcOFF);
		bp.variable_name.push_back(bp.str_muFC);
		for (int i = 0; i < bp.piecewise_num; i++)
			bp.variable_name.push_back(bp.str_zPfc + to_string(i + 1));
		for (int i = 0; i < bp.piecewise_num; i++)
			bp.variable_name.push_back(bp.str_lambda_Pfc + to_string(i + 1));
	}
	if (em.flag)
	{
		for (int i = 0; i < em.can_charge_amount; i++)
			bp.variable_name.push_back(em.str_charging + to_string(i + 1));
		if (em.can_discharge)
		{
			for (int i = 0; i < em.can_charge_amount; i++)
				bp.variable_name.push_back(em.str_discharging + to_string(i + 1));
			for (int i = 0; i < em.can_charge_amount; i++)
				bp.variable_name.push_back(em.str_mu + to_string(i + 1));
		}
	}
	if (ev.flag)
	{
		for (int i = 0; i < ev.can_charge_amount; i++)
			bp.variable_name.push_back(ev.str_charging + to_string(i + 1));
		if (ev.can_discharge)
		{
			for (int i = 0; i < ev.can_charge_amount; i++)
				bp.variable_name.push_back(ev.str_discharging + to_string(i + 1));
			for (int i = 0; i < ev.can_charge_amount; i++)
				bp.variable_name.push_back(ev.str_mu + to_string(i + 1));
		}
	}
	bp.variable = bp.variable_name.size();

	// =-=-=-=-=-=-=- get electric price data -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price' ");
	string simulate_price = turn_value_to_string(0);
	bp.price = get_allDay_price(bp.time_block, simulate_price);
	
	// =-=-=-=-=-=-=- choose column 'big_sunny' 'sunny' 'cloudy' in table solar_data -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_weather' ");
	string weather = turn_value_to_string(0);
	bp.solar = getOrUpdate_SolarInfo_ThroughSampleTime(bp, weather.c_str());

	// =-=-=-=-=-=-=- get households' loads consumption from table 'totalLoad_model' & uncontrollable load from table 'LHEMS_uncontrollable_load' -=-=-=-=-=-=-= //
	bool uncontrollable_load_flag = value_receive("BaseParameter", "parameter_name", "uncontrollable_load_flag");
	bp.load_model = get_totalLoad_power(bp.time_block, uncontrollable_load_flag);

	if (bp.sample_time == 0)
		insert_GHEMS_variable(bp, ess);

	// =-=-=-=-=-=-=- get total weighting from dr_alpha -=-=-=-=-=-=-= //
	if (dr.mode != 0)
	{
		bp.Pgrid_max_array.assign(bp.remain_timeblock, bp.Pgrid_max);
		for (int i = dr.startTime - bp.sample_time; i < dr.endTime -bp.sample_time; i++)
		{
			if (i >= 0)
				bp.Pgrid_max_array[i] = dr.customer_baseLine;
		}
	}

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = '%d-%02d-%02d' WHERE parameter_name = 'lastTime_execute' ", now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday);
	sent_query();

	optimization(bp, ess, dr, pl, em, ev);
	calculateCostInfo(bp, dr, pl);
	updateSingleHouseholdCost(bp, dr);
	
	if (em.flag && em.can_charge_amount)
	{
		update_fullSOC_or_overtime_EM_inPole(em, bp.sample_time);
	}
	if (ev.flag && ev.can_charge_amount)
	{
		update_fullSOC_or_overtime_EV_inPole(ev, bp.sample_time);
	}
	
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET value = (SELECT A%d FROM GHEMS_control_status where equip_name = '%s') WHERE parameter_name = 'now_SOC'", bp.sample_time, ess.str_SOC.c_str());
	sent_query();

	printf("LINE %d: sample_time = %d\n", __LINE__, bp.sample_time);
	printf("LINE %d: next sample_time = %d\n\n", __LINE__, bp.sample_time + 1);

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = '%d' WHERE parameter_name = 'Global_next_simulate_timeblock' ", bp.sample_time + 1);
	sent_query();

	if (bp.sample_time == 95)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET `value` = '0' WHERE `parameter_name` = 'EM_generate_random_user_result'");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET `value` = '0' WHERE `parameter_name` = 'EV_generate_random_user_result'");
		sent_query();
	}

	mysql_close(mysql_con);
	return 0;
}