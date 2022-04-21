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
#include "LHEMS_function.hpp"
#include "LHEMS_constraint.hpp"
#include "scheduling_parameter.hpp"

void optimization(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, COMFORTLEVEL comlv, INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, float *uncontrollable_load, int distributed_group_num)
{
	functionPrint(__func__);

	if (comlv.flag)
	{		
		vector<vector<vector<int>>> comfortLevel_startEnd;
		// [0~3] = level, [][0~1] = start or end, [][][0~44] = 3 times * 15 appliances
		for (int i = 0; i < comlv.comfortLevel; i++)
		{
			comfortLevel_startEnd.push_back(get_comfortLevel_timeInterval(bp.household_id, bp.app_count, comlv.total_timeInterval, i + 1));
		}
		comlv.weighting = calculate_comfortLevel_weighting(bp, comfortLevel_startEnd, comlv.comfortLevel, comlv.total_timeInterval);
	}

	countUninterruptAndVaryingLoads_Flag(bp, uirl, varl);

	int *buff = new int[bp.app_count];
	for (int i = 0; i < bp.app_count; i++)
		buff[i] = 0;
	countLoads_AlreadyOpenedTimes(bp, buff);
	count_interruptLoads_RemainOperateTime(irl, buff);
	count_uninterruptLoads_RemainOperateTime(bp, uirl, irl.number, buff);
	count_varyingLoads_RemainOperateTime(bp, varl, irl.number + uirl.number, buff);

	varl.block = NEW2D(varl.number, bp.remain_timeblock, int);
	// varying_p_d will get error if two varying load have different operation time
	varl.power = NEW2D(varl.number, varl.ot[0], float);
	varl.max_power = new float[varl.number];
	init_VaryingLoads_OperateTimeAndPower(bp, varl);
	putValues_VaryingLoads_OperateTimeAndPower(bp, varl);

	if (dr.mode != 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT MAX(household%d) FROM `LHEMS_demand_response_CBL` WHERE `time_block` BETWEEN %d AND %d AND `comfort_level_flag` = %d", bp.household_id, dr.startTime, dr.endTime - 1, comlv.flag);
		dr.household_CBL = turn_value_to_float(0);
		dr.participate_array = household_participation(dr, bp.household_id, "LHEMS_demand_response_participation");
		bp.Pgrid_max_array.assign(bp.remain_timeblock, bp.Pgrid_max);
		if (bp.sample_time - dr.startTime >= 0)
		{
			for (int j = 0; j < dr.endTime - bp.sample_time; j++)
			{
				if (dr.participate_array[j + (bp.sample_time - dr.startTime)] != 0)
				{
					bp.Pgrid_max_array[j] = dr.household_CBL * dr.participate_array[j + (bp.sample_time - dr.startTime)];
				}
			}
		}
		else if (bp.sample_time - dr.startTime < 0)
		{
			for (int j = dr.startTime - bp.sample_time; j < dr.endTime - bp.sample_time; j++)
			{
				if (dr.participate_array[j - (dr.startTime - bp.sample_time)] != 0)
				{
					bp.Pgrid_max_array[j] = dr.household_CBL * dr.participate_array[j + (bp.sample_time - dr.startTime)];
				}
			}
		}
	}

	// sum by 'row_num_maxAddition' in every constraint below
	int rowTotal = 0;
	if (irl.flag) { rowTotal += irl.number; }
	rowTotal += bp.remain_timeblock;
	if (ess.flag) { rowTotal += bp.remain_timeblock * 4 + 1; }
	if (uirl.flag)
	{
		rowTotal += 1;
		for (int i = 0; i < uirl.number; i++)
		{
			if (uirl.continuous_flag[i] == 0)
			{
				rowTotal += bp.remain_timeblock * uirl.reot[i] * 2;
			}
			else
			{
				rowTotal += bp.remain_timeblock * 2;
			}
		}
	}
	if (varl.flag)
	{
		rowTotal += 1;
		for (int i = 0; i < varl.number; i++)
		{
			if (varl.continuous_flag[i] == 0)
			{
				rowTotal += bp.remain_timeblock * varl.reot[i] * 2;
			}
			else
			{
				rowTotal += bp.remain_timeblock * 2;
			}
		}
	}

	int colTotal = bp.variable * bp.remain_timeblock;

	string prob_name = "LHEMS" + to_string(bp.household_id);
	glp_prob *mip;
	mip = glp_create_prob();
	glp_set_prob_name(mip, prob_name.c_str());
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, rowTotal);
	glp_add_cols(mip, colTotal);

	setting_LHEMS_columnBoundary(irl, uirl, varl, bp, ess, dr, mip);

	float **coefficient = NEW2D(rowTotal, colTotal, float);
	for (int m = 0; m < rowTotal; m++)
	{
		for (int n = 0; n < colTotal; n++)
			coefficient[m][n] = 0.0;
	}

	if (irl.flag)
	{
		summation_interruptLoadRa_biggerThan_Qa(irl, bp, coefficient, mip, irl.number);
	}

	// (Balanced function) Pgrid j - Pess j = sum(Pa j) + Puc j
	pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(irl, uirl, varl, bp, ess, uncontrollable_load, coefficient, mip, bp.remain_timeblock);

	if (ess.flag)
	{
		// SOC j - 1 + sum((Pess * Ts) / (Cess * Vess)) >= SOC threshold, only one constranit formula
		previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(bp, ess, coefficient, mip, 1);
		// SOC j = SOC j - 1 + (Pess j * Ts) / (Cess * Vess)
		previousSOCPlusPessTransToSOC_equalTo_currentSOC(bp, ess, coefficient, mip, bp.remain_timeblock);
		// (Charge limit) Pess + <= z * Pcharge max
		pessPositive_smallerThan_zMultiplyByPchargeMax(bp, ess, coefficient, mip, bp.remain_timeblock);
		// (Discharge limit) Pess - <= (1 - z) * Pdischarge max
		pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(bp, ess, coefficient, mip, bp.remain_timeblock);
		// (Battery power) (Pess +) - (Pess -) = Pess j
		pessPositiveMinusPessNegative_equalTo_Pess(bp, ess, coefficient, mip, bp.remain_timeblock);
	}

	if (uirl.flag)
	{
		// sum(δa j) = 1 (uninterrupt loads)
		summation_uninterruptDelta_equalTo_one(uirl, bp, coefficient, mip, 1);
		// ra j+n >= δa j (uninterrupt loads)
		uninterruptRajToN_biggerThan_uninterruptDelta(uirl, bp, coefficient, mip, bp.remain_timeblock);
	}

	if (varl.flag)
	{
		// sum(δa j) = 1 (varying loads)
		summation_varyingDelta_equalTo_one(varl, bp, coefficient, mip, 1);
		// ra j+n >= δa j (varying loads)
		varyingRajToN_biggerThan_varyingDelta(varl, bp, coefficient, mip, bp.remain_timeblock);
		// ψa j+n  >= δa j * σa n
		varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(varl, bp, buff, irl.number + uirl.number, coefficient, mip, bp.remain_timeblock);
	}

	setting_LHEMS_objectiveFunction(irl, uirl, varl, bp, dr, comlv, mip);

	int *ia = new int[rowTotal * colTotal + 1];
	int *ja = new int[rowTotal * colTotal + 1];
	double *ar = new double[rowTotal * colTotal + 1];
	for (int i = 0; i < rowTotal; i++)
	{
		for (int j = 0; j < colTotal; j++)
		{
			ia[i * (bp.remain_timeblock * bp.variable) + j + 1] = i + 1;
			ja[i * (bp.remain_timeblock * bp.variable) + j + 1] = j + 1;
			ar[i * (bp.remain_timeblock * bp.variable) + j + 1] = coefficient[i][j];
		}
	}
	glp_load_matrix(mip, rowTotal * colTotal, ia, ja, ar);

	glp_iocp parm;
	glp_init_iocp(&parm);

	if (bp.sample_time == 0)
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
	printf("LINE %d: timeblock %d household id %d sol = %f; \n", __LINE__, bp.sample_time, bp.household_id, z);

	if (ess.flag)
	{
		if (z == 0.0 && glp_mip_col_val(mip, find_variableName_position(bp.variable_name, ess.str_SOC) + 1) == 0.0)
		{
			display_coefAndBnds_rowNum();
			printf("Error > sol is 0, No Solution, give up the solution\n");
			printf("%.2f\n", glp_mip_col_val(mip, find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1));
			return;
		}
	}
	else
	{
		if (z == 0.0)
		{
			int count = 0;
			for (int i = 0; i < irl.number; i++)
			{
				count += irl.reot[i];
			}
			for (int i = 0; i < uirl.number; i++)
			{
				count += uirl.reot[i];
			}
			for (int i = 0; i < varl.number; i++)
			{
				count += varl.reot[i];
			}
			if (count != 0)
			{
				display_coefAndBnds_rowNum();
				printf("Error > sol is 0, No Solution, give up the solution\n");
				printf("%.2f\n", glp_mip_col_val(mip, find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1));
				return;
			}
		}
	}

	float *s = new float[bp.time_block];

	for (int i = 1; i <= bp.variable; i++)
	{
		int h = i;
		int l = bp.variable - (bp.app_count - i); // get interrupt & varying ra j
		if (bp.sample_time == 0)
		{
			for (int j = 0; j < bp.time_block; j++)
			{
				s[j] = glp_mip_col_val(mip, h);
				if ((i > irl.number + uirl.number) && (i <= bp.app_count)) //sometimes varying load will have weird, use power model instead of varying load
				{
					s[j] = glp_mip_col_val(mip, l);
					if (s[j] == varl.power_tmp[i - (irl.number + uirl.number + 1)][0] || s[j] == varl.power_tmp[i - (irl.number + uirl.number + 1)][1] || s[j] == varl.power_tmp[i - (irl.number + uirl.number + 1)][2])
						s[j] = 1.0;
				}
				h = (h + bp.variable);
				l = (l + bp.variable);
			}
			insert_status_into_MySQLTable("LHEMS_control_status", bp.str_sql_allTimeblock, s, "equip_name", bp.variable_name[i - 1], "household_id", bp.household_id);
		}
		else
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, bp.variable_name[i - 1].c_str(), bp.household_id);
			fetch_row_value();
			for (int k = 0; k < bp.sample_time; k++)
			{
				s[k] = turn_float(k);
			}

			for (int j = 0; j < bp.remain_timeblock; j++)
			{
				s[j + bp.sample_time] = glp_mip_col_val(mip, h);
				if ((i > irl.number + uirl.number) && (i <= bp.app_count)) //sometimes varying load will have weird, use power model instead of varying load
				{
					s[j + bp.sample_time] = glp_mip_col_val(mip, l);
					if (s[j + bp.sample_time] == varl.power_tmp[i - (irl.number + uirl.number + 1)][0] || s[j + bp.sample_time] == varl.power_tmp[i - (irl.number + uirl.number + 1)][1] || s[j + bp.sample_time] == varl.power_tmp[i - (irl.number + uirl.number + 1)][2])
						s[j + bp.sample_time] = 1.0;
				}
				h = (h + bp.variable);
				l = (l + bp.variable);
			}
			update_status_to_MySQLTable("LHEMS_control_status", s, "equip_name", bp.variable_name[i - 1], "AND", "household_id", bp.household_id);

			for (int j = 0; j < bp.sample_time; j++)
			{
				s[j] = 0;
			}
			insert_status_into_MySQLTable("LHEMS_real_status", bp.str_sql_allTimeblock, s, "equip_name", bp.variable_name[i - 1], "household_id", bp.household_id);
		}
	}

	glp_delete_prob(mip);
	delete[] ia, ja, ar, s;
	delete[] coefficient;
	return;
}

void setting_LHEMS_columnBoundary(INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, glp_prob *mip)
{
	functionPrint(__func__);
	messagePrint(__LINE__, "Setting columns...", 'S', 0, 'Y');

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		if (irl.flag)
		{
			for (int j = 1; j <= irl.number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
		}
		if (uirl.flag)
		{
			for (int j = 1; j <= uirl.number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
		}
		if (varl.flag)
		{
			for (int j = 1; j <= varl.number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, varl.str_varying + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, varl.str_varying + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
		}
		if (bp.Pgrid_flag)
		{
			if (dr.mode == 0)
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + i * bp.variable), GLP_DB, 0.0, bp.Pgrid_max);
			else
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + i * bp.variable), GLP_DB, 0.0, bp.Pgrid_max_array[i]);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + i * bp.variable), GLP_CV);
		}
		if (ess.flag)
		{
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Pess) + 1 + i * bp.variable), GLP_DB, -ess.MIN_power, ess.MAX_power);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Pess) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Pcharge) + 1 + i * bp.variable), GLP_FR, 0.0, ess.MAX_power);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Pcharge) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Pdischarge) + 1 + i * bp.variable), GLP_FR, 0.0, ess.MIN_power);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Pdischarge) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_SOC) + 1 + i * bp.variable), GLP_DB, ess.MIN_SOC, ess.MAX_SOC);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_SOC) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Z) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Z) + 1 + i * bp.variable), GLP_BV);
		}
		if (uirl.flag)
		{
			for (int j = 1; j <= uirl.number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, uirl.str_uninterDelta + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, uirl.str_uninterDelta + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
		}
		if (varl.flag)
		{
			for (int j = 1; j <= varl.number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
			for (int j = 1; j <= varl.number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, varl.max_power[j - 1]);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(j)) + 1 + i * bp.variable), GLP_CV);
			}
		}
	}
}

int determine_realTimeOrOneDayMode_andGetSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM &ess, int real_time, int distributed_group_num)
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
		if (ess.flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.sample_time - 1, ess.str_SOC.c_str(), bp.household_id);
			ess.INIT_SOC = turn_value_to_float(0);
			messagePrint(__LINE__, "SOC = ", 'F', ess.INIT_SOC, 'Y');
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
			bp.sample_time = 0;
			update_distributed_group("next_simulate_timeblock", bp.sample_time, "group_id", distributed_group_num);
			messagePrint(__LINE__, "Truncate LHEMS control status: Group ", 'I', distributed_group_num, 'Y');
		}

		if (ess.flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC'");
			ess.INIT_SOC = turn_value_to_float(0);
			messagePrint(__LINE__, "ini_SOC : ", 'F', ess.INIT_SOC, 'Y');
		}

		if (bp.distributed_household_id == bp.distributed_householdTotal)
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

void countUninterruptAndVaryingLoads_Flag(BASEPARAMETER bp, UNINTERRUPTLOAD &uirl, VARYINGLOAD &varl)
{
	printf("\nFunction: %s\n\t", __func__);
	int flag = 0;
	if (bp.sample_time != 0)
	{
		for (int i = 0; i < uirl.number; i++)
		{
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, (uirl.str_uninterDelta + to_string(i + 1)).c_str(), bp.household_id);
			fetch_row_value();
			for (int j = 0; j < bp.sample_time; j++)
			{
				flag += turn_int(j);
			}
			uirl.continuous_flag[i] = flag;
		}
		for (int i = 0; i < varl.number; i++)
		{
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, (varl.str_varyingDelta + to_string(i + 1)).c_str(), bp.household_id);
			fetch_row_value();
			for (int j = 0; j < bp.sample_time; j++)
			{
				flag += turn_int(j);
			}
			varl.continuous_flag[i] = flag;
		}
	}
	for (int i = 0; i < uirl.number; i++)
		printf("LINE %d: uninterrupt_flag[%d] : %d\n\t", __LINE__, i, uirl.continuous_flag[i]);
	for (int i = 0; i < varl.number; i++)
		printf("LINE %d: varying_flag[%d] : %d\n", __LINE__, i, varl.continuous_flag[i]);
}

void countLoads_AlreadyOpenedTimes(BASEPARAMETER bp, int *buff)
{
	printf("\nFunction: %s\n\t", __func__);
	int coun = 0;
	if (bp.sample_time != 0)
	{
		for (int i = 0; i < bp.app_count; i++)
		{
			coun = 0;

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, bp.variable_name[i].c_str(), bp.household_id);
			fetch_row_value();
			for (int j = 0; j < bp.sample_time; j++)
			{
				coun += turn_int(j);
			}
			buff[i] = coun;
		}
	}
	printf("LINE %d: ", __LINE__);
	for (int i = 0; i < bp.app_count; i++)
		printf("buff[%d]: %d ", i, buff[i]);
	printf("\n");
}

void count_interruptLoads_RemainOperateTime(INTERRUPTLOAD &irl, int *buff)
{
	printf("\nFunction: %s\n\t", __func__);
	for (int i = 0; i < irl.number; i++)
	{
		if ((irl.ot[i] - buff[i]) == irl.ot[i])
		{
			irl.reot[i] = irl.ot[i];
		}
		else if (((irl.ot[i] - buff[i]) < irl.ot[i]) && ((irl.ot[i] - buff[i]) > 0))
		{
			irl.reot[i] = irl.ot[i] - buff[i];
		}
		else if ((irl.ot[i] - buff[i]) <= 0)
		{
			irl.reot[i] = 0;
		}
		printf("LINE %d: load %d : reot = %d\n\t", __LINE__, i, irl.reot[i]);
	}
}

void count_uninterruptLoads_RemainOperateTime(BASEPARAMETER bp, UNINTERRUPTLOAD &uirl, int buff_shift_length, int *buff)
{
	functionPrint(__func__);

	for (int i = 0; i < uirl.number; i++)
	{
		if (uirl.continuous_flag[i] == 0)
		{
			uirl.reot[i] = uirl.ot[i];
		}
		if (uirl.continuous_flag[i] == 1)
		{
			if (((uirl.ot[i] - buff[i + buff_shift_length]) < uirl.ot[i]) && ((uirl.ot[i] - buff[i + buff_shift_length]) > 0))
			{
				uirl.reot[i] = uirl.ot[i] - buff[i + buff_shift_length];
				if (uirl.reot[i] != 0)
				{
					uirl.end[i] = bp.sample_time + uirl.reot[i] - 1;
				}
			}
			else if ((uirl.ot[i] - buff[i + buff_shift_length]) <= 0)
			{
				uirl.reot[i] = 0;
			}
		}
		printf("LINE %d: uninterrupt load %d : reot = %d\n\t", __LINE__, i, uirl.reot[i]);
	}
}

void count_varyingLoads_RemainOperateTime(BASEPARAMETER bp, VARYINGLOAD &varl, int buff_shift_length, int *buff)
{
	functionPrint(__func__);
	
	for (int i = 0; i < varl.number; i++)
	{
		if (varl.continuous_flag[i] == 0)
		{
			varl.reot[i] = varl.ot[i];
		}
		if (varl.continuous_flag[i] == 1)
		{
			if (((varl.ot[i] - buff[i + buff_shift_length]) < varl.ot[i]) && ((varl.ot[i] - buff[i + buff_shift_length]) > 0))
			{
				varl.reot[i] = varl.ot[i] - buff[i + buff_shift_length];
				if (varl.reot[i] != 0)
				{
					varl.end[i] = bp.sample_time + varl.reot[i] - 1;
				}
			}
			else if ((varl.ot[i] - buff[i + buff_shift_length]) <= 0)
			{
				varl.reot[i] = 0;
			}
		}
		printf("LINE %d: uninterrupt load %d : reot = %d\n\t", __LINE__, i, varl.reot[i]);
	}
}

void init_VaryingLoads_OperateTimeAndPower(BASEPARAMETER bp, VARYINGLOAD &varl)
{
	printf("\nFunction: %s \n\t", __func__);
	for (int i = 0; i < varl.number; i++)
	{
		for (int m = 0; m < bp.remain_timeblock; m++)
		{
			varl.block[i][m] = 0;
		}
		for (int m = 0; m < varl.ot[i]; m++)
		{
			varl.power[i][m] = 0.0;
		}
	}
}

void putValues_VaryingLoads_OperateTimeAndPower(BASEPARAMETER bp, VARYINGLOAD &varl)
{
	printf("\nFunction: %s \n\t", __func__);

	for (int i = 0; i < varl.number; i++)
	{
		for (int j = 0; j < varl.block_tmp[i][0]; j++)
		{
			varl.power[i][j] = varl.power_tmp[i][0];
		}
		for (int j = varl.block_tmp[i][0]; j < varl.block_tmp[i][0] + varl.block_tmp[i][1]; j++)
		{
			varl.power[i][j] = varl.power_tmp[i][1];
		}
		for (int j = varl.block_tmp[i][0] + varl.block_tmp[i][1]; j < varl.block_tmp[i][0] + varl.block_tmp[i][1] + varl.block_tmp[i][2]; j++)
		{
			varl.power[i][j] = varl.power_tmp[i][2];
		}
	}

	for (int i = 0; i < varl.number; i++)
	{
		if ((varl.end[i] - bp.sample_time) >= 0)
		{
			if ((varl.start[i] - bp.sample_time) >= 0)
			{
				for (int m = (varl.start[i] - bp.sample_time); m <= (varl.end[i] - bp.sample_time); m++)
				{
					varl.block[i][m] = 1;
				}
			}
			else if ((varl.start[i] - bp.sample_time) < 0)
			{
				for (int m = 0; m <= (varl.end[i] - bp.sample_time); m++)
				{
					varl.block[i][m] = 1;
				}
			}
		}
	}

	for (int i = 0; i < varl.number; i++)
	{
		varl.max_power[i] = 0.0;

		for (int j = 0; j < 3; j++)
		{
			if (varl.power_tmp[i][j] > varl.max_power[i])
			{
				varl.max_power[i] = varl.power_tmp[i][j];
			}
		}
	}
}

void update_loadModel(BASEPARAMETER bp, INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, int distributed_group_num)
{
	functionPrint(__func__);
	float *power_tmp = new float[bp.remain_timeblock];
	for (int i = 0; i < bp.remain_timeblock; i++)
		power_tmp[i] = 0.0;

	for (int i = 0; i < irl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, (irl.str_interrupt + to_string(i + 1)).c_str(), bp.household_id);
		fetch_row_value();
		for (int j = bp.sample_time; j < bp.time_block; j++)
		{
			power_tmp[j - bp.sample_time] += turn_float(j) * irl.power[i];
		}
	}
	for (int i = 0; i < uirl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, (uirl.str_uninterrupt + to_string(i + 1)).c_str(), bp.household_id);
		fetch_row_value();
		for (int j = bp.sample_time; j < bp.time_block; j++)
		{
			power_tmp[j - bp.sample_time] += turn_float(j) * uirl.power[i];
		}
	}
	for (int i = 0; i < varl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM LHEMS_control_status WHERE equip_name = '%s' and household_id = %d", bp.str_sql_allTimeblock, (varl.str_varyingPsi + to_string(i + 1)).c_str(), bp.household_id);
		fetch_row_value();
		for (int j = bp.sample_time; j < bp.time_block; j++)
		{
			power_tmp[j - bp.sample_time] += turn_float(j);
		}
	}
	for (int i = bp.sample_time; i < bp.time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `totalLoad_model` SET `household%d` = '%.3f' WHERE `totalLoad_model`.`time_block` = %d;", bp.household_id, power_tmp[i - bp.sample_time], i);
		sent_query();
	}
	// =-=-=-=-=-=-=- Caculate for total load model -=-=-=-=-=-=-= //
	if (bp.distributed_household_id == bp.distributed_householdTotal)
	{
		update_distributed_group("total_load_flag", 1, "group_id", distributed_group_num);
		if (get_distributed_group("COUNT(group_id) = SUM(total_load_flag)"))
		{
			for (int j = 0; j < bp.time_block; j++)
			{
				float power_total = 0.0;
				for (int i = 1; i <= bp.householdTotal; i++)
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

void HEMS_UCload_rand_operationTime(BASEPARAMETER bp, UNCONTROLLABLELOAD &ucl, int distributed_group_num)
{
	functionPrint(__func__);
	float *result = new float[bp.time_block];
	for (int i = 0; i < bp.time_block; i++)
		result[i] = 0.0;

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE group_id = %d", ucl.hems_group_id);
	ucl.number = turn_value_to_int(0);

	if (!ucl.flag)
	{
		update_distributed_group("uncontrollable_load_flag", 0, "group_id", distributed_group_num);
		ucl.power_array = result;
	}
	else
	{	
		if (ucl.generate_flag)
		{
			srand(time(NULL));
			if (bp.sample_time == 0)
			{
				for (int i = 0; i < ucl.number; i++)
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
				for (int i = 0; i < bp.time_block; i++)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_uncontrollable_load` SET `household%d` = '%.1f' WHERE `time_block` = %d;", bp.household_id, result[i], i);
					sent_query();
				}
				if (bp.distributed_household_id == bp.distributed_householdTotal)
				{
					update_distributed_group("uncontrollable_load_flag", 1, "group_id", distributed_group_num);
					if (get_distributed_group("COUNT(group_id) = SUM(uncontrollable_load_flag)"))
					{
						for (int j = 0; j < bp.time_block; j++)
						{
							float power_total = 0.0;
							for (int i = 1; i <= bp.householdTotal; i++)
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
		}
		for (int i = 0; i < bp.time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `household%d` FROM `LHEMS_uncontrollable_load` WHERE `time_block` = %d;", bp.household_id, i);
			result[i] = turn_value_to_float(0);
		}
		ucl.power_array = result;
	}
}

float *household_participation(DEMANDRESPONSE dr, int household_id, string table)
{
	functionPrint(__func__);

	float *result = new float[dr.endTime - dr.startTime];

	// =-=-=-=-=-=-=- calculate weighting then turn to alpha -=-=-=-=-=-=-= //
	for (int i = dr.startTime; i < dr.endTime; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `%s` WHERE `household_id` = %d", i, table.c_str(), household_id);
		result[i - dr.startTime] = ceil(turn_value_to_float(0));

		printf("\thousehold %d timeblock %d status %.2f\n", household_id, i, result[i - dr.startTime]);
	}

	return result;
}

void init_totalLoad_flag_and_table(BASEPARAMETER bp, int distributed_group_num)
{
	// init totalLoad table
	if (bp.distributed_household_id == 1)
	{
		update_distributed_group("total_load_flag", 0, "group_id", distributed_group_num);
		if (bp.sample_time == 0)
		{
			for (int i = 1; i <= bp.distributed_householdTotal; i++)
			{
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `totalLoad_model` SET `household%d` = '0' ", (distributed_group_num - 1) * bp.distributed_householdTotal + i);
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

float **calculate_comfortLevel_weighting(BASEPARAMETER bp, vector<vector<vector<int>>> comfortLevel_startEnd, int comfortLevel, int total_timeInterval)
{
	functionPrint(__func__);

	// float weighting[app_count][time_block] = {0.0};
	float **weighting = new float *[bp.app_count];
	for (int i = 0; i < bp.app_count; i++)
		weighting[i] = new float[bp.time_block];

	for (int i = 0; i < bp.app_count; i++)
	{
		for (int j = 0; j < bp.time_block; j++)
		{
			weighting[i][j]	= 0.0;
		}
	}
	
		
	for (int i = 0; i < bp.app_count; i++)
	{
		for (int j = 0; j < bp.remain_timeblock; j++)
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
				if (level1_start_time != level1_end_time && (j + bp.sample_time) >= level1_start_time && (j + bp.sample_time) < level1_end_time)
				{
					weighting[i][j + bp.sample_time] += (j + bp.sample_time - level1_start_time) / (level1_end_time - level1_start_time);
				}
				else if (level2_start_time != level2_end_time && (j + bp.sample_time) >= level2_start_time && (j + bp.sample_time) < level2_end_time)
				{
					weighting[i][j + bp.sample_time] += (j + bp.sample_time - level2_start_time) / (level2_end_time - level2_start_time) + 1;
				}
				else if (level3_start_time != level3_end_time && (j + bp.sample_time) >= level3_start_time && (j + bp.sample_time) < level3_end_time)
				{
					weighting[i][j + bp.sample_time] += (j + bp.sample_time - level3_start_time) / (level3_end_time - level3_start_time) + 2;
				}
				else if (level4_start_time != level4_end_time && (j + bp.sample_time) >= level4_start_time && (j + bp.sample_time) < level4_end_time)
				{
					weighting[i][j + bp.sample_time] += (j + bp.sample_time - level4_start_time) / (level4_end_time - level4_start_time) + 3;
				}
				else
				{
					weighting[i][j + bp.sample_time] += 10;
				}
			}
		}
	}
	return weighting;
}

void calculateCostInfo(BASEPARAMETER bp)
{
	functionPrint(__func__);

	vector<float> gridPrice_tmp;
	for (int i = 0; i < bp.time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `LHEMS_control_status` WHERE equip_name = '%s' AND household_id = '%d'", i, bp.str_Pgrid.c_str(), bp.household_id);
		gridPrice_tmp.push_back(turn_value_to_float(0) * bp.price[i] * bp.delta_T);
	}
	float gridPrice_total = accumulate(gridPrice_tmp.begin(), gridPrice_tmp.end(), 0.0);

	if (bp.sample_time == 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `LHEMS_cost` (household_id, origin_grid_price, %s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", bp.str_sql_allTimeblock, bp.household_id, gridPrice_total, gridPrice_tmp[0], gridPrice_tmp[1], gridPrice_tmp[2], gridPrice_tmp[3], gridPrice_tmp[4], gridPrice_tmp[5], gridPrice_tmp[6], gridPrice_tmp[7], gridPrice_tmp[8], gridPrice_tmp[9], gridPrice_tmp[10], gridPrice_tmp[11], gridPrice_tmp[12], gridPrice_tmp[13], gridPrice_tmp[14], gridPrice_tmp[15], gridPrice_tmp[16], gridPrice_tmp[17], gridPrice_tmp[18], gridPrice_tmp[19], gridPrice_tmp[20], gridPrice_tmp[21], gridPrice_tmp[22], gridPrice_tmp[23], gridPrice_tmp[24], gridPrice_tmp[25], gridPrice_tmp[26], gridPrice_tmp[27], gridPrice_tmp[28], gridPrice_tmp[29], gridPrice_tmp[30], gridPrice_tmp[31], gridPrice_tmp[32], gridPrice_tmp[33], gridPrice_tmp[34], gridPrice_tmp[35], gridPrice_tmp[36], gridPrice_tmp[37], gridPrice_tmp[38], gridPrice_tmp[39], gridPrice_tmp[40], gridPrice_tmp[41], gridPrice_tmp[42], gridPrice_tmp[43], gridPrice_tmp[44], gridPrice_tmp[45], gridPrice_tmp[46], gridPrice_tmp[47], gridPrice_tmp[48], gridPrice_tmp[49], gridPrice_tmp[50], gridPrice_tmp[51], gridPrice_tmp[52], gridPrice_tmp[53], gridPrice_tmp[54], gridPrice_tmp[55], gridPrice_tmp[56], gridPrice_tmp[57], gridPrice_tmp[58], gridPrice_tmp[59], gridPrice_tmp[60], gridPrice_tmp[61], gridPrice_tmp[62], gridPrice_tmp[63], gridPrice_tmp[64], gridPrice_tmp[65], gridPrice_tmp[66], gridPrice_tmp[67], gridPrice_tmp[68], gridPrice_tmp[69], gridPrice_tmp[70], gridPrice_tmp[71], gridPrice_tmp[72], gridPrice_tmp[73], gridPrice_tmp[74], gridPrice_tmp[75], gridPrice_tmp[76], gridPrice_tmp[77], gridPrice_tmp[78], gridPrice_tmp[79], gridPrice_tmp[80], gridPrice_tmp[81], gridPrice_tmp[82], gridPrice_tmp[83], gridPrice_tmp[84], gridPrice_tmp[85], gridPrice_tmp[86], gridPrice_tmp[87], gridPrice_tmp[88], gridPrice_tmp[89], gridPrice_tmp[90], gridPrice_tmp[91], gridPrice_tmp[92], gridPrice_tmp[93], gridPrice_tmp[94], gridPrice_tmp[95]);
		sent_query();
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_cost` set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `origin_grid_price` = ''%.3f, `datetime` = CURRENT_TIMESTAMP WHERE household_id = '%d';", gridPrice_tmp[0], gridPrice_tmp[1], gridPrice_tmp[2], gridPrice_tmp[3], gridPrice_tmp[4], gridPrice_tmp[5], gridPrice_tmp[6], gridPrice_tmp[7], gridPrice_tmp[8], gridPrice_tmp[9], gridPrice_tmp[10], gridPrice_tmp[11], gridPrice_tmp[12], gridPrice_tmp[13], gridPrice_tmp[14], gridPrice_tmp[15], gridPrice_tmp[16], gridPrice_tmp[17], gridPrice_tmp[18], gridPrice_tmp[19], gridPrice_tmp[20], gridPrice_tmp[21], gridPrice_tmp[22], gridPrice_tmp[23], gridPrice_tmp[24], gridPrice_tmp[25], gridPrice_tmp[26], gridPrice_tmp[27], gridPrice_tmp[28], gridPrice_tmp[29], gridPrice_tmp[30], gridPrice_tmp[31], gridPrice_tmp[32], gridPrice_tmp[33], gridPrice_tmp[34], gridPrice_tmp[35], gridPrice_tmp[36], gridPrice_tmp[37], gridPrice_tmp[38], gridPrice_tmp[39], gridPrice_tmp[40], gridPrice_tmp[41], gridPrice_tmp[42], gridPrice_tmp[43], gridPrice_tmp[44], gridPrice_tmp[45], gridPrice_tmp[46], gridPrice_tmp[47], gridPrice_tmp[48], gridPrice_tmp[49], gridPrice_tmp[50], gridPrice_tmp[51], gridPrice_tmp[52], gridPrice_tmp[53], gridPrice_tmp[54], gridPrice_tmp[55], gridPrice_tmp[56], gridPrice_tmp[57], gridPrice_tmp[58], gridPrice_tmp[59], gridPrice_tmp[60], gridPrice_tmp[61], gridPrice_tmp[62], gridPrice_tmp[63], gridPrice_tmp[64], gridPrice_tmp[65], gridPrice_tmp[66], gridPrice_tmp[67], gridPrice_tmp[68], gridPrice_tmp[69], gridPrice_tmp[70], gridPrice_tmp[71], gridPrice_tmp[72], gridPrice_tmp[73], gridPrice_tmp[74], gridPrice_tmp[75], gridPrice_tmp[76], gridPrice_tmp[77], gridPrice_tmp[78], gridPrice_tmp[79], gridPrice_tmp[80], gridPrice_tmp[81], gridPrice_tmp[82], gridPrice_tmp[83], gridPrice_tmp[84], gridPrice_tmp[85], gridPrice_tmp[86], gridPrice_tmp[87], gridPrice_tmp[88], gridPrice_tmp[89], gridPrice_tmp[90], gridPrice_tmp[91], gridPrice_tmp[92], gridPrice_tmp[93], gridPrice_tmp[94], gridPrice_tmp[95], gridPrice_total, bp.household_id);
		sent_query();
	}
}

void getLoads_startEndOperationTime_and_power(INTERRUPTLOAD &irl, BASEPARAMETER bp)
{
	const string str_joinTwoLoadListTable = "`load_list` INNER JOIN load_list_select ON load_list.number=load_list_select.number";
	char *s_time = new char[3];
	char *token = strtok(s_time, "-");
	vector<int> time_tmp;
	irl.start = new int[irl.number];
	irl.end = new int[irl.number];
	irl.ot = new int[irl.number];
	irl.reot = new int[irl.number];
	irl.power = new float[irl.number];

	for (int i = 0; i < irl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT load_list.household%d_startEndOperationTime FROM %s WHERE load_list_select.group_id = %d AND load_list_select.household%d = 1 LIMIT 1 OFFSET %d", bp.household_id, str_joinTwoLoadListTable.c_str(), irl.group_id, bp.household_id, i);
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
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT load_list.power1 FROM %s WHERE load_list_select.group_id = %d AND load_list_select.household%d = 1 LIMIT 1 OFFSET %d", str_joinTwoLoadListTable.c_str(), irl.group_id, bp.household_id, i);
		irl.power[i] = turn_value_to_float(0);
		time_tmp.clear();
	}
}

void getLoads_startEndOperationTime_and_power(UNINTERRUPTLOAD &uirl, BASEPARAMETER bp)
{
	const string str_joinTwoLoadListTable = "`load_list` INNER JOIN load_list_select ON load_list.number=load_list_select.number";
	char *s_time = new char[3];
	char *token = strtok(s_time, "-");
	vector<int> time_tmp;
	uirl.start = new int[uirl.number];
	uirl.end = new int[uirl.number];
	uirl.ot = new int[uirl.number];
	uirl.reot = new int[uirl.number];
	uirl.power = new float[uirl.number];
	uirl.continuous_flag = new bool[uirl.number];

	for (int i = 0; i < uirl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT load_list.household%d_startEndOperationTime FROM %s WHERE load_list_select.group_id = %d AND load_list_select.household%d = 1 LIMIT 1 OFFSET %d", bp.household_id, str_joinTwoLoadListTable.c_str(), uirl.group_id, bp.household_id, i);
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
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT load_list.power1 FROM %s WHERE load_list_select.group_id = %d AND load_list_select.household%d = 1 LIMIT 1 OFFSET %d", str_joinTwoLoadListTable.c_str(), uirl.group_id, bp.household_id, i);
		uirl.power[i] = turn_value_to_float(0);
		time_tmp.clear();
	}
}

void getLoads_startEndOperationTime_and_power(VARYINGLOAD &varl, BASEPARAMETER bp)
{
	const string str_joinTwoLoadListTable = "`load_list` INNER JOIN load_list_select ON load_list.number=load_list_select.number";
	char *s_time = new char[3];
	char *token = strtok(s_time, "-");
	vector<int> time_tmp;
	varl.start = new int[varl.number];
	varl.end = new int[varl.number];
	varl.ot = new int[varl.number];
	varl.reot = new int[varl.number];
	varl.block_tmp = NEW2D(varl.number, 3, int);
	varl.power_tmp = NEW2D(varl.number, 3, float);
	varl.continuous_flag = new bool[varl.number];
	
	for (int i = 0; i < varl.number; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT load_list.household%d_startEndOperationTime FROM %s WHERE load_list_select.group_id = %d AND load_list_select.household%d = 1 LIMIT 1 OFFSET %d", bp.household_id, str_joinTwoLoadListTable.c_str(), varl.group_id, bp.household_id, i);
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
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT load_list.power1, load_list.power2, load_list.power3, load_list.block1, load_list.block2, load_list.block3 FROM %s WHERE load_list_select.group_id = %d AND load_list_select.household%d = 1 LIMIT 1 OFFSET %d", str_joinTwoLoadListTable.c_str(), varl.group_id, bp.household_id, i);
		fetch_row_value();
		for (int z = 0; z < 3; z++)
			varl.power_tmp[i][z] = turn_float(z);
		for (int z = 0; z < 3; z++)
			varl.block_tmp[i][z] = turn_int(z + 3);
		time_tmp.clear();
	}
}