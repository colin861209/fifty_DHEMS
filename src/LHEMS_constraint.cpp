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
void summation_interruptLoadRa_biggerThan_Qa(int *interrupt_start, int *interrupt_end, int *interrupt_reot, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < interrupt_num; h++)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (int i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					coefficient[coef_row_num + h][i * variable + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = 1.0;
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (int i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					coefficient[coef_row_num + h][i * variable + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = 1.0;
				}
			}
		}
		glp_set_row_name(mip, bnd_row_num + h, "");
		glp_set_row_bnds(mip, bnd_row_num + h, GLP_LO, ((float)interrupt_reot[h]), 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- demand response -=-=-=-=-=-=-= //
void pgrid_smallerThan_alphaPgridMax(DEMANDRESPONSE dr, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pgrid")] = 1.0;
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, dr.str_alpha)] = -Pgrid_max;

		glp_set_row_name(mip, bnd_row_num + i, "");
		glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void alpha_between_oneminusDu_and_one(DEMANDRESPONSE dr, int *participate_array, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, dr.str_alpha)] = 1.0;

		glp_set_row_name(mip, bnd_row_num + i, "");
		glp_set_row_bnds(mip, bnd_row_num + i, GLP_FX, 1.0, 1.0);
		if (sample_time + i < dr.endTime)
		{
			if (sample_time + i >= dr.startTime)
			{
				if (1 - participate_array[sample_time + i - dr.startTime] == 0)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_DB, (1 - participate_array[sample_time + i - dr.startTime]), 1.0);
					// glp_set_row_bnds(mip, bnd_row_num + i, GLP_DB, 0.0, 1.0);
				}
			}
		}
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- balanced equation -=-=-=-=-=-=-= //
void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(ENERGYSTORAGESYSTEM ess, int *interrupt_start, int *interrupt_end, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, float *uninterrupt_p, int *varying_start, int *varying_end, float *uncontrollable_load, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	if (interruptLoad_flag)
	{
		for (int h = 0; h < interrupt_num; h++)
		{
			if ((interrupt_end[h] - sample_time) >= 0)
			{
				if ((interrupt_start[h] - sample_time) >= 0)
				{
					for (int i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = interrupt_p[h];
					}
				}
				else if ((interrupt_start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (interrupt_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = interrupt_p[h];
					}
				}
			}
		}
	}
	if (uninterruptLoad_flag)
	{
		for (int h = 0; h < uninterrupt_num; h++)
		{
			if ((uninterrupt_end[h] - sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - sample_time) >= 0)
				{
					for (int i = (uninterrupt_start[h] - sample_time); i <= (uninterrupt_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = uninterrupt_p[h];
					}
				}
				else if ((uninterrupt_start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = uninterrupt_p[h];
					}
				}
			}
		}
	}
	if (varyingLoad_flag)
	{
		for (int h = 0; h < varying_num; h++)
		{
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (int i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
			}
		}
	}
	for (int i = 0; i < (time_block - sample_time); i++)
	{
		if (Pgrid_flag)
			coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pgrid")] = -1.0;
		if (ess.flag)
			coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Pess)] = 1.0;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, -uncontrollable_load[i + sample_time], -uncontrollable_load[i + sample_time]);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- battery -=-=-=-=-=-=-= //
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, ess.str_Pess)] = 1.0;
	}
	glp_set_row_name(mip, bnd_row_num, "");
	if (sample_time == 0)
		glp_set_row_bnds(mip, bnd_row_num, GLP_LO, ((ess.threshold_SOC - ess.INIT_SOC) * ess.capacity * ess.voltage) / delta_T, 0.0);

	else
		glp_set_row_bnds(mip, bnd_row_num, GLP_DB, ((ess.threshold_SOC - ess.INIT_SOC) * ess.capacity * ess.voltage) / delta_T, ((0.99 - ess.INIT_SOC) * ess.capacity * ess.voltage) / delta_T);
	// avoid the row max is bigger than SOC max

	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void previousSOCPlusPessTransToSOC_equalTo_currentSOC(ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		for (int j = 0; j <= i; j++)
		{
			coefficient[coef_row_num + i][j * variable + find_variableName_position(variable_name, ess.str_Pess)] = -1.0;
		}
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_SOC)] = ess.capacity * ess.voltage / delta_T;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, (ess.INIT_SOC * ess.capacity * ess.voltage / delta_T), (ess.INIT_SOC * ess.capacity * ess.voltage / delta_T));
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pessPositive_smallerThan_zMultiplyByPchargeMax(ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Pcharge)] = 1.0;
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Z)] = -ess.MAX_power;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Pdischarge)] = 1.0;
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Z)] = ess.MIN_power;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, ess.MIN_power);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pessPositiveMinusPessNegative_equalTo_Pess(ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < (time_block - sample_time); i++)
	{
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Pess)] = 1.0;
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Pcharge)] = -1.0;
		coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, ess.str_Pdischarge)] = 1.0;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- uninterrupt load -=-=-=-=-=-=-= //
void summation_uninterruptDelta_equalTo_one(int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			if ((uninterrupt_end[h] - sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - sample_time) >= 0)
				{
					for (int i = (uninterrupt_start[h] - sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((uninterrupt_start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = 1.0;
					}
				}
			}
			glp_set_row_name(mip, bnd_row_num, "");
			glp_set_row_bnds(mip, bnd_row_num, GLP_FX, 1.0, 1.0);

			coef_row_num += row_num_maxAddition;
			bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
		}
	}
}

void uninterruptRajToN_biggerThan_uninterruptDelta(int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			for (int m = 0; m < uninterrupt_reot[h]; m++)
			{
				if ((uninterrupt_end[h] - sample_time) >= 0)
				{
					if ((uninterrupt_start[h] - sample_time) >= 0)
					{
						for (int i = (uninterrupt_start[h] - sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + (time_block - sample_time) * m + i][(i + m) * variable + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + (time_block - sample_time) * m + i][i * variable + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = -1.0;
						}
					}
					else if ((uninterrupt_start[h] - sample_time) < 0)
					{
						for (int i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + (time_block - sample_time) * m + i][(i + m) * variable + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + (time_block - sample_time) * m + i][i * variable + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < (time_block - sample_time); i++)
					{
						glp_set_row_name(mip, bnd_row_num + (time_block - sample_time) * m + i, "");
						glp_set_row_bnds(mip, bnd_row_num + (time_block - sample_time) * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			coef_row_num += row_num_maxAddition * uninterrupt_reot[h];
			bnd_row_num += row_num_maxAddition * uninterrupt_reot[h];
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition * uninterrupt_reot[h], bnd_row_num, row_num_maxAddition * uninterrupt_reot[h]);
		}
		if (uninterrupt_flag[h] == 1)
		{
			if ((uninterrupt_end[h] - sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - sample_time) <= 0)
				{
					for (int i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < uninterrupt_reot[h]; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = uninterrupt_reot[h]; i < (time_block - sample_time); i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
				coef_row_num += row_num_maxAddition;
				bnd_row_num += row_num_maxAddition;
				saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
			}
		}
	}
}

// =-=-=-=-=-=-=- varying load -=-=-=-=-=-=-= //
void summation_varyingDelta_equalTo_one(int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (int i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = 1.0;
					}
				}
			}
			glp_set_row_name(mip, bnd_row_num, "");
			glp_set_row_bnds(mip, bnd_row_num, GLP_FX, 1.0, 1.0);

			coef_row_num += row_num_maxAddition;
			bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
		}
	}
}

void varyingRajToN_biggerThan_varyingDelta(int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (int m = 0; m < varying_reot[h]; m++)
			{
				if ((varying_end[h] - sample_time) >= 0)
				{
					if ((varying_start[h] - sample_time) >= 0)
					{
						for (int i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + (time_block - sample_time) * m + i][(i + m) * variable + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + (time_block - sample_time) * m + i][i * variable + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0;
						}
					}
					else if ((varying_start[h] - sample_time) < 0)
					{
						for (int i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + (time_block - sample_time) * m + i][(i + m) * variable + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + (time_block - sample_time) * m + i][i * variable + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < (time_block - sample_time); i++)
					{
						glp_set_row_name(mip, bnd_row_num + (time_block - sample_time) * m + i, "");
						glp_set_row_bnds(mip, bnd_row_num + (time_block - sample_time) * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			coef_row_num += row_num_maxAddition * varying_reot[h];
			bnd_row_num += row_num_maxAddition * varying_reot[h];
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition * varying_reot[h], bnd_row_num, row_num_maxAddition * varying_reot[h]);
		}
		if (varying_flag[h] == 1)
		{
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) <= 0)
				{
					for (int i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < varying_reot[h]; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = varying_reot[h]; i < (time_block - sample_time); i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
				coef_row_num += row_num_maxAddition;
				bnd_row_num += row_num_maxAddition;
				saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
			}
		}
	}
}

void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, int **varying_t_d, float **varying_p_d, int *buff, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (int m = 0; m < varying_reot[h]; m++)
			{
				if ((varying_end[h] - sample_time) >= 0)
				{
					if ((varying_start[h] - sample_time) >= 0)
					{
						for (int i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + (time_block - sample_time) * m + i][(i * variable) + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							coefficient[coef_row_num + (time_block - sample_time) * m + i][((i + m) * variable) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
						}
					}
					else if ((varying_start[h] - sample_time) < 0)
					{
						for (int i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + (time_block - sample_time) * m + i][(i * variable) + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							coefficient[coef_row_num + (time_block - sample_time) * m + i][((i + m) * variable) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
						}
					}
					for (int i = 0; i < (time_block - sample_time); i++)
					{
						glp_set_row_name(mip, bnd_row_num + (time_block - sample_time) * m + i, "");
						glp_set_row_bnds(mip, bnd_row_num + (time_block - sample_time) * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			coef_row_num += row_num_maxAddition * varying_reot[h];
			bnd_row_num += row_num_maxAddition * varying_reot[h];
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition * varying_reot[h], bnd_row_num, row_num_maxAddition * varying_reot[h]);
		}
		if (varying_flag[h] == 1)
		{
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (int i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][(i * variable) + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + uninterrupt_num]]));
						coefficient[coef_row_num + i][(i * variable) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][(i * variable) + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + uninterrupt_num]]));
						coefficient[coef_row_num + i][(i * variable) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < (time_block - sample_time); i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
			}
			coef_row_num += row_num_maxAddition;
			bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
		}
	}
}

// =-=-=-=-=-=-=- objective function -=-=-=-=-=-=-= //
void setting_LHEMS_objectiveFunction(DEMANDRESPONSE dr, COMFORTLEVEL comlv, float* price, int *participate_array, glp_prob *mip)
{
	functionPrint(__func__);
	
	for (int j = 0; j < (time_block - sample_time); j++)
	{
		glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + j * variable), price[j + sample_time] * delta_T);
		
		if (comlv.flag)
		{	
			float comfort_c = 6.6;
			for (int i = 0; i < interrupt_num; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "interrupt" + to_string(i + 1)) + 1 + j * variable), comfort_c * comlv.weighting[i][(j + sample_time)]);
			}
			for (int i = 0; i < uninterrupt_num; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "uninterrupt" + to_string(i + 1)) + 1 + j * variable), comfort_c * comlv.weighting[i + interrupt_num][(j + sample_time)]);
			}
			for (int i = 0; i < varying_num; i++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "varying" + to_string(i + 1)) + 1 + j * variable), comfort_c * comlv.weighting[i + interrupt_num + uninterrupt_num][(j + sample_time)]);
			}
		}
	}
	if (dr.mode != 0)
	{
		if (sample_time - dr.startTime >= 0)
		{
			for (int j = 0; j < dr.endTime - sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + j * variable), (price[j + sample_time] + participate_array[j + (sample_time - dr.startTime)] * dr.feedback_price) * delta_T);
			}
		}
		else if (sample_time - dr.startTime < 0)
		{
			for (int j = dr.startTime - sample_time; j < dr.endTime - sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + j * variable), (price[j + sample_time] + participate_array[j - (dr.startTime - sample_time)] * dr.feedback_price) * delta_T);
			}
		}
	}
}