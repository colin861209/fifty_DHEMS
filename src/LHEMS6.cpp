#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //162 165 76~86 579~581�Ȯɵ��ѱ�
#include <glpk.h> //435~438�h�gsame_day
#include <math.h>
#include <mysql.h>
#include <iostream> //504~513 sample_time ��b�iGLPK�e�A�o�˹L�@�Ѥ~���|�n���s��
#include "SQLFunction.hpp"
#include "fifty_LHEMS_function.hpp"
// use function 'find_variableName_position' needs
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

#define distributed_group_num 6

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

	// =-=-=-=-=-=-=- determine which mode and get SOC if in need -=-=-=-=-=-=-= //
	int real_time = value_receive("BaseParameter", "parameter_name", "real_time");
	real_time = determine_realTimeOrOneDayMode_andGetSOC(bp, ess, real_time, distributed_group_num);
	if ((bp.sample_time + 1) == 97)
	{
		messagePrint(__LINE__, "Time block to the end !!");
		exit(0);
	}
	bp.remain_timeblock = bp.time_block - bp.sample_time;
	messagePrint(__LINE__, "sample time from database = ", 'I', bp.sample_time);

	// =-=-=-=-=-=-=- get load_list loads category's amount -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM load_list WHERE group_id = 1");
	irl.number = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM load_list WHERE group_id = 2");
	uirl.number = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM load_list WHERE group_id = 3");
	varl.number = turn_value_to_int(0);
	bp.app_count = irl.number + uirl.number + varl.number;

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
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT MAX(household%d) FROM `LHEMS_demand_response_CBL` WHERE `time_block` BETWEEN %d AND %d AND `comfort_level_flag` = %d", bp.household_id, dr.startTime, dr.endTime - 1, comlv.flag);
		dr.household_CBL = turn_value_to_float(0);
	}
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

	// =-=-=-=-=-=-=- get each hosueholds' loads info -=-=-=-=-=-=-= //
	irl.start = new int[irl.number];
	irl.end = new int[irl.number];
	irl.ot = new int[irl.number];
	irl.reot = new int[irl.number];
	irl.power = new float[irl.number];

	uirl.start = new int[uirl.number];
	uirl.end = new int[uirl.number];
	uirl.ot = new int[uirl.number];
	uirl.reot = new int[uirl.number];
	uirl.power = new float[uirl.number];
	uirl.continuous_flag = new bool[uirl.number];

	varl.start = new int[varl.number];
	varl.end = new int[varl.number];
	varl.ot = new int[varl.number];
	varl.reot = new int[varl.number];
	varl.block_tmp = NEW2D(varl.number, 3, int);
	varl.power_tmp = NEW2D(varl.number, 3, float);
	varl.continuous_flag = new bool[varl.number];

	char *s_time = new char[3];
	char *token = strtok(s_time, "-");
	vector<int> time_tmp;
	for (int i = 0; i < irl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d_startEndOperationTime FROM load_list WHERE group_id = 1 and number = %d", bp.household_id, i + 1);
		char *seo_time = turn_value_to_string(0);
		token = strtok(seo_time, "~");
		while (token != NULL)
		{
			time_tmp.push_back(atoi(token));
			token = strtok(NULL, "~");
		}
		irl.start[i] = time_tmp[0];
		irl.end[i] = time_tmp[1] - 1;
		irl.ot[i] = time_tmp[2];
		irl.reot[i] = 0;
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = 1 and number = %d", i + 1);
		irl.power[i] = turn_value_to_float(0);
		time_tmp.clear();
	}
	for (int i = 0; i < uirl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d_startEndOperationTime FROM load_list WHERE group_id = 2 and number = %d", bp.household_id, i + 1 + irl.number);
		char *seo_time = turn_value_to_string(0);
		token = strtok(seo_time, "~");
		while (token != NULL)
		{
			time_tmp.push_back(atoi(token));
			token = strtok(NULL, "~");
		}
		uirl.start[i] = time_tmp[0];
		uirl.end[i] = time_tmp[1] - 1;
		uirl.ot[i] = time_tmp[2];
		uirl.reot[i] = 0;
		uirl.continuous_flag[i] = 0;
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = 2 and number = %d", i + 1 + irl.number);
		uirl.power[i] = turn_value_to_float(0);
		time_tmp.clear();
	}
	for (int i = 0; i < varl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d_startEndOperationTime FROM load_list WHERE group_id = 3 and number = %d", bp.household_id, i + 1 + irl.number + uirl.number);
		char *seo_time = turn_value_to_string(0);
		token = strtok(seo_time, "~");
		while (token != NULL)
		{
			time_tmp.push_back(atoi(token));
			token = strtok(NULL, "~");
		}
		varl.start[i] = time_tmp[0];
		varl.end[i] = time_tmp[1] - 1;
		varl.ot[i] = time_tmp[2];
		varl.reot[i] = 0;
		varl.continuous_flag[i] = 0;
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1, power2, power3, block1, block2, block3 FROM load_list WHERE group_id = 3 and number = %d", i + 1 + irl.number + uirl.number);
		fetch_row_value();
		for (int z = 0; z < 3; z++)
			varl.power_tmp[i][z] = turn_float(z);
		for (int z = 0; z < 3; z++)
			varl.block_tmp[i][z] = turn_int(z + 3);
		time_tmp.clear();
	}

	optimization(bp, ess, dr, comlv, irl, uirl, varl, ucl.power_array, distributed_group_num);

	update_loadModel(bp, irl, uirl, varl, distributed_group_num);
	calculateCostInfo(bp);

	printf("LINE %d: sample_time = %d\n", __LINE__, bp.sample_time);
	if (bp.distributed_household_id == bp.distributed_householdTotal)
	{
		printf("LINE %d: next sample_time = %d\n\n", __LINE__, bp.sample_time + 1);
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

	printf("LINE %d: next distributed household_id = %d\n", __LINE__, bp.distributed_household_id);
	printf("LINE %d: next real household_id = %d\n", __LINE__, (distributed_group_num - 1) * bp.distributed_householdTotal + bp.distributed_household_id);
	update_distributed_group("household_id", bp.distributed_household_id, "group_id", distributed_group_num);

	if (bp.sample_time == 95)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET `value` = '0' WHERE `parameter_name` = 'generate_uncontrollable_load_flag'");
		sent_query();
	}
	mysql_close(mysql_con);
	return 0;
}