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
void summation_publicLoadRa_biggerThan_QaMinusD(PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < pl.number; i++)
    {
        if ((pl.end[i] - sample_time) >= 0)
        {
            if ((pl.start[i] - sample_time) >= 0)
            {
                for (int j = (pl.start[i] - sample_time); j <= (pl.end[i] - sample_time); j++)
                {
                    coefficient[coef_row_num + i][j * variable + find_variableName_position(variable_name, pl.str_publicLoad + to_string(i + 1))] = 1.0;
                }
            }
            else if ((pl.start[i] - sample_time) < 0)
            {
                for (int j = 0; j <= (pl.end[i] - sample_time); j++)
                {
                    coefficient[coef_row_num + i][j * variable + find_variableName_position(variable_name, pl.str_publicLoad + to_string(i + 1))] = 1.0;
                }
            }
            if (dr_mode != 0)
            {
                if (dr_endTime - sample_time >= 0)
                {
                    if ((dr_startTime - sample_time) >= 0)
                    {
                        for (int j = (dr_startTime - sample_time); j < (dr_endTime - sample_time); j++)
                        {
                            coefficient[coef_row_num + i][j * variable + find_variableName_position(variable_name, pl.str_publicLoad + to_string(i + 1))] = 0.0;
                        }
                    }
                    else if ((dr_startTime - sample_time) < 0)
                    {
                        for (int j = 0; j < (dr_endTime - sample_time); j++)
                        {
                            coefficient[coef_row_num + i][j * variable + find_variableName_position(variable_name, pl.str_publicLoad + to_string(i + 1))] = 0.0;
                        }
                    }
                }
            }
        }        
        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_LO, ((float)pl.remain_operation_time[i]), 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- grid with demand response or sell -=-=-=-=-=-=-= //
void pgrid_smallerThan_muGridMultiplyByPgridMaxArray(int dr_mode, vector<float> Pgrid_max_array, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pgrid")] = 1.0;
        if (dr_mode != 0)
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "mu_grid")] = -Pgrid_max_array[i];
        else
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "mu_grid")] = -Pgrid_max;

        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- sell -=-=-=-=-=-=-= //
void psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Psell")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "mu_grid")] = Psell_max;

        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, Psell_max);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void psell_smallerThan_PfuelCellPlusPsolar(bool Pfc_flag, float *solar2, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Psell")] = 1.0;
        if (Pfc_flag)
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pfc")] = -1.0;

        glp_set_row_name(mip, bnd_row_num + i, "");
        glp_set_row_bnds(mip, bnd_row_num + i, GLP_UP, 0.0, solar2[i + sample_time]);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- battery -=-=-=-=-=-=-= //
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "Pess")] = 1.0;
    }
    glp_set_row_name(mip, bnd_row_num, "");
    if (sample_time == 0)
        glp_set_row_bnds(mip, bnd_row_num, GLP_LO, ((SOC_thres - SOC_ini) * Cbat * Vsys) / delta_T, 0.0);

    else
        glp_set_row_bnds(mip, bnd_row_num, GLP_DB, ((SOC_thres - SOC_ini) * Cbat * Vsys) / delta_T, ((0.99 - SOC_ini) * Cbat * Vsys) / delta_T);
    // avoid the row max is bigger than SOC max

    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void previousSOCPlusPessTransToSOC_equalTo_currentSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        for (int j = 0; j <= i; j++)
        {
            coefficient[coef_row_num + i][j * variable + find_variableName_position(variable_name, "Pess")] = -1.0;
        }
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC")] = Cbat * Vsys / delta_T;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, (SOC_ini * Cbat * Vsys / delta_T), (SOC_ini * Cbat * Vsys / delta_T));
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pessPositive_smallerThan_zMultiplyByPchargeMax(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pcharge")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Z")] = -Pbat_max;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pdischarge")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Z")] = Pbat_min;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, Pbat_min);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pessPositiveMinusPessNegative_equalTo_Pess(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pess")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pcharge")] = -1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pdischarge")] = 1.0;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- balanced equation -=-=-=-=-=-=-= //
void pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoadPlusPchargingEM(PUBLICLOAD pl, ELECTRICMOTOR em, float *solar2, float *load_model, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    if (pl.flag)
    {
        for (int h = 0; h < pl.number; h++)
        {
            if ((pl.end[h] - sample_time) >= 0)
            {
                if ((pl.start[h] - sample_time) >= 0)
                {
                    for (int i = (pl.start[h] - sample_time); i <= (pl.end[h] - sample_time); i++)
                    {
                        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, pl.str_publicLoad + to_string(h + 1))] = pl.power[h];
                    }
                }
                else if ((pl.start[h] - sample_time) < 0)
                {
                    for (int i = 0; i <= (pl.end[h] - sample_time); i++)
                    {
                        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, pl.str_publicLoad + to_string(h + 1))] = pl.power[h];
                    }
                }
            }
        }
    }

    if (em.flag)
    {
        for (int n = 0; n < em.can_charge_amount; n++)
        {
            for (int i = 0; i < em.departure_timeblock[n] - sample_time; i++)
            {
                coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, em.str_charging + to_string(n + 1))] = em.normal_charging_power;
                if (em.can_discharge)
                    coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, em.str_discharging + to_string(n + 1))] = -em.normal_charging_power;
            }
        }
    }
    
    for (int i = 0; i < (time_block - sample_time); i++)
    {
        if (Pgrid_flag)
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pgrid")] = -1.0;
        if (Pess_flag)
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pess")] = 1.0;
        if (Pfc_flag)
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pfc")] = -1.0;
        if (Psell_flag)
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Psell")] = 1.0;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        if (solar2[i + sample_time] - load_model[i + sample_time] < 0)
            glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, solar2[i + sample_time] - load_model[i + sample_time], solar2[i + sample_time] - load_model[i + sample_time]);
        else
            glp_set_row_bnds(mip, (bnd_row_num + i), GLP_DB, -0.0001, solar2[i + sample_time] - load_model[i + sample_time]);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- demand response -=-=-=-=-=-=-= //
void targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    float dr_sumOfCBL = 0.0;
    if (dr_startTime - sample_time >= 0)
    {
        for (int i = (dr_startTime - sample_time); i < (dr_endTime - sample_time); i++)
        {
            coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "Pgrid")] = -1.0 * delta_T;
            dr_sumOfCBL += dr_customer_baseLine * delta_T;
        }
    }
    else
    {
        for (int i = 0; i < (dr_endTime - sample_time); i++)
        {
            coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "Pgrid")] = -1.0 * delta_T;
        }
        for (int i = dr_startTime; i < sample_time; i++)
        {
            snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `GHEMS_control_status` WHERE equip_name = 'Pgrid'", i);
            float previous_grid_power = turn_value_to_float(0);
            dr_sumOfCBL += (dr_customer_baseLine - previous_grid_power) * delta_T;
        }
        for (int i = sample_time; i < dr_endTime; i++)
        {
            dr_sumOfCBL += dr_customer_baseLine * delta_T;
        }
    }
    glp_set_row_name(mip, bnd_row_num, "");
    glp_set_row_bnds(mip, bnd_row_num, GLP_LO, dr_minDecrease_power - dr_sumOfCBL, 0.0);
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- EM -=-=-=-=-=-=-= //
void EM_Rcharging_smallerThan_mu(ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int i = 0; i < (time_block - sample_time); i++)
    {
    	for (int n = 0; n < em.can_charge_amount; n++)
    	{
    		if(i < em.departure_timeblock[n] - sample_time)
    		{
                coefficient[coef_row_num + (time_block - sample_time) * n + i][i * variable + find_variableName_position(variable_name, em.str_charging + to_string(n + 1))] = -1;
                coefficient[coef_row_num + (time_block - sample_time) * n + i][i * variable + find_variableName_position(variable_name, em.str_mu + to_string(n + 1))] = 1;

                glp_set_row_name(mip, (bnd_row_num + (time_block - sample_time) * n + i), "");
                glp_set_row_bnds(mip, (bnd_row_num + (time_block - sample_time) * n + i), GLP_LO, 0.0, 0.0);
			}
		}
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void EM_Rdischarging_smallerThan_oneMinusMu(ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int i = 0; i < (time_block - sample_time); i++)
    {
    	for (int n = 0; n < em.can_charge_amount; n++)
    	{
    		if(i < em.departure_timeblock[n] - sample_time)
    		{
                coefficient[coef_row_num + (time_block - sample_time) * n + i][i * variable + find_variableName_position(variable_name, em.str_discharging + to_string(n + 1))] = 1;
                coefficient[coef_row_num + (time_block - sample_time) * n + i][i * variable + find_variableName_position(variable_name, em.str_mu + to_string(n + 1))] = 1;

                glp_set_row_name(mip, (bnd_row_num + (time_block - sample_time) * n + i), "");
                glp_set_row_bnds(mip, (bnd_row_num + (time_block - sample_time) * n + i), GLP_UP, 0.0, 1.0);
			}
		}
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void EM_RchargeMinusRdischarge_biggerThan_zero(ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        for (int n = 0; n < em.can_charge_amount; n++)
        {
            if(i < em.departure_timeblock[n] - sample_time)
    		{
                coefficient[coef_row_num + (time_block - sample_time) * n + i][i * variable + find_variableName_position(variable_name, em.str_charging + to_string(n + 1))] = 1;
                coefficient[coef_row_num + (time_block - sample_time) * n + i][i * variable + find_variableName_position(variable_name, em.str_discharging + to_string(n + 1))] = -1;

                glp_set_row_name(mip, (bnd_row_num + (time_block - sample_time) * n + i), "");
                glp_set_row_bnds(mip, (bnd_row_num + (time_block - sample_time) * n + i), GLP_LO, 0.0, 0.0);
			}   
        }
    }
}

void EM_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int n = 0; n < em.can_charge_amount ; n++)
    {
        for (int i = 0; i < (time_block - sample_time); i++)
        {
            if (i < em.departure_timeblock[n] - sample_time)
            {
                for (int j = 0; j <= i ; j++)
                {
                    coefficient[coef_row_num + (time_block - sample_time) * n + i][j * variable + find_variableName_position(variable_name, em.str_charging + to_string(n + 1))] = 1;
                    if (em.can_discharge)
                        coefficient[coef_row_num + (time_block - sample_time) * n + i][j * variable + find_variableName_position(variable_name, em.str_discharging + to_string(n + 1))] = -1;
                }
            }
            glp_set_row_name(mip, (bnd_row_num + (time_block - sample_time) * n + i), "");
            glp_set_row_bnds(mip, (bnd_row_num + (time_block - sample_time) * n + i), GLP_LO, (em.MIN_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * delta_T), 0.0);
        }
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void EM_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);
    
    for (int n = 0; n < em.can_charge_amount ; n++)
    {
        for (int i = 0; i < (em.departure_timeblock[n] - sample_time); i++)
        {
            coefficient[coef_row_num + n][i * variable + find_variableName_position(variable_name, em.str_charging + to_string(n + 1))] = 1;
            if (em.can_discharge)
                coefficient[coef_row_num + n][i * variable + find_variableName_position(variable_name, em.str_discharging + to_string(n + 1))] = -1;
        }
        
        float leastTimeblock_toChargeSOCThreshold = ceil((em.threshold_SOC - em.start_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * delta_T));
        if (em.departure_timeblock[n] - em.start_timeblock[n] >= leastTimeblock_toChargeSOCThreshold)
        {
            if (em.can_discharge)
            {   
                // Error handling: all users force charging except already got threshold users,
                // avoid use ceil due to return become 2 got error, but in fact 1 is enough to reach the threshold
                if (em.departure_timeblock[n] - sample_time == 1 && em.now_SOC[n] < em.threshold_SOC)
                {
                    
                    glp_set_row_name(mip, (bnd_row_num + n), "");
                    glp_set_row_bnds(mip, (bnd_row_num + n), GLP_FX, 1.0, 1.0);
                }
                else
                {
                    glp_set_row_name(mip, (bnd_row_num + n), "");
                    glp_set_row_bnds(mip, (bnd_row_num + n), GLP_LO, ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * delta_T)), 0.0);
                }
            }
            else
            {
                glp_set_row_name(mip, (bnd_row_num + n), "");
                glp_set_row_bnds(mip, (bnd_row_num + n), GLP_LO, ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * delta_T)), 0.0);
            }
        }
        else
        {
            // avoid EM user setting too less time and EM can't charge to threshold although charging every time
            glp_set_row_name(mip, (bnd_row_num + n), "");
            glp_set_row_bnds(mip, (bnd_row_num + n), GLP_LO, float(em.departure_timeblock[n] - sample_time), 0.0);
        }
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- fuel cell -=-=-=-=-=-=-= //
void pfcOnPlusPfcOff_equalTo_pfuelCell(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pfc")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "PfcON")] = -1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "PfcOFF")] = -1.0;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pfcOn_smallerThan_mufcMultiplyByPfcMax(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "PfcON")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "muFC")] = -Pfc_max;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pfcOn_biggerThan_mufcMultiplyByPfcMin(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "PfcON")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "muFC")] = -1.5;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_LO, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "PfcOFF")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "muFC")] = 0.0;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(float *P_power, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pfc")] = 1.0;

        for (int k = 1; k <= piecewise_num; k++) //X=z1*x1+(x2-x1)*s1...
        {
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "zPfc" + to_string(k))] = -P_power[k - 1];
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "lambda_Pfc" + to_string(k))] = -1.0 * (P_power[k] - P_power[k - 1]);
        }

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(float *P_power_all, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pfct")] = 1.0;
        for (int k = 1; k <= piecewise_num; k++) //Y=z1*y1+(y2-y1)*s1...
        {
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "zPfc" + to_string(k))] = -P_power_all[k - 1];
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "lambda_Pfc" + to_string(k))] = -1.0 * (P_power_all[k] - P_power_all[k - 1]);
        }

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_LO, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void zPfcOnePlusZPfcTwo_etc_equalTo_one(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        for (int k = 1; k <= piecewise_num; k++)
        {
            coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "zPfc" + to_string(k))] = 1.0;
        }

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 1.0, 1.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void lambdaPfc_smallerThan_zpfc(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        for (int k = 1; k <= piecewise_num; k++)
        {
            coefficient[coef_row_num + (time_block - sample_time) * k + i][i * variable + find_variableName_position(variable_name, "zPfc" + to_string(k))] = -1.0;
            coefficient[coef_row_num + (time_block - sample_time) * k + i][i * variable + find_variableName_position(variable_name, "lambda_Pfc" + to_string(k))] = 1.0;

            glp_set_row_name(mip, (bnd_row_num + (time_block - sample_time) * k + i), "");
            glp_set_row_bnds(mip, (bnd_row_num + (time_block - sample_time) * k + i), GLP_UP, 0.0, 0.0);
        }
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- battery discharge specific percentage a day -=-=-=-=-=-=-= //
void SOCPositiveMinusSOCNegative_equalTo_SOCchange(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_change")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_increase")] = -1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_decrease")] = 1.0;

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_increase")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_Z")] = -((Pbat_max * delta_T) / (Cbat * Vsys));

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_decrease")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_Z")] = ((Pbat_min * delta_T) / (Cbat * Vsys));

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_UP, 0.0, (Pbat_min * delta_T) / (Cbat * Vsys));
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void SOCchange_equalTo_PessTransToSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "SOC_change")] = 1.0;
        coefficient[coef_row_num + i][i * variable + find_variableName_position(variable_name, "Pess")] = -delta_T / (Cbat * Vsys);

        glp_set_row_name(mip, (bnd_row_num + i), "");
        glp_set_row_bnds(mip, (bnd_row_num + i), GLP_FX, 0.0, 0.0);
    }
    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

void summation_SOCNegative_biggerThan_targetDischargeSOC(float target_dischargeSOC, float already_dischargeSOC, float **coefficient, glp_prob *mip, int row_num_maxAddition)
{
    functionPrint(__func__);

    for (int i = 0; i < (time_block - sample_time); i++)
    {
        coefficient[coef_row_num][i * variable + find_variableName_position(variable_name, "SOC_decrease")] = 1.0;
    }
    glp_set_row_name(mip, bnd_row_num, "");
    glp_set_row_bnds(mip, bnd_row_num, GLP_LO, target_dischargeSOC - already_dischargeSOC, 0.0);

    coef_row_num += row_num_maxAddition;
    bnd_row_num += row_num_maxAddition;
    saving_coefAndBnds_rowNum(coef_row_num, row_num_maxAddition, bnd_row_num, row_num_maxAddition);
}

// =-=-=-=-=-=-=- objective function -=-=-=-=-=-=-= //
void setting_GHEMS_ObjectiveFunction(ELECTRICMOTOR em, float *price, glp_prob *mip)
{
    functionPrint(__func__);

    for (int j = 0; j < (time_block - sample_time); j++)
	{
		if (Pgrid_flag)
			glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + j * variable), price[j + sample_time] * delta_T);
		if (Psell_flag)
			glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Psell") + 1 + j * variable), price[j + sample_time] * delta_T * (-1));
		if (Pfc_flag)
			glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pfct") + 1 + j * variable), Hydro_Price / Hydro_Cons * delta_T); //FC cost
        if (em.flag && em.can_discharge)
        {
            for (int n = 0; n < em.can_charge_amount; n++)         
			    glp_set_obj_coef(mip, (find_variableName_position(variable_name, em.str_discharging + to_string(n + 1)) + 1 + j * variable), -em.normal_charging_power * price[j + sample_time] * delta_T);
        }
	}
	if (dr_mode != 0)
	{
		if (sample_time - dr_startTime >= 0)
		{
			for (int j = 0; j < dr_endTime - sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + j * variable), (price[j + sample_time] + dr_feedback_price) * delta_T);
			}
		}
		else if (sample_time - dr_startTime < 0)
		{
			for (int j = dr_startTime - sample_time; j < dr_endTime - sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + j * variable), (price[j + sample_time] + dr_feedback_price) * delta_T);
			}
		}
	}
}