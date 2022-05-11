#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //162 165 76~86 579~581�Ȯɵ��ѱ�
#include <glpk.h> //435~438�h�gsame_day
#include <math.h>
#include <mysql.h>
#include <iostream> //504~513 sample_time ��b�iGLPK�e�A�o�˹L�@�Ѥ~���|�n���s��
#include "SQLFunction.hpp"
#include "LHEMS_function.hpp"
// use function 'find_variableName_position' needs
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

#define distributed_group_num 8

int main(void)
{
	BASEPARAMETER bp;
	ENERGYSTORAGESYSTEM ess;
	DEMANDRESPONSE dr;
	COMFORTLEVEL comlv;
	INTERRUPTLOAD irl;
    UNINTERRUPTLOAD uirl;
	VARYINGLOAD varl;
	UNCONTROLLABLELOAD ucl;

	if (!connect_mysql("DHEMS_fiftyHousehold"))
		messagePrint(__LINE__, "Failed to Connect MySQL");
	else
		messagePrint(__LINE__, "Success to Connect MySQL");

	// =-=-=-=-=-=-=- get BaseParameter values -=-=-=-=-=-=-= //
	comlv.flag = value_receive("BaseParameter", "parameter_name", "comfortLevel_flag");
	bp.time_block = value_receive("BaseParameter", "parameter_name", "time_block");
	bp.sample_time = get_distributed_group("next_simulate_timeblock", "group_id", distributed_group_num);
	// householdTotal / distributed_householdTotal	總用戶數 / 各組總用戶
	// household_id	/ distributed_household_id		當前實際用戶 / 各組當前用戶
	bp.householdTotal = value_receive("BaseParameter", "parameter_name", "householdAmount");
	bp.distributed_householdTotal = bp.householdTotal / value_receive("BaseParameter", "parameter_name", "householdDistributed");
	bp.Pgrid_max = value_receive("BaseParameter", "parameter_name", "Pgridmax", 'F');
	bp.distributed_household_id = get_distributed_group("household_id", "group_id", distributed_group_num);
	bp.household_id = (distributed_group_num - 1) * bp.distributed_householdTotal + bp.distributed_household_id;
	bp.divide = (bp.time_block / 24);
	bp.delta_T = 1.0 / float(bp.divide);
	ess.capacity = value_receive("BaseParameter", "parameter_name", "Cbat", 'F');
	ess.battery_rate = value_receive("BaseParameter", "parameter_name", "battery_rate", 'F');
	ess.MIN_SOC = value_receive("BaseParameter", "parameter_name", "SOCmin", 'F');
	ess.MAX_SOC = value_receive("BaseParameter", "parameter_name", "SOCmax", 'F');
	ess.threshold_SOC = value_receive("BaseParameter", "parameter_name", "SOCthres", 'F');
	ess.MIN_power = ess.capacity * ess.battery_rate;
	ess.MAX_power = ess.capacity * ess.battery_rate;

	// =-=-=-=-=-=-=- determine real_time is 0 or 1 -=-=-=-=-=-=-= //
	int real_time = value_receive("BaseParameter", "parameter_name", "real_time");
	real_time = determine_realTimeOrOneDayMode_andGetSOC(bp, ess, real_time, distributed_group_num);
	if ((bp.sample_time + 1) == 97)
	{
		messagePrint(__LINE__, "Time block to the end !!");
		exit(0);
	}
	bp.remain_timeblock = bp.time_block - bp.sample_time;
	messagePrint(__LINE__, "sample time from database = ", 'I', bp.sample_time);

	
	// =-=-=-=-=-=-=- get HEMS flag -=-=-=-=-=-=-= //
	irl.flag = flag_receive("LHEMS_flag", irl.str_interrupt);
	uirl.flag = flag_receive("LHEMS_flag", uirl.str_uninterrupt);
	varl.flag = flag_receive("LHEMS_flag", varl.str_varying);
	bp.Pgrid_flag = flag_receive("LHEMS_flag", bp.str_Pgrid);
	ess.flag = flag_receive("LHEMS_flag", ess.str_Pess);
	ucl.flag = value_receive("BaseParameter", "parameter_name", "uncontrollable_load_flag");
	if (ucl.flag)
	{
		ucl.generate_flag = value_receive("BaseParameter", "parameter_name", "generate_uncontrollable_load_flag");
	}

	// =-=-=-=-=-=-=- get load_list loads category's amount -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(household%d) FROM load_list_select WHERE group_id = %d", bp.household_id, irl.group_id);
	irl.number = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(household%d) FROM load_list_select WHERE group_id = %d", bp.household_id, uirl.group_id);
	uirl.number = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(household%d) FROM load_list_select WHERE group_id = %d", bp.household_id, varl.group_id);
	varl.number = turn_value_to_int(0);
	bp.app_count = irl.number + uirl.number + varl.number;
	// =-=-=-=-=-=-=- get each hosueholds' loads info -=-=-=-=-=-=-= //
	getLoads_startEndOperationTime_and_power(irl, bp);
	getLoads_startEndOperationTime_and_power(uirl, bp);
	getLoads_startEndOperationTime_and_power(varl, bp);

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
	
	// =-=-=-=-=-=-=- Define variable name and use in GLPK -=-=-=-=-=-=-= //
	// Most important thing, helping in GLPK big matrix setting
	if (irl.flag == 1)
	{
		for (int i = 0; i < irl.number; i++)
			bp.variable_name.push_back(irl.str_interrupt + to_string(i + 1));
	}
	if (uirl.flag == 1)
	{
		for (int i = 0; i < uirl.number; i++)
			bp.variable_name.push_back(uirl.str_uninterrupt + to_string(i + 1));
	}
	if (varl.flag == 1)
	{
		for (int i = 0; i < varl.number; i++)
			bp.variable_name.push_back(varl.str_varying + to_string(i + 1));
	}
	if (bp.Pgrid_flag == 1)
		bp.variable_name.push_back(bp.str_Pgrid);
	if (ess.flag == 1)
	{
		bp.variable_name.push_back(ess.str_Pess);
		bp.variable_name.push_back(ess.str_Pcharge);
		bp.variable_name.push_back(ess.str_Pdischarge);
		bp.variable_name.push_back(ess.str_SOC);
		bp.variable_name.push_back(ess.str_Z);
	}
	if (uirl.flag == 1)
	{
		for (int i = 0; i < uirl.number; i++)
			bp.variable_name.push_back(uirl.str_uninterDelta + to_string(i + 1));
	}
	if (varl.flag == 1)
	{
		for (int i = 0; i < varl.number; i++)
			bp.variable_name.push_back(varl.str_varyingDelta + to_string(i + 1));
		for (int i = 0; i < varl.number; i++)
			bp.variable_name.push_back(varl.str_varyingPsi + to_string(i + 1));
	}
	bp.variable = bp.variable_name.size();

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET value = %d WHERE `BaseParameter`.`parameter_name` = 'local_variable_num' ", bp.variable);
	sent_query();

	// =-=-=-=-=-=-=- get electric price data -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price' ");
	string simulate_price = turn_value_to_string(0);
	bp.price = get_allDay_price(bp.time_block, simulate_price);
	
	HEMS_UCload_rand_operationTime(bp, ucl, distributed_group_num);
	init_totalLoad_flag_and_table(bp, distributed_group_num);

	optimization(bp, ess, dr, comlv, irl, uirl, varl, ucl.power_array, distributed_group_num);

	update_loadModel(bp, irl, uirl, varl, distributed_group_num);
	calculateCostInfo(bp);

	messagePrint(__LINE__, "sample_time = ", 'I', bp.sample_time);
	if (bp.distributed_household_id == bp.distributed_householdTotal)
	{
		messagePrint(__LINE__, "next sample_time = ", 'I', bp.sample_time + 1);
		update_distributed_group("next_simulate_timeblock", bp.sample_time + 1, "group_id", distributed_group_num);
		if (get_distributed_group("SUM(next_simulate_timeblock) / COUNT(group_id)") == bp.sample_time + 1)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET value = %d WHERE `parameter_name` = 'next_simulate_timeblock' ", bp.sample_time + 1);
			sent_query();
		}
	}

	if (bp.distributed_household_id < bp.distributed_householdTotal)
		bp.distributed_household_id++;
	else
		bp.distributed_household_id = 1;

	messagePrint(__LINE__, "next distributed household_id = ", 'I', bp.distributed_household_id);
	messagePrint(__LINE__, "next real household_id = ", 'I', (distributed_group_num - 1) * bp.distributed_householdTotal + bp.distributed_household_id);
	update_distributed_group("household_id", bp.distributed_household_id, "group_id", distributed_group_num);

	if (bp.sample_time == 95)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET `value` = '0' WHERE `parameter_name` = 'generate_uncontrollable_load_flag'");
		sent_query();
	}
	mysql_close(mysql_con);
	return 0;
}