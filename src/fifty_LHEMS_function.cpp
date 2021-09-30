#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql/mysql.h>
#include <iostream>
#include <numeric>
#include "SQLFunction.hpp"
#include "fifty_LHEMS_function.hpp"
#include "LHEMS_constraint.hpp"
#include "scheduling_parameter.hpp"

int coef_row_num = 0, bnd_row_num = 1;
char column[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";

void optimization(vector<string> variable_name, int household_id, int *interrupt_start, int *interrupt_end, int *interrupt_ot, int *interrupt_reot, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_ot, int *uninterrupt_reot, float *uninterrupt_p, bool *uninterrupt_flag, int *varying_start, int *varying_end, int *varying_ot, int *varying_reot, bool *varying_flag, int **varying_t_pow, float **varying_p_pow, int app_count, float *price, float *uncontrollable_load, int distributed_group_num)
{
	functionPrint(__func__);

	float **comfortLevelWeighting;
	if (comfortLevel_flag)
	{
		int comfortLevel = 4;
		int total_timeInterval = 3;
		vector<vector<vector<int>>> comfortLevel_startEnd;
		// [0~3] = level, [][0~1] = start or end, [][][0~44] = 3 times * 15 appliances
		for (int i = 0; i < comfortLevel; i++)
		{
			comfortLevel_startEnd.push_back(get_comfortLevel_timeInterval(household_id, app_count, total_timeInterval, i + 1));
		}
		comfortLevelWeighting = calculate_comfortLevel_weighting(comfortLevel_startEnd, comfortLevel, total_timeInterval, app_count);
	}

	countUninterruptAndVaryingLoads_Flag(uninterrupt_flag, varying_flag, household_id);

	int *buff = new int[app_count];
	for (int i = 0; i < app_count; i++)
		buff[i] = 0;
	countLoads_AlreadyOpenedTimes(buff, household_id);
	count_interruptLoads_RemainOperateTime(interrupt_num, interrupt_ot, interrupt_reot, buff);
	count_uninterruptAndVaryingLoads_RemainOperateTime(2, uninterrupt_num, uninterrupt_ot, uninterrupt_reot, uninterrupt_end, uninterrupt_flag, buff);
	count_uninterruptAndVaryingLoads_RemainOperateTime(3, varying_num, varying_ot, varying_reot, varying_end, varying_flag, buff);

	int **varying_t_d = NEW2D(varying_num, (time_block - sample_time), int);
	// varying_p_d will get error if two varying load have different operation time
	float **varying_p_d = NEW2D(varying_num, varying_ot[0], float);
	float *varying_p_max = new float[varying_num];
	init_VaryingLoads_OperateTimeAndPower(varying_t_d, varying_p_d, varying_ot);
	putValues_VaryingLoads_OperateTimeAndPower(varying_t_d, varying_p_d, varying_t_pow, varying_p_pow, varying_start, varying_end, varying_p_max);

	// float *weighting_array;
	int *participate_array;
	if (dr_mode != 0)
	{
		// weighting_array = household_alpha_upperBnds(distributed_group_num);
		participate_array = household_participation(household_id, "LHEMS_demand_response_participation");
	}

	int rowTotal = (time_block - sample_time) * 200 + 1;
	int colTotal = variable * (time_block - sample_time);

	string prob_name = "LHEMS" + to_string(household_id);
	glp_prob *mip;
	mip = glp_create_prob();
	glp_set_prob_name(mip, prob_name.c_str());
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, rowTotal);
	glp_add_cols(mip, colTotal);

	setting_LHEMS_columnBoundary(variable_name, mip, varying_p_max);

	float **coefficient = NEW2D(rowTotal, colTotal, float);
	for (int m = 0; m < rowTotal; m++)
	{
		for (int n = 0; n < colTotal; n++)
			coefficient[m][n] = 0.0;
	}

	if (interruptLoad_flag)
	{
		summation_interruptLoadRa_biggerThan_Qa(interrupt_start, interrupt_end, interrupt_reot, coefficient, mip, interrupt_num);
	}

	if (dr_mode != 0)
	{
		// 0 < Pgrid j < αu j *Pgrid max
		pgrid_smallerThan_alphaPgridMax(coefficient, mip, time_block - sample_time);
		// (1 - Du j) < alpha < 1 in operate time, else alpha is 1
		alpha_between_oneminusDu_and_one(participate_array, coefficient, mip, time_block - sample_time);
	}

	// (Balanced function) Pgrid j - Pess j = sum(Pa j) + Puc j
	pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(interrupt_start, interrupt_end, interrupt_p, uninterrupt_start, uninterrupt_end, uninterrupt_p, varying_start, varying_end, uncontrollable_load, coefficient, mip, time_block - sample_time);

	if (Pess_flag)
	{
		// SOC j - 1 + sum((Pess * Ts) / (Cess * Vess)) >= SOC threshold, only one constranit formula
		previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(coefficient, mip, 1);
		// SOC j = SOC j - 1 + (Pess j * Ts) / (Cess * Vess)
		previousSOCPlusPessTransToSOC_equalTo_currentSOC(coefficient, mip, time_block - sample_time);
		// (Charge limit) Pess + <= z * Pcharge max
		pessPositive_smallerThan_zMultiplyByPchargeMax(coefficient, mip, time_block - sample_time);
		// (Discharge limit) Pess - <= (1 - z) * Pdischarge max
		pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(coefficient, mip, time_block - sample_time);
		// (Battery power) (Pess +) - (Pess -) = Pess j
		pessPositiveMinusPessNegative_equalTo_Pess(coefficient, mip, time_block - sample_time);
	}

	if (uninterruptLoad_flag)
	{
		// sum(δa j) = 1 (uninterrupt loads)
		summation_uninterruptDelta_equalTo_one(uninterrupt_start, uninterrupt_end, uninterrupt_reot, uninterrupt_flag, coefficient, mip, 1);
		// ra j+n >= δa j (uninterrupt loads)
		uninterruptRajToN_biggerThan_uninterruptDelta(uninterrupt_start, uninterrupt_end, uninterrupt_reot, uninterrupt_flag, coefficient, mip, time_block - sample_time);
	}

	if (varyingLoad_flag)
	{
		// sum(δa j) = 1 (varying loads)
		summation_varyingDelta_equalTo_one(varying_start, varying_end, varying_reot, varying_flag, coefficient, mip, 1);
		// ra j+n >= δa j (varying loads)
		varyingRajToN_biggerThan_varyingDelta(varying_start, varying_end, varying_reot, varying_flag, coefficient, mip, time_block - sample_time);
		// ψa j+n  >= δa j * σa n
		varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(varying_start, varying_end, varying_reot, varying_flag, varying_t_d, varying_p_d, buff, coefficient, mip, time_block - sample_time);
	}

	setting_LHEMS_objectiveFunction(price, participate_array, comfortLevelWeighting, mip);

	int *ia = new int[rowTotal * colTotal + 1];
	int *ja = new int[rowTotal * colTotal + 1];
	double *ar = new double[rowTotal * colTotal + 1];
	for (int i = 0; i < rowTotal; i++)
	{
		for (int j = 0; j < colTotal; j++)
		{
			ia[i * ((time_block - sample_time) * variable) + j + 1] = i + 1;
			ja[i * ((time_block - sample_time) * variable) + j + 1] = j + 1;
			ar[i * ((time_block - sample_time) * variable) + j + 1] = coefficient[i][j];
		}
	}
	glp_load_matrix(mip, rowTotal * colTotal, ia, ja, ar);

	glp_iocp parm;
	glp_init_iocp(&parm);

	if (sample_time == 0)
		parm.tm_lim = 120000;
	else
		parm.tm_lim = 60000;

	parm.presolve = GLP_ON;
	parm.msg_lev = GLP_MSG_ERR;
	//not cloudy
	// parm.ps_heur = GLP_ON;
	// parm.bt_tech = GLP_BT_BPH;
	// parm.br_tech = GLP_BR_PCH;

	//cloud
	parm.gmi_cuts = GLP_ON;
	// parm.ps_heur = GLP_ON;
	parm.bt_tech = GLP_BT_BFS;
	parm.br_tech = GLP_BR_PCH;

	//no fc+ no sell
	//fc+no sell
	// parm.gmi_cuts = GLP_ON;
	// parm.bt_tech = GLP_BT_BPH;
	// parm.br_tech = GLP_BR_PCH;

	//FC+sell
	//parm.fp_heur = GLP_ON;
	// parm.bt_tech = GLP_BT_BPH;
	//parm.br_tech = GLP_BR_PCH;

	int err = glp_intopt(mip, &parm);

	double z = glp_mip_obj_val(mip);
	printf("\n");
	printf("LINE %d: timeblock %d household id %d sol = %f; \n", __LINE__, sample_time, household_id, z);

	if (Pess_flag)
	{
		if (z == 0.0 && glp_mip_col_val(mip, find_variableName_position(variable_name, "SOC") + 1) == 0.0)
		{
			display_coefAndBnds_rowNum();
			printf("Error > sol is 0, No Solution, give up the solution\n");
			printf("%.2f\n", glp_mip_col_val(mip, find_variableName_position(variable_name, "Pgrid") + 1));
			return;
		}
	}
	else
	{
		if (z == 0.0)
		{
			display_coefAndBnds_rowNum();
			printf("Error > sol is 0, No Solution, give up the solution\n");
			printf("%.2f\n", glp_mip_col_val(mip, find_variableName_position(variable_name, "Pgrid") + 1));
			return;
		}
	}

	float *s = new float[time_block];

	for (int i = 1; i <= variable; i++)
	{
		int h = i;
		int l = variable - (app_count - i); // get interrupt & varying ra j
		if (sample_time == 0)
		{
			for (int j = 0; j < time_block; j++)
			{
				s[j] = glp_mip_col_val(mip, h);
				if ((i > interrupt_num + uninterrupt_num) && (i <= app_count)) //sometimes varying load will have weird, use power model instead of varying load
				{
					s[j] = glp_mip_col_val(mip, l);
					if (s[j] == varying_p_pow[0][0] || s[j] == varying_p_pow[0][1] || s[j] == varying_p_pow[0][2])
						s[j] = 1.0;
				}
				h = (h + variable);
				l = (l + variable);
			}
			insert_status_into_MySQLTable("LHEMS_control_status", column, s, "equip_name", variable_name[i - 1], "household_id", household_id);
		}
		else
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, variable_name[i - 1].c_str(), household_id);
			fetch_row_value();
			for (int k = 0; k < sample_time; k++)
			{
				s[k] = turn_float(k);
			}

			for (int j = 0; j < (time_block - sample_time); j++)
			{
				s[j + sample_time] = glp_mip_col_val(mip, h);
				if ((i > interrupt_num + uninterrupt_num) && (i <= app_count)) //sometimes varying load will have weird, use power model instead of varying load
				{
					s[j + sample_time] = glp_mip_col_val(mip, l);
					if (s[j + sample_time] == varying_p_pow[0][0] || s[j + sample_time] == varying_p_pow[0][1] || s[j + sample_time] == varying_p_pow[0][2])
						s[j + sample_time] = 1.0;
				}
				h = (h + variable);
				l = (l + variable);
			}
			update_status_to_MySQLTable("LHEMS_control_status", s, "equip_name", variable_name[i - 1], "AND", "household_id", household_id);

			for (int j = 0; j < sample_time; j++)
			{
				s[j] = 0;
			}
			insert_status_into_MySQLTable("LHEMS_real_status", column, s, "equip_name", variable_name[i - 1], "household_id", household_id);
		}
	}

	glp_delete_prob(mip);
	delete[] ia, ja, ar, s;
	delete[] coefficient;
	return;
}

void setting_LHEMS_columnBoundary(vector<string> variable_name, glp_prob *mip, float *varying_p_max)
{
	functionPrint(__func__);
	messagePrint(__LINE__, "Setting columns...", 'S', 0, 'Y');

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		if (interruptLoad_flag)
		{
			for (int j = 1; j <= interrupt_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(variable_name, "interrupt" + to_string(j)) + 1 + i * variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(variable_name, "interrupt" + to_string(j)) + 1 + i * variable), GLP_BV);
			}
		}
		if (uninterruptLoad_flag)
		{
			for (int j = 1; j <= uninterrupt_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(variable_name, "uninterrupt" + to_string(j)) + 1 + i * variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(variable_name, "uninterrupt" + to_string(j)) + 1 + i * variable), GLP_BV);
			}
		}
		if (varyingLoad_flag)
		{
			for (int j = 1; j <= varying_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(variable_name, "varying" + to_string(j)) + 1 + i * variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(variable_name, "varying" + to_string(j)) + 1 + i * variable), GLP_BV);
			}
		}
		if (Pgrid_flag)
		{
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable), GLP_DB, 0.0, Pgrid_max);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable), GLP_CV);
		}
		if (Pess_flag)
		{
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pess") + 1 + i * variable), GLP_DB, -Pbat_min, Pbat_max);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pess") + 1 + i * variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pcharge") + 1 + i * variable), GLP_FR, 0.0, Pbat_max);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pcharge") + 1 + i * variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pdischarge") + 1 + i * variable), GLP_FR, 0.0, Pbat_min);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pdischarge") + 1 + i * variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC") + 1 + i * variable), GLP_DB, SOC_min, SOC_max);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC") + 1 + i * variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Z") + 1 + i * variable), GLP_DB, 0.0, 1.0);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "Z") + 1 + i * variable), GLP_BV);
		}
		if (dr_mode != 0)
		{
			glp_set_col_bnds(mip, (find_variableName_position(variable_name, "dr_alpha") + 1 + i * variable), GLP_DB, 0.0, 1.0);
			glp_set_col_kind(mip, (find_variableName_position(variable_name, "dr_alpha") + 1 + i * variable), GLP_CV);
		}
		if (uninterruptLoad_flag)
		{
			for (int j = 1; j <= uninterrupt_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(variable_name, "uninterDelta" + to_string(j)) + 1 + i * variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(variable_name, "uninterDelta" + to_string(j)) + 1 + i * variable), GLP_BV);
			}
		}
		if (varyingLoad_flag)
		{
			for (int j = 1; j <= varying_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(variable_name, "varyingDelta" + to_string(j)) + 1 + i * variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(variable_name, "varyingDelta" + to_string(j)) + 1 + i * variable), GLP_BV);
			}
			for (int j = 1; j <= varying_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(variable_name, "varyingPsi" + to_string(j)) + 1 + i * variable), GLP_DB, 0.0, varying_p_max[j - 1]);
				glp_set_col_kind(mip, (find_variableName_position(variable_name, "varyingPsi" + to_string(j)) + 1 + i * variable), GLP_CV);
			}
		}
	}
}

int determine_realTimeOrOneDayMode_andGetSOC(int real_time, vector<string> variable_name, int distributed_group_num)
{
	// 'Realtime mode' if same day & real time = 1;
	// 'One day mode' =>
	// 		1. SOC = 0.7 if real_time = 0,
	// 		2. Use Previous SOC if real_time = 1.
	functionPrint(__func__);
	if (real_time == 1)
	{
		messagePrint(__LINE__, "Real Time Mode...", 'S', 0, 'Y');

		if (truncate_table_flag())
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE LHEMS_real_status");
			sent_query();
		}

		// get previous SOC value
		if (Pess_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", sample_time - 1, "SOC", household_id);
			SOC_ini = turn_value_to_float(0);
			messagePrint(__LINE__, "SOC = ", 'F', SOC_ini, 'Y');
		}
	}
	else
	{
		messagePrint(__LINE__, "First Time Mode...", 'S', 0, 'Y');

		if (truncate_table_flag())
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE LHEMS_control_status");
			sent_query();
			snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE LHEMS_real_status");
			sent_query();
			snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE LHEMS_cost");
			sent_query();
			update_distributed_group("real_time", 0, "group_id", distributed_group_num);
			sample_time = 0;
			update_distributed_group("next_simulate_timeblock", sample_time, "group_id", distributed_group_num);
			messagePrint(__LINE__, "Truncate LHEMS control status: Group ", 'I', distributed_group_num, 'Y');
		}

		if (Pess_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC'");
			SOC_ini = turn_value_to_float(0);
			messagePrint(__LINE__, "ini_SOC : ", 'F', SOC_ini, 'Y');
		}

		if (distributed_household_id == distributed_householdTotal)
		{
			real_time = 1;
			update_distributed_group("real_time", real_time, "group_id", distributed_group_num);
			messagePrint(__LINE__, "real time = 1 in group id ", 'I', distributed_group_num, 'Y');
			if (get_distributed_group("COUNT(group_id) = SUM(real_time)"))
			{
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %d WHERE parameter_name = 'real_time' ", real_time);
				sent_query();
				messagePrint(__LINE__, "BaseParameter real time => ", 'I', real_time, 'Y');
			}
		}
	}

	return real_time;
}

void countUninterruptAndVaryingLoads_Flag(bool *uninterrupt_flag, bool *varying_flag, int household_id)
{
	printf("\nFunction: %s\n\t", __func__);
	int flag = 0;
	if (sample_time != 0)
	{
		for (int i = 0; i < uninterrupt_num; i++)
		{
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, ("uninterDelta" + to_string(i + 1)).c_str(), household_id);
			fetch_row_value();
			for (int j = 0; j < sample_time; j++)
			{
				flag += turn_int(j);
			}
			uninterrupt_flag[i] = flag;
		}
		for (int i = 0; i < varying_num; i++)
		{
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, ("varyingDelta" + to_string(i + 1)).c_str(), household_id);
			fetch_row_value();
			for (int j = 0; j < sample_time; j++)
			{
				flag += turn_int(j);
			}
			varying_flag[i] = flag;
		}
	}
	for (int i = 0; i < uninterrupt_num; i++)
		printf("LINE %d: uninterrupt_flag[%d] : %d\n\t", __LINE__, i, uninterrupt_flag[i]);
	for (int i = 0; i < varying_num; i++)
		printf("LINE %d: varying_flag[%d] : %d\n", __LINE__, i, varying_flag[i]);
}

void countLoads_AlreadyOpenedTimes(int *buff, int household_id)
{
	printf("\nFunction: %s\n\t", __func__);
	int coun = 0;
	if (sample_time != 0)
	{
		for (int i = 0; i < app_count; i++)
		{
			coun = 0;

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, variable_name[i].c_str(), household_id);
			fetch_row_value();
			for (int j = 0; j < sample_time; j++)
			{
				coun += turn_int(j);
			}
			buff[i] = coun;
		}
	}
	printf("LINE %d: ", __LINE__);
	for (int i = 0; i < app_count; i++)
		printf("buff[%d]: %d ", i, buff[i]);
	printf("\n");
}

void count_interruptLoads_RemainOperateTime(int interrupt_num, int *interrupt_ot, int *interrupt_reot, int *buff)
{
	printf("\nFunction: %s\n\t", __func__);
	for (int i = 0; i < interrupt_num; i++)
	{
		if ((interrupt_ot[i] - buff[i]) == interrupt_ot[i])
		{
			interrupt_reot[i] = interrupt_ot[i];
		}
		else if (((interrupt_ot[i] - buff[i]) < interrupt_ot[i]) && ((interrupt_ot[i] - buff[i]) > 0))
		{
			interrupt_reot[i] = interrupt_ot[i] - buff[i];
		}
		else if ((interrupt_ot[i] - buff[i]) <= 0)
		{
			interrupt_reot[i] = 0;
		}
		printf("LINE %d: load %d : reot = %d\n\t", __LINE__, i, interrupt_reot[i]);
	}
}

void count_uninterruptAndVaryingLoads_RemainOperateTime(int group_id, int loads_total, int *total_operateTime, int *remain_operateTime, int *end_time, bool *flag, int *buff)
{
	switch (group_id)
	{
	case 2:
		printf("\nFunction: %s group id : %d\n\t", __func__, group_id);
		for (int i = 0; i < uninterrupt_num; i++)
		{
			if (flag[i] == 0)
			{
				remain_operateTime[i] = total_operateTime[i];
			}
			if (flag[i] == 1)
			{
				if (((total_operateTime[i] - buff[i + interrupt_num]) < total_operateTime[i]) && ((total_operateTime[i] - buff[i + interrupt_num]) > 0))
				{
					remain_operateTime[i] = total_operateTime[i] - buff[i + interrupt_num];
					if (remain_operateTime[i] != 0)
					{
						end_time[i] = sample_time + remain_operateTime[i] - 1;
					}
				}
				else if ((total_operateTime[i] - buff[i + interrupt_num]) <= 0)
				{
					remain_operateTime[i] = 0;
				}
			}
			printf("LINE %d: uninterrupt load %d : reot = %d\n\t", __LINE__, i, remain_operateTime[i]);
		}
		break;
	case 3:
		printf("\nFunction: %s group id : %d\n\t", __func__, group_id);
		for (int i = 0; i < varying_num; i++)
		{
			if (flag[i] == 0)
			{
				remain_operateTime[i] = total_operateTime[i];
			}
			if (flag[i] == 1)
			{
				if (((total_operateTime[i] - buff[i + interrupt_num + uninterrupt_num]) < total_operateTime[i]) && ((total_operateTime[i] - buff[i + interrupt_num + uninterrupt_num]) > 0))
				{
					remain_operateTime[i] = total_operateTime[i] - buff[i + interrupt_num + uninterrupt_num];
					if (remain_operateTime[i] != 0)
					{
						end_time[i] = sample_time + remain_operateTime[i] - 1;
					}
				}
				else if ((total_operateTime[i] - buff[i + interrupt_num + uninterrupt_num]) <= 0)
				{
					remain_operateTime[i] = 0;
				}
			}
			printf("LINE %d: varying load %d : reot = %d\n", __LINE__, i, remain_operateTime[i]);
		}
		break;
	default:
		printf("\nFunction: %s no matching group id : %d\n\t", __func__, group_id);
		break;
	}
}

void init_VaryingLoads_OperateTimeAndPower(int **varying_t_d, float **varying_p_d, int *varying_ot)
{
	printf("\nFunction: %s \n\t", __func__);
	for (int i = 0; i < varying_num; i++)
	{
		for (int m = 0; m < (time_block - sample_time); m++)
		{
			varying_t_d[i][m] = 0;
		}
		for (int m = 0; m < varying_ot[i]; m++)
		{
			varying_p_d[i][m] = 0.0;
		}
	}
}

void putValues_VaryingLoads_OperateTimeAndPower(int **varying_t_d, float **varying_p_d, int **varying_t_pow, float **varying_p_pow, int *varying_start, int *varying_end, float *varying_p_max)
{
	printf("\nFunction: %s \n\t", __func__);

	for (int i = 0; i < varying_num; i++)
	{
		for (int j = 0; j < varying_t_pow[i][0]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][0];
		}
		for (int j = varying_t_pow[i][0]; j < varying_t_pow[i][0] + varying_t_pow[i][1]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][1];
		}
		for (int j = varying_t_pow[i][0] + varying_t_pow[i][1]; j < varying_t_pow[i][0] + varying_t_pow[i][1] + varying_t_pow[i][2]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][2];
		}
	}

	for (int i = 0; i < varying_num; i++)
	{
		if ((varying_end[i] - sample_time) >= 0)
		{
			if ((varying_start[i] - sample_time) >= 0)
			{
				for (int m = (varying_start[i] - sample_time); m <= (varying_end[i] - sample_time); m++)
				{
					varying_t_d[i][m] = 1;
				}
			}
			else if ((varying_start[i] - sample_time) < 0)
			{
				for (int m = 0; m <= (varying_end[i] - sample_time); m++)
				{
					varying_t_d[i][m] = 1;
				}
			}
		}
	}

	for (int i = 0; i < varying_num; i++)
	{
		varying_p_max[i] = 0.0;

		for (int j = 0; j < 3; j++)
		{
			if (varying_p_pow[i][j] > varying_p_max[i])
			{
				varying_p_max[i] = varying_p_pow[i][j];
			}
		}
	}
	// printf("LINE %d: Varying loads power model : ", __LINE__);
	// for (int i = 0; i < varying_num; i++)
	// {
	// 	for (int j = 0; j < varying_t_pow[i][0] + varying_t_pow[i][1] + varying_t_pow[i][2]; j++)
	// 		printf("%.2f ", varying_p_d[i][j]);
	// 	printf("\n\tLINE %d: Varying loads Max power = %.2f\n", __LINE__, varying_p_max[i]);
	// }
}

void update_loadModel(float *interrupt_p, float *uninterrupt_p, int household_id, int distributed_group_num)
{
	functionPrint(__func__);
	float *power_tmp = new float[time_block - sample_time];
	for (int i = 0; i < time_block - sample_time; i++)
		power_tmp[i] = 0.0;

	for (int i = 0; i < interrupt_num; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, ("interrupt" + to_string(i + 1)).c_str(), household_id);
		fetch_row_value();
		for (int j = sample_time; j < time_block; j++)
		{
			power_tmp[j - sample_time] += turn_float(j) * interrupt_p[i];
		}
	}
	for (int i = 0; i < uninterrupt_num; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, ("uninterrupt" + to_string(i + 1)).c_str(), household_id);
		fetch_row_value();
		for (int j = sample_time; j < time_block; j++)
		{
			power_tmp[j - sample_time] += turn_float(j) * uninterrupt_p[i];
		}
	}
	for (int i = 0; i < varying_num; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", column, ("varyingPsi" + to_string(i + 1)).c_str(), household_id);
		fetch_row_value();
		for (int j = sample_time; j < time_block; j++)
		{
			power_tmp[j - sample_time] += turn_float(j);
		}
	}
	for (int i = sample_time; i < time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `totalLoad_model` SET `household%d` = '%.3f' WHERE `totalLoad_model`.`time_block` = %d;", household_id, power_tmp[i - sample_time], i);
		sent_query();
	}
	// =-=-=-=-=-=-=- Caculate for total load model -=-=-=-=-=-=-= //
	if (distributed_household_id == distributed_householdTotal)
	{
		update_distributed_group("total_load_flag", 1, "group_id", distributed_group_num);
		if (get_distributed_group("COUNT(group_id) = SUM(total_load_flag)"))
		{
			for (int j = 0; j < time_block; j++)
			{
				float power_total = 0.0;
				for (int i = 1; i <= householdTotal; i++)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d FROM totalLoad_model WHERE time_block = %d", i, j);
					power_total += turn_value_to_float(0);
				}
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `totalLoad_model` SET `totalLoad` = '%.3f', `time` = CURRENT_TIMESTAMP WHERE `totalLoad_model`.`time_block` = %d;", power_total, j);
				sent_query();
			}
		}
	}
}

float *rand_operationTime(int distributed_group_num)
{
	functionPrint(__func__);
	float *result = new float[time_block];
	for (int i = 0; i < time_block; i++)
		result[i] = 0.0;

	if (value_receive("BaseParameter", "parameter_name", "uncontrollable_load_flag") == 0)
	{
		update_distributed_group("uncontrollable_load_flag", 0, "group_id", distributed_group_num);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_uncontrollable_load` SET `household%d` = '0.0' ", household_id);
		sent_query();
		if (distributed_household_id == distributed_householdTotal)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_uncontrollable_load` SET `totalLoad` = '0.0' ");
			sent_query();
		}
		return result;
	}

	srand(time(NULL));
	if (sample_time == 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE group_id = 4");
		int uncontrollableLoad_num = turn_value_to_int(0);
		for (int i = 0; i < uncontrollableLoad_num; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT uncontrollable_loads, power1 FROM load_list WHERE group_id = 4 LIMIT %d, %d", i, i + 1);
			fetch_row_value();
			char *seo_time = mysql_row[0];
			float power = atof(mysql_row[1]);
			char *tmp;
			tmp = strtok(seo_time, "~");
			vector<int> time_seperate;
			while (tmp != NULL)
			{
				time_seperate.push_back(atoi(tmp));
				tmp = strtok(NULL, "~");
			}

			int operate_count = 0;
			for (int i = time_seperate[0]; i < time_seperate[1] - 1; i++)
			{
				if (operate_count != time_seperate[2])
				{
					int operate_tmp = rand() % 2;
					float operate_power = operate_tmp * power;
					operate_count += operate_tmp;
					result[i] += operate_power;
				}
			}
			time_seperate.clear();
		}
		for (int i = 0; i < time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_uncontrollable_load` SET `household%d` = '%.1f' WHERE `time_block` = %d;", household_id, result[i], i);
			sent_query();
		}
		if (distributed_household_id == distributed_householdTotal)
		{
			update_distributed_group("uncontrollable_load_flag", 1, "group_id", distributed_group_num);
			if (get_distributed_group("COUNT(group_id) = SUM(uncontrollable_load_flag)"))
			{
				for (int j = 0; j < time_block; j++)
				{
					float power_total = 0.0;
					for (int i = 1; i <= householdTotal; i++)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "SELECT household%d FROM `LHEMS_uncontrollable_load` WHERE time_block = %d", i, j);
						power_total += turn_value_to_float(0);
					}
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_uncontrollable_load` SET `totalLoad` = '%.1f' WHERE `time_block` = %d;", power_total, j);
					sent_query();
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `household%d` FROM `LHEMS_uncontrollable_load` WHERE `time_block` = %d;", household_id, i);
			result[i] = turn_value_to_float(0);
		}
	}

	return result;
}

float *household_alpha_upperBnds(int distributed_group_num)
{
	functionPrint(__func__);

	float *result = new float[dr_endTime - dr_startTime];
	string sql_table = "LHEMS_history_control_status";

	if (sample_time == 0)
	{
		update_distributed_group("demand_response_alpha_flag", 0, "group_id", distributed_group_num);
		snprintf(sql_buffer, sizeof(sql_buffer), "DELETE FROM `demand_response_alpha` WHERE household_id = %d", household_id);
		sent_query();
		if (distributed_household_id == distributed_householdTotal)
			update_distributed_group("demand_response_alpha_flag", 1, "group_id", distributed_group_num);
	}

	// =-=-=-=-=-=-=- calculate weighting then turn to alpha -=-=-=-=-=-=-= //
	for (int i = dr_startTime; i < dr_endTime; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(A%d) FROM `%s` WHERE `equip_name` = 'Pgrid'", i, sql_table.c_str());
		float total_load = turn_value_to_float(0);
		// total load = 0 while all households' Pgrid = 0, when result[] = 0/0 will be nan
		if (total_load != 0)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(A%d) FROM `%s` WHERE `equip_name` = 'Pgrid' && `household_id` = %d", i, sql_table.c_str(), household_id);
			float each_household_load = turn_value_to_float(0);
			result[i - dr_startTime] = each_household_load / total_load;
		}
		else
		{
			result[i - dr_startTime] = 0.0;
		}

		printf("\thousehold %d timeblock %d weighting %.3f\n", household_id, i, result[i - dr_startTime]);
		result[i - dr_startTime] = (Pgrid_max - result[i - dr_startTime] * dr_minDecrease_power) / Pgrid_max;
	}
	if (sample_time != 0)
	{
		if (sample_time - dr_endTime < 0)
		{
			if (sample_time <= dr_startTime)
			{
				for (int i = 0; i < dr_endTime - dr_startTime; i++)
				{
					printf("\tUpdate household %d timeblock %d alpha %.3f\n", household_id, i + dr_startTime, result[i]);
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `demand_response_alpha` SET A%d = %.3f WHERE household_id = %d AND dr_timeblock = %d", sample_time, result[i], household_id, i + dr_startTime);
					sent_query();
				}
			}
			else if (sample_time > dr_startTime)
			{
				for (int i = 0; i < dr_endTime - sample_time; i++)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `demand_response_alpha` SET A%d = %.3f WHERE household_id = %d AND dr_timeblock = %d", sample_time, result[i + sample_time - dr_startTime], household_id, i + sample_time);
					sent_query();
					printf("\tUpdate household %d timeblock %d alpha %.3f\n", household_id, i + sample_time, result[i + sample_time - dr_startTime]);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < dr_endTime - dr_startTime; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO demand_response_alpha (A0, dr_timeblock, household_id) VALUES('%.3f', %d, '%d');", result[i], i + dr_startTime, household_id);
			sent_query();
			printf("\tInsert household %d timeblock %d alpha %.3f\n", household_id, i + dr_startTime, result[i]);
		}
	}

	return result;
}

int *household_participation(int household_id, string table)
{
	functionPrint(__func__);

	int *result = new int[dr_endTime - dr_startTime];

	// =-=-=-=-=-=-=- calculate weighting then turn to alpha -=-=-=-=-=-=-= //
	for (int i = dr_startTime; i < dr_endTime; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(A%d) FROM `%s` WHERE `household_id` = %d", i, table.c_str(), household_id);
		result[i - dr_startTime] = turn_value_to_int(0);

		printf("\thousehold %d timeblock %d status %d\n", household_id, i, result[i - dr_startTime]);
	}

	return result;
}

void init_totalLoad_flag_and_table(int distributed_group_num)
{
	// init totalLoad table
	if (distributed_household_id == 1)
	{
		update_distributed_group("total_load_flag", 0, "group_id", distributed_group_num);
		if (sample_time == 0)
		{
			for (int i = 1; i <= distributed_householdTotal; i++)
			{
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `totalLoad_model` SET `household%d` = '0' ", (distributed_group_num - 1) * distributed_householdTotal + i);
				sent_query();
			}
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `totalLoad_model` SET `totalLoad` = '0' ");
			sent_query();
		}
	}
}

int truncate_table_flag()
{
	// household id = 1 in every sample time,
	// otherwise if any group finish each household optimize, we don't truncate table
	int result = 0;
	result = get_distributed_group("COUNT(group_id) = SUM(household_id)");
	if (result)
		return result;
	else
		return 0;
}

int get_distributed_group(string target, string condition_col, int condition_num)
{
	if (condition_col.empty() && condition_num == -1)
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM `distributed_group` ", target.c_str());
	else
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM `distributed_group` WHERE %s = %d", target.c_str(), condition_col.c_str(), condition_num);

	return turn_value_to_int(0);
}

void update_distributed_group(string target, int target_value, string condition_col, int condition_num)
{
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `distributed_group` SET `%s` = '%d' WHERE `%s` = %d;", target.c_str(), target_value, condition_col.c_str(), condition_num);
	sent_query();
}

vector<vector<int>> get_comfortLevel_timeInterval(int household_id, int app_count, int total_timeInterval, int comfort_level)
{
	functionPrint(__func__);

	vector<vector<int>> comfortLevel_startEnd;
	comfortLevel_startEnd.push_back(vector<int>());
	comfortLevel_startEnd.push_back(vector<int>());
	for (int j = 0; j < app_count; j++)
	{
		for (int i = 0; i < total_timeInterval; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT level%d_startEndTime%d FROM LHEMS_comfort_level where household_id = %d AND appliances_num = %d", comfort_level, i + 1, household_id, j + 1);
			char *timeString = turn_value_to_string(0);
			if (atoi(timeString) != -999)
			{
				char *token;
				int i = 0;
				token = strtok(timeString, "~");
				while (token != NULL)
				{
					comfortLevel_startEnd[i].push_back(atoi(token));
					token = strtok(NULL, "~");
					i++;
				}
			}
			else
			{
				comfortLevel_startEnd[0].push_back(0);
				comfortLevel_startEnd[1].push_back(0);
			}
		}
	}
	return comfortLevel_startEnd;
}

float **calculate_comfortLevel_weighting(vector<vector<vector<int>>> comfortLevel_startEnd, int comfortLevel, int total_timeInterval, int app_count)
{
	functionPrint(__func__);

	// float weighting[app_count][time_block] = {0.0};
	float **weighting = new float *[app_count];
	for (int i = 0; i < app_count; i++)
		weighting[i] = new float[time_block];

	for (int i = 0; i < app_count; i++)
	{
		for (int j = 0; j < time_block; j++)
		{
			weighting[i][j]	= 0.0;
		}
	}
	
		
	for (int i = 0; i < app_count; i++)
	{
		for (int j = 0; j < time_block - sample_time; j++)
		{
			for (int k = 0; k < total_timeInterval; k++)
			{
				int level1_start_time = comfortLevel_startEnd[0][0][i * total_timeInterval + k];
				int level2_start_time = comfortLevel_startEnd[1][0][i * total_timeInterval + k];
				int level3_start_time = comfortLevel_startEnd[2][0][i * total_timeInterval + k];
				int level4_start_time = comfortLevel_startEnd[3][0][i * total_timeInterval + k];
				int level1_end_time = comfortLevel_startEnd[0][1][i * total_timeInterval + k];
				int level2_end_time = comfortLevel_startEnd[1][1][i * total_timeInterval + k];
				int level3_end_time = comfortLevel_startEnd[2][1][i * total_timeInterval + k];
				int level4_end_time = comfortLevel_startEnd[3][1][i * total_timeInterval + k];
				if (level1_start_time != level1_end_time && (j + sample_time) >= level1_start_time && (j + sample_time) < level1_end_time)
				{
					weighting[i][j + sample_time] += (j + sample_time - level1_start_time) / (level1_end_time - level1_start_time);
				}
				else if (level2_start_time != level2_end_time && (j + sample_time) >= level2_start_time && (j + sample_time) < level2_end_time)
				{
					weighting[i][j + sample_time] += (j + sample_time - level2_start_time) / (level2_end_time - level2_start_time) + 1;
				}
				else if (level3_start_time != level3_end_time && (j + sample_time) >= level3_start_time && (j + sample_time) < level3_end_time)
				{
					weighting[i][j + sample_time] += (j + sample_time - level3_start_time) / (level3_end_time - level3_start_time) + 2;
				}
				else if (level4_start_time != level4_end_time && (j + sample_time) >= level4_start_time && (j + sample_time) < level4_end_time)
				{
					weighting[i][j + sample_time] += (j + sample_time - level4_start_time) / (level4_end_time - level4_start_time) + 3;
				}
				else
				{
					weighting[i][j + sample_time] += 10;
				}
			}
		}
	}
	return weighting;
}

void calculateCostInfo(float* price)
{
	functionPrint(__func__);

	vector<float> gridPrice_tmp;
	for (int i = 0; i < time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `LHEMS_control_status` WHERE equip_name = 'Pgrid' AND household_id = '%d'", i, household_id);
		gridPrice_tmp.push_back(turn_value_to_float(0) * price[i] * delta_T);
	}
	float gridPrice_total = accumulate(gridPrice_tmp.begin(), gridPrice_tmp.end(), 0);

	if (sample_time == 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `LHEMS_cost` (household_id, origin_grid_price, %s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, household_id, gridPrice_total, gridPrice_tmp[0], gridPrice_tmp[1], gridPrice_tmp[2], gridPrice_tmp[3], gridPrice_tmp[4], gridPrice_tmp[5], gridPrice_tmp[6], gridPrice_tmp[7], gridPrice_tmp[8], gridPrice_tmp[9], gridPrice_tmp[10], gridPrice_tmp[11], gridPrice_tmp[12], gridPrice_tmp[13], gridPrice_tmp[14], gridPrice_tmp[15], gridPrice_tmp[16], gridPrice_tmp[17], gridPrice_tmp[18], gridPrice_tmp[19], gridPrice_tmp[20], gridPrice_tmp[21], gridPrice_tmp[22], gridPrice_tmp[23], gridPrice_tmp[24], gridPrice_tmp[25], gridPrice_tmp[26], gridPrice_tmp[27], gridPrice_tmp[28], gridPrice_tmp[29], gridPrice_tmp[30], gridPrice_tmp[31], gridPrice_tmp[32], gridPrice_tmp[33], gridPrice_tmp[34], gridPrice_tmp[35], gridPrice_tmp[36], gridPrice_tmp[37], gridPrice_tmp[38], gridPrice_tmp[39], gridPrice_tmp[40], gridPrice_tmp[41], gridPrice_tmp[42], gridPrice_tmp[43], gridPrice_tmp[44], gridPrice_tmp[45], gridPrice_tmp[46], gridPrice_tmp[47], gridPrice_tmp[48], gridPrice_tmp[49], gridPrice_tmp[50], gridPrice_tmp[51], gridPrice_tmp[52], gridPrice_tmp[53], gridPrice_tmp[54], gridPrice_tmp[55], gridPrice_tmp[56], gridPrice_tmp[57], gridPrice_tmp[58], gridPrice_tmp[59], gridPrice_tmp[60], gridPrice_tmp[61], gridPrice_tmp[62], gridPrice_tmp[63], gridPrice_tmp[64], gridPrice_tmp[65], gridPrice_tmp[66], gridPrice_tmp[67], gridPrice_tmp[68], gridPrice_tmp[69], gridPrice_tmp[70], gridPrice_tmp[71], gridPrice_tmp[72], gridPrice_tmp[73], gridPrice_tmp[74], gridPrice_tmp[75], gridPrice_tmp[76], gridPrice_tmp[77], gridPrice_tmp[78], gridPrice_tmp[79], gridPrice_tmp[80], gridPrice_tmp[81], gridPrice_tmp[82], gridPrice_tmp[83], gridPrice_tmp[84], gridPrice_tmp[85], gridPrice_tmp[86], gridPrice_tmp[87], gridPrice_tmp[88], gridPrice_tmp[89], gridPrice_tmp[90], gridPrice_tmp[91], gridPrice_tmp[92], gridPrice_tmp[93], gridPrice_tmp[94], gridPrice_tmp[95]);
		sent_query();
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_cost` set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `origin_grid_price` = ''%.3f, `datetime` = CURRENT_TIMESTAMP WHERE household_id = '%d';", gridPrice_tmp[0], gridPrice_tmp[1], gridPrice_tmp[2], gridPrice_tmp[3], gridPrice_tmp[4], gridPrice_tmp[5], gridPrice_tmp[6], gridPrice_tmp[7], gridPrice_tmp[8], gridPrice_tmp[9], gridPrice_tmp[10], gridPrice_tmp[11], gridPrice_tmp[12], gridPrice_tmp[13], gridPrice_tmp[14], gridPrice_tmp[15], gridPrice_tmp[16], gridPrice_tmp[17], gridPrice_tmp[18], gridPrice_tmp[19], gridPrice_tmp[20], gridPrice_tmp[21], gridPrice_tmp[22], gridPrice_tmp[23], gridPrice_tmp[24], gridPrice_tmp[25], gridPrice_tmp[26], gridPrice_tmp[27], gridPrice_tmp[28], gridPrice_tmp[29], gridPrice_tmp[30], gridPrice_tmp[31], gridPrice_tmp[32], gridPrice_tmp[33], gridPrice_tmp[34], gridPrice_tmp[35], gridPrice_tmp[36], gridPrice_tmp[37], gridPrice_tmp[38], gridPrice_tmp[39], gridPrice_tmp[40], gridPrice_tmp[41], gridPrice_tmp[42], gridPrice_tmp[43], gridPrice_tmp[44], gridPrice_tmp[45], gridPrice_tmp[46], gridPrice_tmp[47], gridPrice_tmp[48], gridPrice_tmp[49], gridPrice_tmp[50], gridPrice_tmp[51], gridPrice_tmp[52], gridPrice_tmp[53], gridPrice_tmp[54], gridPrice_tmp[55], gridPrice_tmp[56], gridPrice_tmp[57], gridPrice_tmp[58], gridPrice_tmp[59], gridPrice_tmp[60], gridPrice_tmp[61], gridPrice_tmp[62], gridPrice_tmp[63], gridPrice_tmp[64], gridPrice_tmp[65], gridPrice_tmp[66], gridPrice_tmp[67], gridPrice_tmp[68], gridPrice_tmp[69], gridPrice_tmp[70], gridPrice_tmp[71], gridPrice_tmp[72], gridPrice_tmp[73], gridPrice_tmp[74], gridPrice_tmp[75], gridPrice_tmp[76], gridPrice_tmp[77], gridPrice_tmp[78], gridPrice_tmp[79], gridPrice_tmp[80], gridPrice_tmp[81], gridPrice_tmp[82], gridPrice_tmp[83], gridPrice_tmp[84], gridPrice_tmp[85], gridPrice_tmp[86], gridPrice_tmp[87], gridPrice_tmp[88], gridPrice_tmp[89], gridPrice_tmp[90], gridPrice_tmp[91], gridPrice_tmp[92], gridPrice_tmp[93], gridPrice_tmp[94], gridPrice_tmp[95], gridPrice_total, household_id);
		sent_query();
	}
}