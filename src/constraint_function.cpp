#include "optimize.hpp"

// common
void optimize::previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(int row_num_maxAddition)
{
    float socThreshold_trans = (rxipt.bp.SOC_threshold-rxipt.bp.SOC_ini)*rxipt.bp.Cbat*rxipt.bp.Vsys/rxipt.bp.delta_T;
    float socMax_trans = (0.99-rxipt.bp.SOC_ini)*rxipt.bp.Cbat*rxipt.bp.Vsys/rxipt.bp.delta_T;
    for (int i = 0; i < remain_timeblock; i++)
	{
		coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "Pess")] = 1.0;
	}
	glp_set_row_name(mip, bnd_row_num, "");
	if (sample_time == 0)
		glp_set_row_bnds(mip, bnd_row_num, GLP_LO, socThreshold_trans, 0.0);

	else
		glp_set_row_bnds(mip, bnd_row_num, GLP_DB, socThreshold_trans, socMax_trans);
	// avoid the row max is bigger than SOC max

	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::previousSOCPlusPessTransToSOC_equalTo_currentSOC(int row_num_maxAddition)
{
    float trans = rxipt.bp.Cbat*rxipt.bp.Vsys/rxipt.bp.delta_T;
    for (int i = 0; i < remain_timeblock; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			coefficient[coef_row_num + i][j * variable_num + find_variableName_position(variable_name, "Pess")] = -1.0;
		}
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "SOC")] = trans;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, rxipt.bp.SOC_ini*trans, rxipt.bp.SOC_ini*trans);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pessPositive_smallerThan_zMultiplyByPchargeMax(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
	{
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pcharge")] = 1.0;
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Z")] = -rxipt.bp.Pbat_max;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
	{
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pdischarge")] = 1.0;
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Z")] = rxipt.bp.Pbat_min;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, rxipt.bp.Pbat_min);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pessPositiveMinusPessNegative_equalTo_Pess(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
	{
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pess")] = 1.0;
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pcharge")] = -1.0;
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pdischarge")] = 1.0;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// hems
void optimize::summation_interruptLoadRa_biggerThan_Qa(int row_num_maxAddition)
{
    for (int h = 0; h < rxipt.irl.load_num; h++)
	{
		if ((rxipt.irl.end[h] - sample_time) >= 0)
		{
			if ((rxipt.irl.start[h] - sample_time) >= 0)
			{
				for (int i = (rxipt.irl.start[h] - sample_time); i <= (rxipt.irl.end[h] - sample_time); i++)
				{
					coefficient[coef_row_num + h][i * variable_num + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = 1.0;
				}
			}
			else if ((rxipt.irl.start[h] - sample_time) < 0)
			{
				for (int i = 0; i <= (rxipt.irl.end[h] - sample_time); i++)
				{
					coefficient[coef_row_num + h][i * variable_num + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = 1.0;
				}
			}
		}
		glp_set_row_name(mip, bnd_row_num + h, "");
		glp_set_row_bnds(mip, bnd_row_num + h, GLP_LO, ((float)rxipt.irl.reot[h]), 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pgrid_smallerThan_alphaPgridMax(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
	{
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pgrid")] = 1.0;
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "dr_alpha")] = -rxipt.bp.Pgrid_max;

		glp_set_row_name(mip, bnd_row_num + i, "");
		glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, 0.0);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::alpha_between_oneminusDu_and_one(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
	{
		coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "dr_alpha")] = 1.0;

		glp_set_row_name(mip, bnd_row_num + i, "");
		glp_set_row_bnds(mip, bnd_row_num + i, GLP_FX, 1.0, 1.0);
		if (sample_time + i < rxipt.dr.endTime)
		{
			if (sample_time + i >= rxipt.dr.startTime)
			{
				if (1 - rxipt.dr.participate_status[sample_time + i - rxipt.dr.startTime] == 0)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_DB, (1 - rxipt.dr.participate_status[sample_time + i - rxipt.dr.startTime]), 1.0);
					// glp_set_row_bnds(mip, bnd_row_num + i, GLP_DB, 0.0, 1.0);
				}
			}
		}
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(int row_num_maxAddition)
{
    if (rxipt.fg.interrupt)
	{
		for (int h = 0; h < rxipt.irl.load_num; h++)
		{
			if ((rxipt.irl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.irl.start[h] - sample_time) >= 0)
				{
					for (int i = (rxipt.irl.start[h] - sample_time); i <= (rxipt.irl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = rxipt.irl.power[h];
					}
				}
				else if ((rxipt.irl.start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (rxipt.irl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "interrupt" + to_string(h + 1))] = rxipt.irl.power[h];
					}
				}
			}
		}
	}
	if (rxipt.fg.uninterrupt)
	{
		for (int h = 0; h < rxipt.uirl.load_num; h++)
		{
			if ((rxipt.uirl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.uirl.start[h] - sample_time) >= 0)
				{
					for (int i = (rxipt.uirl.start[h] - sample_time); i <= (rxipt.uirl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = rxipt.uirl.power[h];
					}
				}
				else if ((rxipt.uirl.start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (rxipt.uirl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = rxipt.uirl.power[h];
					}
				}
			}
		}
	}
	if (rxipt.fg.varying)
	{
		for (int h = 0; h < rxipt.varl.load_num; h++)
		{
			if ((rxipt.varl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.varl.start[h] - sample_time) >= 0)
				{
					for (int i = (rxipt.varl.start[h] - sample_time); i <= (rxipt.varl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((rxipt.varl.start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (rxipt.varl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
			}
		}
	}
	for (int i = 0; i < remain_timeblock; i++)
	{
		if (rxipt.fg.Pgrid)
			coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pgrid")] = -1.0;
		if (rxipt.fg.Pess)
			coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pess")] = 1.0;

		glp_set_row_name(mip, (bnd_row_num + i), "");
		glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
		// glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, -uncontrollable_load[i + sample_time], -uncontrollable_load[i + sample_time]);
	}
	coef_row_num += row_num_maxAddition;
	bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::summation_uninterruptDelta_equalTo_one(int row_num_maxAddition)
{
    for (int h = 0; h < rxipt.uirl.load_num; h++)
	{
		if (rxipt.uirl.continue_flag[h] == 0)
		{
			if ((rxipt.uirl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.uirl.start[h] - sample_time) >= 0)
				{
					for (int i = (rxipt.uirl.start[h] - sample_time); i <= ((rxipt.uirl.end[h] - rxipt.uirl.reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((rxipt.uirl.start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= ((rxipt.uirl.end[h] - rxipt.uirl.reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = 1.0;
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

void optimize::uninterruptRajToN_biggerThan_uninterruptDelta(int row_num_maxAddition)
{
    for (int h = 0; h < rxipt.uirl.load_num; h++)
	{
		if (rxipt.uirl.continue_flag[h] == 0)
		{
			for (int m = 0; m < rxipt.uirl.reot[h]; m++)
			{
				if ((rxipt.uirl.end[h] - sample_time) >= 0)
				{
					if ((rxipt.uirl.start[h] - sample_time) >= 0)
					{
						for (int i = (rxipt.uirl.start[h] - sample_time); i <= ((rxipt.uirl.end[h] - rxipt.uirl.reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + remain_timeblock * m + i][(i + m) * variable_num + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + remain_timeblock * m + i][i * variable_num + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = -1.0;
						}
					}
					else if ((rxipt.uirl.start[h] - sample_time) < 0)
					{
						for (int i = 0; i <= ((rxipt.uirl.end[h] - rxipt.uirl.reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + remain_timeblock * m + i][(i + m) * variable_num + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + remain_timeblock * m + i][i * variable_num + find_variableName_position(variable_name, "uninterDelta" + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < remain_timeblock; i++)
					{
						glp_set_row_name(mip, bnd_row_num + remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bnd_row_num + remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			coef_row_num += row_num_maxAddition * rxipt.uirl.reot[h];
			bnd_row_num += row_num_maxAddition * rxipt.uirl.reot[h];
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition*rxipt.uirl.reot[h], bnd_row_num, row_num_maxAddition*rxipt.uirl.reot[h]);
		}
		if (rxipt.uirl.continue_flag[h] == 1)
		{
			if ((rxipt.uirl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.uirl.start[h] - sample_time) <= 0)
				{
					for (int i = 0; i <= (rxipt.uirl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "uninterrupt" + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < rxipt.uirl.reot[h]; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = rxipt.uirl.reot[h]; i < remain_timeblock; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
				coef_row_num += row_num_maxAddition;
				bnd_row_num += row_num_maxAddition;
				saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition*rxipt.uirl.reot[h]);
			}
		}
	}
}

void optimize::summation_varyingDelta_equalTo_one(int row_num_maxAddition)
{
    for (int h = 0; h < rxipt.varl.load_num; h++)
	{
		if (rxipt.varl.continue_flag[h] == 0)
		{
			if ((rxipt.varl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.varl.start[h] - sample_time) >= 0)
				{
					for (int i = (rxipt.varl.start[h] - sample_time); i <= ((rxipt.varl.end[h] - rxipt.varl.reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((rxipt.varl.start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= ((rxipt.varl.end[h] - rxipt.varl.reot[h] + 1) - sample_time); i++)
					{
						coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = 1.0;
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

void optimize::varyingRajToN_biggerThan_varyingDelta(int row_num_maxAddition)
{
    for (int h = 0; h < rxipt.varl.load_num; h++)
	{
		if (rxipt.varl.continue_flag[h] == 0)
		{
			for (int m = 0; m < rxipt.varl.reot[h]; m++)
			{
				if ((rxipt.varl.end[h] - sample_time) >= 0)
				{
					if ((rxipt.varl.start[h] - sample_time) >= 0)
					{
						for (int i = (rxipt.varl.start[h] - sample_time); i <= ((rxipt.varl.end[h] - rxipt.varl.reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + remain_timeblock * m + i][(i + m) * variable_num + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + remain_timeblock * m + i][i * variable_num + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0;
						}
					}
					else if ((rxipt.varl.start[h] - sample_time) < 0)
					{
						for (int i = 0; i <= ((rxipt.varl.end[h] - rxipt.varl.reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + remain_timeblock * m + i][(i + m) * variable_num + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = 1.0;
							coefficient[coef_row_num + remain_timeblock * m + i][i * variable_num + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0;
						}
					}
					for (int i = 0; i < remain_timeblock; i++)
					{
						glp_set_row_name(mip, bnd_row_num + remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bnd_row_num + remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			coef_row_num += row_num_maxAddition * rxipt.varl.reot[h];
			bnd_row_num += row_num_maxAddition * rxipt.varl.reot[h];
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition*rxipt.varl.reot[h]);
		}
		if (rxipt.varl.continue_flag[h] == 1)
		{
			if ((rxipt.varl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.varl.start[h] - sample_time) <= 0)
				{
					for (int i = 0; i <= (rxipt.varl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < rxipt.varl.reot[h]; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 1.0, 1.0);
				}
				for (int i = rxipt.varl.reot[h]; i < remain_timeblock; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
				coef_row_num += row_num_maxAddition;
				bnd_row_num += row_num_maxAddition;
				saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition*rxipt.varl.reot[h]);
			}
		}
	}
}

void optimize::varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(int row_num_maxAddition)
{
    for (int h = 0; h < rxipt.varl.load_num; h++)
	{
		if (rxipt.varl.continue_flag[h] == 0)
		{
			for (int m = 0; m < rxipt.varl.reot[h]; m++)
			{
				if ((rxipt.varl.end[h] - sample_time) >= 0)
				{
					if ((rxipt.varl.start[h] - sample_time) >= 0)
					{
						for (int i = (rxipt.varl.start[h] - sample_time); i <= ((rxipt.varl.end[h] - rxipt.varl.reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + remain_timeblock * m + i][(i * variable_num) + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0 * (((float)rxipt.varl.block[h][i]) * (rxipt.varl.power[h][m]));
							coefficient[coef_row_num + remain_timeblock * m + i][((i + m) * variable_num) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
						}
					}
					else if ((rxipt.varl.start[h] - sample_time) < 0)
					{
						for (int i = 0; i <= ((rxipt.varl.end[h] - rxipt.varl.reot[h] + 1) - sample_time); i++)
						{
							coefficient[coef_row_num + remain_timeblock * m + i][(i * variable_num) + find_variableName_position(variable_name, "varyingDelta" + to_string(h + 1))] = -1.0 * (((float)rxipt.varl.block[h][i]) * (rxipt.varl.power[h][m]));
							coefficient[coef_row_num + remain_timeblock * m + i][((i + m) * variable_num) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
						}
					}
					for (int i = 0; i < remain_timeblock; i++)
					{
						glp_set_row_name(mip, bnd_row_num + remain_timeblock * m + i, "");
						glp_set_row_bnds(mip, bnd_row_num + remain_timeblock * m + i, GLP_LO, 0.0, 0.0);
					}
				}
			}
			coef_row_num += row_num_maxAddition * rxipt.varl.reot[h];
			bnd_row_num += row_num_maxAddition * rxipt.varl.reot[h];
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition*rxipt.varl.reot[h]);
		}
		if (rxipt.varl.continue_flag[h] == 1)
		{
			if ((rxipt.varl.end[h] - sample_time) >= 0)
			{
				if ((rxipt.varl.start[h] - sample_time) >= 0)
				{
					for (int i = (rxipt.varl.start[h] - sample_time); i <= (rxipt.varl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][(i * variable_num) + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = -1.0 * ((float)(rxipt.varl.block[h][i]) * (rxipt.varl.power[h][i + rxipt.varl.alreadyot[h]]));
						coefficient[coef_row_num + i][(i * variable_num) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
				else if ((rxipt.varl.start[h] - sample_time) < 0)
				{
					for (int i = 0; i <= (rxipt.varl.end[h] - sample_time); i++)
					{
						coefficient[coef_row_num + i][(i * variable_num) + find_variableName_position(variable_name, "varying" + to_string(h + 1))] = -1.0 * ((float)(rxipt.varl.block[h][i]) * (rxipt.varl.power[h][i + rxipt.varl.alreadyot[h]]));
						coefficient[coef_row_num + i][(i * variable_num) + find_variableName_position(variable_name, "varyingPsi" + to_string(h + 1))] = 1.0;
					}
				}
				for (int i = 0; i < remain_timeblock; i++)
				{
					glp_set_row_name(mip, bnd_row_num + i, "");
					glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, 0.0, 0.0);
				}
			}
			coef_row_num += row_num_maxAddition;
			bnd_row_num += row_num_maxAddition;
			saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition*rxipt.varl.reot[h]);
		}
	}
}

// cems
void optimize::summation_publicLoadRa_biggerThan_QaMinusD(int row_num_maxAddition)
{
    for (int i = 0; i < rxipt.pl.load_num; i++)
    {
        if ((rxipt.pl.end[i] - sample_time) >= 0)
        {
            if ((rxipt.pl.start[i] - sample_time) >= 0)
            {
                for (int j = (rxipt.pl.start[i] - sample_time); j <= (rxipt.pl.end[i] - sample_time); j++)
                {
                    coefficient[coef_row_num + i][j * variable_num + find_variableName_position(variable_name, "publicLoad" + to_string(i + 1))] = 1.0;
                }
            }
            else if ((rxipt.pl.start[i] - sample_time) < 0)
            {
                for (int j = 0; j <= (rxipt.pl.end[i] - sample_time); j++)
                {
                    coefficient[coef_row_num + i][j * variable_num + find_variableName_position(variable_name, "publicLoad" + to_string(i + 1))] = 1.0;
                }
            }
            if (rxipt.fg.dr_mode != 0)
            {
                if (rxipt.dr.endTime - sample_time >= 0)
                {
                    if ((rxipt.dr.startTime - sample_time) >= 0)
                    {
                        for (int j = (rxipt.dr.startTime - sample_time); j < (rxipt.dr.endTime - sample_time); j++)
                        {
                            coefficient[coef_row_num + i][j * variable_num + find_variableName_position(variable_name, "publicLoad" + to_string(i + 1))] = 0.0;
                        }
                    }
                    else if ((rxipt.dr.startTime - sample_time) < 0)
                    {
                        for (int j = 0; j < (rxipt.dr.endTime - sample_time); j++)
                        {
                            coefficient[coef_row_num + i][j * variable_num + find_variableName_position(variable_name, "publicLoad" + to_string(i + 1))] = 0.0;
                        }
                    }
                }
            }
        }        
        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, ((float)rxipt.pl.reot[i]), 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pgrid_smallerThan_muGridMultiplyByPgridMaxArray(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
    {
        coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pgrid")] = 1.0;
        if (rxipt.fg.dr_mode != 0)
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "mu_grid")] = -rxipt.dr.Pgrid_max_array[i];
        else
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "mu_grid")] = -rxipt.bp.Pgrid_max;

        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
    {
        coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Psell")] = 1.0;
        coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "mu_grid")] = rxipt.bp.Psell_max;

        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, rxipt.bp.Psell_max);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::psell_smallerThan_PfuelCellPlusPsolar(int row_num_maxAddition)
{
    for (int i = 0; i < remain_timeblock; i++)
    {
        coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Psell")] = 1.0;
        if (rxipt.fg.Pfc)
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pfc")] = -1.0;

        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, rxipt.bp.weather[i + sample_time]);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoad(int row_num_maxAddition)
{
    if (rxipt.fg.publicLoad)
    {
        for (int h = 0; h < rxipt.pl.load_num; h++)
        {
            if ((rxipt.pl.end[h] - sample_time) >= 0)
            {
                if ((rxipt.pl.start[h] - sample_time) >= 0)
                {
                    for (int i = (rxipt.pl.start[h] - sample_time); i <= (rxipt.pl.end[h] - sample_time); i++)
                    {
                        coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "publicLoad" + to_string(h + 1))] = rxipt.pl.power[h];
                    }
                }
                else if ((rxipt.pl.start[h] - sample_time) < 0)
                {
                    for (int i = 0; i <= (rxipt.pl.end[h] - sample_time); i++)
                    {
                        coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "publicLoad" + to_string(h + 1))] = rxipt.pl.power[h];
                    }
                }
            }
        }
    }

    for (int i = 0; i < remain_timeblock; i++)
    {
        if (rxipt.fg.Pgrid)
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pgrid")] = -1.0;
        if (rxipt.fg.Pess)
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pess")] = 1.0;
        if (rxipt.fg.Pfc)
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Pfc")] = -1.0;
        if (rxipt.fg.Psell)
            coefficient[coef_row_num + i][i * variable_num + find_variableName_position(variable_name, "Psell")] = 1.0;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        if (rxipt.bp.weather[i + sample_time] - rxipt.bp.load_model[i + sample_time] < 0)
            glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, rxipt.bp.weather[i + sample_time] - rxipt.bp.load_model[i + sample_time], rxipt.bp.weather[i + sample_time] - rxipt.bp.load_model[i + sample_time]);
        else
            glp_set_row_bnds(mip, (bnd_row_num + i), GLP_DB, -0.0001, rxipt.bp.weather[i + sample_time] - rxipt.bp.load_model[i + sample_time]);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void optimize::targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(int row_num_maxAddition)
{
    float dr_sumOfCBL = 0.0;
    if (rxipt.dr.startTime - sample_time >= 0)
    {
        for (int i = (rxipt.dr.startTime - sample_time); i < (rxipt.dr.endTime - sample_time); i++)
        {
            coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "Pgrid")] = -1.0 * rxipt.bp.delta_T;
            dr_sumOfCBL += rxipt.dr.customer_baseLine * rxipt.bp.delta_T;
        }
    }
    else
    {
        for (int i = 0; i < (rxipt.dr.endTime - sample_time); i++)
        {
            coefficient[coef_row_num][i * variable_num + find_variableName_position(variable_name, "Pgrid")] = -1.0 * rxipt.bp.delta_T;
            dr_sumOfCBL += rxipt.dr.customer_baseLine * rxipt.bp.delta_T;
        }
        dr_sumOfCBL += rxipt.dr.already_decrease_power;
    }
    glp_set_row_name(mip, bnd_row_num, "");
    glp_set_row_bnds(mip, bnd_row_num, GLP_LO, rxipt.dr.customer_baseLine - dr_sumOfCBL, 0.0);
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

/* temporary won't use
// fuel cell
void optimize::pfcOnPlusPfcOff_equalTo_pfuelCell(int row_num_maxAddition){}
void optimize::pfcOn_smallerThan_mufcMultiplyByPfcMax(int row_num_maxAddition){}
void optimize::pfcOn_biggerThan_mufcMultiplyByPfcMin(int row_num_maxAddition){}
void optimize::pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(int row_num_maxAddition){}
void optimize::pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(int row_num_maxAddition){}
void optimize::pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(int row_num_maxAddition){}
void optimize::zPfcOnePlusZPfcTwo_etc_equalTo_one(int row_num_maxAddition){}
void optimize::lambdaPfc_smallerThan_zpfc(int row_num_maxAddition){}
// soc change
void optimize::SOCPositiveMinusSOCNegative_equalTo_SOCchange(int row_num_maxAddition){}
void optimize::SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(int row_num_maxAddition){}
void optimize::SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(int row_num_maxAddition){}
void optimize::SOCchange_equalTo_PessTransToSOC(int row_num_maxAddition){}
void optimize::summation_SOCNegative_biggerThan_targetDischargeSOC(int row_num_maxAddition){}
*/