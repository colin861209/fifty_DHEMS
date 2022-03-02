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

// =-=-=-=-=-=-=- balanced equation -=-=-=-=-=-=-= //
void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float *uncontrollable_load, float **coefficient, glp_prob *mip, int row_num_maxAddition)
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
	if (uirl.flag)
	{
		for (int h = 0; h < uirl.number; h++)
		{
			if ((uirl.end[h] - bp.sample_time) >= 0)
			{
				if ((uirl.start[h] - bp.sample_time) >= 0)
				{
					for (int i = (uirl.start[h] - bp.sample_time); i <= (uirl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(h + 1))] = uirl.power[h];
					}
				}
				else if ((uirl.start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (uirl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(h + 1))] = uirl.power[h];
					}
				}
			}
		}
	}
	if (varl.flag)
	{
		for (int h = 0; h < varl.number; h++)
		{
			if ((varl.end[h] - bp.sample_time) >= 0)
			{
				if ((varl.start[h] - bp.sample_time) >= 0)
				{
					for (int i = (varl.start[h] - bp.sample_time); i <= (varl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varl.start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (varl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(h + 1))] = 1.0;
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
void summation_uninterruptDelta_equalTo_one(UNINTERRUPTLOAD uirl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < uirl.number; h++)
	{
		if (uirl.continuous_flag[h] == 0)
		{
			if ((uirl.end[h] - bp.sample_time) >= 0)
			{
				if ((uirl.start[h] - bp.sample_time) >= 0)
				{
					for (int i = (uirl.start[h] - bp.sample_time); i <= ((uirl.end[h] - uirl.reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterDelta + to_string(h + 1))] = 1.0;
					}
				}
				else if ((uirl.start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= ((uirl.end[h] - uirl.reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterDelta + to_string(h + 1))] = 1.0;
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

void uninterruptRajToN_biggerThan_uninterruptDelta(UNINTERRUPTLOAD uirl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < uirl.number; h++)
	{
		if (uirl.continuous_flag[h] == 0)
		{
			for (int m = 0; m < uirl.reot[h]; m++)
			{
				if ((uirl.end[h] - bp.sample_time) >= 0)
				{
					if ((uirl.start[h] - bp.sample_time) >= 0)
					{
						for (int i = (uirl.start[h] - bp.sample_time); i <= ((uirl.end[h] - uirl.reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterDelta + to_string(h + 1))] = -1.0;
						}
					}
					else if ((uirl.start[h] - bp.sample_time) < 0)
					{
						for (int i = 0; i <= ((uirl.end[h] - uirl.reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterDelta + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < bp.remain_timeblock; i++)
					{
						glp_set_row_name(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			bp.coef_row_num += row_num_maxAddition * uirl.reot[h];
			bp.bnd_row_num += row_num_maxAddition * uirl.reot[h];
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition * uirl.reot[h], bp.bnd_row_num, row_num_maxAddition * uirl.reot[h]);
		}
		if (uirl.continuous_flag[h] == 1)
		{
			if ((uirl.end[h] - bp.sample_time) >= 0)
			{
				if ((uirl.start[h] - bp.sample_time) <= 0)
				{
					for (int i = 0; i <= (uirl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < uirl.reot[h]; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = uirl.reot[h]; i < bp.remain_timeblock; i++)
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
void summation_varyingDelta_equalTo_one(VARYINGLOAD varl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < varl.number; h++)
	{
		if (varl.continuous_flag[h] == 0)
		{
			if ((varl.end[h] - bp.sample_time) >= 0)
			{
				if ((varl.start[h] - bp.sample_time) >= 0)
				{
					for (int i = (varl.start[h] - bp.sample_time); i <= ((varl.end[h] - varl.reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varl.start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= ((varl.end[h] - varl.reot[h] + 1) - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(h + 1))] = 1.0;
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

void varyingRajToN_biggerThan_varyingDelta(VARYINGLOAD varl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < varl.number; h++)
	{
		if (varl.continuous_flag[h] == 0)
		{
			for (int m = 0; m < varl.reot[h]; m++)
			{
				if ((varl.end[h] - bp.sample_time) >= 0)
				{
					if ((varl.start[h] - bp.sample_time) >= 0)
					{
						for (int i = (varl.start[h] - bp.sample_time); i <= ((varl.end[h] - varl.reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, varl.str_varying + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(h + 1))] = -1.0;
						}
					}
					else if ((varl.start[h] - bp.sample_time) < 0)
					{
						for (int i = 0; i <= ((varl.end[h] - varl.reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i + m) * bp.variable + find_variableName_position(bp.variable_name, varl.str_varying + to_string(h + 1))] = 1.0;
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < bp.remain_timeblock; i++)
					{
						glp_set_row_name(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			bp.coef_row_num += row_num_maxAddition * varl.reot[h];
			bp.bnd_row_num += row_num_maxAddition * varl.reot[h];
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition * varl.reot[h], bp.bnd_row_num, row_num_maxAddition * varl.reot[h]);
		}
		if (varl.continuous_flag[h] == 1)
		{
			if ((varl.end[h] - bp.sample_time) >= 0)
			{
				if ((varl.start[h] - bp.sample_time) <= 0)
				{
					for (int i = 0; i <= (varl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, varl.str_varying + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < varl.reot[h]; i++)
				{
					glp_set_row_name(mip, bp.bnd_row_num + i, "");
					glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = varl.reot[h]; i < bp.remain_timeblock; i++)
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

void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(VARYINGLOAD varl, BASEPARAMETER &bp, int *buff, int buff_shift_length, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < varl.number; h++)
	{
		if (varl.continuous_flag[h] == 0)
		{
			for (int m = 0; m < varl.reot[h]; m++)
			{
				if ((varl.end[h] - bp.sample_time) >= 0)
				{
					if ((varl.start[h] - bp.sample_time) >= 0)
					{
						for (int i = (varl.start[h] - bp.sample_time); i <= ((varl.end[h] - varl.reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(h + 1))] = -1.0 * (((float)varl.block[h][i]) * (varl.power[h][m]));
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][((i + m) * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(h + 1))] = 1.0;
						}
					}
					else if ((varl.start[h] - bp.sample_time) < 0)
					{
						for (int i = 0; i <= ((varl.end[h] - varl.reot[h] + 1) - bp.sample_time); i++)
						{
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][(i * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varyingDelta + to_string(h + 1))] = -1.0 * (((float)varl.block[h][i]) * (varl.power[h][m]));
							coefficient[bp.coef_row_num + bp.remain_timeblock * m + i][((i + m) * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(h + 1))] = 1.0;
						}
					}
					for (int i = 0; i < bp.remain_timeblock; i++)
					{
						glp_set_row_name(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bp.bnd_row_num + bp.remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			bp.coef_row_num += row_num_maxAddition * varl.reot[h];
			bp.bnd_row_num += row_num_maxAddition * varl.reot[h];
			saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition * varl.reot[h], bp.bnd_row_num, row_num_maxAddition * varl.reot[h]);
		}
		if (varl.continuous_flag[h] == 1)
		{
			if ((varl.end[h] - bp.sample_time) >= 0)
			{
				if ((varl.start[h] - bp.sample_time) >= 0)
				{
					for (int i = (varl.start[h] - bp.sample_time); i <= (varl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varying + to_string(h + 1))] = -1.0 * ((float)(varl.block[h][i]) * (varl.power[h][i + buff[h + buff_shift_length]]));
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varl.start[h] - bp.sample_time) < 0)
				{
					for (int i = 0; i <= (varl.end[h] - bp.sample_time); i++)
					{
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varying + to_string(h + 1))] = -1.0 * ((float)(varl.block[h][i]) * (varl.power[h][i + buff[h + buff_shift_length]]));
						coefficient[bp.coef_row_num + i][(i * bp.variable) + find_variableName_position(bp.variable_name, varl.str_varyingPsi + to_string(h + 1))] = 1.0;
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
void setting_LHEMS_objectiveFunction(INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, BASEPARAMETER bp, DEMANDRESPONSE dr, COMFORTLEVEL comlv, float* price, int *participate_array, glp_prob *mip)
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
			for (int i = 0; i < uirl.number; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, uirl.str_uninterrupt + to_string(i + 1)) + 1 + j * bp.variable), comfort_c * comlv.weighting[i + irl.number][(j + bp.sample_time)]);
			}
			for (int i = 0; i < varl.number; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, varl.str_varying + to_string(i + 1)) + 1 + j * bp.variable), comfort_c * comlv.weighting[i + irl.number + uirl.number][(j + bp.sample_time)]);
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