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

vector<string> variable_name;
#define distributed_group_num 9
// base parameter
int time_block = 0, variable = 0, divide = 0, sample_time = 0, distributed_householdTotal = 0, interrupt_num, uninterrupt_num, varying_num, app_count, distributed_household_id, household_id, householdTotal;
bool Pgrid_flag, Pfc_flag, interruptLoad_flag, uninterruptLoad_flag, varyingLoad_flag;
float delta_T = 0.0;
float Pgrid_max = 0.0, Psell_max;

int main(void)
{
	ENERGYSTORAGESYSTEM ess;
	DEMANDRESPONSE dr;
	COMFORTLEVEL comlv;
	if (!connect_mysql("DHEMS_fiftyHousehold"))
		messagePrint(__LINE__, "Failed to Connect MySQL");
	else
		messagePrint(__LINE__, "Success to Connect MySQL");

	// =-=-=-=-=-=-=- get BaseParameter values -=-=-=-=-=-=-= //	
	comlv.flag = value_receive("BaseParameter", "parameter_name", "comfortLevel_flag");
	time_block = value_receive("BaseParameter", "parameter_name", "time_block");
	// householdTotal / distributed_householdTotal	總用戶數 / 各組總用戶
	// household_id	/ distributed_household_id		當前實際用戶 / 各組當前用戶
	householdTotal = value_receive("BaseParameter", "parameter_name", "householdAmount");
	distributed_householdTotal = householdTotal / value_receive("BaseParameter", "parameter_name", "householdDistributed");
	ess.capacity = value_receive("BaseParameter", "parameter_name", "Cbat", 'F');
	ess.battery_rate = value_receive("BaseParameter", "parameter_name", "battery_rate", 'F');
	ess.MIN_SOC = value_receive("BaseParameter", "parameter_name", "SOCmin", 'F');
	ess.MAX_SOC = value_receive("BaseParameter", "parameter_name", "SOCmax", 'F');
	ess.threshold_SOC = value_receive("BaseParameter", "parameter_name", "SOCthres", 'F');
	ess.MIN_power = ess.capacity * ess.battery_rate;
	ess.MAX_power = ess.capacity * ess.battery_rate;
	divide = (time_block / 24);
	delta_T = 1.0 / (float)divide;

	Pgrid_max = value_receive("BaseParameter", "parameter_name", "Pgridmax", 'F');
	distributed_household_id = get_distributed_group("household_id", "group_id", distributed_group_num);
	household_id = (distributed_group_num - 1) * distributed_householdTotal + distributed_household_id;

	// =-=-=-=-=-=-=- determine which mode and get SOC if in need -=-=-=-=-=-=-= //
	int real_time = value_receive("BaseParameter", "parameter_name", "real_time");
	sample_time = get_distributed_group("next_simulate_timeblock", "group_id", distributed_group_num);

	// =-=-=-=-=-=-=- get load_list loads category's amount -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM load_list WHERE group_id = 1");
	interrupt_num = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM load_list WHERE group_id = 2");
	uninterrupt_num = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM load_list WHERE group_id = 3");
	varying_num = turn_value_to_int(0);
	app_count = interrupt_num + uninterrupt_num + varying_num;

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
	interruptLoad_flag = flag_receive("LHEMS_flag", "interrupt");
	uninterruptLoad_flag = flag_receive("LHEMS_flag", "uninterrupt");
	varyingLoad_flag = flag_receive("LHEMS_flag", "varying");
	Pgrid_flag = flag_receive("LHEMS_flag", "Pgrid");
	ess.flag = flag_receive("LHEMS_flag", ess.str_Pess);
	
	real_time = determine_realTimeOrOneDayMode_andGetSOC(ess, real_time, variable_name, distributed_group_num);
	if ((sample_time + 1) == 97)
	{
		messagePrint(__LINE__, "Time block to the end !!");
		exit(0);
	}

	messagePrint(__LINE__, "sample time from database = ", 'I', sample_time);
	// =-=-=-=-=-=-=- Define variable name and use in GLPK -=-=-=-=-=-=-= //
	// Most important thing, helping in GLPK big matrix setting
	if (interruptLoad_flag == 1)
	{
		for (int i = 0; i < interrupt_num; i++)
			variable_name.push_back("interrupt" + to_string(i + 1));
	}
	if (uninterruptLoad_flag == 1)
	{
		for (int i = 0; i < uninterrupt_num; i++)
			variable_name.push_back("uninterrupt" + to_string(i + 1));
	}
	if (varyingLoad_flag == 1)
	{
		for (int i = 0; i < varying_num; i++)
			variable_name.push_back("varying" + to_string(i + 1));
	}
	if (Pgrid_flag == 1)
		variable_name.push_back("Pgrid");
	if (ess.flag == 1)
	{
		variable_name.push_back(ess.str_Pess);
		variable_name.push_back(ess.str_Pcharge);
		variable_name.push_back(ess.str_Pdischarge);
		variable_name.push_back(ess.str_SOC);
		variable_name.push_back(ess.str_Z);
	}
	if (dr.mode != 0)
		variable_name.push_back(dr.str_alpha);
	if (uninterruptLoad_flag == 1)
	{
		for (int i = 0; i < uninterrupt_num; i++)
			variable_name.push_back("uninterDelta" + to_string(i + 1));
	}
	if (varyingLoad_flag == 1)
	{
		for (int i = 0; i < varying_num; i++)
			variable_name.push_back("varyingDelta" + to_string(i + 1));
		for (int i = 0; i < varying_num; i++)
			variable_name.push_back("varyingPsi" + to_string(i + 1));
	}
	variable = variable_name.size();

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET value = %d WHERE `BaseParameter`.`parameter_name` = 'local_variable_num' ", variable);
	sent_query();

	// =-=-=-=-=-=-=- get electric price data -=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price' ");
	string simulate_price = turn_value_to_string(0);
	float *price = new float[time_block];
	for (int i = 0; i < time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM price WHERE price_period = %d", simulate_price.c_str(), i);
		price[i] = turn_value_to_float(0);
	}

	float *uncontrollable_load = rand_operationTime(distributed_group_num);
	init_totalLoad_flag_and_table(distributed_group_num);

	// =-=-=-=-=-=-=- get each hosueholds' loads info -=-=-=-=-=-=-= //
	int *interrupt_start = new int[interrupt_num];
	int *interrupt_end = new int[interrupt_num];
	int *interrupt_ot = new int[interrupt_num];
	int *interrupt_reot = new int[interrupt_num];
	float *interrupt_p = new float[interrupt_num];

	int *uninterrupt_start = new int[uninterrupt_num];
	int *uninterrupt_end = new int[uninterrupt_num];
	int *uninterrupt_ot = new int[uninterrupt_num];
	int *uninterrupt_reot = new int[uninterrupt_num];
	float *uninterrupt_p = new float[uninterrupt_num];
	bool *uninterrupt_flag = new bool[uninterrupt_num];

	int *varying_start = new int[varying_num];
	int *varying_end = new int[varying_num];
	int *varying_ot = new int[varying_num];
	int *varying_reot = new int[varying_num];
	int **varying_t_pow = NEW2D(varying_num, 3, int);
	float **varying_p_pow = NEW2D(varying_num, 3, float);
	bool *varying_flag = new bool[varying_num];

	char *s_time = new char[3];
	char *token = strtok(s_time, "-");
	vector<int> time_tmp;
	for (int i = 0; i < interrupt_num; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d_startEndOperationTime FROM load_list WHERE group_id = 1 and number = %d", household_id, i + 1);
		char *seo_time = turn_value_to_string(0);
		token = strtok(seo_time, "~");
		while (token != NULL)
		{
			time_tmp.push_back(atoi(token));
			token = strtok(NULL, "~");
		}
		interrupt_start[i] = (int)time_tmp[0];
		interrupt_end[i] = (int)time_tmp[1] - 1;
		interrupt_ot[i] = (int)time_tmp[2];
		interrupt_reot[i] = 0;
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = 1 and number = %d", i + 1);
		interrupt_p[i] = turn_value_to_float(0);
		time_tmp.clear();
	}
	for (int i = 0; i < uninterrupt_num; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d_startEndOperationTime FROM load_list WHERE group_id = 2 and number = %d", household_id, i + 1 + interrupt_num);
		char *seo_time = turn_value_to_string(0);
		token = strtok(seo_time, "~");
		while (token != NULL)
		{
			time_tmp.push_back(atoi(token));
			token = strtok(NULL, "~");
		}
		uninterrupt_start[i] = (int)time_tmp[0];
		uninterrupt_end[i] = (int)time_tmp[1] - 1;
		uninterrupt_ot[i] = (int)time_tmp[2];
		uninterrupt_reot[i] = 0;
		uninterrupt_flag[i] = 0;
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = 2 and number = %d", i + 1 + interrupt_num);
		uninterrupt_p[i] = turn_value_to_float(0);
		time_tmp.clear();
	}
	for (int i = 0; i < varying_num; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d_startEndOperationTime FROM load_list WHERE group_id = 3 and number = %d", household_id, i + 1 + interrupt_num + uninterrupt_num);
		char *seo_time = turn_value_to_string(0);
		token = strtok(seo_time, "~");
		while (token != NULL)
		{
			time_tmp.push_back(atoi(token));
			token = strtok(NULL, "~");
		}
		varying_start[i] = (int)time_tmp[0];
		varying_end[i] = (int)time_tmp[1] - 1;
		varying_ot[i] = (int)time_tmp[2];
		varying_reot[i] = 0;
		varying_flag[i] = 0;
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1, power2, power3, block1, block2, block3 FROM load_list WHERE group_id = 3 and number = %d", i + 1 + interrupt_num + uninterrupt_num);
		fetch_row_value();
		for (int z = 0; z < 3; z++)
			varying_p_pow[i][z] = turn_float(z);
		for (int z = 0; z < 3; z++)
			varying_t_pow[i][z] = turn_int(z + 3);
		time_tmp.clear();
	}

	optimization(ess, dr, comlv, variable_name, household_id, interrupt_start, interrupt_end, interrupt_ot, interrupt_reot, interrupt_p, uninterrupt_start, uninterrupt_end, uninterrupt_ot, uninterrupt_reot, uninterrupt_p, uninterrupt_flag, varying_start, varying_end, varying_ot, varying_reot, varying_flag, varying_t_pow, varying_p_pow, app_count, price, uncontrollable_load, distributed_group_num);

	update_loadModel(interrupt_p, uninterrupt_p, household_id, distributed_group_num);
	calculateCostInfo(price);

	printf("LINE %d: sample_time = %d\n", __LINE__, sample_time);
	if (distributed_household_id == distributed_householdTotal)
	{
		printf("LINE %d: next sample_time = %d\n\n", __LINE__, sample_time + 1);
		update_distributed_group("next_simulate_timeblock", sample_time + 1, "group_id", distributed_group_num);
		if (get_distributed_group("SUM(next_simulate_timeblock) / COUNT(group_id)") == sample_time + 1)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `BaseParameter` SET value = %d WHERE `parameter_name` = 'next_simulate_timeblock' ", sample_time + 1);
			sent_query();
		}
	}

	if (distributed_household_id < distributed_householdTotal)
		distributed_household_id++;
	else
		distributed_household_id = 1;

	printf("LINE %d: next distributed household_id = %d\n", __LINE__, distributed_household_id);
	printf("LINE %d: next real household_id = %d\n", __LINE__, (distributed_group_num - 1) * distributed_householdTotal + distributed_household_id);
	update_distributed_group("household_id", distributed_household_id, "group_id", distributed_group_num);

	mysql_close(mysql_con);
	return 0;
}