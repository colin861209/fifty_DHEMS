#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql/mysql.h>
#include <iostream>
#include "SQLFunction.hpp"
#include "fifty_LHEMS_function.hpp"
#include "scheduling_parameter.hpp"
#include "LHEMS_constraint.hpp"

// =-=-=-=-=-=-=- interrupt load -=-=-=-=-=-=-= //
void summation_interruptLoadRa_biggerThan_Qa(INTERRUPTLOAD irl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < irl.number; h++)
	{
		if ((irl.end[h] - bp.sample_time) >= 0)
		{
			if ((irl.start[h] - bp.sample_time) >= 0)
			{
				for (int i = (irl.start[h] - bp.sample_time); i <= (irl.end[h] - bp.sample_time); i++)
				{
					coefficient[bp.coef_row_num + h][i * bp.variable + find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(h + 1))] = 1.0;
				}
			}
			else if ((irl.start[h] - bp.sample_time) < 0)
			{
				for (int i = 0; i <= (irl.end[h] - bp.sample_time); i++)
				{
					coefficient[bp.coef_row_num + h][i * bp.variable + find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(h + 1))] = 1.0;
				}
			}
		}
		glp_set_row_name(mip, bp.bnd_row_num + h, "");
		glp_set_row_bnds(mip, bp.bnd_row_num + h, GLP_LO, ((float)irl.reot[h]), 0.0);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- demand response -=-=-=-=-=-=-= //
void pgrid_smallerThan_alphaPgridMax(BASEPARAMETER &bp, DEMANDRESPONSE dr, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pgrid)] = 1.0;
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, dr.str_alpha)] = -bp.Pgrid_max;

		glp_set_row_name(mip, bp.bnd_row_num + i, "");
		glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_UP, 0.0, 0.0);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void alpha_between_oneminusDu_and_one(BASEPARAMETER &bp, DEMANDRESPONSE dr, int *participate_array, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, dr.str_alpha)] = 1.0;

		glp_set_row_name(mip, bp.bnd_row_num + i, "");
		glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_FX, 1.0, 1.0);
		if (bp.sample_time + i < dr.endTime)
		{
			if (bp.sample_time + i >= dr.startTime)
			{
				if (1 - participate_array[bp.sample_time + i - dr.startTime] == 0)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_DB, (1 - participate_array[bp.sample_time + i - dr.startTime]), 1.0);
					// glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_DB, 0.0, 1.0);
				}
			}
		}
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- balanced equation -=-=-=-=-=-=-= //
void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(INTERRUPTLOAD irl, BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, int *uninterrupt_start, int *uninterrupt_end, float *uninterrupt_p, int *varying_start, int *varying_end, float *uncontrollable_load, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	if (irl.flag)
	{
		for (int h = 0; h < irl.number; h++)
		{
			if ((irl.end[h] - bp.sample_time) >= 0)
			{
				if ((irl.start[h] - bp.sample_time) >= 0)
				{
					for (int i = (irl.start[h] - bp.sample_time); i <= (irl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(h + 1))] = irl.power[h];
					}
				}
				else if ((irl.start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (irl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(h + 1))] = irl.power[h];
					}
				}
			}
		}
	}
	if (bp.uninterruptLoad_flag)
	{
		for (int h = 0; h < bp.uninterrupt_num; h++)
		{
			if ((uninterrupt_end[h] - bp.sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - bp.sample_time) >= 0)
				{
					for (int i = (uninterrupt_start[h] - bp.sample_time); i <= (uninterrupt_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterrupt + to_string(h + 1))] = uninterrupt_p[h];
					}
				}
				else if ((uninterrupt_start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (uninterrupt_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterrupt + to_string(h + 1))] = uninterrupt_p[h];
					}
				}
			}
		}
	}
	if (bp.varyingLoad_flag)
	{
		for (int h = 0; h < bp.varying_num; h++)
		{
			if ((varying_end[h] - bp.sample_time) >= 0)
			{
				if ((varying_start[h] - bp.sample_time) >= 0)
				{
					for (int i = (varying_start[h] - bp.sample_time); i <= (varying_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varyingPsi + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varying_start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (varying_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varyingPsi + to_string(h + 1))] = 1.0;
					}
				}
			}
		}
	}
	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		if (bp.Pgrid_flag)
			coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pgrid)] = -1.0;
		if (ess.flag)
			coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pess)] = 1.0;

		glp_set_row_name(mip, (bp.bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, -uncontrollable_load[i + bp.sample_time], -uncontrollable_load[i + bp.sample_time]);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- battery -=-=-=-=-=-=-= //
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pess)] = 1.0;
	}
	glp_set_row_name(mip, bp.bnd_row_num, "");
	if (bp.sample_time == 0)
		glp_set_row_bnds(mip, bp.bnd_row_num, GLP_LO, ((ess.threshold_SOC - ess.INIT_SOC) * ess.capacity) / bp.delta_T, 0.0);

	else
		glp_set_row_bnds(mip, bp.bnd_row_num, GLP_DB, ((ess.threshold_SOC - ess.INIT_SOC) * ess.capacity) / bp.delta_T, ((0.99 - ess.INIT_SOC) * ess.capacity) / bp.delta_T);
	// avoid the row max is bigger than SOC max

	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void previousSOCPlusPessTransToSOC_equalTo_currentSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pess)] = -1.0;
		}
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_SOC)] = ess.capacity / bp.delta_T;

		glp_set_row_name(mip, (bp.bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, (ess.INIT_SOC * ess.capacity / bp.delta_T), (ess.INIT_SOC * ess.capacity / bp.delta_T));
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pessPositive_smallerThan_zMultiplyByPchargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pcharge)] = 1.0;
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Z)] = -ess.MAX_power;

		glp_set_row_name(mip, (bp.bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, 0.0);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pdischarge)] = 1.0;
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Z)] = ess.MIN_power;

		glp_set_row_name(mip, (bp.bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, ess.MIN_power);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pessPositiveMinusPessNegative_equalTo_Pess(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pess)] = 1.0;
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pcharge)] = -1.0;
		coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pdischarge)] = 1.0;

		glp_set_row_name(mip, (bp.bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, 0.0, 0.0);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- uninterrupt load -=-=-=-=-=-=-= //
void summation_uninterruptDelta_equalTo_one(BASEPARAMETER &bp, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < bp.uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			if ((uninterrupt_end[h] - bp.sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - bp.sample_time) >= 0)
				{
					for (int i = (uninterrupt_start[h] - bp.sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterDelta + to_string(h + 1))] = 1.0;
					}
				}
				else if ((uninterrupt_start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterDelta + to_string(h + 1))] = 1.0;
					}
				}
			}
			glp_set_row_name(mip, bp.bnd_row_num, "");
			glp_set_row_bnds(mip, bp.bnd_row_num, GLP_FX, 1.0, 1.0);

			bp.coef_row_num += row_num_maxAddition;
			bp.bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
		}
	}
}

void uninterruptRajToN_biggerThan_uninterruptDelta(BASEPARAMETER &bp, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < bp.uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			for (int m = 0; m < uninterrupt_reot[h]; m++)
			{
				if ((uninterrupt_end[h] - bp.sample_time) >= 0)
				{
					if ((uninterrupt_start[h] - bp.sample_time) >= 0)
					{
						for (int i = (uninterrupt_start[h] - bp.sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterrupt + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterDelta + to_string(h + 1))] = -1.0;
						}
					}
					else if ((uninterrupt_start[h] - bp.sample_time) < 0)
					{
						for (int i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterrupt + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterDelta + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < bp.remain_timeblock; i++)
					{
						glp_set_row_name(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			bp.coef_row_num += row_num_maxAddition * uninterrupt_reot[h];
			bp.bnd_row_num += row_num_maxAddition * uninterrupt_reot[h];
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition * uninterrupt_reot[h], bp.bnd_row_num, row_num_maxAddition * uninterrupt_reot[h]);
		}
		if (uninterrupt_flag[h] == 1)
		{
			if ((uninterrupt_end[h] - bp.sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - bp.sample_time) <= 0)
				{
					for (int i = 0; i <= (uninterrupt_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_uninterrupt + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < uninterrupt_reot[h]; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = uninterrupt_reot[h]; i < bp.remain_timeblock; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
				bp.coef_row_num += row_num_maxAddition;
				bp.bnd_row_num += row_num_maxAddition;
				saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
			}
		}
	}
}

// =-=-=-=-=-=-=- varying load -=-=-=-=-=-=-= //
void summation_varyingDelta_equalTo_one(BASEPARAMETER &bp, int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < bp.varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			if ((varying_end[h] - bp.sample_time) >= 0)
			{
				if ((varying_start[h] - bp.sample_time) >= 0)
				{
					for (int i = (varying_start[h] - bp.sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varyingDelta + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varying_start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varyingDelta + to_string(h + 1))] = 1.0;
					}
				}
			}
			glp_set_row_name(mip, bp.bnd_row_num, "");
			glp_set_row_bnds(mip, bp.bnd_row_num, GLP_FX, 1.0, 1.0);

			bp.coef_row_num += row_num_maxAddition;
			bp.bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
		}
	}
}

void varyingRajToN_biggerThan_varyingDelta(BASEPARAMETER &bp, int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < bp.varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (int m = 0; m < varying_reot[h]; m++)
			{
				if ((varying_end[h] - bp.sample_time) >= 0)
				{
					if ((varying_start[h] - bp.sample_time) >= 0)
					{
						for (int i = (varying_start[h] - bp.sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, bp.str_varying + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varyingDelta + to_string(h + 1))] = -1.0;
						}
					}
					else if ((varying_start[h] - bp.sample_time) < 0)
					{
						for (int i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, bp.str_varying + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varyingDelta + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < bp.remain_timeblock; i++)
					{
						glp_set_row_name(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			bp.coef_row_num += row_num_maxAddition * varying_reot[h];
			bp.bnd_row_num += row_num_maxAddition * varying_reot[h];
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition * varying_reot[h], bp.bnd_row_num, row_num_maxAddition * varying_reot[h]);
		}
		if (varying_flag[h] == 1)
		{
			if ((varying_end[h] - bp.sample_time) >= 0)
			{
				if ((varying_start[h] - bp.sample_time) <= 0)
				{
					for (int i = 0; i <= (varying_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_varying + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < varying_reot[h]; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = varying_reot[h]; i < bp.remain_timeblock; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
				bp.coef_row_num += row_num_maxAddition;
				bp.bnd_row_num += row_num_maxAddition;
				saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
			}
		}
	}
}

void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(BASEPARAMETER &bp, int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, int **varying_t_d, float **varying_p_d, int *buff, int interrupt_num, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < bp.varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (int m = 0; m < varying_reot[h]; m++)
			{
				if ((varying_end[h] - bp.sample_time) >= 0)
				{
					if ((varying_start[h] - bp.sample_time) >= 0)
					{
						for (int i = (varying_start[h] - bp.sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varyingDelta + to_string(h + 1))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][((i + m) * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varyingPsi + to_string(h + 1))] = 1.0;
						}
					}
					else if ((varying_start[h] - bp.sample_time) < 0)
					{
						for (int i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varyingDelta + to_string(h + 1))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][((i + m) * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varyingPsi + to_string(h + 1))] = 1.0;
						}
					}
					for (int i = 0; i < bp.remain_timeblock; i++)
					{
						glp_set_row_name(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			bp.coef_row_num += row_num_maxAddition * varying_reot[h];
			bp.bnd_row_num += row_num_maxAddition * varying_reot[h];
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition * varying_reot[h], bp.bnd_row_num, row_num_maxAddition * varying_reot[h]);
		}
		if (varying_flag[h] == 1)
		{
			if ((varying_end[h] - bp.sample_time) >= 0)
			{
				if ((varying_start[h] - bp.sample_time) >= 0)
				{
					for (int i = (varying_start[h] - bp.sample_time); i <= (varying_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varying + to_string(h + 1))] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + bp.uninterrupt_num]]));
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varyingPsi + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varying_start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (varying_end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varying + to_string(h + 1))] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + bp.uninterrupt_num]]));
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, bp.str_varyingPsi + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < bp.remain_timeblock; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
			}
			bp.coef_row_num += row_num_maxAddition;
			bp.bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
		}
	}
}

// =-=-=-=-=-=-=- objective function -=-=-=-=-=-=-= //
void setting_LHEMS_objectiveFunction(INTERRUPTLOAD irl, BASEPARAMETER bp, DEMANDRESPONSE dr, COMFORTLEVEL comlv, float* price, int *participate_array, glp_prob *mip)
{
	functionPrint(__func__);
	
	for (int j = 0; j < bp.remain_timeblock; j++)
	{
		glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + j * bp.variable), price[j + bp.sample_time] * bp.delta_T);
		
		if (comlv.flag)
		{	
			float comfort_c = 6.6;
			for (int i = 0; i < irl.number; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, irl.str_interrupt + to_string(i + 1)) + 1 + j * bp.variable), comfort_c * comlv.weighting[i][(j + bp.sample_time)]);
			}
			for (int i = 0; i < bp.uninterrupt_num; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_uninterrupt + to_string(i + 1)) + 1 + j * bp.variable), comfort_c * comlv.weighting[i + irl.number][(j + bp.sample_time)]);
			}
			for (int i = 0; i < bp.varying_num; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_varying + to_string(i + 1)) + 1 + j * bp.variable), comfort_c * comlv.weighting[i + irl.number + bp.uninterrupt_num][(j + bp.sample_time)]);
			}
		}
	}
	if (dr.mode != 0)
	{
		if (bp.sample_time - dr.startTime >= 0)
		{
			for (int j = 0; j < dr.endTime - bp.sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + j * bp.variable), (price[j + bp.sample_time] + participate_array[j + (bp.sample_time - dr.startTime)] * dr.feedback_price) * bp.delta_T);
			}
		}
		else if (bp.sample_time - dr.startTime < 0)
		{
			for (int j = dr.startTime - bp.sample_time; j < dr.endTime - bp.sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + j * bp.variable), (price[j + bp.sample_time] + participate_array[j - (dr.startTime - bp.sample_time)] * dr.feedback_price) * bp.delta_T);
			}
		}
	}
}