#include "optimize.hpp"


optimize::optimize(IMPORT ipt, OBJECTIVETARGET maxmin, ENERGYMANAGESYSTEM ems_type, int group_id)
{
    rxipt = ipt;
    variable_num = rxipt.bp.variable_num;
    variable_name = rxipt.variable_name;
    mip = glp_create_prob();
    
    if (maxmin == OBJECTIVETARGET::MAXIMUM)
        glp_set_obj_dir(mip, GLP_MAX);
    else
        glp_set_obj_dir(mip, GLP_MIN);

    switch (ems_type)
    {
    case HEMS:
        set_name(ENERGYMANAGESYSTEM::HEMS, group_id);
        get_remainTimeblock_and_sampleTime(ENERGYMANAGESYSTEM::HEMS);
        set_rows_total();
        set_cols_total();
        init_coef_matrix();
        set_columnBoundary(ENERGYMANAGESYSTEM::HEMS);
        break;
    
    default:
        set_name();
        get_remainTimeblock_and_sampleTime();
        set_rows_total();
        set_cols_total();
        init_coef_matrix();
        set_columnBoundary();
        break;
    }
}

optimize::~optimize()
{
    glp_delete_prob(mip);
	delete[] coefficient;
}

// =-=-=- private function -=-=-= //
void optimize::set_name(ENERGYMANAGESYSTEM ems_type, int group_id)
{
    string prob_name;
    switch (ems_type)
    {
    case HEMS:
        prob_name = "LHEMS"+to_string(group_id);
        glp_set_prob_name(mip, prob_name.c_str());
        break;
    
    default:
        prob_name = "GHEMS";
        glp_set_prob_name(mip, prob_name.c_str());
        break;
    }
}

void optimize::get_remainTimeblock_and_sampleTime(ENERGYMANAGESYSTEM ems_type)
{
    switch (ems_type)
    {
    case HEMS:
        sample_time=rxipt.bp.next_simulate_timeblock;
        remain_timeblock = rxipt.bp.time_block-sample_time;
        break;
    
    default:
        sample_time=rxipt.bp.Global_next_simulate_timeblock;
        remain_timeblock = rxipt.bp.time_block-sample_time;
        break;
    }
}

void optimize::set_rows_total()
{
    rowTotal = remain_timeblock*200+1;
    glp_add_rows(mip, rowTotal);
}

void optimize::set_cols_total()
{
    colTotal = variable_num*remain_timeblock;
    glp_add_cols(mip, colTotal);
}

void optimize::init_coef_matrix()
{
    coefficient=NEW2D(rowTotal, colTotal, float);
    for (int i = 0; i < rowTotal; i++)
    {
        for (int j = 0; j < colTotal; j++)
            coefficient[i][j] = 0.0;
    }
}

void optimize::set_columnBoundary(ENERGYMANAGESYSTEM ems_type)
{
    switch (ems_type)
    {
    case HEMS:
        for (int i = 0; i < remain_timeblock; i++)
        {
            if (rxipt.fg.interrupt)
            {
                for (int j = 1; j <= rxipt.irl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "interrupt" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "interrupt" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }
            }
            if (rxipt.fg.uninterrupt)
            {
                for (int j = 1; j <= rxipt.uirl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "uninterrupt" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "uninterrupt" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }
            }
            if (rxipt.fg.varying)
            {
                for (int j = 1; j <= rxipt.varl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "varying" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "varying" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }
            }
            if (rxipt.fg.Pgrid)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable_num), GLP_DB, 0.0, rxipt.bp.Pgrid_max);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable_num), GLP_CV);
            }
            if (rxipt.fg.Pess)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pess") + 1 + i * variable_num), GLP_DB, -rxipt.bp.Pbat_min, rxipt.bp.Pbat_max);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pess") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pcharge") + 1 + i * variable_num), GLP_FR, 0.0, rxipt.bp.Pbat_max);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pcharge") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pdischarge") + 1 + i * variable_num), GLP_FR, 0.0, rxipt.bp.Pbat_min);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pdischarge") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC") + 1 + i * variable_num), GLP_DB, rxipt.bp.SOC_min, rxipt.bp.SOC_max);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Z") + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Z") + 1 + i * variable_num), GLP_BV);
            }
            if (rxipt.fg.dr_mode != 0)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "dr_alpha") + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "dr_alpha") + 1 + i * variable_num), GLP_CV);
            }
            if (rxipt.fg.uninterrupt)
            {
                for (int j = 1; j <= rxipt.uirl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "uninterDelta" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "uninterDelta" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }
            }
            if (rxipt.fg.varying)
            {
                for (int j = 1; j <= rxipt.varl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "varyingDelta" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "varyingDelta" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }
                for (int j = 1; j <= rxipt.varl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "varyingPsi" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, rxipt.varl.max_power[j - 1]);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "varyingPsi" + to_string(j)) + 1 + i * variable_num), GLP_CV);
                }
            }
        }
        break;

    default:
        for (int i = 0; i < remain_timeblock; i++)
        {
            if (rxipt.fg.publicLoad == 1)
            {
                for (int j = 1; j <= rxipt.pl.load_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "publicLoad" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "publicLoad" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }
            }
            if (rxipt.fg.Pgrid == 1)
            {
                if (rxipt.fg.dr_mode == 0)
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable_num), GLP_DB, 0.0, rxipt.bp.Pgrid_max); //Pgrid
                else
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable_num), GLP_DB, 0.0, rxipt.dr.Pgrid_max_array[i]); //Pgrid

                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pgrid") + 1 + i * variable_num), GLP_CV);
            }
            if (rxipt.fg.mu_grid == 1)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "mu_grid") + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "mu_grid") + 1 + i * variable_num), GLP_BV);
            }
            if (rxipt.fg.Psell == 1)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Psell") + 1 + i * variable_num), GLP_DB, -0.00001, rxipt.bp.Psell_max);
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Psell") + 1 + i * variable_num), GLP_CV);
            }
            if (rxipt.fg.Pess == 1)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pess") + 1 + i * variable_num), GLP_DB, -rxipt.bp.Pbat_min, rxipt.bp.Pbat_max); // Pess
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pess") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pcharge") + 1 + i * variable_num), GLP_FR, 0.0, rxipt.bp.Pbat_max); // Pess +
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pcharge") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pdischarge") + 1 + i * variable_num), GLP_FR, 0.0, rxipt.bp.Pbat_min); // Pess -
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pdischarge") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC") + 1 + i * variable_num), GLP_DB, rxipt.bp.SOC_min, rxipt.bp.SOC_max); //SOC
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Z") + 1 + i * variable_num), GLP_DB, 0.0, 1.0); //Z
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Z") + 1 + i * variable_num), GLP_BV);
                if (rxipt.fg.SOCchange)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC_change") + 1 + i * variable_num), GLP_DB, (-rxipt.bp.Pbat_min * rxipt.bp.delta_T) / (rxipt.bp.Cbat * rxipt.bp.Vsys), (rxipt.bp.Pbat_max * rxipt.bp.delta_T) / (rxipt.bp.Cbat * rxipt.bp.Vsys));
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC_change") + 1 + i * variable_num), GLP_CV);
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC_increase") + 1 + i * variable_num), GLP_DB, 0.0, (rxipt.bp.Pbat_max * rxipt.bp.delta_T) / (rxipt.bp.Cbat * rxipt.bp.Vsys));
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC_increase") + 1 + i * variable_num), GLP_CV);
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC_decrease") + 1 + i * variable_num), GLP_DB, 0.0, (rxipt.bp.Pbat_min * rxipt.bp.delta_T) / (rxipt.bp.Cbat * rxipt.bp.Vsys));
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC_decrease") + 1 + i * variable_num), GLP_CV);
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "SOC_Z") + 1 + i * variable_num), GLP_DB, 0.0, 1.0);
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "SOC_Z") + 1 + i * variable_num), GLP_BV);
                }
            }
            if (rxipt.fg.Pfc == 1)
            {
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pfc") + 1 + i * variable_num), GLP_DB, -0.00001, rxipt.bp.Pfc_max); //Pfc
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pfc") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "Pfct") + 1 + i * variable_num), GLP_LO, 0.0, 0.0); //Total_Pfc
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "Pfct") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "PfcON") + 1 + i * variable_num), GLP_DB, -0.00001, rxipt.bp.Pfc_max); //Pfc_ON_POWER
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "PfcON") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "PfcOFF") + 1 + i * variable_num), GLP_FX, 0.0, 0.0); //Pfc_OFF_POWER
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "PfcOFF") + 1 + i * variable_num), GLP_CV);
                glp_set_col_bnds(mip, (find_variableName_position(variable_name, "muFC") + 1 + i * variable_num), GLP_DB, 0.0, 1.0); //ufc
                glp_set_col_kind(mip, (find_variableName_position(variable_name, "muFC") + 1 + i * variable_num), GLP_BV);

                for (int j = 1; j <= rxipt.bp.piecewise_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "zPfc" + to_string(j)) + 1 + i * variable_num), GLP_DB, 0.0, 1.0); //z_Pfc
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "zPfc" + to_string(j)) + 1 + i * variable_num), GLP_BV);
                }

                for (int j = 1; j <= rxipt.bp.piecewise_num; j++)
                {
                    glp_set_col_bnds(mip, (find_variableName_position(variable_name, "lambda_Pfc" + to_string(j)) + 1 + i * variable_num), GLP_LO, 0.0, 0.0); //λ_Pfc
                    glp_set_col_kind(mip, (find_variableName_position(variable_name, "lambda_Pfc" + to_string(j)) + 1 + i * variable_num), GLP_CV);
                }
            }
        }
        break;
    }
}

int optimize::find_variableName_position(vector<string> variableNameArray, string target)
{
	auto it = find(variableNameArray.begin(), variableNameArray.end(), target);

	// If element was found
	if (it != variableNameArray.end())
		return (it - variableNameArray.begin());
	else
		return -1;
}

// public
// common
int optimize::verify_solution_after_sovle_GLPK(ENERGYMANAGESYSTEM ems_type)
{
    int *ia = new int[rowTotal * colTotal + 1];
	int *ja = new int[rowTotal * colTotal + 1];
	double *ar = new double[rowTotal * colTotal + 1];
	for (int i = 0; i < rowTotal; i++)
	{
		for (int j = 0; j < colTotal; j++)
		{
			ia[i * (remain_timeblock * variable_num) + j + 1] = i + 1;
			ja[i * (remain_timeblock * variable_num) + j + 1] = j + 1;
			ar[i * (remain_timeblock * variable_num) + j + 1] = coefficient[i][j];
		}
	}
    glp_load_matrix(mip, rowTotal*colTotal, ia, ja, ar);
    
    // setting solver
    glp_iocp parm;
	glp_init_iocp(&parm);
    parm.presolve = GLP_ON;
	parm.msg_lev = GLP_MSG_ERR;

    switch (ems_type)
    {
    case HEMS:
        if (rxipt.bp.next_simulate_timeblock == 0)
            parm.tm_lim = 120000;
        else
            parm.tm_lim = 60000;
        parm.gmi_cuts = GLP_ON;
        parm.bt_tech = GLP_BT_BFS;
        parm.br_tech = GLP_BR_PCH;
        break;
    
    default:
    	if (rxipt.bp.Global_next_simulate_timeblock == 0)
            parm.tm_lim = 120000;
        else
            parm.tm_lim = 60000;
        parm.gmi_cuts = GLP_ON;
        parm.bt_tech = GLP_BT_BPH;
        parm.br_tech = GLP_BR_PCH;
        break;
    }
    int err = glp_intopt(mip, &parm);
    switch (ems_type)
    {
    case HEMS:
        if (glp_mip_obj_val(mip) == 0.0)
        {
            cout << "Error > Objective value and SOC is 0, No Solution, give up the solution" << endl;
            return -1;
        }
        else
        {
            printf("obj_val = %f; \n", glp_mip_obj_val(mip));
            return 1;
        }
        break;
    
    default:
        if (glp_mip_obj_val(mip) == 0.0 && glp_mip_col_val(mip, find_variableName_position(variable_name, "SOC") + 1) == 0.0)
        {
            cout << "Error > Objective value and SOC is 0, No Solution, give up the solution" << endl;
            return -1;
        }
        else
        {
            printf("obj_val = %f; \n", glp_mip_obj_val(mip));
            return 1;
        }
        break;
    }
	
}

// hems
void optimize::setting_hems_coefficient()
{
    if (rxipt.fg.interrupt)
    {
        summation_interruptLoadRa_biggerThan_Qa(rxipt.irl.load_num);
    }
    if (rxipt.fg.dr_mode != 0)
    {
		pgrid_smallerThan_alphaPgridMax(remain_timeblock);
		alpha_between_oneminusDu_and_one(remain_timeblock);
    }
    pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(remain_timeblock);
    if (rxipt.fg.Pess)
    {
        previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(1);
        previousSOCPlusPessTransToSOC_equalTo_currentSOC(remain_timeblock);
        pessPositive_smallerThan_zMultiplyByPchargeMax(remain_timeblock);
        pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(remain_timeblock);
        pessPositiveMinusPessNegative_equalTo_Pess(remain_timeblock);
    }
    if (rxipt.fg.uninterrupt)
    {
        summation_uninterruptDelta_equalTo_one(1);
        uninterruptRajToN_biggerThan_uninterruptDelta(remain_timeblock);
    }
    if (rxipt.fg.varying)
    {
        summation_varyingDelta_equalTo_one(1);
        varyingRajToN_biggerThan_varyingDelta(remain_timeblock);
        varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(remain_timeblock);
    }
}

void optimize::setting_hems_objectiveFunction()
{
    vector<float> price = rxipt.bp.price;
    for (int j = 0; j < remain_timeblock; j++)
	{
		glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid")+1 + j*variable_num), price[j + sample_time] * rxipt.bp.delta_T);
	}
	if (rxipt.fg.dr_mode != 0)
	{
		if (sample_time-rxipt.dr.startTime >= 0)
		{
			for (int j = 0; j < rxipt.dr.endTime-sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid")+1 + j*variable_num), (price[j+sample_time] + rxipt.dr.participate_status[j+(sample_time-rxipt.dr.startTime)]*rxipt.dr.feedback_price) * rxipt.bp.delta_T);
			}
		}
		else if (sample_time-rxipt.dr.startTime < 0)
		{
			for (int j = rxipt.dr.startTime-sample_time; j < rxipt.dr.endTime-sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid")+1 + j*variable_num), (price[j+sample_time] + rxipt.dr.participate_status[j-(rxipt.dr.startTime-sample_time)]*rxipt.dr.feedback_price) * rxipt.bp.delta_T);
			}
		}
	}
}

// cems
void optimize::setting_cems_coefficient()
{
    if (rxipt.fg.publicLoad)
    {
		summation_publicLoadRa_biggerThan_QaMinusD(rxipt.pl.load_num);
        
    }
    if (rxipt.fg.Pgrid)
    {
		pgrid_smallerThan_muGridMultiplyByPgridMaxArray(remain_timeblock);
        
    }
    if (rxipt.fg.Psell)
    {
		psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(remain_timeblock);
		psell_smallerThan_PfuelCellPlusPsolar(remain_timeblock);
        
    }
    if (rxipt.fg.Pess)
    {
		previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(1);
		previousSOCPlusPessTransToSOC_equalTo_currentSOC(remain_timeblock);
		pessPositive_smallerThan_zMultiplyByPchargeMax(remain_timeblock);
		pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(remain_timeblock);
        pessPositiveMinusPessNegative_equalTo_Pess(remain_timeblock);
    }
	pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoad(remain_timeblock);

    if (rxipt.fg.dr_mode != 0)
    {
		targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(1);
    }
    if (rxipt.fg.Pfc)
    {
        // temporary won't use
    }
    if (rxipt.fg.SOCchange)
    {
        // temporary won't use
    }
}

void optimize::setting_cems_objectiveFunction()
{
    vector<float> price = rxipt.bp.price;
    for (int j = 0; j < remain_timeblock; j++)
	{
		if (rxipt.fg.Pgrid)
			glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid")+1 + j*variable_num), price[j+sample_time] * rxipt.bp.delta_T);
		if (rxipt.fg.Psell)
			glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Psell")+1 + j*variable_num), price[j+sample_time] * rxipt.bp.delta_T * (-1));
		if (rxipt.fg.Pfc)
			glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pfct")+1 + j*variable_num), rxipt.bp.hydro_price / rxipt.bp.hydro_cons * rxipt.bp.delta_T); //FC cost
	}
	if (rxipt.fg.dr_mode != 0)
	{
		if (sample_time - rxipt.dr.startTime >= 0)
		{
			for (int j = 0; j < rxipt.dr.endTime - sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid")+1 + j*variable_num), (price[j+sample_time]+rxipt.dr.feedback_price) * rxipt.bp.delta_T);
			}
		}
		else if (sample_time - rxipt.dr.startTime < 0)
		{
			for (int j = rxipt.dr.startTime - sample_time; j < rxipt.dr.endTime - sample_time; j++)
			{
				glp_set_obj_coef(mip, (find_variableName_position(variable_name, "Pgrid")+1 + j*variable_num), (price[j+sample_time]+rxipt.dr.feedback_price) * rxipt.bp.delta_T);
			}
		}
	}
}

void optimize::saving_result(ENERGYMANAGESYSTEM ems_type)
{
    switch (ems_type)
    {
    case HEMS:
        for (int i = 0; i < variable_num; i++)
        {
            vector<float> variable_result;
            int z=i;
            int varyPsi_num=1;
            if (sample_time == 0)
            {
                for (int j = 0; j < rxipt.bp.time_block; j++)
                {
                    variable_result.push_back(glp_mip_col_val(mip, z+1));
                    if (ems_type == ENERGYMANAGESYSTEM::HEMS && i == find_variableName_position(variable_name, "varying"+to_string(i-(rxipt.irl.load_num+rxipt.uirl.load_num)+1)))
                    {
                        variable_result[j] = glp_mip_col_val(mip, varyPsi_num + find_variableName_position(variable_name, "varyingPsi"+to_string(i-(rxipt.irl.load_num+rxipt.uirl.load_num)+1)));
                        if (variable_result[j] > 0.0)
                        {
                            variable_result[j]=1.0;
                        }
                    }
                    z += variable_num;
                    varyPsi_num += variable_num;
                }
            }
            else
            {
                for (int j = 0; j < rxipt.bp.time_block-sample_time; j++)
                {
                    variable_result.push_back(glp_mip_col_val(mip, z+1));
                    if (ems_type == ENERGYMANAGESYSTEM::HEMS && i == find_variableName_position(variable_name, "varying"+to_string(i-(rxipt.irl.load_num+rxipt.uirl.load_num)+1)))
                    {
                        variable_result[j] = glp_mip_col_val(mip, varyPsi_num + find_variableName_position(variable_name, "varyingPsi"+to_string(i-(rxipt.irl.load_num+rxipt.uirl.load_num)+1)));
                        if (variable_result[j] > 0.0)
                        {
                            variable_result[j]=1.0;
                        }
                    }
                    z += variable_num;
                    varyPsi_num += variable_num;
                }
            }
            solve_result.push_back(variable_result);
        }
        break;
    
    default:
        for (int i = 0; i < variable_num; i++)
        {
            vector<float> variable_result;
            int z=i;
            if (sample_time == 0)
            {
                for (int j = 0; j < rxipt.bp.time_block; j++)
                {
                    variable_result.push_back(glp_mip_col_val(mip, z+1));                
                    z += variable_num;
                }   
            }
            else
            {
                for (int j = 0; j < rxipt.bp.time_block-sample_time; j++)
                {
                    variable_result.push_back(glp_mip_col_val(mip, z+1));
                    z += variable_num;
                }
            }
            solve_result.push_back(variable_result);
        }
        break;
    }
}
