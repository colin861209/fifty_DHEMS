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
#include "GHEMS_constraint.hpp"

// =-=-=-=-=-=-=- public load -=-=-=-=-=-=-= //
void summation_forceToStopPublicLoadRa_biggerThan_QaMinusD(BASEPARAMETER &bp, DEMANDRESPONSE dr, PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < pl.forceToStop_number; i++)
    {
        if ((pl.forceToStop_end[i] - bp.sample_time) >= 0)
        {
            if ((pl.forceToStop_start[i] - bp.sample_time) >= 0)
            {
                for (int j = (pl.forceToStop_start[i] - bp.sample_time); j <= (pl.forceToStop_end[i] - bp.sample_time); j++)
                {
                    coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(i + 1))] = 1.0;
                }
            }
            else if ((pl.forceToStop_start[i] - bp.sample_time) < 0)
            {
                for (int j = 0; j <= (pl.forceToStop_end[i] - bp.sample_time); j++)
                {
                    coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(i + 1))] = 1.0;
                }
            }
            if (dr.mode != 0)
            {
                if (dr.endTime - bp.sample_time >= 0)
                {
                    if ((dr.startTime - bp.sample_time) >= 0)
                    {
                        for (int j = (dr.startTime - bp.sample_time); j < (dr.endTime - bp.sample_time); j++)
                        {
                            coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(i + 1))] = 0.0;
                        }
                    }
                    else if ((dr.startTime - bp.sample_time) < 0)
                    {
                        for (int j = 0; j < (dr.endTime - bp.sample_time); j++)
                        {
                            coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(i + 1))] = 0.0;
                        }
                    }
                }
            }
        }        
        glp_set_row_name(mip, bp.bnd_row_num + i, "");
        glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, ((float)pl.forceToStop_remain_operation_time[i]), 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void summation_interruptPublicLoadRa_biggerThan_Qa(BASEPARAMETER &bp, DEMANDRESPONSE dr, PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < pl.interrupt_number; i++)
	{
		if ((pl.interrupt_end[i] - bp.sample_time) >= 0)
		{
			if ((pl.interrupt_start[i] - bp.sample_time) >= 0)
			{
				for (int j = (pl.interrupt_start[i] - bp.sample_time); j <= (pl.interrupt_end[i] - bp.sample_time); j++)
				{
					coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_interrupt_publicLoad + to_string(i + 1))] = 1.0;
				}
			}
			else if ((pl.interrupt_start[i] - bp.sample_time) < 0)
			{
				for (int j = 0; j <= (pl.interrupt_end[i] - bp.sample_time); j++)
				{
					coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_interrupt_publicLoad + to_string(i + 1))] = 1.0;
				}
			}
		}
		glp_set_row_name(mip, bp.bnd_row_num + i, "");
		glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, ((float)pl.interrupt_remain_operation_time[i]), 0.0);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void summation_periodicPublicLoadRa_biggerThan_Qa(BASEPARAMETER &bp, DEMANDRESPONSE dr, PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
	functionPrint(__func__);

	for (int i = 0; i < pl.periodic_number; i++)
	{
		if ((pl.periodic_end[i] - bp.sample_time) >= 0)
		{
			if ((pl.periodic_start[i] - bp.sample_time) >= 0)
			{
				for (int j = (pl.periodic_start[i] - bp.sample_time); j <= (pl.periodic_end[i] - bp.sample_time); j++)
				{
					coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_periodic_publicLoad + to_string(i + 1))] = 1.0;
				}
			}
			else if ((pl.periodic_start[i] - bp.sample_time) < 0)
			{
				for (int j = 0; j <= (pl.periodic_end[i] - bp.sample_time); j++)
				{
					coefficient[bp.coef_row_num + i][j * bp.variable + find_variableName_position(bp.variable_name, pl.str_periodic_publicLoad + to_string(i + 1))] = 1.0;
				}
			}
		}
		glp_set_row_name(mip, bp.bnd_row_num + i, "");
		glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_LO, ((float)pl.periodic_remain_operation_time[i]), 0.0);
	}
	bp.coef_row_num += row_num_maxAddition;
	bp.bnd_row_num += row_num_maxAddition;
	saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- grid with demand response or sell -=-=-=-=-=-=-= //
void pgrid_smallerThan_muGridMultiplyByPgridMaxArray(BASEPARAMETER &bp, int dr_mode, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pgrid)] = 1.0;
        if (dr_mode != 0)
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_mu_grid)] = -bp.Pgrid_max_array[i];
        else
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_mu_grid)] = -bp.Pgrid_max;

        glp_set_row_name(mip, bp.bnd_row_num + i, "");
        glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_UP, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- sell -=-=-=-=-=-=-= //
void psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Psell)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_mu_grid)] = bp.Psell_max;

        glp_set_row_name(mip, bp.bnd_row_num + i, "");
        glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_UP, 0.0, bp.Psell_max);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void psell_smallerThan_PfuelCellPlusPsolar(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Psell)] = 1.0;
        if (bp.Pfc_flag)
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pfc)] = -1.0;

        glp_set_row_name(mip, bp.bnd_row_num + i, "");
        glp_set_row_bnds(mip, bp.bnd_row_num + i, GLP_UP, 0.0, bp.solar[i + bp.sample_time]);
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

// =-=-=-=-=-=-=- balanced equation -=-=-=-=-=-=-= //
void pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoadPlusPchargingEMPlusPchargingEV(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, PUBLICLOAD pl, UNCONTROLLABLELOAD ucl, ELECTRICMOTOR em, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    if (pl.flag)
    {
        for (int h = 0; h < pl.forceToStop_number; h++)
        {
            if ((pl.forceToStop_end[h] - bp.sample_time) >= 0)
            {
                if ((pl.forceToStop_start[h] - bp.sample_time) >= 0)
                {
                    for (int i = (pl.forceToStop_start[h] - bp.sample_time); i <= (pl.forceToStop_end[h] - bp.sample_time); i++)
                    {
                        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(h + 1))] = pl.forceToStop_power[h];
                    }
                }
                else if ((pl.forceToStop_start[h] - bp.sample_time) < 0)
                {
                    for (int i = 0; i <= (pl.forceToStop_end[h] - bp.sample_time); i++)
                    {
                        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(h + 1))] = pl.forceToStop_power[h];
                    }
                }
            }
        }
        for (int h = 0; h < pl.interrupt_number; h++)
        {
            if ((pl.interrupt_end[h] - bp.sample_time) >= 0)
            {
                if ((pl.interrupt_start[h] - bp.sample_time) >= 0)
                {
                    for (int i = (pl.interrupt_start[h] - bp.sample_time); i <= (pl.interrupt_end[h] - bp.sample_time); i++)
                    {
                        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, pl.str_interrupt_publicLoad + to_string(h + 1))] = pl.interrupt_power[h];
                    }
                }
                else if ((pl.interrupt_start[h] - bp.sample_time) < 0)
                {
                    for (int i = 0; i <= (pl.interrupt_end[h] - bp.sample_time); i++)
                    {
                        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, pl.str_interrupt_publicLoad + to_string(h + 1))] = pl.interrupt_power[h];
                    }
                }
            }
        }
        for (int h = 0; h < pl.periodic_number; h++)
        {
            if ((pl.periodic_end[h] - bp.sample_time) >= 0)
            {
                if ((pl.periodic_start[h] - bp.sample_time) >= 0)
                {
                    for (int i = (pl.periodic_start[h] - bp.sample_time); i <= (pl.periodic_end[h] - bp.sample_time); i++)
                    {
                        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, pl.str_periodic_publicLoad + to_string(h + 1))] = pl.periodic_power[h];
                    }
                }
                else if ((pl.periodic_start[h] - bp.sample_time) < 0)
                {
                    for (int i = 0; i <= (pl.periodic_end[h] - bp.sample_time); i++)
                    {
                        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, pl.str_periodic_publicLoad + to_string(h + 1))] = pl.periodic_power[h];
                    }
                }
            }
        }
    }

    if (em.flag)
    {
        for (int n = 0; n < em.can_charge_amount; n++)
        {
            for (int i = 0; i < em.departure_timeblock[n] - bp.sample_time; i++)
            {
                coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1))] = em.normal_charging_power;
                if (em.can_discharge)
                    coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1))] = -em.normal_charging_power;
            }
        }
    }
    
    if (ev.flag)
    {
        for (int n = 0; n < ev.can_charge_amount; n++)
        {
            for (int i = 0; i < ev.departure_timeblock[n] - bp.sample_time; i++)
            {
                coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1))] = ev.charging_power;
                if (em.can_discharge)
                    coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1))] = -ev.charging_power;
            }
        }
    }

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        if (bp.Pgrid_flag)
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pgrid)] = -1.0;
        if (ess.flag)
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pess)] = 1.0;
        if (bp.Pfc_flag)
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pfc)] = -1.0;
        if (bp.Psell_flag)
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Psell)] = 1.0;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, bp.solar[i + bp.sample_time] - bp.load_model[i + bp.sample_time] - ucl.power_array[i + bp.sample_time], bp.solar[i + bp.sample_time] - bp.load_model[i + bp.sample_time] - ucl.power_array[i + bp.sample_time]);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- demand response -=-=-=-=-=-=-= //
void targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(BASEPARAMETER &bp, DEMANDRESPONSE dr, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    float dr_sumOfCBL = 0.0;
    if (dr.startTime - bp.sample_time >= 0)
    {
        for (int i = (dr.startTime - bp.sample_time); i < (dr.endTime - bp.sample_time); i++)
        {
            coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pgrid)] = -1.0 * bp.delta_T;
            dr_sumOfCBL += dr.customer_baseLine * bp.delta_T;
        }
    }
    else
    {
        for (int i = 0; i < (dr.endTime - bp.sample_time); i++)
        {
            coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pgrid)] = -1.0 * bp.delta_T;
        }
        for (int i = dr.startTime; i < bp.sample_time; i++)
        {
            snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `GHEMS_control_status` WHERE equip_name = '%s'", i, bp.str_Pgrid.c_str());
            float previous_grid_power = turn_value_to_float(0);
            dr_sumOfCBL += (dr.customer_baseLine - previous_grid_power) * bp.delta_T;
        }
        for (int i = bp.sample_time; i < dr.endTime; i++)
        {
            dr_sumOfCBL += dr.customer_baseLine * bp.delta_T;
        }
    }
    glp_set_row_name(mip, bp.bnd_row_num, "");
    glp_set_row_bnds(mip, bp.bnd_row_num, GLP_LO, dr.minDecrease_power - dr_sumOfCBL, 0.0);
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void summation_EMEVPcharge_smallerThan_PgridPlusPessPlusPpv(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, ELECTRICMOTOR em, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    if (em.flag)
    {
        for (int n = 0; n < em.can_charge_amount; n++)
        {
            for (int i = 0; i < em.departure_timeblock[n] - bp.sample_time; i++)
            {
                coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1))] = em.normal_charging_power;
            }
        }
    }
    
    if (ev.flag)
    {
        for (int n = 0; n < ev.can_charge_amount; n++)
        {
            for (int i = 0; i < ev.departure_timeblock[n] - bp.sample_time; i++)
            {
                coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1))] = ev.charging_power;
            }
        }
    }

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, bp.Pgrid_max_array[i] + ess.MIN_power + bp.solar[i]);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- EM -=-=-=-=-=-=-= //
void EM_Rcharging_smallerThan_mu(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int i = 0; i < bp.remain_timeblock; i++)
    {
    	for (int n = 0; n < em.can_charge_amount; n++)
    	{
    		if (i < em.departure_timeblock[n] - bp.sample_time)
    		{
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1))] = -1;
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_mu + to_string(n + 1))] = 1;

                glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_LO, 0.0, 0.0);
			}
		}
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void EM_Rdischarging_smallerThan_oneMinusMu(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int i = 0; i < bp.remain_timeblock; i++)
    {
    	for (int n = 0; n < em.can_charge_amount; n++)
    	{
    		if (i < em.departure_timeblock[n] - bp.sample_time)
    		{
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1))] = 1;
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_mu + to_string(n + 1))] = 1;

                glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_UP, 0.0, 1.0);
			}
		}
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void EM_RchargeMinusRdischarge_biggerThan_zero(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        for (int n = 0; n < em.can_charge_amount; n++)
        {
            if (i < em.departure_timeblock[n] - bp.sample_time)
    		{
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1))] = 1;
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1))] = -1;

                glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_LO, 0.0, 0.0);
			}   
        }
    }
}

void EM_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int n = 0; n < em.can_charge_amount ; n++)
    {
        for (int i = 0; i < bp.remain_timeblock; i++)
        {
            if (i < em.departure_timeblock[n] - bp.sample_time)
            {
                for (int j = 0; j <= i ; j++)
                {
                    coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][j * bp.variable + find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1))] = 1;
                    if (em.can_discharge)
                        coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][j * bp.variable + find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1))] = -1;
                }
            }
            glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
            glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_LO, (em.MIN_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T), 0.0);
        }
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void EM_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int n = 0; n < em.can_charge_amount ; n++)
    {
        for (int i = 0; i < (em.departure_timeblock[n] - bp.sample_time); i++)
        {
            coefficient[bp.coef_row_num + n][i * bp.variable + find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1))] = 1;
            if (em.can_discharge)
                coefficient[bp.coef_row_num + n][i * bp.variable + find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1))] = -1;
        }
        
        float leastTimeblock_toChargeSOCThreshold = ceil((em.threshold_SOC - em.start_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T));
        if (em.departure_timeblock[n] - em.start_timeblock[n] >= leastTimeblock_toChargeSOCThreshold)
        {
            if (em.can_discharge)
            {   
                // Error handling: all users force charging except already got threshold users,
                // avoid use ceil because if return 2 will got error, but in fact 1 is enough to reach the threshold
                if (em.departure_timeblock[n] - bp.sample_time == 1 && em.now_SOC[n] < em.threshold_SOC)
                {                    
                    glp_set_row_name(mip, (bp.bnd_row_num + n), "");
                    glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_FX, 1.0, 1.0);
                }
                else
                {
                    glp_set_row_name(mip, (bp.bnd_row_num + n), "");
                    glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_LO, ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)), 0.0);
                }
            }
            else
            {
                glp_set_row_name(mip, (bp.bnd_row_num + n), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_LO, ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)), 0.0);
            }
        }
        else
        {
            // avoid EM user setting too less time and EM can't charge to threshold although charging every time
            glp_set_row_name(mip, (bp.bnd_row_num + n), "");
            glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_LO, float(em.departure_timeblock[n] - bp.sample_time), 0.0);
        }
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- EV -=-=-=-=-=-=-= //
void EV_Rcharging_smallerThan_mu(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int i = 0; i < bp.remain_timeblock; i++)
    {
    	for (int n = 0; n < ev.can_charge_amount; n++)
    	{
    		if (i < ev.departure_timeblock[n] - bp.sample_time)
    		{
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1))] = -1;
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_mu + to_string(n + 1))] = 1;

                glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_LO, 0.0, 0.0);
			}
		}
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void EV_Rdischarging_smallerThan_oneMinusMu(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int i = 0; i < bp.remain_timeblock; i++)
    {
    	for (int n = 0; n < ev.can_charge_amount; n++)
    	{
    		if (i < ev.departure_timeblock[n] - bp.sample_time)
    		{
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1))] = 1;
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_mu + to_string(n + 1))] = 1;

                glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_UP, 0.0, 1.0);
			}
		}
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void EV_RchargeMinusRdischarge_biggerThan_zero(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        for (int n = 0; n < ev.can_charge_amount; n++)
        {
            if (i < ev.departure_timeblock[n] - bp.sample_time)
    		{
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1))] = 1;
                coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1))] = -1;

                glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_LO, 0.0, 0.0);
			}   
        }
    }
}

void EV_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int n = 0; n < ev.can_charge_amount ; n++)
    {
        for (int i = 0; i < bp.remain_timeblock; i++)
        {
            if (i < ev.departure_timeblock[n] - bp.sample_time)
            {
                for (int j = 0; j <= i ; j++)
                {
                    coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][j * bp.variable + find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1))] = 1;
                    if (ev.can_discharge)
                        coefficient[bp.coef_row_num + bp.remain_timeblock * n + i][j * bp.variable + find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1))] = -1;
                }
            }
            glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), "");
            glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * n + i), GLP_LO, (ev.MIN_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T), 0.0);
        }
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void EV_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int n = 0; n < ev.can_charge_amount ; n++)
    {
        for (int i = 0; i < (ev.departure_timeblock[n] - bp.sample_time); i++)
        {
            coefficient[bp.coef_row_num + n][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1))] = 1;
            if (ev.can_discharge)
                coefficient[bp.coef_row_num + n][i * bp.variable + find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1))] = -1;
        }
        
        float leastTimeblock_toChargeSOCThreshold = ceil((ev.threshold_SOC - ev.start_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T));
        if (ev.departure_timeblock[n] - ev.start_timeblock[n] >= leastTimeblock_toChargeSOCThreshold)
        {
            if (ev.can_discharge)
            {   
                // Error handling: all users force charging except already got threshold users,
                // force charging last 2 timeblock because EV SOC increase slowly
                if (ev.departure_timeblock[n] - bp.sample_time <= 2 && ev.now_SOC[n] < ev.threshold_SOC)
                {                    
                    glp_set_row_name(mip, (bp.bnd_row_num + n), "");
                    glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_FX, ev.departure_timeblock[n] - bp.sample_time, ev.departure_timeblock[n] - bp.sample_time);
                }
                else
                {
                    glp_set_row_name(mip, (bp.bnd_row_num + n), "");
                    glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_LO, round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)), 0.0);
                }
            }
            else
            {
                glp_set_row_name(mip, (bp.bnd_row_num + n), "");
                glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_LO, round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)), 0.0);
            }
        }
        else
        {
            // avoid EM user setting too less time and EV can't charge to threshold although charging every time
            glp_set_row_name(mip, (bp.bnd_row_num + n), "");
            glp_set_row_bnds(mip, (bp.bnd_row_num + n), GLP_LO, float(ev.departure_timeblock[n] - bp.sample_time), 0.0);
        }
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- fuel cell -=-=-=-=-=-=-= //
void pfcOnPlusPfcOff_equalTo_pfuelCell(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pfc)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_PfcON)] = -1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_PfcOFF)] = -1.0;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pfcOn_smallerThan_mufcMultiplyByPfcMax(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_PfcON)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_muFC)] = -bp.Pfc_max;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pfcOn_biggerThan_mufcMultiplyByPfcMin(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_PfcON)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_muFC)] = -1.5;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_LO, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_PfcOFF)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_muFC)] = 0.0;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(BASEPARAMETER &bp, float *P_power, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pfc)] = 1.0;

        for (int k = 1; k <= bp.piecewise_num; k++) //X=z1*x1+(x2-x1)*s1...
        {
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_zPfc + to_string(k))] = -P_power[k - 1];
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_lambda_Pfc + to_string(k))] = -1.0 * (P_power[k] - P_power[k - 1]);
        }

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(BASEPARAMETER &bp, float *P_power_all, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_Pfct)] = 1.0;
        for (int k = 1; k <= bp.piecewise_num; k++) //Y=z1*y1+(y2-y1)*s1...
        {
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_zPfc + to_string(k))] = -P_power_all[k - 1];
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_lambda_Pfc + to_string(k))] = -1.0 * (P_power_all[k] - P_power_all[k - 1]);
        }

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_LO, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void zPfcOnePlusZPfcTwo_etc_equalTo_one(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        for (int k = 1; k <= bp.piecewise_num; k++)
        {
            coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_zPfc + to_string(k))] = 1.0;
        }

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, 1.0, 1.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void lambdaPfc_smallerThan_zpfc(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        for (int k = 1; k <= bp.piecewise_num; k++)
        {
            coefficient[bp.coef_row_num + bp.remain_timeblock * k + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_zPfc + to_string(k))] = -1.0;
            coefficient[bp.coef_row_num + bp.remain_timeblock * k + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_lambda_Pfc + to_string(k))] = 1.0;

            glp_set_row_name(mip, (bp.bnd_row_num + bp.remain_timeblock * k + i), "");
            glp_set_row_bnds(mip, (bp.bnd_row_num + bp.remain_timeblock * k + i), GLP_UP, 0.0, 0.0);
        }
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- battery discharge specific percentage a day -=-=-=-=-=-=-= //
void SOCPositiveMinusSOCNegative_equalTo_SOCchange(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_change)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_increase)] = -1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_decrease)] = 1.0;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_increase)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_Z)] = -((ess.MAX_power * bp.delta_T) / ess.capacity);

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_decrease)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_Z)] = ((ess.MIN_power * bp.delta_T) / ess.capacity);

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_UP, 0.0, (ess.MIN_power * bp.delta_T) / ess.capacity);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void SOCchange_equalTo_PessTransToSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_change)] = 1.0;
        coefficient[bp.coef_row_num + i][i * bp.variable + find_variableName_position(bp.variable_name, ess.str_Pess)] = -bp.delta_T / ess.capacity;

        glp_set_row_name(mip, (bp.bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bp.bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

void summation_SOCNegative_biggerThan_targetDischargeSOC(BASEPARAMETER &bp, float target_dischargeSOC, float already_dischargeSOC, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < bp.remain_timeblock; i++)
    {
        coefficient[bp.coef_row_num][i * bp.variable + find_variableName_position(bp.variable_name, bp.str_SOC_decrease)] = 1.0;
    }
    glp_set_row_name(mip, bp.bnd_row_num, "");
    glp_set_row_bnds(mip, bp.bnd_row_num, GLP_LO, target_dischargeSOC - already_dischargeSOC, 0.0);

    bp.coef_row_num += row_num_maxAddition;
    bp.bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(bp.coef_row_num, row_num_maxAddition, bp.bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- objective function -=-=-=-=-=-=-= //
void setting_GHEMS_ObjectiveFunction(BASEPARAMETER bp, DEMANDRESPONSE dr, ELECTRICMOTOR em, ELECTRICVEHICLE ev, float *price, glp_prob *mip)
{
    functionPrint(__func__);

    for (int j = 0; j < bp.remain_timeblock; j++)
	{
		if (bp.Pgrid_flag)
			glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + j * bp.variable), price[j + bp.sample_time] * bp.delta_T);
		if (bp.Psell_flag)
			glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Psell) + 1 + j * bp.variable), price[j + bp.sample_time] * bp.delta_T * (-1));
		if (bp.Pfc_flag)
			glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pfct) + 1 + j * bp.variable), Hydro_Price / Hydro_Cons * bp.delta_T); //FC cost
        if (em.flag && em.can_discharge)
        {
            for (int n = 0; n < em.can_charge_amount; n++)         
			    glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1)) + 1 + j * bp.variable), -em.normal_charging_power * price[j + bp.sample_time] * bp.delta_T);
        }
        if (ev.flag && ev.can_discharge)
        {
            for (int n = 0; n < ev.can_charge_amount; n++)         
			    glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1)) + 1 + j * bp.variable), -ev.charging_power * price[j + bp.sample_time] * bp.delta_T);
        }
	}
	if (dr.mode != 0)
	{
		if (bp.sample_time - dr.startTime >= 0)
		{
			for (int j = 0; j < dr.endTime - bp.sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + j * bp.variable), (price[j + bp.sample_time] + dr.feedback_price) * bp.delta_T);
			}
		}
		else if (bp.sample_time - dr.startTime < 0)
		{
			for (int j = dr.startTime - bp.sample_time; j < dr.endTime - bp.sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + j * bp.variable), (price[j + bp.sample_time] + dr.feedback_price) * bp.delta_T);
			}
		}
	}
}