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

#include <string>
#include <vector>
#include <algorithm>
#include <random>

#define P_1 1.63
#define P_2 2.38
#define P_3 3.52
#define P_4 4.80
#define P_5 5.66
#define P_6 6.41

float Hydro_Cons = 0.04; // unit kWh/g
float Hydro_Price = 0.0;
char column[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";

void optimization(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, PUBLICLOAD pl, UNCONTROLLABLELOAD ucl, ELECTRICMOTOR em, ELECTRICVEHICLE ev)
{
	functionPrint(__func__);

	if (pl.flag)
	{
		// force to stop public load
		pl.forceToStop_start.assign(pl.forceToStop_number, 0);
		pl.forceToStop_end.assign(pl.forceToStop_number, 0);
		pl.forceToStop_operation_time.assign(pl.forceToStop_number, 0);
		pl.forceToStop_remain_operation_time.assign(pl.forceToStop_number, 0);
		pl.forceToStop_power.assign(pl.forceToStop_number, 0);
		float **f_publicLoad = getPublicLoad(5, pl.forceToStop_number);
		for (int i = 0; i < pl.forceToStop_number; i++)
		{
			pl.forceToStop_start[i] = int(f_publicLoad[i][0]);
			pl.forceToStop_end[i] = int(f_publicLoad[i][1]) - 1;
			pl.forceToStop_operation_time[i] = int(f_publicLoad[i][2]);
			pl.forceToStop_power[i] = f_publicLoad[i][3];
		}
		int *f_buff = countPublicLoads_AlreadyOpenedTimes(bp, pl.forceToStop_number, pl.str_forceToStop_publicLoad);
		pl.forceToStop_remain_operation_time = count_publicLoads_RemainOperateTime(pl.forceToStop_number, pl.forceToStop_operation_time, f_buff);
		
		// interrupt public load
		pl.interrupt_start.assign(pl.interrupt_number, 0);
		pl.interrupt_end.assign(pl.interrupt_number, 0);
		pl.interrupt_operation_time.assign(pl.interrupt_number, 0);
		pl.interrupt_remain_operation_time.assign(pl.interrupt_number, 0);
		pl.interrupt_power.assign(pl.interrupt_number, 0);
		float **i_publicLoad = getPublicLoad(6, pl.interrupt_number);
		for (int i = 0; i < pl.interrupt_number; i++)
		{
			pl.interrupt_start[i] = int(i_publicLoad[i][0]);
			pl.interrupt_end[i] = int(i_publicLoad[i][1]) - 1;
			pl.interrupt_operation_time[i] = int(i_publicLoad[i][2]);
			pl.interrupt_power[i] = i_publicLoad[i][3];
		}
		int *i_buff = countPublicLoads_AlreadyOpenedTimes(bp, pl.interrupt_number, pl.str_interrupt_publicLoad);
		pl.interrupt_remain_operation_time = count_publicLoads_RemainOperateTime(pl.interrupt_number, pl.interrupt_operation_time, i_buff);
		
		// periodic public load
		pl.periodic_start.assign(pl.periodic_number, 0);
		pl.periodic_end.assign(pl.periodic_number, 0);
		pl.periodic_operation_time.assign(pl.periodic_number, 0);
		pl.periodic_remain_operation_time.assign(pl.periodic_number, 0);
		pl.periodic_power.assign(pl.periodic_number, 0);
		float **p_publicLoad = getPublicLoad(7, pl.periodic_number);
		for (int i = 0; i < pl.periodic_number; i++)
		{
			pl.periodic_start[i] = int(p_publicLoad[i][0]);
			pl.periodic_end[i] = int(p_publicLoad[i][1]) - 1;
			pl.periodic_operation_time[i] = int(p_publicLoad[i][2]);
			pl.periodic_power[i] = p_publicLoad[i][3];
		}
		int *p_buff = countPublicLoads_AlreadyOpenedTimes(bp, pl.periodic_number, pl.str_periodic_publicLoad);
		pl.periodic_remain_operation_time = count_publicLoads_RemainOperateTime(pl.periodic_number, pl.periodic_operation_time, p_buff);
	}
	
	if (em.flag && em.can_charge_amount)
	{
		for (int i = 0; i < em.can_charge_amount; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `EM_Pole`.`Pole_ID`, `EM_Pole`.`SOC`, `EM_Pole`.`BAT_CAP`, `EM_motor_type`.`voltage`, `EM_Pole`.`Departure_timeblock`, `EM_Pole`.`number`, `EM_user_result`.`Start_timeblock`, `EM_user_result`.`Start_SOC` FROM `EM_Pole` INNER JOIN `EM_user_result` ON `EM_Pole`.`number`=`EM_user_result`.`number` INNER JOIN `EM_motor_type` ON `EM_user_result`.`type`=`EM_motor_type`.`id` WHERE `sure` = 1 LIMIT 1 OFFSET %d", i);
			fetch_row_value();
			em.Pole_ID.push_back(turn_int(0));
			em.now_SOC.push_back(turn_float(1));
			em.battery_capacity.push_back(turn_float(2)*turn_float(3)/1000);
			em.departure_timeblock.push_back(turn_int(4));
			em.number.push_back(turn_int(5));
			em.start_timeblock.push_back(turn_int(6));
			em.start_SOC.push_back(turn_float(7));
		}
	}

	if (ev.flag && ev.can_charge_amount)
	{
		for (int i = 0; i < ev.can_charge_amount; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `EV_Pole`.`Pole_ID`, `EV_Pole`.`SOC`, `EV_Pole`.`BAT_CAP`, `EV_Pole`.`Departure_timeblock`, `EV_Pole`.`number`, `EV_user_result`.`Start_timeblock`, `EV_user_result`.`Start_SOC` FROM `EV_Pole` INNER JOIN `EV_user_result` ON `EV_Pole`.`number`=`EV_user_result`.`number` WHERE `sure` = 1 LIMIT 1 OFFSET %d", i);
			fetch_row_value();
			ev.Pole_ID.push_back(turn_int(0));
			ev.now_SOC.push_back(turn_float(1));
			ev.battery_capacity.push_back(turn_float(2));
			ev.departure_timeblock.push_back(turn_int(3));
			ev.number.push_back(turn_int(4));
			ev.start_timeblock.push_back(turn_int(5));
			ev.start_SOC.push_back(turn_float(6));
		}
	}

	// sum by 'row_num_maxAddition' in every constraint below
	int rowTotal = 0;
	if (pl.flag) { rowTotal += pl.forceToStop_number + pl.interrupt_number + pl.periodic_number; }
	if (bp.Pgrid_flag) { rowTotal += bp.remain_timeblock; }
	if (bp.Psell_flag)	{ rowTotal += bp.remain_timeblock * 2;}
	if (ess.flag) { rowTotal += bp.remain_timeblock * 4 + 1; }
	rowTotal += bp.remain_timeblock;
	if (dr.mode != 0) { rowTotal += 1; }
	if (dr.mode != 0 && (ev.flag && ev.can_charge_amount) || (em.flag && em.can_charge_amount)) { rowTotal += bp.remain_timeblock; }
	if (bp.Pfc_flag) { rowTotal += bp.remain_timeblock * (7 + bp.piecewise_num); }
	if (em.flag && em.can_charge_amount) { rowTotal += bp.remain_timeblock * em.can_charge_amount + em.can_charge_amount; }
	if (em.flag && em.can_charge_amount && em.can_discharge) { rowTotal += bp.remain_timeblock * (2 * em.can_charge_amount); }
	if (ev.flag && ev.can_charge_amount) { rowTotal += bp.remain_timeblock * ev.can_charge_amount + ev.can_charge_amount; }
	if (ev.flag && ev.can_charge_amount && ev.can_discharge) { rowTotal += bp.remain_timeblock * (2 * ev.can_charge_amount); }

	int colTotal = bp.variable * bp.remain_timeblock;
	glp_prob *mip;
	mip = glp_create_prob();
	glp_set_prob_name(mip, "GHEMS");
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, rowTotal);
	glp_add_cols(mip, colTotal);

	setting_GLPK_columnBoundary(bp, ess, dr, pl, em, ev, mip);

	float **coefficient = NEW2D(rowTotal, colTotal, float);
	for (int m = 0; m < rowTotal; m++)
	{
		for (int n = 0; n < colTotal; n++)
			coefficient[m][n] = 0.0;
	}

	if (pl.flag)
	{
		summation_forceToStopPublicLoadRa_biggerThan_QaMinusD(bp, dr, pl, coefficient, mip, pl.forceToStop_number);
		summation_interruptPublicLoadRa_biggerThan_Qa(bp, dr, pl, coefficient, mip, pl.interrupt_number);
		summation_periodicPublicLoadRa_biggerThan_Qa(bp, dr, pl, coefficient, mip, pl.periodic_number);
	}

	if (bp.Pgrid_flag)
	{
		// 0 < Pgrid j < μgrid j * Pgrid j, max
		pgrid_smallerThan_muGridMultiplyByPgridMaxArray(bp, dr.mode, coefficient, mip, bp.remain_timeblock);
	}
	
	if (bp.Psell_flag)
	{
		// Psell j < (1 - μgrid j) * Psell max
		psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(bp, coefficient, mip, bp.remain_timeblock);
		// Psell j <= Pfc j + Ppv j
		psell_smallerThan_PfuelCellPlusPsolar(bp, coefficient, mip, bp.remain_timeblock);
		
	}

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

	// Pgrid j + Pfc j + Ppv j - Pess j - Psell j = sum(Pu,a j) + Pc,a + sum(Pem, n j) + sum(Pev, n j)
	pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoadPlusPchargingEMPlusPchargingEV(bp, ess, pl, ucl, em, ev, coefficient, mip, bp.remain_timeblock);

	// dr constraint
	if (dr.mode != 0)
	{
		targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(bp, dr, coefficient, mip, 1);

		if ((ev.flag && ev.can_charge_amount) || (em.flag && em.can_charge_amount))
		{
			summation_EMEVPcharge_smallerThan_PgridPlusPessPlusPpv(bp, ess, em, ev, coefficient, mip, bp.remain_timeblock);
		}
	}

	if (bp.Pfc_flag)
	{
		Hydro_Price = value_receive("BaseParameter", "parameter_name", "hydrogen_price", 'F');
		// =-=-=-=-=-=-=- Fuel Cell model setting and get piecewise data point -=-=-=-=-=-=-= //
		int data_sampling = 101;
		float *data_power = new float[data_sampling];
		float *data_power_all = new float[data_sampling];

		float *P_power = new float[bp.point_num];
		float *P_power_all = new float[bp.point_num];

		float fc_power_interval = bp.Pfc_max / (data_sampling - 1);
		for (int i = 0; i < data_sampling; i++)
		{
			float efficiency = 0.0;
			float PLR = i * fc_power_interval / bp.Pfc_max;
			if (PLR <= 0.05)
				efficiency = 0.2716;
			else
				efficiency = (0.9033 * (pow(PLR, 5)) - 2.9996 * (pow(PLR, 4)) + 3.6503 * (pow(PLR, 3)) - 2.0704 * (pow(PLR, 2)) + 0.4623 * (pow(PLR, 1)) + 0.3747);

			data_power[i] = i * fc_power_interval;
			data_power_all[i] = i * fc_power_interval / efficiency;
		}

		for (int j = 0; j < bp.point_num; j++)
		{
			P_power[j] = data_power[j * 100 / bp.piecewise_num];
			P_power_all[j] = data_power_all[j * 100 / bp.piecewise_num];
			printf("\tLINE %d: x_%d:%f  y_%d:%f\n", __LINE__, j * (100 / bp.piecewise_num), data_power[j * 100 / bp.piecewise_num], j * 100 / bp.piecewise_num, data_power_all[j * 100 / bp.piecewise_num]);
		}
		//pfc=pfc_on+pfc_off
		pfcOnPlusPfcOff_equalTo_pfuelCell(bp, coefficient, mip, bp.remain_timeblock);	
		//pfc_on<=ufc*Pfc_max
		pfcOn_smallerThan_mufcMultiplyByPfcMax(bp, coefficient, mip, bp.remain_timeblock);
		//pfc_on>=ufc*pfc_min
		pfcOn_biggerThan_mufcMultiplyByPfcMin(bp, coefficient, mip, bp.remain_timeblock);
		//pfc_off j<=(1-ufc)*pfc off
		pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(bp, coefficient, mip, bp.remain_timeblock);
		//pfc=x1z1+(x2-x1)s1......
		pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(bp, P_power, coefficient, mip, bp.remain_timeblock);
		//pfc_tatol=y1z1+(y2-y1)s1......
		pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(bp, P_power_all, coefficient, mip, bp.remain_timeblock);
		//z1+z2+z3+.....=1
		zPfcOnePlusZPfcTwo_etc_equalTo_one(bp, coefficient, mip, bp.remain_timeblock);
		// 0 <= λi j <= zi j
		lambdaPfc_smallerThan_zpfc(bp, coefficient, mip, (bp.remain_timeblock * bp.piecewise_num + bp.remain_timeblock));
	}

	if (bp.SOC_change_flag)
	{
		float already_dischargeSOC = getPrevious_battery_dischargeSOC(bp.time_block, bp.sample_time, bp.str_SOC_decrease);
		// (SOC +) - (SOC -) = SOC change
		SOCPositiveMinusSOCNegative_equalTo_SOCchange(bp, coefficient, mip, bp.remain_timeblock);
		// SOC + <= Z' * (Pcharge max * Ts)/(Cess * Vess)
		SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(bp, ess, coefficient, mip, bp.remain_timeblock);
		// SOC - <= (1 - Z') * (Pdischarge max * Ts)/(Cess * Vess)
		SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(bp, ess, coefficient, mip, bp.remain_timeblock);
		// SOC change = (Pess * Ts)/(Cess * Vess)
		SOCchange_equalTo_PessTransToSOC(bp, ess, coefficient, mip, bp.remain_timeblock);
		// sum(SOC -) >= 0.8
		summation_SOCNegative_biggerThan_targetDischargeSOC(bp, 0.8, already_dischargeSOC, coefficient, mip, 1);
	}

	if (em.flag && em.can_charge_amount)
	{
		if (em.can_discharge)
		{	
			// EM's r n c,j <= μ n j
			EM_Rcharging_smallerThan_mu(bp, em, coefficient, mip, em.can_charge_amount * bp.remain_timeblock);
			// EM's r n d,j <= (1-μ n j)
			EM_Rdischarging_smallerThan_oneMinusMu(bp, em, coefficient, mip, em.can_charge_amount * bp.remain_timeblock);
		}

		// EM's SOC min <= SOC j - 1 + ((P n c,max * r n c,j * Ts / E n cap) - (P n d,max * r n d,j * Ts / E n cap))
		EM_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(bp, em, coefficient, mip, em.can_charge_amount * bp.remain_timeblock);
		
		// EM's SOC threshold, <= SOC j - 1 + sum((P n c,max * r n c,j * Ts) / E n cap - (P n d,max * r n d,j * Ts / E n cap))), each EM have only one constranit formula
		EM_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(bp, em, coefficient, mip, em.can_charge_amount);	
	}
	
	if (ev.flag && ev.can_charge_amount)
	{
		if (ev.can_discharge)
		{	
			// EV's r n c,j <= μ n j
			EV_Rcharging_smallerThan_mu(bp, ev, coefficient, mip, ev.can_charge_amount * bp.remain_timeblock);
			// EV's r n d,j <= (1-μ n j)
			EV_Rdischarging_smallerThan_oneMinusMu(bp, ev, coefficient, mip, ev.can_charge_amount * bp.remain_timeblock);
		}

		// EV's SOC min <= SOC j - 1 + ((P n c,max * r n c,j * Ts / E n cap) - (P n d,max * r n d,j * Ts / E n cap))
		EV_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(bp, ev, coefficient, mip, ev.can_charge_amount * bp.remain_timeblock);
		
		// EV's SOC threshold, <= SOC j - 1 + sum((P n c,max * r n c,j * Ts) / E n cap - (P n d,max * r n d,j * Ts / E n cap))), each EM have only one constranit formula
		EV_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(bp, ev, coefficient, mip, ev.can_charge_amount);	
	}
	
	setting_GHEMS_ObjectiveFunction(bp, dr, em, ev, bp.price, mip);

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
	/*==============================GLPK????????��x�X}====================================*/
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
	// parm.gmi_cuts = GLP_ON;
	// parm.ps_heur = GLP_ON;
	// parm.bt_tech = GLP_BT_BFS;
	// parm.br_tech = GLP_BR_PCH;

	//no fc+ no sell
	//fc+no sell
	parm.gmi_cuts = GLP_ON;
	parm.bt_tech = GLP_BT_BPH;
	parm.br_tech = GLP_BR_PCH;

	//FC+sell
	//parm.fp_heur = GLP_ON;
	// parm.bt_tech = GLP_BT_BPH;
	//parm.br_tech = GLP_BR_PCH;

	int err = glp_intopt(mip, &parm);

	double z = glp_mip_obj_val(mip);
	printf("\n");
	printf("sol = %f; \n", z);

	if (z == 0.0 && glp_mip_col_val(mip, find_variableName_position(bp.variable_name, ess.str_SOC) + 1) == 0.0)
	{
		display_coefAndBnds_rowNum();
		printf("Error > sol is 0, No Solution, give up the solution\n");
		printf("%.2f\n", glp_mip_col_val(mip, find_variableName_position(bp.variable_name, ess.str_SOC) + 1));
		if (em.flag)
		{
			if (em.can_discharge)
			{
				for (int n = 0; n < em.can_charge_amount; n++)
				{
					if (em.departure_timeblock[n]-bp.sample_time < ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)))
					{
						printf("EM Number %d, %d >= %.0f (%d), dis=%.0f\n", em.number[n], em.departure_timeblock[n]-bp.sample_time, ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)), em.departure_timeblock[n]-bp.sample_time>=ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)), glp_mip_col_val(mip, find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1)) + 1));
					}
				}
			}
			else
			{
				for (int n = 0; n < em.can_charge_amount; n++)
				{
					if (em.departure_timeblock[n]-bp.sample_time < ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)))
					{
						printf("EM Number %d, %d >= %.0f (%d)\n", em.number[n], em.departure_timeblock[n]-bp.sample_time, ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)), em.departure_timeblock[n]-bp.sample_time>=ceil((em.threshold_SOC - em.now_SOC[n]) * em.battery_capacity[n] / (em.normal_charging_power * bp.delta_T)));
					}
				}
			}
		}
		if (ev.flag)
		{
			if (ev.can_discharge)
			{
				for (int n = 0; n < ev.can_charge_amount; n++)
				{
					if (ev.departure_timeblock[n]-bp.sample_time < round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)))
					{
						printf("EV Number %d, %d >= %.0f (%d), dis=%.0f\n", ev.number[n], ev.departure_timeblock[n]-bp.sample_time, round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)), ev.departure_timeblock[n]-bp.sample_time>=round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)), glp_mip_col_val(mip, find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1)) + 1));
					}
				}
			}
			else
			{
				for (int n = 0; n < ev.can_charge_amount; n++)
				{
					if (ev.departure_timeblock[n]-bp.sample_time < round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)))
					{
						printf("EV Number %d, %d >= %.0f (%d)\n", ev.number[n], ev.departure_timeblock[n]-bp.sample_time, round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)), ev.departure_timeblock[n]-bp.sample_time>=round((ev.threshold_SOC - ev.now_SOC[n]) * ev.battery_capacity[n] / (ev.charging_power * bp.delta_T)));
					}
				}
			}
		}
		return;
	}

	/*==============================��N?M?????????G???X==================================*/
	float *s = new float[bp.time_block];
	if (em.flag || ev.flag)
	{
		int size_without_EMEV = bp.variable;
		if (em.flag)
		{
			for (int n = 0; n < em.can_charge_amount; n++)
			{
				float c_status = glp_mip_col_val(mip, find_variableName_position(bp.variable_name, em.str_charging + to_string(n + 1)) + 1);
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `charging_status` = '%d' WHERE `Pole_ID` = '%d'", int(c_status), em.Pole_ID[n]);
				sent_query();
				if (em.can_discharge)
				{
					float d_status = glp_mip_col_val(mip, find_variableName_position(bp.variable_name, em.str_discharging + to_string(n + 1)) + 1);
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `discharge_status` = '%d' WHERE `Pole_ID` = '%d'", int(d_status), em.Pole_ID[n]);
					sent_query();
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_chargingOrDischarging_status` SET `A%d` = '%d' WHERE `user_number` = %d", bp.sample_time, int(c_status)-int(d_status), em.number[n]);
					sent_query();
				}
				else
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_chargingOrDischarging_status` SET `A%d` = '%d' WHERE `user_number` = %d", bp.sample_time, int(c_status), em.number[n]);
					sent_query();
				}
			}
			if (em.can_discharge)
				size_without_EMEV -= em.can_charge_amount * 3;
			else
				size_without_EMEV -= em.can_charge_amount;
		}
		if (ev.flag)
		{
			for (int n = 0; n < ev.can_charge_amount; n++)
			{
				float c_status = glp_mip_col_val(mip, find_variableName_position(bp.variable_name, ev.str_charging + to_string(n + 1)) + 1);
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `charging_status` = '%d' WHERE `Pole_ID` = '%d'", int(c_status), ev.Pole_ID[n]);
				sent_query();
				if (ev.can_discharge)
				{
					float d_status = glp_mip_col_val(mip, find_variableName_position(bp.variable_name, ev.str_discharging + to_string(n + 1)) + 1);
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `discharge_status` = '%d' WHERE `Pole_ID` = '%d'", int(d_status), ev.Pole_ID[n]);
					sent_query();
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_chargingOrDischarging_status` SET `A%d` = '%d' WHERE `user_number` = %d", bp.sample_time, int(c_status)-int(d_status), ev.number[n]);
					sent_query();
				}
				else
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_chargingOrDischarging_status` SET `A%d` = '%d' WHERE `user_number` = %d", bp.sample_time, int(c_status), ev.number[n]);
					sent_query();
				}
			}
			if (ev.can_discharge)
				size_without_EMEV -= ev.can_charge_amount * 3;
			else
				size_without_EMEV -= ev.can_charge_amount;
		}
		// other variable without EM and EV
		for (int i = 1; i <= size_without_EMEV; i++)
		{
			int h = i;
			if (bp.sample_time == 0)
			{
				for (int j = 0; j < bp.time_block; j++)
				{
					s[j] = glp_mip_col_val(mip, h);
					h = (h + bp.variable);
				}
				insert_status_into_MySQLTable("GHEMS_control_status", column, s, "equip_name", bp.variable_name[i - 1]);
			}
			else
			{
				// =-=-=-=-=-=-=-=-=-=- history about the control status from each control id -=-=-=-=-=-=-=-=-=-= //
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM GHEMS_control_status WHERE equip_name = '%s'", column, bp.variable_name[i - 1].c_str());
				fetch_row_value();
				for (int k = 0; k < bp.sample_time; k++)
				{
					s[k] = turn_float(k);
				}
				// =-=-=-=-=-=-=-=-=-=- change new result after the sample time -=-=-=-=-=-=-=-=-=-= //
				for (int j = 0; j < bp.remain_timeblock; j++)
				{
					s[j + bp.sample_time] = glp_mip_col_val(mip, h);
					h = (h + bp.variable);
				}
				update_status_to_MySQLTable("GHEMS_control_status", s, "equip_name", bp.variable_name[i - 1]);
				
				// =-=-=-=-=-=-=-=-=-=- result update from the sample time until end timeblock (96) -=-=-=-=-=-=-=-=-=-= //
				for (int j = 0; j < bp.sample_time; j++)
				{
					s[j] = 0;
				}
				insert_status_into_MySQLTable("GHEMS_real_status", column, s, "equip_name", bp.variable_name[i - 1]);
			}
		}
	}
	else
	{
		for (int i = 1; i <= bp.variable; i++)
		{
			int h = i;
			if (bp.sample_time == 0)
			{
				for (int j = 0; j < bp.time_block; j++)
				{
					s[j] = glp_mip_col_val(mip, h);
					h = (h + bp.variable);
				}
				insert_status_into_MySQLTable("GHEMS_control_status", column, s, "equip_name", bp.variable_name[i - 1]);
			}
			else
			{
				// =-=-=-=-=-=-=-=-=-=- history about the control status from each control id -=-=-=-=-=-=-=-=-=-= //
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM GHEMS_control_status WHERE equip_name = '%s'", column, bp.variable_name[i - 1].c_str());
				fetch_row_value();
				for (int k = 0; k < bp.sample_time; k++)
				{
					s[k] = turn_float(k);
				}
				// =-=-=-=-=-=-=-=-=-=- change new result after the sample time -=-=-=-=-=-=-=-=-=-= //
				for (int j = 0; j < bp.remain_timeblock; j++)
				{
					s[j + bp.sample_time] = glp_mip_col_val(mip, h);
					h = (h + bp.variable);
				}
				update_status_to_MySQLTable("GHEMS_control_status", s, "equip_name", bp.variable_name[i - 1]);
				
				// =-=-=-=-=-=-=-=-=-=- result update from the sample time until end timeblock (96) -=-=-=-=-=-=-=-=-=-= //
				for (int j = 0; j < bp.sample_time; j++)
				{
					s[j] = 0;
				}
				insert_status_into_MySQLTable("GHEMS_real_status", column, s, "equip_name", bp.variable_name[i - 1]);
			}
		}
	}

	glp_delete_prob(mip);
	delete[] ia, ja, ar, s;
	delete[] coefficient;
	return;
}

void setting_GLPK_columnBoundary(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, PUBLICLOAD pl, ELECTRICMOTOR em, ELECTRICVEHICLE ev, glp_prob *mip)
{
	functionPrint(__func__);
	messagePrint(__LINE__, "Setting columns...", 'S', 0, 'Y');
	for (int i = 0; i < bp.remain_timeblock; i++)
	{
		if (pl.flag == 1)
		{
			for (int j = 1; j <= pl.forceToStop_number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, pl.str_forceToStop_publicLoad + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
			for (int j = 1; j <= pl.interrupt_number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, pl.str_interrupt_publicLoad + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, pl.str_interrupt_publicLoad + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}		
			for (int j = 1; j <= pl.periodic_number; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, pl.str_periodic_publicLoad + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, pl.str_periodic_publicLoad + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
		}
		if (bp.Pgrid_flag == 1)
		{
			if (dr.mode == 0)
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + i * bp.variable), GLP_DB, 0.0, bp.Pgrid_max); //Pgrid
			else
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + i * bp.variable), GLP_DB, 0.0, bp.Pgrid_max_array[i]); //Pgrid

			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_Pgrid) + 1 + i * bp.variable), GLP_CV);
		}
		if (bp.mu_grid_flag == 1)
		{
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_mu_grid) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_mu_grid) + 1 + i * bp.variable), GLP_BV);
		}
		if (bp.Psell_flag == 1)
		{
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Psell) + 1 + i * bp.variable), GLP_DB, -0.00001, bp.Psell_max);
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_Psell) + 1 + i * bp.variable), GLP_CV);
		}
		if (ess.flag == 1)
		{
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Pess) + 1 + i * bp.variable), GLP_DB, -ess.MIN_power, ess.MAX_power); // Pess
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Pess) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Pcharge) + 1 + i * bp.variable), GLP_FR, 0.0, ess.MAX_power); // Pess +
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Pcharge) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Pdischarge) + 1 + i * bp.variable), GLP_FR, 0.0, ess.MIN_power); // Pess -
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Pdischarge) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_SOC) + 1 + i * bp.variable), GLP_DB, ess.MIN_SOC, ess.MAX_SOC); //SOC
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_SOC) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ess.str_Z) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0); //Z
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ess.str_Z) + 1 + i * bp.variable), GLP_BV);
			if (bp.SOC_change_flag)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_change) + 1 + i * bp.variable), GLP_DB, (-ess.MIN_power * bp.delta_T) / ess.capacity, (ess.MAX_power * bp.delta_T) / ess.capacity);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_change) + 1 + i * bp.variable), GLP_CV);
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_increase) + 1 + i * bp.variable), GLP_DB, 0.0, (ess.MAX_power * bp.delta_T) / ess.capacity);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_increase) + 1 + i * bp.variable), GLP_CV);
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_decrease) + 1 + i * bp.variable), GLP_DB, 0.0, (ess.MIN_power * bp.delta_T) / ess.capacity);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_decrease) + 1 + i * bp.variable), GLP_CV);
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_Z) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_SOC_Z) + 1 + i * bp.variable), GLP_BV);
			}
		}
		if (bp.Pfc_flag == 1)
		{
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Pfc) + 1 + i * bp.variable), GLP_DB, -0.00001, bp.Pfc_max); //Pfc
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_Pfc) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_Pfct) + 1 + i * bp.variable), GLP_LO, 0.0, 0.0); //Total_Pfc
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_Pfct) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_PfcON) + 1 + i * bp.variable), GLP_DB, -0.00001, bp.Pfc_max); //Pfc_ON_POWER
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_PfcON) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_PfcOFF) + 1 + i * bp.variable), GLP_FX, 0.0, 0.0); //Pfc_OFF_POWER
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_PfcOFF) + 1 + i * bp.variable), GLP_CV);
			glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_muFC) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0); //ufc
			glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_muFC) + 1 + i * bp.variable), GLP_BV);

			for (int j = 1; j <= bp.piecewise_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_zPfc + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0); //z_Pfc
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_zPfc + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}

			for (int j = 1; j <= bp.piecewise_num; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, bp.str_lambda_Pfc + to_string(j)) + 1 + i * bp.variable), GLP_LO, 0.0, 0.0); //λ_Pfc
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, bp.str_lambda_Pfc + to_string(j)) + 1 + i * bp.variable), GLP_CV);
			}
		}
		if (em.flag)
		{
			for (int j = 1; j <= em.can_charge_amount; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, em.str_charging + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, em.str_charging + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
			if (em.can_discharge)
			{
				for (int j = 1; j <= em.can_charge_amount; j++)
				{
					glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, em.str_discharging + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
					glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, em.str_discharging + to_string(j)) + 1 + i * bp.variable), GLP_BV);
				}
				for (int j = 1; j <= em.can_charge_amount; j++)
				{
					glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, em.str_mu + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
					glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, em.str_mu + to_string(j)) + 1 + i * bp.variable), GLP_BV);
				}
			}
		}
		if (ev.flag)
		{
			for (int j = 1; j <= ev.can_charge_amount; j++)
			{
				glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ev.str_charging + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
				glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ev.str_charging + to_string(j)) + 1 + i * bp.variable), GLP_BV);
			}
			if (ev.can_discharge)
			{
				for (int j = 1; j <= ev.can_charge_amount; j++)
				{
					glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ev.str_discharging + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
					glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ev.str_discharging + to_string(j)) + 1 + i * bp.variable), GLP_BV);
				}
				for (int j = 1; j <= ev.can_charge_amount; j++)
				{
					glp_set_col_bnds(mip, (find_variableName_position(bp.variable_name, ev.str_mu + to_string(j)) + 1 + i * bp.variable), GLP_DB, 0.0, 1.0);
					glp_set_col_kind(mip, (find_variableName_position(bp.variable_name, ev.str_mu + to_string(j)) + 1 + i * bp.variable), GLP_BV);
				}
			}
		}
	}
}

int determine_realTimeOrOneDayMode_andGetSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM &ess, ELECTRICMOTOR em, ELECTRICVEHICLE ev, int real_time)
{
	// 'Realtime mode' if same day & real time = 1;
	// 'One day mode' =>
	// 		1. SOC = 0.7 if real_time = 0,
	// 		2. Use Previous SOC if real_time = 1.
	functionPrint(__func__);
	if (real_time == 1)
	{
		messagePrint(__LINE__, "Real Time Mode...", 'S', 0, 'Y');

		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE GHEMS_real_status"); //clean GHEMS_real_status;
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'now_SOC' "); //get now_SOC
		ess.INIT_SOC = turn_value_to_float(0);
		if (ess.INIT_SOC > 100)
			ess.INIT_SOC = 99.8;

		messagePrint(__LINE__, "NOW REAL SOC = ", 'F', ess.INIT_SOC, 'Y');
		messagePrint(__LINE__, "Should same as previous SOC or 99.8 (previous SOC > 100)", 'S', 0, 'Y');
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE GHEMS_control_status");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE GHEMS_real_status");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE cost");
		sent_query();

		// em & ev
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE `EM_chargingOrDischarging_status`");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE `EV_chargingOrDischarging_status`");
		sent_query();

		// Reset columns which record related ev power after optimization
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_user_number` SET `total_power` = '0', `normal_power` = '0', `discharge_normal_power` = '0';");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_user_number` SET `total_power` = '0', `normal_power` = '0', `discharge_normal_power` = '0';");
		sent_query();

		// make sure EM pole is no users using
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `EM_Pole` WHERE `number` IS NOT NULL"); 
		if (turn_value_to_int(0) != 0)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `number` = NULL, `sure` = '0', `charging_status` = '0', `discharge_status` = '0', `full` = '0', `wait` = '0', `SOC` = NULL, `BAT_CAP` = NULL, `Start_timeblock` = NULL, `Departure_timeblock` = NULL;");
			sent_query();
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `EV_Pole` WHERE `number` IS NOT NULL"); 
		if (turn_value_to_int(0) != 0)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `number` = NULL, `sure` = '0', `charging_status` = '0', `discharge_status` = '0', `full` = '0', `wait` = '0', `SOC` = NULL, `BAT_CAP` = NULL, `Start_timeblock` = NULL, `Departure_timeblock` = NULL;");
			sent_query();
		}

		// TRUNCATE EM result table
		if (em.flag && em.generate_result_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE `EM_user_result`");
			sent_query();
		}
		else
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_user_result` SET `Departure_SOC` = NULL, `Real_departure_timeblock` = NULL");
			sent_query();
		}
		if (ev.flag && ev.generate_result_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE `EV_user_result`");
			sent_query();
		}
		else
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_user_result` SET `Departure_SOC` = NULL, `Real_departure_timeblock` = NULL");
			sent_query();
		}

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC' "); //get ini_SOC
		ess.INIT_SOC = turn_value_to_float(0);
		messagePrint(__LINE__, "ini_SOC : ", 'F', ess.INIT_SOC, 'Y');

		bp.sample_time = 0;
		real_time = 1; //if you don't want do real_time,please commend it.
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %d WHERE parameter_name = 'Global_real_time' ", real_time);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %d WHERE parameter_name = 'Global_next_simulate_timeblock' ", bp.sample_time);
		sent_query();
	}

	return real_time;
}

float *getOrUpdate_SolarInfo_ThroughSampleTime(BASEPARAMETER bp, const char *weather)
{
	functionPrint(__func__);
	printf("\tWeather : %s\n", weather);
	float *solar2 = new float[bp.time_block];
	if (bp.sample_time == 0)
	{
		for (int i = 0; i < bp.time_block; i++)
		{

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM solar_data WHERE time_block = %d", weather, i);
			solar2[i] = turn_value_to_float(0);

			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE solar_day SET value =%.3f WHERE time_block = %d", solar2[i], i);
			sent_query();
		}
	}
	else
	{
		for (int i = 0; i < bp.time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM solar_day WHERE time_block = %d", i);
			solar2[i] = turn_value_to_float(0);
		}
	}
	return solar2;
}

void updateTableCost(BASEPARAMETER bp, float *totalLoad, float *totalLoad_price, float *real_grid_pirce, float *publicLoad, float *publicLoad_price, float *fuelCell_kW_price, float *Hydrogen_g_consumption, float *real_sell_pirce, float *demandResponse_feedback, float totalLoad_sum, float totalLoad_priceSum, float real_grid_pirceSum, float publicLoad_sum, float publicLoad_priceSum, float fuelCell_kW_priceSum, float Hydrogen_g_consumptionSum, float real_sell_pirceSum, float totalLoad_taipowerPriceSum, float demandResponse_feedbackSum)
{
	functionPrint(__func__);

	if (bp.sample_time == 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "public_load_power", publicLoad[0], publicLoad[1], publicLoad[2], publicLoad[3], publicLoad[4], publicLoad[5], publicLoad[6], publicLoad[7], publicLoad[8], publicLoad[9], publicLoad[10], publicLoad[11], publicLoad[12], publicLoad[13], publicLoad[14], publicLoad[15], publicLoad[16], publicLoad[17], publicLoad[18], publicLoad[19], publicLoad[20], publicLoad[21], publicLoad[22], publicLoad[23], publicLoad[24], publicLoad[25], publicLoad[26], publicLoad[27], publicLoad[28], publicLoad[29], publicLoad[30], publicLoad[31], publicLoad[32], publicLoad[33], publicLoad[34], publicLoad[35], publicLoad[36], publicLoad[37], publicLoad[38], publicLoad[39], publicLoad[40], publicLoad[41], publicLoad[42], publicLoad[43], publicLoad[44], publicLoad[45], publicLoad[46], publicLoad[47], publicLoad[48], publicLoad[49], publicLoad[50], publicLoad[51], publicLoad[52], publicLoad[53], publicLoad[54], publicLoad[55], publicLoad[56], publicLoad[57], publicLoad[58], publicLoad[59], publicLoad[60], publicLoad[61], publicLoad[62], publicLoad[63], publicLoad[64], publicLoad[65], publicLoad[66], publicLoad[67], publicLoad[68], publicLoad[69], publicLoad[70], publicLoad[71], publicLoad[72], publicLoad[73], publicLoad[74], publicLoad[75], publicLoad[76], publicLoad[77], publicLoad[78], publicLoad[79], publicLoad[80], publicLoad[81], publicLoad[82], publicLoad[83], publicLoad[84], publicLoad[85], publicLoad[86], publicLoad[87], publicLoad[88], publicLoad[89], publicLoad[90], publicLoad[91], publicLoad[92], publicLoad[93], publicLoad[94], publicLoad[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'publicLoad' ", publicLoad_sum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "public_load_price", publicLoad_price[0], publicLoad_price[1], publicLoad_price[2], publicLoad_price[3], publicLoad_price[4], publicLoad_price[5], publicLoad_price[6], publicLoad_price[7], publicLoad_price[8], publicLoad_price[9], publicLoad_price[10], publicLoad_price[11], publicLoad_price[12], publicLoad_price[13], publicLoad_price[14], publicLoad_price[15], publicLoad_price[16], publicLoad_price[17], publicLoad_price[18], publicLoad_price[19], publicLoad_price[20], publicLoad_price[21], publicLoad_price[22], publicLoad_price[23], publicLoad_price[24], publicLoad_price[25], publicLoad_price[26], publicLoad_price[27], publicLoad_price[28], publicLoad_price[29], publicLoad_price[30], publicLoad_price[31], publicLoad_price[32], publicLoad_price[33], publicLoad_price[34], publicLoad_price[35], publicLoad_price[36], publicLoad_price[37], publicLoad_price[38], publicLoad_price[39], publicLoad_price[40], publicLoad_price[41], publicLoad_price[42], publicLoad_price[43], publicLoad_price[44], publicLoad_price[45], publicLoad_price[46], publicLoad_price[47], publicLoad_price[48], publicLoad_price[49], publicLoad_price[50], publicLoad_price[51], publicLoad_price[52], publicLoad_price[53], publicLoad_price[54], publicLoad_price[55], publicLoad_price[56], publicLoad_price[57], publicLoad_price[58], publicLoad_price[59], publicLoad_price[60], publicLoad_price[61], publicLoad_price[62], publicLoad_price[63], publicLoad_price[64], publicLoad_price[65], publicLoad_price[66], publicLoad_price[67], publicLoad_price[68], publicLoad_price[69], publicLoad_price[70], publicLoad_price[71], publicLoad_price[72], publicLoad_price[73], publicLoad_price[74], publicLoad_price[75], publicLoad_price[76], publicLoad_price[77], publicLoad_price[78], publicLoad_price[79], publicLoad_price[80], publicLoad_price[81], publicLoad_price[82], publicLoad_price[83], publicLoad_price[84], publicLoad_price[85], publicLoad_price[86], publicLoad_price[87], publicLoad_price[88], publicLoad_price[89], publicLoad_price[90], publicLoad_price[91], publicLoad_price[92], publicLoad_price[93], publicLoad_price[94], publicLoad_price[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'publicLoadSpend(threeLevelPrice)' ", publicLoad_priceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "total_load_power", totalLoad[0], totalLoad[1], totalLoad[2], totalLoad[3], totalLoad[4], totalLoad[5], totalLoad[6], totalLoad[7], totalLoad[8], totalLoad[9], totalLoad[10], totalLoad[11], totalLoad[12], totalLoad[13], totalLoad[14], totalLoad[15], totalLoad[16], totalLoad[17], totalLoad[18], totalLoad[19], totalLoad[20], totalLoad[21], totalLoad[22], totalLoad[23], totalLoad[24], totalLoad[25], totalLoad[26], totalLoad[27], totalLoad[28], totalLoad[29], totalLoad[30], totalLoad[31], totalLoad[32], totalLoad[33], totalLoad[34], totalLoad[35], totalLoad[36], totalLoad[37], totalLoad[38], totalLoad[39], totalLoad[40], totalLoad[41], totalLoad[42], totalLoad[43], totalLoad[44], totalLoad[45], totalLoad[46], totalLoad[47], totalLoad[48], totalLoad[49], totalLoad[50], totalLoad[51], totalLoad[52], totalLoad[53], totalLoad[54], totalLoad[55], totalLoad[56], totalLoad[57], totalLoad[58], totalLoad[59], totalLoad[60], totalLoad[61], totalLoad[62], totalLoad[63], totalLoad[64], totalLoad[65], totalLoad[66], totalLoad[67], totalLoad[68], totalLoad[69], totalLoad[70], totalLoad[71], totalLoad[72], totalLoad[73], totalLoad[74], totalLoad[75], totalLoad[76], totalLoad[77], totalLoad[78], totalLoad[79], totalLoad[80], totalLoad[81], totalLoad[82], totalLoad[83], totalLoad[84], totalLoad[85], totalLoad[86], totalLoad[87], totalLoad[88], totalLoad[89], totalLoad[90], totalLoad[91], totalLoad[92], totalLoad[93], totalLoad[94], totalLoad[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'totalLoad' ", totalLoad_sum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "total_load_price", totalLoad_price[0], totalLoad_price[1], totalLoad_price[2], totalLoad_price[3], totalLoad_price[4], totalLoad_price[5], totalLoad_price[6], totalLoad_price[7], totalLoad_price[8], totalLoad_price[9], totalLoad_price[10], totalLoad_price[11], totalLoad_price[12], totalLoad_price[13], totalLoad_price[14], totalLoad_price[15], totalLoad_price[16], totalLoad_price[17], totalLoad_price[18], totalLoad_price[19], totalLoad_price[20], totalLoad_price[21], totalLoad_price[22], totalLoad_price[23], totalLoad_price[24], totalLoad_price[25], totalLoad_price[26], totalLoad_price[27], totalLoad_price[28], totalLoad_price[29], totalLoad_price[30], totalLoad_price[31], totalLoad_price[32], totalLoad_price[33], totalLoad_price[34], totalLoad_price[35], totalLoad_price[36], totalLoad_price[37], totalLoad_price[38], totalLoad_price[39], totalLoad_price[40], totalLoad_price[41], totalLoad_price[42], totalLoad_price[43], totalLoad_price[44], totalLoad_price[45], totalLoad_price[46], totalLoad_price[47], totalLoad_price[48], totalLoad_price[49], totalLoad_price[50], totalLoad_price[51], totalLoad_price[52], totalLoad_price[53], totalLoad_price[54], totalLoad_price[55], totalLoad_price[56], totalLoad_price[57], totalLoad_price[58], totalLoad_price[59], totalLoad_price[60], totalLoad_price[61], totalLoad_price[62], totalLoad_price[63], totalLoad_price[64], totalLoad_price[65], totalLoad_price[66], totalLoad_price[67], totalLoad_price[68], totalLoad_price[69], totalLoad_price[70], totalLoad_price[71], totalLoad_price[72], totalLoad_price[73], totalLoad_price[74], totalLoad_price[75], totalLoad_price[76], totalLoad_price[77], totalLoad_price[78], totalLoad_price[79], totalLoad_price[80], totalLoad_price[81], totalLoad_price[82], totalLoad_price[83], totalLoad_price[84], totalLoad_price[85], totalLoad_price[86], totalLoad_price[87], totalLoad_price[88], totalLoad_price[89], totalLoad_price[90], totalLoad_price[91], totalLoad_price[92], totalLoad_price[93], totalLoad_price[94], totalLoad_price[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'LoadSpend(threeLevelPrice)' ", totalLoad_priceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "real_buy_grid_price", real_grid_pirce[0], real_grid_pirce[1], real_grid_pirce[2], real_grid_pirce[3], real_grid_pirce[4], real_grid_pirce[5], real_grid_pirce[6], real_grid_pirce[7], real_grid_pirce[8], real_grid_pirce[9], real_grid_pirce[10], real_grid_pirce[11], real_grid_pirce[12], real_grid_pirce[13], real_grid_pirce[14], real_grid_pirce[15], real_grid_pirce[16], real_grid_pirce[17], real_grid_pirce[18], real_grid_pirce[19], real_grid_pirce[20], real_grid_pirce[21], real_grid_pirce[22], real_grid_pirce[23], real_grid_pirce[24], real_grid_pirce[25], real_grid_pirce[26], real_grid_pirce[27], real_grid_pirce[28], real_grid_pirce[29], real_grid_pirce[30], real_grid_pirce[31], real_grid_pirce[32], real_grid_pirce[33], real_grid_pirce[34], real_grid_pirce[35], real_grid_pirce[36], real_grid_pirce[37], real_grid_pirce[38], real_grid_pirce[39], real_grid_pirce[40], real_grid_pirce[41], real_grid_pirce[42], real_grid_pirce[43], real_grid_pirce[44], real_grid_pirce[45], real_grid_pirce[46], real_grid_pirce[47], real_grid_pirce[48], real_grid_pirce[49], real_grid_pirce[50], real_grid_pirce[51], real_grid_pirce[52], real_grid_pirce[53], real_grid_pirce[54], real_grid_pirce[55], real_grid_pirce[56], real_grid_pirce[57], real_grid_pirce[58], real_grid_pirce[59], real_grid_pirce[60], real_grid_pirce[61], real_grid_pirce[62], real_grid_pirce[63], real_grid_pirce[64], real_grid_pirce[65], real_grid_pirce[66], real_grid_pirce[67], real_grid_pirce[68], real_grid_pirce[69], real_grid_pirce[70], real_grid_pirce[71], real_grid_pirce[72], real_grid_pirce[73], real_grid_pirce[74], real_grid_pirce[75], real_grid_pirce[76], real_grid_pirce[77], real_grid_pirce[78], real_grid_pirce[79], real_grid_pirce[80], real_grid_pirce[81], real_grid_pirce[82], real_grid_pirce[83], real_grid_pirce[84], real_grid_pirce[85], real_grid_pirce[86], real_grid_pirce[87], real_grid_pirce[88], real_grid_pirce[89], real_grid_pirce[90], real_grid_pirce[91], real_grid_pirce[92], real_grid_pirce[93], real_grid_pirce[94], real_grid_pirce[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'realGridPurchase' ", real_grid_pirceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name,%s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "real_sell_grid_price", real_sell_pirce[0], real_sell_pirce[1], real_sell_pirce[2], real_sell_pirce[3], real_sell_pirce[4], real_sell_pirce[5], real_sell_pirce[6], real_sell_pirce[7], real_sell_pirce[8], real_sell_pirce[9], real_sell_pirce[10], real_sell_pirce[11], real_sell_pirce[12], real_sell_pirce[13], real_sell_pirce[14], real_sell_pirce[15], real_sell_pirce[16], real_sell_pirce[17], real_sell_pirce[18], real_sell_pirce[19], real_sell_pirce[20], real_sell_pirce[21], real_sell_pirce[22], real_sell_pirce[23], real_sell_pirce[24], real_sell_pirce[25], real_sell_pirce[26], real_sell_pirce[27], real_sell_pirce[28], real_sell_pirce[29], real_sell_pirce[30], real_sell_pirce[31], real_sell_pirce[32], real_sell_pirce[33], real_sell_pirce[34], real_sell_pirce[35], real_sell_pirce[36], real_sell_pirce[37], real_sell_pirce[38], real_sell_pirce[39], real_sell_pirce[40], real_sell_pirce[41], real_sell_pirce[42], real_sell_pirce[43], real_sell_pirce[44], real_sell_pirce[45], real_sell_pirce[46], real_sell_pirce[47], real_sell_pirce[48], real_sell_pirce[49], real_sell_pirce[50], real_sell_pirce[51], real_sell_pirce[52], real_sell_pirce[53], real_sell_pirce[54], real_sell_pirce[55], real_sell_pirce[56], real_sell_pirce[57], real_sell_pirce[58], real_sell_pirce[59], real_sell_pirce[60], real_sell_pirce[61], real_sell_pirce[62], real_sell_pirce[63], real_sell_pirce[64], real_sell_pirce[65], real_sell_pirce[66], real_sell_pirce[67], real_sell_pirce[68], real_sell_pirce[69], real_sell_pirce[70], real_sell_pirce[71], real_sell_pirce[72], real_sell_pirce[73], real_sell_pirce[74], real_sell_pirce[75], real_sell_pirce[76], real_sell_pirce[77], real_sell_pirce[78], real_sell_pirce[79], real_sell_pirce[80], real_sell_pirce[81], real_sell_pirce[82], real_sell_pirce[83], real_sell_pirce[84], real_sell_pirce[85], real_sell_pirce[86], real_sell_pirce[87], real_sell_pirce[88], real_sell_pirce[89], real_sell_pirce[90], real_sell_pirce[91], real_sell_pirce[92], real_sell_pirce[93], real_sell_pirce[94], real_sell_pirce[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'maximumSell' ", real_sell_pirceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "FC_price", fuelCell_kW_price[0], fuelCell_kW_price[1], fuelCell_kW_price[2], fuelCell_kW_price[3], fuelCell_kW_price[4], fuelCell_kW_price[5], fuelCell_kW_price[6], fuelCell_kW_price[7], fuelCell_kW_price[8], fuelCell_kW_price[9], fuelCell_kW_price[10], fuelCell_kW_price[11], fuelCell_kW_price[12], fuelCell_kW_price[13], fuelCell_kW_price[14], fuelCell_kW_price[15], fuelCell_kW_price[16], fuelCell_kW_price[17], fuelCell_kW_price[18], fuelCell_kW_price[19], fuelCell_kW_price[20], fuelCell_kW_price[21], fuelCell_kW_price[22], fuelCell_kW_price[23], fuelCell_kW_price[24], fuelCell_kW_price[25], fuelCell_kW_price[26], fuelCell_kW_price[27], fuelCell_kW_price[28], fuelCell_kW_price[29], fuelCell_kW_price[30], fuelCell_kW_price[31], fuelCell_kW_price[32], fuelCell_kW_price[33], fuelCell_kW_price[34], fuelCell_kW_price[35], fuelCell_kW_price[36], fuelCell_kW_price[37], fuelCell_kW_price[38], fuelCell_kW_price[39], fuelCell_kW_price[40], fuelCell_kW_price[41], fuelCell_kW_price[42], fuelCell_kW_price[43], fuelCell_kW_price[44], fuelCell_kW_price[45], fuelCell_kW_price[46], fuelCell_kW_price[47], fuelCell_kW_price[48], fuelCell_kW_price[49], fuelCell_kW_price[50], fuelCell_kW_price[51], fuelCell_kW_price[52], fuelCell_kW_price[53], fuelCell_kW_price[54], fuelCell_kW_price[55], fuelCell_kW_price[56], fuelCell_kW_price[57], fuelCell_kW_price[58], fuelCell_kW_price[59], fuelCell_kW_price[60], fuelCell_kW_price[61], fuelCell_kW_price[62], fuelCell_kW_price[63], fuelCell_kW_price[64], fuelCell_kW_price[65], fuelCell_kW_price[66], fuelCell_kW_price[67], fuelCell_kW_price[68], fuelCell_kW_price[69], fuelCell_kW_price[70], fuelCell_kW_price[71], fuelCell_kW_price[72], fuelCell_kW_price[73], fuelCell_kW_price[74], fuelCell_kW_price[75], fuelCell_kW_price[76], fuelCell_kW_price[77], fuelCell_kW_price[78], fuelCell_kW_price[79], fuelCell_kW_price[80], fuelCell_kW_price[81], fuelCell_kW_price[82], fuelCell_kW_price[83], fuelCell_kW_price[84], fuelCell_kW_price[85], fuelCell_kW_price[86], fuelCell_kW_price[87], fuelCell_kW_price[88], fuelCell_kW_price[89], fuelCell_kW_price[90], fuelCell_kW_price[91], fuelCell_kW_price[92], fuelCell_kW_price[93], fuelCell_kW_price[94], fuelCell_kW_price[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'fuelCellSpend' ", fuelCell_kW_priceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "hydrogen_consumption", Hydrogen_g_consumption[0], Hydrogen_g_consumption[1], Hydrogen_g_consumption[2], Hydrogen_g_consumption[3], Hydrogen_g_consumption[4], Hydrogen_g_consumption[5], Hydrogen_g_consumption[6], Hydrogen_g_consumption[7], Hydrogen_g_consumption[8], Hydrogen_g_consumption[9], Hydrogen_g_consumption[10], Hydrogen_g_consumption[11], Hydrogen_g_consumption[12], Hydrogen_g_consumption[13], Hydrogen_g_consumption[14], Hydrogen_g_consumption[15], Hydrogen_g_consumption[16], Hydrogen_g_consumption[17], Hydrogen_g_consumption[18], Hydrogen_g_consumption[19], Hydrogen_g_consumption[20], Hydrogen_g_consumption[21], Hydrogen_g_consumption[22], Hydrogen_g_consumption[23], Hydrogen_g_consumption[24], Hydrogen_g_consumption[25], Hydrogen_g_consumption[26], Hydrogen_g_consumption[27], Hydrogen_g_consumption[28], Hydrogen_g_consumption[29], Hydrogen_g_consumption[30], Hydrogen_g_consumption[31], Hydrogen_g_consumption[32], Hydrogen_g_consumption[33], Hydrogen_g_consumption[34], Hydrogen_g_consumption[35], Hydrogen_g_consumption[36], Hydrogen_g_consumption[37], Hydrogen_g_consumption[38], Hydrogen_g_consumption[39], Hydrogen_g_consumption[40], Hydrogen_g_consumption[41], Hydrogen_g_consumption[42], Hydrogen_g_consumption[43], Hydrogen_g_consumption[44], Hydrogen_g_consumption[45], Hydrogen_g_consumption[46], Hydrogen_g_consumption[47], Hydrogen_g_consumption[48], Hydrogen_g_consumption[49], Hydrogen_g_consumption[50], Hydrogen_g_consumption[51], Hydrogen_g_consumption[52], Hydrogen_g_consumption[53], Hydrogen_g_consumption[54], Hydrogen_g_consumption[55], Hydrogen_g_consumption[56], Hydrogen_g_consumption[57], Hydrogen_g_consumption[58], Hydrogen_g_consumption[59], Hydrogen_g_consumption[60], Hydrogen_g_consumption[61], Hydrogen_g_consumption[62], Hydrogen_g_consumption[63], Hydrogen_g_consumption[64], Hydrogen_g_consumption[65], Hydrogen_g_consumption[66], Hydrogen_g_consumption[67], Hydrogen_g_consumption[68], Hydrogen_g_consumption[69], Hydrogen_g_consumption[70], Hydrogen_g_consumption[71], Hydrogen_g_consumption[72], Hydrogen_g_consumption[73], Hydrogen_g_consumption[74], Hydrogen_g_consumption[75], Hydrogen_g_consumption[76], Hydrogen_g_consumption[77], Hydrogen_g_consumption[78], Hydrogen_g_consumption[79], Hydrogen_g_consumption[80], Hydrogen_g_consumption[81], Hydrogen_g_consumption[82], Hydrogen_g_consumption[83], Hydrogen_g_consumption[84], Hydrogen_g_consumption[85], Hydrogen_g_consumption[86], Hydrogen_g_consumption[87], Hydrogen_g_consumption[88], Hydrogen_g_consumption[89], Hydrogen_g_consumption[90], Hydrogen_g_consumption[91], Hydrogen_g_consumption[92], Hydrogen_g_consumption[93], Hydrogen_g_consumption[94], Hydrogen_g_consumption[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'hydrogenConsumption(g)' ", Hydrogen_g_consumptionSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (cost_name, %s) VALUES('%s','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", column, "demand_response_feedback", demandResponse_feedback[0], demandResponse_feedback[1], demandResponse_feedback[2], demandResponse_feedback[3], demandResponse_feedback[4], demandResponse_feedback[5], demandResponse_feedback[6], demandResponse_feedback[7], demandResponse_feedback[8], demandResponse_feedback[9], demandResponse_feedback[10], demandResponse_feedback[11], demandResponse_feedback[12], demandResponse_feedback[13], demandResponse_feedback[14], demandResponse_feedback[15], demandResponse_feedback[16], demandResponse_feedback[17], demandResponse_feedback[18], demandResponse_feedback[19], demandResponse_feedback[20], demandResponse_feedback[21], demandResponse_feedback[22], demandResponse_feedback[23], demandResponse_feedback[24], demandResponse_feedback[25], demandResponse_feedback[26], demandResponse_feedback[27], demandResponse_feedback[28], demandResponse_feedback[29], demandResponse_feedback[30], demandResponse_feedback[31], demandResponse_feedback[32], demandResponse_feedback[33], demandResponse_feedback[34], demandResponse_feedback[35], demandResponse_feedback[36], demandResponse_feedback[37], demandResponse_feedback[38], demandResponse_feedback[39], demandResponse_feedback[40], demandResponse_feedback[41], demandResponse_feedback[42], demandResponse_feedback[43], demandResponse_feedback[44], demandResponse_feedback[45], demandResponse_feedback[46], demandResponse_feedback[47], demandResponse_feedback[48], demandResponse_feedback[49], demandResponse_feedback[50], demandResponse_feedback[51], demandResponse_feedback[52], demandResponse_feedback[53], demandResponse_feedback[54], demandResponse_feedback[55], demandResponse_feedback[56], demandResponse_feedback[57], demandResponse_feedback[58], demandResponse_feedback[59], demandResponse_feedback[60], demandResponse_feedback[61], demandResponse_feedback[62], demandResponse_feedback[63], demandResponse_feedback[64], demandResponse_feedback[65], demandResponse_feedback[66], demandResponse_feedback[67], demandResponse_feedback[68], demandResponse_feedback[69], demandResponse_feedback[70], demandResponse_feedback[71], demandResponse_feedback[72], demandResponse_feedback[73], demandResponse_feedback[74], demandResponse_feedback[75], demandResponse_feedback[76], demandResponse_feedback[77], demandResponse_feedback[78], demandResponse_feedback[79], demandResponse_feedback[80], demandResponse_feedback[81], demandResponse_feedback[82], demandResponse_feedback[83], demandResponse_feedback[84], demandResponse_feedback[85], demandResponse_feedback[86], demandResponse_feedback[87], demandResponse_feedback[88], demandResponse_feedback[89], demandResponse_feedback[90], demandResponse_feedback[91], demandResponse_feedback[92], demandResponse_feedback[93], demandResponse_feedback[94], demandResponse_feedback[95]);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'demandResponse_feedbackPrice' ", demandResponse_feedbackSum);
		sent_query();
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", totalLoad[0], totalLoad[1], totalLoad[2], totalLoad[3], totalLoad[4], totalLoad[5], totalLoad[6], totalLoad[7], totalLoad[8], totalLoad[9], totalLoad[10], totalLoad[11], totalLoad[12], totalLoad[13], totalLoad[14], totalLoad[15], totalLoad[16], totalLoad[17], totalLoad[18], totalLoad[19], totalLoad[20], totalLoad[21], totalLoad[22], totalLoad[23], totalLoad[24], totalLoad[25], totalLoad[26], totalLoad[27], totalLoad[28], totalLoad[29], totalLoad[30], totalLoad[31], totalLoad[32], totalLoad[33], totalLoad[34], totalLoad[35], totalLoad[36], totalLoad[37], totalLoad[38], totalLoad[39], totalLoad[40], totalLoad[41], totalLoad[42], totalLoad[43], totalLoad[44], totalLoad[45], totalLoad[46], totalLoad[47], totalLoad[48], totalLoad[49], totalLoad[50], totalLoad[51], totalLoad[52], totalLoad[53], totalLoad[54], totalLoad[55], totalLoad[56], totalLoad[57], totalLoad[58], totalLoad[59], totalLoad[60], totalLoad[61], totalLoad[62], totalLoad[63], totalLoad[64], totalLoad[65], totalLoad[66], totalLoad[67], totalLoad[68], totalLoad[69], totalLoad[70], totalLoad[71], totalLoad[72], totalLoad[73], totalLoad[74], totalLoad[75], totalLoad[76], totalLoad[77], totalLoad[78], totalLoad[79], totalLoad[80], totalLoad[81], totalLoad[82], totalLoad[83], totalLoad[84], totalLoad[85], totalLoad[86], totalLoad[87], totalLoad[88], totalLoad[89], totalLoad[90], totalLoad[91], totalLoad[92], totalLoad[93], totalLoad[94], totalLoad[95], "total_load_power");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'totalLoad' ", totalLoad_sum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", totalLoad_price[0], totalLoad_price[1], totalLoad_price[2], totalLoad_price[3], totalLoad_price[4], totalLoad_price[5], totalLoad_price[6], totalLoad_price[7], totalLoad_price[8], totalLoad_price[9], totalLoad_price[10], totalLoad_price[11], totalLoad_price[12], totalLoad_price[13], totalLoad_price[14], totalLoad_price[15], totalLoad_price[16], totalLoad_price[17], totalLoad_price[18], totalLoad_price[19], totalLoad_price[20], totalLoad_price[21], totalLoad_price[22], totalLoad_price[23], totalLoad_price[24], totalLoad_price[25], totalLoad_price[26], totalLoad_price[27], totalLoad_price[28], totalLoad_price[29], totalLoad_price[30], totalLoad_price[31], totalLoad_price[32], totalLoad_price[33], totalLoad_price[34], totalLoad_price[35], totalLoad_price[36], totalLoad_price[37], totalLoad_price[38], totalLoad_price[39], totalLoad_price[40], totalLoad_price[41], totalLoad_price[42], totalLoad_price[43], totalLoad_price[44], totalLoad_price[45], totalLoad_price[46], totalLoad_price[47], totalLoad_price[48], totalLoad_price[49], totalLoad_price[50], totalLoad_price[51], totalLoad_price[52], totalLoad_price[53], totalLoad_price[54], totalLoad_price[55], totalLoad_price[56], totalLoad_price[57], totalLoad_price[58], totalLoad_price[59], totalLoad_price[60], totalLoad_price[61], totalLoad_price[62], totalLoad_price[63], totalLoad_price[64], totalLoad_price[65], totalLoad_price[66], totalLoad_price[67], totalLoad_price[68], totalLoad_price[69], totalLoad_price[70], totalLoad_price[71], totalLoad_price[72], totalLoad_price[73], totalLoad_price[74], totalLoad_price[75], totalLoad_price[76], totalLoad_price[77], totalLoad_price[78], totalLoad_price[79], totalLoad_price[80], totalLoad_price[81], totalLoad_price[82], totalLoad_price[83], totalLoad_price[84], totalLoad_price[85], totalLoad_price[86], totalLoad_price[87], totalLoad_price[88], totalLoad_price[89], totalLoad_price[90], totalLoad_price[91], totalLoad_price[92], totalLoad_price[93], totalLoad_price[94], totalLoad_price[95], "total_load_price");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'LoadSpend(threeLevelPrice)' ", totalLoad_priceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", real_grid_pirce[0], real_grid_pirce[1], real_grid_pirce[2], real_grid_pirce[3], real_grid_pirce[4], real_grid_pirce[5], real_grid_pirce[6], real_grid_pirce[7], real_grid_pirce[8], real_grid_pirce[9], real_grid_pirce[10], real_grid_pirce[11], real_grid_pirce[12], real_grid_pirce[13], real_grid_pirce[14], real_grid_pirce[15], real_grid_pirce[16], real_grid_pirce[17], real_grid_pirce[18], real_grid_pirce[19], real_grid_pirce[20], real_grid_pirce[21], real_grid_pirce[22], real_grid_pirce[23], real_grid_pirce[24], real_grid_pirce[25], real_grid_pirce[26], real_grid_pirce[27], real_grid_pirce[28], real_grid_pirce[29], real_grid_pirce[30], real_grid_pirce[31], real_grid_pirce[32], real_grid_pirce[33], real_grid_pirce[34], real_grid_pirce[35], real_grid_pirce[36], real_grid_pirce[37], real_grid_pirce[38], real_grid_pirce[39], real_grid_pirce[40], real_grid_pirce[41], real_grid_pirce[42], real_grid_pirce[43], real_grid_pirce[44], real_grid_pirce[45], real_grid_pirce[46], real_grid_pirce[47], real_grid_pirce[48], real_grid_pirce[49], real_grid_pirce[50], real_grid_pirce[51], real_grid_pirce[52], real_grid_pirce[53], real_grid_pirce[54], real_grid_pirce[55], real_grid_pirce[56], real_grid_pirce[57], real_grid_pirce[58], real_grid_pirce[59], real_grid_pirce[60], real_grid_pirce[61], real_grid_pirce[62], real_grid_pirce[63], real_grid_pirce[64], real_grid_pirce[65], real_grid_pirce[66], real_grid_pirce[67], real_grid_pirce[68], real_grid_pirce[69], real_grid_pirce[70], real_grid_pirce[71], real_grid_pirce[72], real_grid_pirce[73], real_grid_pirce[74], real_grid_pirce[75], real_grid_pirce[76], real_grid_pirce[77], real_grid_pirce[78], real_grid_pirce[79], real_grid_pirce[80], real_grid_pirce[81], real_grid_pirce[82], real_grid_pirce[83], real_grid_pirce[84], real_grid_pirce[85], real_grid_pirce[86], real_grid_pirce[87], real_grid_pirce[88], real_grid_pirce[89], real_grid_pirce[90], real_grid_pirce[91], real_grid_pirce[92], real_grid_pirce[93], real_grid_pirce[94], real_grid_pirce[95], "real_buy_grid_price");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'realGridPurchase' ", real_grid_pirceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", real_sell_pirce[0], real_sell_pirce[1], real_sell_pirce[2], real_sell_pirce[3], real_sell_pirce[4], real_sell_pirce[5], real_sell_pirce[6], real_sell_pirce[7], real_sell_pirce[8], real_sell_pirce[9], real_sell_pirce[10], real_sell_pirce[11], real_sell_pirce[12], real_sell_pirce[13], real_sell_pirce[14], real_sell_pirce[15], real_sell_pirce[16], real_sell_pirce[17], real_sell_pirce[18], real_sell_pirce[19], real_sell_pirce[20], real_sell_pirce[21], real_sell_pirce[22], real_sell_pirce[23], real_sell_pirce[24], real_sell_pirce[25], real_sell_pirce[26], real_sell_pirce[27], real_sell_pirce[28], real_sell_pirce[29], real_sell_pirce[30], real_sell_pirce[31], real_sell_pirce[32], real_sell_pirce[33], real_sell_pirce[34], real_sell_pirce[35], real_sell_pirce[36], real_sell_pirce[37], real_sell_pirce[38], real_sell_pirce[39], real_sell_pirce[40], real_sell_pirce[41], real_sell_pirce[42], real_sell_pirce[43], real_sell_pirce[44], real_sell_pirce[45], real_sell_pirce[46], real_sell_pirce[47], real_sell_pirce[48], real_sell_pirce[49], real_sell_pirce[50], real_sell_pirce[51], real_sell_pirce[52], real_sell_pirce[53], real_sell_pirce[54], real_sell_pirce[55], real_sell_pirce[56], real_sell_pirce[57], real_sell_pirce[58], real_sell_pirce[59], real_sell_pirce[60], real_sell_pirce[61], real_sell_pirce[62], real_sell_pirce[63], real_sell_pirce[64], real_sell_pirce[65], real_sell_pirce[66], real_sell_pirce[67], real_sell_pirce[68], real_sell_pirce[69], real_sell_pirce[70], real_sell_pirce[71], real_sell_pirce[72], real_sell_pirce[73], real_sell_pirce[74], real_sell_pirce[75], real_sell_pirce[76], real_sell_pirce[77], real_sell_pirce[78], real_sell_pirce[79], real_sell_pirce[80], real_sell_pirce[81], real_sell_pirce[82], real_sell_pirce[83], real_sell_pirce[84], real_sell_pirce[85], real_sell_pirce[86], real_sell_pirce[87], real_sell_pirce[88], real_sell_pirce[89], real_sell_pirce[90], real_sell_pirce[91], real_sell_pirce[92], real_sell_pirce[93], real_sell_pirce[94], real_sell_pirce[95], "real_sell_grid_price");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'maximumSell' ", real_sell_pirceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", fuelCell_kW_price[0], fuelCell_kW_price[1], fuelCell_kW_price[2], fuelCell_kW_price[3], fuelCell_kW_price[4], fuelCell_kW_price[5], fuelCell_kW_price[6], fuelCell_kW_price[7], fuelCell_kW_price[8], fuelCell_kW_price[9], fuelCell_kW_price[10], fuelCell_kW_price[11], fuelCell_kW_price[12], fuelCell_kW_price[13], fuelCell_kW_price[14], fuelCell_kW_price[15], fuelCell_kW_price[16], fuelCell_kW_price[17], fuelCell_kW_price[18], fuelCell_kW_price[19], fuelCell_kW_price[20], fuelCell_kW_price[21], fuelCell_kW_price[22], fuelCell_kW_price[23], fuelCell_kW_price[24], fuelCell_kW_price[25], fuelCell_kW_price[26], fuelCell_kW_price[27], fuelCell_kW_price[28], fuelCell_kW_price[29], fuelCell_kW_price[30], fuelCell_kW_price[31], fuelCell_kW_price[32], fuelCell_kW_price[33], fuelCell_kW_price[34], fuelCell_kW_price[35], fuelCell_kW_price[36], fuelCell_kW_price[37], fuelCell_kW_price[38], fuelCell_kW_price[39], fuelCell_kW_price[40], fuelCell_kW_price[41], fuelCell_kW_price[42], fuelCell_kW_price[43], fuelCell_kW_price[44], fuelCell_kW_price[45], fuelCell_kW_price[46], fuelCell_kW_price[47], fuelCell_kW_price[48], fuelCell_kW_price[49], fuelCell_kW_price[50], fuelCell_kW_price[51], fuelCell_kW_price[52], fuelCell_kW_price[53], fuelCell_kW_price[54], fuelCell_kW_price[55], fuelCell_kW_price[56], fuelCell_kW_price[57], fuelCell_kW_price[58], fuelCell_kW_price[59], fuelCell_kW_price[60], fuelCell_kW_price[61], fuelCell_kW_price[62], fuelCell_kW_price[63], fuelCell_kW_price[64], fuelCell_kW_price[65], fuelCell_kW_price[66], fuelCell_kW_price[67], fuelCell_kW_price[68], fuelCell_kW_price[69], fuelCell_kW_price[70], fuelCell_kW_price[71], fuelCell_kW_price[72], fuelCell_kW_price[73], fuelCell_kW_price[74], fuelCell_kW_price[75], fuelCell_kW_price[76], fuelCell_kW_price[77], fuelCell_kW_price[78], fuelCell_kW_price[79], fuelCell_kW_price[80], fuelCell_kW_price[81], fuelCell_kW_price[82], fuelCell_kW_price[83], fuelCell_kW_price[84], fuelCell_kW_price[85], fuelCell_kW_price[86], fuelCell_kW_price[87], fuelCell_kW_price[88], fuelCell_kW_price[89], fuelCell_kW_price[90], fuelCell_kW_price[91], fuelCell_kW_price[92], fuelCell_kW_price[93], fuelCell_kW_price[94], fuelCell_kW_price[95], "FC_price");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'fuelCellSpend' ", fuelCell_kW_priceSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", Hydrogen_g_consumption[0], Hydrogen_g_consumption[1], Hydrogen_g_consumption[2], Hydrogen_g_consumption[3], Hydrogen_g_consumption[4], Hydrogen_g_consumption[5], Hydrogen_g_consumption[6], Hydrogen_g_consumption[7], Hydrogen_g_consumption[8], Hydrogen_g_consumption[9], Hydrogen_g_consumption[10], Hydrogen_g_consumption[11], Hydrogen_g_consumption[12], Hydrogen_g_consumption[13], Hydrogen_g_consumption[14], Hydrogen_g_consumption[15], Hydrogen_g_consumption[16], Hydrogen_g_consumption[17], Hydrogen_g_consumption[18], Hydrogen_g_consumption[19], Hydrogen_g_consumption[20], Hydrogen_g_consumption[21], Hydrogen_g_consumption[22], Hydrogen_g_consumption[23], Hydrogen_g_consumption[24], Hydrogen_g_consumption[25], Hydrogen_g_consumption[26], Hydrogen_g_consumption[27], Hydrogen_g_consumption[28], Hydrogen_g_consumption[29], Hydrogen_g_consumption[30], Hydrogen_g_consumption[31], Hydrogen_g_consumption[32], Hydrogen_g_consumption[33], Hydrogen_g_consumption[34], Hydrogen_g_consumption[35], Hydrogen_g_consumption[36], Hydrogen_g_consumption[37], Hydrogen_g_consumption[38], Hydrogen_g_consumption[39], Hydrogen_g_consumption[40], Hydrogen_g_consumption[41], Hydrogen_g_consumption[42], Hydrogen_g_consumption[43], Hydrogen_g_consumption[44], Hydrogen_g_consumption[45], Hydrogen_g_consumption[46], Hydrogen_g_consumption[47], Hydrogen_g_consumption[48], Hydrogen_g_consumption[49], Hydrogen_g_consumption[50], Hydrogen_g_consumption[51], Hydrogen_g_consumption[52], Hydrogen_g_consumption[53], Hydrogen_g_consumption[54], Hydrogen_g_consumption[55], Hydrogen_g_consumption[56], Hydrogen_g_consumption[57], Hydrogen_g_consumption[58], Hydrogen_g_consumption[59], Hydrogen_g_consumption[60], Hydrogen_g_consumption[61], Hydrogen_g_consumption[62], Hydrogen_g_consumption[63], Hydrogen_g_consumption[64], Hydrogen_g_consumption[65], Hydrogen_g_consumption[66], Hydrogen_g_consumption[67], Hydrogen_g_consumption[68], Hydrogen_g_consumption[69], Hydrogen_g_consumption[70], Hydrogen_g_consumption[71], Hydrogen_g_consumption[72], Hydrogen_g_consumption[73], Hydrogen_g_consumption[74], Hydrogen_g_consumption[75], Hydrogen_g_consumption[76], Hydrogen_g_consumption[77], Hydrogen_g_consumption[78], Hydrogen_g_consumption[79], Hydrogen_g_consumption[80], Hydrogen_g_consumption[81], Hydrogen_g_consumption[82], Hydrogen_g_consumption[83], Hydrogen_g_consumption[84], Hydrogen_g_consumption[85], Hydrogen_g_consumption[86], Hydrogen_g_consumption[87], Hydrogen_g_consumption[88], Hydrogen_g_consumption[89], Hydrogen_g_consumption[90], Hydrogen_g_consumption[91], Hydrogen_g_consumption[92], Hydrogen_g_consumption[93], Hydrogen_g_consumption[94], Hydrogen_g_consumption[95], "hydrogen_consumption");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'hydrogenConsumption(g)' ", Hydrogen_g_consumptionSum);
		sent_query();

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '%s';", demandResponse_feedback[0], demandResponse_feedback[1], demandResponse_feedback[2], demandResponse_feedback[3], demandResponse_feedback[4], demandResponse_feedback[5], demandResponse_feedback[6], demandResponse_feedback[7], demandResponse_feedback[8], demandResponse_feedback[9], demandResponse_feedback[10], demandResponse_feedback[11], demandResponse_feedback[12], demandResponse_feedback[13], demandResponse_feedback[14], demandResponse_feedback[15], demandResponse_feedback[16], demandResponse_feedback[17], demandResponse_feedback[18], demandResponse_feedback[19], demandResponse_feedback[20], demandResponse_feedback[21], demandResponse_feedback[22], demandResponse_feedback[23], demandResponse_feedback[24], demandResponse_feedback[25], demandResponse_feedback[26], demandResponse_feedback[27], demandResponse_feedback[28], demandResponse_feedback[29], demandResponse_feedback[30], demandResponse_feedback[31], demandResponse_feedback[32], demandResponse_feedback[33], demandResponse_feedback[34], demandResponse_feedback[35], demandResponse_feedback[36], demandResponse_feedback[37], demandResponse_feedback[38], demandResponse_feedback[39], demandResponse_feedback[40], demandResponse_feedback[41], demandResponse_feedback[42], demandResponse_feedback[43], demandResponse_feedback[44], demandResponse_feedback[45], demandResponse_feedback[46], demandResponse_feedback[47], demandResponse_feedback[48], demandResponse_feedback[49], demandResponse_feedback[50], demandResponse_feedback[51], demandResponse_feedback[52], demandResponse_feedback[53], demandResponse_feedback[54], demandResponse_feedback[55], demandResponse_feedback[56], demandResponse_feedback[57], demandResponse_feedback[58], demandResponse_feedback[59], demandResponse_feedback[60], demandResponse_feedback[61], demandResponse_feedback[62], demandResponse_feedback[63], demandResponse_feedback[64], demandResponse_feedback[65], demandResponse_feedback[66], demandResponse_feedback[67], demandResponse_feedback[68], demandResponse_feedback[69], demandResponse_feedback[70], demandResponse_feedback[71], demandResponse_feedback[72], demandResponse_feedback[73], demandResponse_feedback[74], demandResponse_feedback[75], demandResponse_feedback[76], demandResponse_feedback[77], demandResponse_feedback[78], demandResponse_feedback[79], demandResponse_feedback[80], demandResponse_feedback[81], demandResponse_feedback[82], demandResponse_feedback[83], demandResponse_feedback[84], demandResponse_feedback[85], demandResponse_feedback[86], demandResponse_feedback[87], demandResponse_feedback[88], demandResponse_feedback[89], demandResponse_feedback[90], demandResponse_feedback[91], demandResponse_feedback[92], demandResponse_feedback[93], demandResponse_feedback[94], demandResponse_feedback[95], "demand_response_feedback");
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'demandResponse_feedbackPrice' ", demandResponse_feedbackSum);
		sent_query();
	}

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE BaseParameter SET value = %f WHERE parameter_name = 'LoadSpend(taipowerPrice)' ", totalLoad_taipowerPriceSum);
	sent_query();

	messagePrint(__LINE__, "public loads power cost(kW): ", 'F', publicLoad_sum, 'Y');
	messagePrint(__LINE__, "public loads power cost by three level electric price(NTD): ", 'F', publicLoad_priceSum, 'Y');
	messagePrint(__LINE__, "total loads power cost(kW): ", 'F', totalLoad_sum, 'Y');
	messagePrint(__LINE__, "total loads power cost by taipower(NTD): ", 'F', totalLoad_taipowerPriceSum, 'Y');
	messagePrint(__LINE__, "total loads power cost by three level electric price(NTD): ", 'F', totalLoad_priceSum, 'Y');
	messagePrint(__LINE__, "buy total grid(NTD): ", 'F', real_grid_pirceSum, 'Y');
	messagePrint(__LINE__, "sell total grid(NTD):  ", 'F', real_sell_pirceSum, 'Y');
	messagePrint(__LINE__, "fuelCell cost(NTD): ", 'F', fuelCell_kW_priceSum, 'Y');
	messagePrint(__LINE__, "hydrogen comsumotion(g): ", 'F', Hydrogen_g_consumptionSum, 'Y');
	messagePrint(__LINE__, "demand response feedback price(NTD): ", 'F', demandResponse_feedbackSum, 'Y');
	// step1_bill = opt_cost_result - opt_sell_result;
	// step1_sell = opt_sell_result;
}

void calculateCostInfo(BASEPARAMETER bp, DEMANDRESPONSE dr, PUBLICLOAD pl)
{
	functionPrint(__func__);

	float totalLoad[bp.time_block] = {0.0}, totalLoad_price[bp.time_block] = {0.0}, real_grid_pirce[bp.time_block] = {0.0}, publicLoad[bp.time_block] = {0.0}, publicLoad_price[bp.time_block] = {0.0};
	float totalLoad_sum = 0.0, totalLoad_priceSum = 0.0, real_grid_pirceSum = 0.0, publicLoad_sum = 0.0, publicLoad_priceSum = 0.0, totalLoad_taipowerPriceSum = 0.0;
	float fuelCell_kW_price[bp.time_block] = {0.0}, Hydrogen_g_consumption[bp.time_block] = {0.0};
	float fuelCell_kW_priceSum = 0.0, Hydrogen_g_consumptionSum = 0.0;
	float real_sell_pirce[bp.time_block] = {0.0}, real_sell_pirceSum = 0.0;
	float demandResponse_feedback[bp.time_block] = {0.0}, demandResponse_feedbackSum = 0.0;

	for (int i = 0; i < bp.sample_time; i++)
	{
		if (pl.flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "public_load_power");
			publicLoad[i] = turn_value_to_float(0);
			publicLoad_sum += publicLoad[i];

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "public_load_price");
			publicLoad_price[i] = turn_value_to_float(0);
			publicLoad_priceSum += publicLoad_price[i];
		}

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "total_load_power");
		totalLoad[i] = turn_value_to_float(0);
		totalLoad_sum += totalLoad[i];

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "total_load_price");
		totalLoad_price[i] = turn_value_to_float(0);
		totalLoad_priceSum += totalLoad_price[i];

		if (bp.Pgrid_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "real_buy_grid_price");
			float grid_tmp = turn_value_to_float(0);
			real_grid_pirce[i] = grid_tmp;
			real_grid_pirceSum += real_grid_pirce[i];
			if (dr.mode != 0)
			{
				if (i >= dr.startTime && i < dr.endTime)
				{
					demandResponse_feedback[i] = dr.feedback_price * (dr.customer_baseLine - grid_tmp) * bp.delta_T;
					demandResponse_feedbackSum += demandResponse_feedback[i];
				}
			}
		}
		if (bp.Psell_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "real_sell_grid_price");
			real_sell_pirce[i] = turn_value_to_float(0);
			real_sell_pirceSum += real_sell_pirce[i];
		}
		if (bp.Pfc_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "FC_price");
			fuelCell_kW_price[i] = turn_value_to_float(0);
			fuelCell_kW_priceSum += fuelCell_kW_price[i];

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM cost WHERE cost_name = '%s'", i, "hydrogen_consumption");
			Hydrogen_g_consumption[i] = turn_value_to_float(0);
			Hydrogen_g_consumptionSum += Hydrogen_g_consumption[i];
		}
	}

	for (int i = bp.sample_time; i < bp.time_block; i++)
	{
		// =-=-=-=-=-=-=- calculate total load spend how much money if only use grid power -=-=-=-=-=-=-= //
		if (pl.flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT totalLoad FROM totalLoad_model WHERE time_block = %d ", i);
			totalLoad[i] = turn_value_to_float(0);

			for (int j = 0; j < pl.forceToStop_number; j++)
			{
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM GHEMS_control_status WHERE equip_name = '%s' ", i, (pl.str_forceToStop_publicLoad+to_string(j+1)).c_str());
				int status_tmp = turn_value_to_int(0);
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = '5' LIMIT %d, %d", j, j + 1);
				float power_tmp = turn_value_to_float(0);
				publicLoad[i] += status_tmp * power_tmp;
			}
			for (int j = 0; j < pl.interrupt_number; j++)
			{
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM GHEMS_control_status WHERE equip_name = '%s' ", i, (pl.str_interrupt_publicLoad+to_string(j+1)).c_str());
				int status_tmp = turn_value_to_int(0);
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = '6' LIMIT %d, %d", j, j + 1);
				float power_tmp = turn_value_to_float(0);
				publicLoad[i] += status_tmp * power_tmp;
			}
			for (int j = 0; j < pl.periodic_number; j++)
			{
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM GHEMS_control_status WHERE equip_name = '%s' ", i, (pl.str_periodic_publicLoad+to_string(j+1)).c_str());
				int status_tmp = turn_value_to_int(0);
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = '7' LIMIT %d, %d", j, j + 1);
				float power_tmp = turn_value_to_float(0);
				publicLoad[i] += status_tmp * power_tmp;
			}
			publicLoad_sum += publicLoad[i];
			publicLoad_price[i] = publicLoad[i] * bp.price[i] * bp.delta_T;
			publicLoad_priceSum += publicLoad_price[i];
		}
		totalLoad[i] += publicLoad[i];
		totalLoad_sum += totalLoad[i];
		totalLoad_price[i] = totalLoad[i] * bp.price[i] * bp.delta_T;
		totalLoad_priceSum += totalLoad_price[i];

		// =-=-=-=-=-=-=- calcalte optimize Pgrid consumption spend how much money -=-=-=-=-=-=-= //
		if (bp.Pgrid_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM GHEMS_control_status WHERE equip_name = '%s' ", i, bp.str_Pgrid.c_str());
			float grid_tmp = turn_value_to_float(0);
			real_grid_pirce[i] = grid_tmp * bp.price[i] * bp.delta_T;
			real_grid_pirceSum += real_grid_pirce[i];
			if (dr.mode != 0)
			{
				if (i >= dr.startTime && i < dr.endTime)
				{
					demandResponse_feedback[i] = dr.feedback_price * (dr.customer_baseLine - grid_tmp) * bp.delta_T;
					demandResponse_feedbackSum += demandResponse_feedback[i];
				}
			}
		}

		// =-=-=-=-=-=-=- calcalte optimize Psell consumption save how much money -=-=-=-=-=-=-= //
		if (bp.Psell_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM GHEMS_control_status WHERE equip_name = '%s' ", i, bp.str_Psell.c_str());
			real_sell_pirce[i] = turn_value_to_float(0) * bp.price[i] * bp.delta_T;
			real_sell_pirceSum += real_sell_pirce[i];
		}

		// =-=-=-=-=-=-=- calcalte optimize Pfct consumption how much money & how many grams hydrogen -=-=-=-=-=-=-= //
		if (bp.Pfc_flag)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM GHEMS_control_status WHERE equip_name = '%s' ", i, bp.str_Pfct.c_str());
			float fuelCell_tmp = turn_value_to_float(0);
			fuelCell_kW_price[i] = fuelCell_tmp * Hydro_Price / Hydro_Cons * bp.delta_T;
			fuelCell_kW_priceSum += fuelCell_kW_price[i];
			Hydrogen_g_consumption[i] = fuelCell_tmp / Hydro_Cons * bp.delta_T;
			Hydrogen_g_consumptionSum += Hydrogen_g_consumption[i];
		}
	}

	//NOW taipower cost reference --> https://www.taipower.com.tw/upload/238/2018070210412196443.pdf
	if (totalLoad_sum <= (120.0 / 30.0))
		totalLoad_taipowerPriceSum = totalLoad_sum * P_1;

	else if ((totalLoad_sum > (120.0 / 30.0)) & (totalLoad_sum <= 330.0 / 30.0))
		totalLoad_taipowerPriceSum = (120.0 * P_1 + (totalLoad_sum * 30.0 - 120.0) * P_2) / 30.0;

	else if ((totalLoad_sum > (330.0 / 30.0)) & (totalLoad_sum <= 500.0 / 30.0))
		totalLoad_taipowerPriceSum = (120.0 * P_1 + (330.0 - 120.0) * P_2 + (totalLoad_sum * 30.0 - 330.0) * P_3) / 30.0;

	else if ((totalLoad_sum > (500.0 / 30.0)) & (totalLoad_sum <= 700.0 / 30.0))
		totalLoad_taipowerPriceSum = (120.0 * P_1 + (330.0 - 120.0) * P_2 + (500.0 - 330.0) * P_3 + (totalLoad_sum * 30.0 - 500.0) * P_4) / 30.0;

	else if ((totalLoad_sum > (700.0 / 30.0)) & (totalLoad_sum <= 1000.0 / 30.0))
		totalLoad_taipowerPriceSum = (120.0 * P_1 + (330.0 - 120.0) * P_2 + (500.0 - 330.0) * P_3 + (700.0 - 500.0) * P_4 + (totalLoad_sum * 30.0 - 700.0) * P_5) / 30.0;

	else if (totalLoad_sum > (1000.0 / 30.0))
		totalLoad_taipowerPriceSum = (120.0 * P_1 + (330.0 - 120.0) * P_2 + (500.0 - 330.0) * P_3 + (700.0 - 500.0) * P_4 + (1000.0 - 700.0) * P_5 + (totalLoad_sum * 30.0 - 1000.0) * P_6) / 30.0;

	updateTableCost(bp, totalLoad, totalLoad_price, real_grid_pirce, publicLoad, publicLoad_price, fuelCell_kW_price, Hydrogen_g_consumption, real_sell_pirce, demandResponse_feedback, totalLoad_sum, totalLoad_priceSum, real_grid_pirceSum, publicLoad_sum, publicLoad_priceSum, fuelCell_kW_priceSum, Hydrogen_g_consumptionSum, real_sell_pirceSum, totalLoad_taipowerPriceSum, demandResponse_feedbackSum);
}

void updateSingleHouseholdCost(BASEPARAMETER bp, DEMANDRESPONSE dr)
{
	functionPrint(__func__);
	
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `BaseParameter` WHERE parameter_name = 'householdAmount'");
	float householdTotal = turn_value_to_float(0);
	// O_{u,ca}^{cost}
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `BaseParameter` WHERE parameter_name = 'publicLoadSpend(threeLevelPrice)'");
	float single_public_price = turn_value_to_float(0) / householdTotal;
	
	vector<float> each_household_feedbackTotalPrice;
	each_household_feedbackTotalPrice.assign(householdTotal, 0);
	if (dr.mode != 0)
	{
		vector<float> C_uj;
		for (int j = dr.startTime; j < dr.endTime; j++)
		{
			C_uj.assign(householdTotal, 0);
			// O_{dr}^{j,feedback}
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `A%d` FROM `cost` WHERE `cost_name` = 'demand_response_feedback'", j);
			float each_timeblock_feedbackPrice = turn_value_to_float(0);

			for (int i = 0; i < householdTotal; i++)
			{
				// P_{u, grid}^{avg}
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT MAX(household%d) FROM `LHEMS_demand_response_CBL` WHERE `time_block` BETWEEN %d AND %d AND `comfort_level_flag` = (SELECT value FROM BaseParameter WHERE parameter_name = 'comfortLevel_flag')", i + 1, dr.startTime, dr.endTime - 1);
				float P_uavg = turn_value_to_float(0);
				
				// D_{u}^{j}
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `LHEMS_demand_response_participation` WHERE household_id = %d", j, i + 1);
				int participate = turn_value_to_int(0);

				// P_{u, grid}^{j}
				snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `A%d` FROM `LHEMS_control_status` WHERE `equip_name` = 'Pgrid' AND `household_id` = %d ", j, i + 1);
				float P_ugrid = turn_value_to_float(0);

				C_uj[i] = participate * dr.feedback_price * (P_uavg - P_ugrid) * bp.delta_T;
			}

			float sum_C_uj = accumulate(C_uj.begin(), C_uj.end(), 0);

			for (int i = 0; i < householdTotal; i++)
			{
				// O_{u, dr}^{feedback}
				each_household_feedbackTotalPrice[i] += each_timeblock_feedbackPrice * C_uj[i] / sum_C_uj;
			}
		}	
	}

	for (int i = 0; i < householdTotal; i++)
	{
		// T_{u, price}
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT origin_grid_price FROM `LHEMS_cost` WHERE household_id = %d", i + 1);
		float single_origin_grid_price = turn_value_to_float(0);
		// T_{u, price}^{total}
		float origin_pay_price = single_origin_grid_price + single_public_price;
		float real_grid_price = 0;
		for (int j = 0; j < bp.time_block; j++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT round((SELECT `A%d` FROM `LHEMS_cost` WHERE `household_id` = %d)/SUM(A%d), 4) FROM `LHEMS_cost` ORDER BY `household_id` ASC", j, i + 1, j);
			float each_timeblock_household_percent =  turn_value_to_float(0);
			// O_{total}^{j, cost}
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `A%d` FROM `cost` WHERE cost_name = 'real_buy_grid_price'", j);
			float each_timeblock_gridCost = turn_value_to_float(0);
			// O_{u, cost}
			real_grid_price += each_timeblock_gridCost * each_timeblock_household_percent;
		}
		
		// O_{u, cost}^{total}
		float final_pay_price = real_grid_price - each_household_feedbackTotalPrice[i];
		float saving_efficiency = (origin_pay_price - final_pay_price) / origin_pay_price;
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LHEMS_cost` SET `real_grid_price` = '%.3f', `public_price` = '%.3f', `feedback_price` = '%.3f', `origin_pay_price` = '%.3f', `final_pay_price` = '%.3f', `saving_efficiency` = '%.5f' WHERE `household_id` = %d", real_grid_price, single_public_price, each_household_feedbackTotalPrice[i], origin_pay_price, final_pay_price, saving_efficiency, i + 1);
		sent_query();
	}	
}

void insert_GHEMS_variable(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess)
{
	functionPrint(__func__);
	messagePrint(__LINE__, "Vsys = ", 'F', ess.battery_rate, 'Y');
	messagePrint(__LINE__, "Cbat = ", 'F', ess.capacity, 'Y');
	messagePrint(__LINE__, "Pbat_min = ", 'F', ess.MIN_power, 'Y');
	messagePrint(__LINE__, "Pbat_max = ", 'F', ess.MAX_power, 'Y');
	messagePrint(__LINE__, "Pgrid_max = ", 'F', bp.Pgrid_max, 'Y');
	messagePrint(__LINE__, "Psell_max = ", 'F', bp.Psell_max, 'Y');
	messagePrint(__LINE__, "Pfc_max = ", 'F', bp.Pfc_max, 'Y');

	string ghems_variable = "`battery_rate`, `Cbat`, `Pbat_min`, `Pbat_max`, `Pgrid_max`, `Psell_max`, `Pfc_max`, `datetime`";
	snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `GHEMS_variable` (%s) VALUES ( '%.3f', '%.3f', '%.3f', '%.3f', '%.3f', '%.3f', '%.3f', CURRENT_TIMESTAMP)", ghems_variable.c_str(), ess.battery_rate, ess.capacity, ess.MIN_power, ess.MAX_power, bp.Pgrid_max, bp.Psell_max, bp.Pfc_max);
	sent_query();
}

float getPrevious_battery_dischargeSOC(int time_block, int sample_time, string target_equip_name)
{
	functionPrint(__func__);
	float dischargeSOC = 0.0;
	float totaldischargeSOC = 0.0;
	for (int i = 0; i < sample_time; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `GHEMS_control_status` WHERE `equip_name` = '%s'", i, target_equip_name.c_str());
		float SOC_tmp = turn_value_to_float(0);
		if (SOC_tmp != -404)
			dischargeSOC += SOC_tmp;
	}
	for (int i = sample_time; i < time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM `GHEMS_control_status` WHERE `equip_name` = '%s'", i, target_equip_name.c_str());
		float SOC_tmp = turn_value_to_float(0);
		if (SOC_tmp != -404)
			totaldischargeSOC += SOC_tmp;
	}
	totaldischargeSOC += dischargeSOC;
	messagePrint(__LINE__, "Already discharge SOC from SOC = ", 'F', dischargeSOC, 'Y');
	messagePrint(__LINE__, "Total discharge SOC from SOC = ", 'F', totaldischargeSOC, 'Y');
	return dischargeSOC;
}

float *get_totalLoad_power(int time_block, bool uncontrollable_load_flag)
{
	functionPrint(__func__);
	float *load_model = new float[time_block];
	for (int i = 0; i < time_block; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT totalLoad FROM totalLoad_model WHERE time_block = %d", i);
		load_model[i] = turn_value_to_float(0);
		if (uncontrollable_load_flag == 1)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT totalLoad FROM LHEMS_uncontrollable_load WHERE time_block = %d", i);
			load_model[i] += turn_value_to_float(0);
		}
	}

	return load_model;
}

int *countPublicLoads_AlreadyOpenedTimes(BASEPARAMETER bp, int publicLoad_num, string publicLoad_name)
{
	functionPrint(__func__);
	int *buff = new int[publicLoad_num];
	for (int i = 0; i < publicLoad_num; i++)
	{
		buff[i] = 0;
	}
	if (bp.sample_time != 0)
	{
		for (int i = 0; i < publicLoad_num; i++)
		{
			int coun = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM GHEMS_control_status WHERE equip_name = '%s'", column, (publicLoad_name + to_string(i + 1)).c_str());
			fetch_row_value();
			for (int j = 0; j < bp.sample_time; j++)
			{
				coun += turn_int(j);
			}
			buff[i] = coun;
			messagePrint(__LINE__, "Public load opened times: ", 'I', buff[i], 'Y');
		}
	}
	return buff;
}

vector<int> count_publicLoads_RemainOperateTime(int public_num, vector<int> public_ot, int *buff)
{
	functionPrint(__func__);
	vector<int> public_reot;
	public_reot.assign(public_num, 0);
	for (int i = 0; i < public_num; i++)
	{
		if ((public_ot[i] - buff[i]) == public_ot[i])
		{
			public_reot[i] = public_ot[i];
		}
		else if (((public_ot[i] - buff[i]) < public_ot[i]) && ((public_ot[i] - buff[i]) > 0))
		{
			public_reot[i] = public_ot[i] - buff[i];
		}
		else if ((public_ot[i] - buff[i]) <= 0)
		{
			public_reot[i] = 0;
		}
		messagePrint(__LINE__, "Public load remain times: ", 'I', public_reot[i], 'Y');
	}
	return public_reot;
}

// uncontrollable load
void Global_UCload_rand_operationTime(BASEPARAMETER bp, UNCONTROLLABLELOAD &ucl)
{
	functionPrint(__func__);

	float *result = new float[bp.time_block];
	
	
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `load_list` WHERE `group_id` = 8");
	ucl.number =  turn_value_to_int(0);
	
	if (!ucl.flag)
	{
		for (int i = 0; i < ucl.number; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `GHEMS_uncontrollable_load` SET `uncontrollable_load%d` = '0.0' ", i + 1);
			sent_query();		
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `GHEMS_uncontrollable_load` SET `totalLoad` = '0.0' ");
		sent_query();
		
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
					for (int i = 0; i < bp.time_block; i++)
						result[i] = 0.0;
					
					snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `uncontrollable_loads`, `power1` FROM `load_list` WHERE `group_id` = 8 LIMIT %d, %d", i, i + 1);
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
					for (int j = time_seperate[0]; j < time_seperate[1]; j++)
					{
						if (operate_count != time_seperate[2])
						{
							int operate_tmp = rand() % 2;
							float operate_power = operate_tmp * power;
							operate_count += operate_tmp;
							result[j] += operate_power;
						}
					}
					time_seperate.clear();
					
					for (int j = 0; j < bp.time_block; j++)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `GHEMS_uncontrollable_load` SET `uncontrollable_load%d` = '%.1f' WHERE `time_block` = %d;", i + 1, result[j], j);
						sent_query();
					}
				}
				for (int j = 0; j < bp.time_block; j++)
				{
					float power_total = 0.0;
					for (int i = 0; i < ucl.number; i++)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `uncontrollable_load%d` FROM `GHEMS_uncontrollable_load` WHERE time_block = %d", i + 1, j);
						power_total += turn_value_to_float(0);
					}
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `GHEMS_uncontrollable_load` SET `totalLoad` = '%.1f' WHERE `time_block` = %d;", power_total, j);
					sent_query();
				}
			}
		}
		for (int i = 0; i < bp.time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `totalLoad` FROM `GHEMS_uncontrollable_load` WHERE `time_block` = %d;", i);
			result[i] = turn_value_to_float(0);
		}
		ucl.power_array = result;
	}
}

void update_fullSOC_or_overtime_EM_inPole(ELECTRICMOTOR em, int sample_time)
{
	functionPrint(__func__);

	float normal_power = 0.0, discharge_normal_power = 0.0;
	
	for (int i = 0; i < em.total_charging_pole; i++)
	{
		// check each pole is allowed charging or discharging, update SOC
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `EM_Pole`.`Pole_ID`, `EM_Pole`.`number`, `EM_Pole`.`sure`, `EM_Pole`.`charging_status`, `EM_Pole`.`discharge_status`, `EM_Pole`.`full`, `EM_Pole`.`wait`, `EM_Pole`.`SOC`, `EM_Pole`.`BAT_CAP`, `EM_Pole`.`P_charging_pole`, `EM_Pole`.`Departure_timeblock`,  `EM_motor_type`.`voltage` FROM `EM_Pole` LEFT JOIN `EM_user_result` ON `EM_Pole`.`number`=`EM_user_result`.`number` LEFT JOIN `EM_motor_type` ON `EM_user_result`.`type`=`EM_motor_type`.`id` WHERE EM_Pole.id = '%d'", i + 1);
		fetch_row_value();
		int pole_id = turn_int(0);
		int number = turn_int(1);
		bool allow_chargeOrDischarge = turn_int(2);
		bool charging_status = turn_int(3);
		bool discharging_status = turn_int(4);
		bool SOC_full = turn_int(5);
		int wait = turn_int(6);
		float SOC = turn_float(7);
		float charging_pole_power = turn_float(9);
		float departure_timeblock = turn_float(10);
		float BAT_capacity = (turn_float(8) * turn_float(11)) / 1000;
		
		if (allow_chargeOrDischarge)
		{
			// charging now, vehicle SOC increase
			if (charging_status)
			{
				normal_power += charging_pole_power;
				
				SOC += (charging_pole_power * 0.25) / BAT_capacity;
				if (SOC >= em.MAX_SOC)
				{
					SOC = em.MAX_SOC;
					if (i < em.normal_charging_pole)
					{
						if (em.can_discharge)
							snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
						else
							snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `sure` = 0, `charging_status` = '0', `full` = 1, `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
						sent_query();
					}
				}
				else
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET SOC = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
					sent_query();
				}
			}
			// normal pole discharging now, vehicle SOC decrease
			else if (discharging_status)
			{
				discharge_normal_power -= charging_pole_power;

				SOC -= (charging_pole_power * 0.25) / BAT_capacity;
				if (SOC <= em.MIN_SOC)
				{
					SOC = em.MIN_SOC;
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
					sent_query();
				}
				else
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
					sent_query();
				}
			}	
			
			// vehicle charging times up, set full = 1, and determine later
			if (departure_timeblock - 1 <= sample_time)
			{
				// setting correspond pole id not to charge or discharge
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `sure` = '0', `charging_status` = '0' , `discharge_status` = '0', `full` = '1', `SOC` = '%.3f' WHERE `Pole_ID` = %d;", SOC, pole_id);
				sent_query();
			}
		}
		
		// check full flag, and start decrease wait (stay time) until become 0 
		if (SOC_full)
		{
			if (departure_timeblock <= sample_time)
			{
				if (wait == 0)
				{
					// clean pole and record result
					empty_charging_pole("EM_Pole", pole_id);
					record_vehicle_result("EM_user_result", SOC, sample_time, number);
				}
				else
				{
					wait--;
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_Pole` SET `wait` = '%d' WHERE `Pole_ID` = %d;", wait, pole_id);
					sent_query();
				}
			}
		}
		if (sample_time == 95)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `number` FROM `EM_Pole` WHERE `id` = '%d'", i + 1);
			int number = turn_value_to_int(0);
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `SOC` FROM `EM_Pole` WHERE `id` = '%d'", i + 1);
			float SOC = turn_value_to_float(0);
			empty_charging_pole("EM_Pole", pole_id);
			record_vehicle_result("EM_user_result", SOC, sample_time, number);
		}
	}

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EM_user_number` SET `total_power` = %.2f, `normal_power` = '%.2f',`discharge_normal_power` = '%.2f' WHERE `timeblock` = '%d'", normal_power, normal_power, discharge_normal_power, sample_time);
	sent_query();
}

void update_fullSOC_or_overtime_EV_inPole(ELECTRICVEHICLE ev, int sample_time)
{
	functionPrint(__func__);

	float normal_power = 0.0, discharge_normal_power = 0.0;
	
	for (int i = 0; i < ev.total_charging_pole; i++)
	{
		// check each pole is allowed charging or discharging, update SOC
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `Pole_ID`, `number`, `sure`, `charging_status`, `discharge_status`, `full`, `wait`, `SOC`, `BAT_CAP`, `P_charging_pole`, `Departure_timeblock` FROM `EV_Pole` WHERE EV_Pole.id = '%d'", i + 1);
		fetch_row_value();
		int pole_id = turn_int(0);
		int number = turn_int(1);
		bool allow_chargeOrDischarge = turn_int(2);
		bool charging_status = turn_int(3);
		bool discharging_status = turn_int(4);
		bool SOC_full = turn_int(5);
		int wait = turn_int(6);
		float SOC = turn_float(7);
		float charging_pole_power = turn_float(9);
		float departure_timeblock = turn_float(10);
		float BAT_capacity = turn_float(8);
		
		if (allow_chargeOrDischarge)
		{
			// charging now, vehicle SOC increase
			if (charging_status)
			{
				normal_power += charging_pole_power;
				
				SOC += (charging_pole_power * 0.25) / BAT_capacity;
				if (SOC >= ev.MAX_SOC)
				{
					SOC = ev.MAX_SOC;
					if (i < ev.charging_power)
					{
						if (ev.can_discharge)
							snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
						else
							snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `sure` = 0, `charging_status` = '0', `full` = 1, `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
						sent_query();
					}
				}
				else
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
					sent_query();
				}
			}
			// normal pole discharging now, vehicle SOC decrease
			else if (discharging_status)
			{
				discharge_normal_power -= charging_pole_power;

				SOC -= (charging_pole_power * 0.25) / BAT_capacity;
				if (SOC <= ev.MIN_SOC)
				{
					SOC = ev.MIN_SOC;
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
					sent_query();
				}
				else
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `SOC` = %.3f WHERE `Pole_ID` = '%d' ", SOC, pole_id);
					sent_query();
				}
			}	
			
			// vehicle charging times up, set full = 1, and determine later
			if (departure_timeblock - 1 <= sample_time)
			{
				// setting correspond pole id not to charge or discharge
				snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `sure` = '0', `charging_status` = '0' , `discharge_status` = '0', `full` = '1', `SOC` = '%.3f' WHERE `Pole_ID` = %d;", SOC, pole_id);
				sent_query();
			}
		}
		
		// check full flag, and start decrease wait (stay time) until become 0 
		if (SOC_full)
		{
			if (departure_timeblock <= sample_time)
			{
				if (wait == 0)
				{
					// clean pole and record result
					empty_charging_pole("EV_Pole", pole_id);
					record_vehicle_result("EV_user_result", SOC, sample_time, number);
				}
				else
				{
					wait--;
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_Pole` SET `wait` = '%d' WHERE `Pole_ID` = %d;", wait, pole_id);
					sent_query();
				}
			}
		}
		if (sample_time == 95)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `number` FROM `EV_Pole` WHERE `id` = '%d'", i + 1);
			int number = turn_value_to_int(0);
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `SOC` FROM `EV_Pole` WHERE `id` = '%d'", i + 1);
			float SOC = turn_value_to_float(0);
			empty_charging_pole("EV_Pole", pole_id);
			record_vehicle_result("EV_user_result", SOC, sample_time, number);
		}
	}

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `EV_user_number` SET `total_power` = %.2f, `normal_power` = '%.2f',`discharge_normal_power` = '%.2f' WHERE `timeblock` = '%d'", normal_power, normal_power, discharge_normal_power, sample_time);
	sent_query();
}

// Contained in 'update_fullSOC_or_overtime_EM_inPole'
void record_vehicle_result(string table, float SOC, int sample_time, int number)
{
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `%s` SET `Departure_SOC` = '%.2f', `Real_departure_timeblock` = '%d' WHERE `number` = %d;", table.c_str(), SOC, sample_time, number);
	sent_query();
}

// Contained in 'update_fullSOC_or_overtime_EM_inPole'
void empty_charging_pole(string table, int pole_id)
{
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `%s` SET `number` = NULL, `sure` = '0', `charging_status` = '0', `discharge_status` = '0', `full` = '0' , `wait` = 0, `SOC` = NULL, `BAT_CAP` = NULL , `Start_timeblock` = NULL , `Departure_timeblock` = NULL WHERE `Pole_ID` = %d;", table.c_str(), pole_id);
	sent_query();	
}

int enter_newEMInfo_inPole(ELECTRICMOTOR em, int sample_time)
{
	functionPrint(__func__);

	// count normal not using charging pole id
	vector<int> empty_normal_pole, usingNow_normal_pole;
	for (int i = 0; i < em.total_charging_pole; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `Pole_ID` FROM `EM_Pole` WHERE `id` = '%d'", i + 1);
		int pole_id = turn_value_to_int(0);
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `sure` FROM `EM_Pole` WHERE `id` = '%d'", i + 1);
		bool allow_chargeOrDischarge = turn_value_to_int(0);
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `full` FROM `EM_Pole` WHERE `id` = '%d'", i + 1);
		bool SOC_full = turn_value_to_int(0);

		if (allow_chargeOrDischarge || SOC_full)
			usingNow_normal_pole.push_back(pole_id);
		else
			empty_normal_pole.push_back(pole_id);
	}
	
	// get current timeblock will enter how many new EM and number
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `user_number` FROM `EM_user_number` WHERE `timeblock` = '%d'", sample_time);
	int normal_EM_amount = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(user_number) FROM `EM_user_number` WHERE `timeblock` < '%d'", sample_time);
	int normal_start_number = turn_value_to_int(0);
	
	// When sample_time=0 start_number will return -999, so set start number = 0
	if (normal_start_number == -999) {normal_start_number = 0;}
	// get which type and how many user will get in parking lot in this timeblock
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(capacity) FROM `EM_motor_type`");
	int type_count = turn_value_to_int(0);
	vector<float> normal_capacity, fast_capacity, super_fast_capacity;
	vector<int> normal_type_id, fast_type_id, super_fast_type_id, normal_user, fast_user, super_fast_user;
	for (int i = 0; i < type_count; i++)
	{
		switch (i)
		{
		case 0:
		case 1:
		case 3:
		case 4:
			break;
		default:
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `type_%d` FROM `EM_wholeDay_userChargingNumber` WHERE `timeblock` = '%d';", i, sample_time);
			normal_user.push_back(turn_value_to_int(0));
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `capacity` FROM `EM_motor_type` WHERE `id` = %d", i);
			normal_capacity.push_back(turn_value_to_float(0));
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `id` FROM `EM_motor_type` WHERE `id` = %d", i);
			normal_type_id.push_back(turn_value_to_int(0));
			break;
		}
	}
	
	if (em.generate_result_flag)
	{
		messagePrint(__LINE__, "Generate random vehicle result", 'S', 0, 'Y');
		int normal_soc_mean = value_receive("EM_Parameter_of_randomResult", "parameter_name", "normal_soc_mean");
		int normal_soc_variance = value_receive("EM_Parameter_of_randomResult", "parameter_name", "normal_soc_variance");
		int normal_time_mean = value_receive("EM_Parameter_of_randomResult", "parameter_name", "normal_time_mean");
		int normal_time_variance = value_receive("EM_Parameter_of_randomResult", "parameter_name", "normal_time_variance");
		int normal_wait_mean = value_receive("EM_Parameter_of_randomResult", "parameter_name", "normal_wait_mean");
		int normal_wait_variance = value_receive("EM_Parameter_of_randomResult", "parameter_name", "normal_wait_variance");
		
		// insert normal/fast/super fast user reuslt, EM content include: motor type, battery capacity, start timeblock, departure timeblock, SOC, stay time
		generate_vehicle_result(em, "EM_user_result", normal_EM_amount, normal_time_mean, normal_time_variance, normal_soc_mean, normal_soc_variance, normal_wait_mean, normal_wait_variance, normal_start_number, normal_user, normal_capacity, normal_type_id, empty_normal_pole, sample_time);
	}
	else
	{
		messagePrint(__LINE__, "Fetch vehicle result", 'S', 0, 'Y');
		fetch_vehicle_result("EM_user_result", "EM_Pole", "EM_chargingOrDischarging_status", normal_EM_amount, normal_start_number, empty_normal_pole, sample_time);
	}
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `EM_Pole` WHERE `sure` = 1");
	return turn_value_to_int(0);
}

int enter_newEVInfo_inPole(ELECTRICVEHICLE ev, int sample_time)
{
	functionPrint(__func__);

	// count normal not using charging pole id
	vector<int> empty_pole, usingNow_pole;
	for (int i = 0; i < ev.total_charging_pole; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `Pole_ID` FROM `EV_Pole` WHERE `id` = '%d'", i + 1);
		int pole_id = turn_value_to_int(0);
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `sure` FROM `EV_Pole` WHERE `id` = '%d'", i + 1);
		bool allow_chargeOrDischarge = turn_value_to_int(0);
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `full` FROM `EV_Pole` WHERE `id` = '%d'", i + 1);
		bool SOC_full = turn_value_to_int(0);

		if (allow_chargeOrDischarge || SOC_full)
			usingNow_pole.push_back(pole_id);
		else
			empty_pole.push_back(pole_id);
	}
	
	// get current timeblock will enter how many new EM and number
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `user_number` FROM `EV_user_number` WHERE `timeblock` = '%d'", sample_time);
	int EV_amount = turn_value_to_int(0);
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT SUM(user_number) FROM `EV_user_number` WHERE `timeblock` < '%d'", sample_time);
	int start_number = turn_value_to_int(0);
	
	// When sample_time=0 start_number will return -999, so set start number = 0
	if (start_number == -999) {start_number = 0;}
	// get which type and how many user will get in parking lot in this timeblock
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(`capacity(kWh)`) FROM `EV_motor_type`");
	int type_count = turn_value_to_int(0);
	vector<float> capacity;
	vector<int> type_id, user;
	for (int i = 0; i < type_count; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `type_%d` FROM `EV_wholeDay_userChargingNumber` WHERE `timeblock` = '%d';", i, sample_time);
		user.push_back(turn_value_to_int(0));
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `capacity(kWh)` FROM `EV_motor_type` WHERE `id` = %d", i);
		capacity.push_back(turn_value_to_float(0));
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `id` FROM `EV_motor_type` WHERE `id` = %d", i);
		type_id.push_back(turn_value_to_int(0));
	}
	
	if (ev.generate_result_flag)
	{
		messagePrint(__LINE__, "Generate random vehicle result", 'S', 0, 'Y');
		int soc_mean = value_receive("EV_Parameter_of_randomResult", "parameter_name", "soc_mean");
		int soc_variance = value_receive("EV_Parameter_of_randomResult", "parameter_name", "soc_variance");
		int time_mean = value_receive("EV_Parameter_of_randomResult", "parameter_name", "time_mean");
		int time_variance = value_receive("EV_Parameter_of_randomResult", "parameter_name", "time_variance");
		int wait_mean = value_receive("EV_Parameter_of_randomResult", "parameter_name", "wait_mean");
		int wait_variance = value_receive("EV_Parameter_of_randomResult", "parameter_name", "wait_variance");
		
		generate_vehicle_result(ev, "EV_user_result", EV_amount, time_mean, time_variance, soc_mean, soc_variance, wait_mean, wait_variance, start_number, user, capacity, type_id, empty_pole, sample_time);
	}
	else
	{
		messagePrint(__LINE__, "Fetch vehicle result", 'S', 0, 'Y');
		fetch_vehicle_result("EV_user_result", "EV_Pole", "EV_chargingOrDischarging_status", EV_amount, start_number, empty_pole, sample_time);
	}
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT COUNT(*) FROM `EV_Pole` WHERE `sure` = 1");
	return turn_value_to_int(0);
}

// Contained in 'enter_newEMInfo_inPole'
void generate_vehicle_result(ELECTRICMOTOR em, string table, int EM_amount, int time_mean, int time_variance, int soc_mean, int soc_variance, int wait_mean, int wait_variance, int start_number, vector<int> user, vector<float> capacity, vector<int> type_id, vector<int> empty_pole, int sample_time)
{
	int x = 0, type_num = 0;
	int empty_pole_amount = empty_pole.size();
	mt19937 generator(time(NULL));

	// current timeblock will enter 'EM_amount' new EM and number
	for (int i = 0; i < EM_amount; i++)
	{
		if (empty_pole_amount > 0)
		{
			normal_distribution<float> time_dis(time_mean, time_variance);
			normal_distribution<float> soc_dis(soc_mean, soc_variance);
			normal_distribution<float> wait_dis(wait_mean, wait_variance);
			float BAT_capacity = 0.0;
			// FIXME: always insert the same type vehicle
			// decide which type vehicle to insert

			if (/*x == 0 &&*/ user[x] != 0) {BAT_capacity = capacity[x]; user[x] -= 1; type_num = type_id[x]; goto decide_type;} else {x++;}
			if (/*x == 1 &&*/ user[x] != 0) {BAT_capacity = capacity[x]; user[x] -= 1; type_num = type_id[x]; goto decide_type;} else {x++;}
			if (/*x == 2 &&*/ user[x] != 0) {BAT_capacity = capacity[x]; user[x] -= 1; type_num = type_id[x]; goto decide_type;} else {x++;}
			if (/*x == 3 &&*/ user[x] != 0) {BAT_capacity = capacity[x]; user[x] -= 1; type_num = type_id[x]; goto decide_type;} else {x++;}
			if (/*x == 4 &&*/ user[x] != 0) {BAT_capacity = capacity[x]; user[x] -= 1; type_num = type_id[x]; goto decide_type;} else {x++;}
			if (/*x == 5 &&*/ user[x] != 0) {BAT_capacity = capacity[x]; user[x] -= 1; type_num = type_id[x]; goto decide_type;} else {x++;}

			decide_type:
			int start_timeblock = sample_time;
			int rand_parktime = time_dis(generator);
			int rand_wait = wait_dis(generator);
			float rand_SOC = soc_dis(generator) / 100;

			if (rand_SOC < em.MIN_SOC) {rand_SOC = em.MIN_SOC;}
			if (rand_SOC > em.threshold_SOC) {rand_SOC = em.threshold_SOC;}
			if (rand_parktime <= 0) {rand_parktime = time_mean + time_variance;}
			float departure_timeblock = start_timeblock + rand_parktime;
			if (departure_timeblock > 95) {departure_timeblock = 95;}
			if (rand_wait < 0) {rand_wait = 1;}

			enter_charging_pole("EM_Pole", "EM_chargingOrDischarging_status", start_number+i+1, rand_wait, rand_SOC, BAT_capacity, start_timeblock, departure_timeblock, empty_pole[i]);
			insert_vehicle_result(table, empty_pole[i], start_number+i+1, type_num, BAT_capacity, start_timeblock, rand_SOC, departure_timeblock, rand_wait);
			
			empty_pole_amount--;
		}
		else
		{
			insert_vehicle_result(table, 0, start_number+i+1, 0, 0, sample_time, 0, 0, 0);
		}
	}
}

void generate_vehicle_result(ELECTRICVEHICLE ev, string table, int EV_amount, int time_mean, int time_variance, int soc_mean, int soc_variance, int wait_mean, int wait_variance, int start_number, vector<int> user, vector<float> capacity, vector<int> type_id, vector<int> empty_pole, int sample_time)
{
	int x = 0, type_num = 0;
	int empty_pole_amount = empty_pole.size();
	mt19937 generator(time(NULL));

	// current timeblock will enter 'EV_amount' new EM and number
	for (int i = 0; i < EV_amount; i++)
	{
		if (empty_pole_amount > 0)
		{
			normal_distribution<float> time_dis(time_mean, time_variance);
			normal_distribution<float> soc_dis(soc_mean, soc_variance);
			normal_distribution<float> wait_dis(wait_mean, wait_variance);
			float BAT_capacity = 0.0;
			// FIXME: always insert the same type vehicle
			// decide which type vehicle to insert
			do
			{
				if (user[x] != 0)
				{
					BAT_capacity = capacity[x];
					user[x] -= 1;
					type_num = type_id[x];
					goto decide_type;
				}
				else
				{
					x++;
				}
			} while (1);

			decide_type:
			int start_timeblock = sample_time;
			int rand_parktime = time_dis(generator);
			int rand_wait = wait_dis(generator);
			float rand_SOC = soc_dis(generator) / 100;

			if (rand_SOC < ev.MIN_SOC) {rand_SOC = ev.MIN_SOC;}
			if (rand_SOC > ev.threshold_SOC) {rand_SOC = ev.threshold_SOC;}
			if (rand_parktime <= 0) {rand_parktime = time_mean + time_variance;}
			float departure_timeblock = start_timeblock + rand_parktime;
			if (departure_timeblock > 95) {departure_timeblock = 95;}
			if (rand_wait < 0) {rand_wait = 1;}

			enter_charging_pole("EV_Pole", "EV_chargingOrDischarging_status", start_number+i+1, rand_wait, rand_SOC, BAT_capacity, start_timeblock, departure_timeblock, empty_pole[i]);
			insert_vehicle_result(table, empty_pole[i], start_number+i+1, type_num, BAT_capacity, start_timeblock, rand_SOC, departure_timeblock, rand_wait);
			
			empty_pole_amount--;
		}
		else
		{
			insert_vehicle_result(table, 0, start_number+i+1, 0, 0, sample_time, 0, 0, 0);
		}
	}
}

// Contained in 'enter_newEMInfo_inPole'
void fetch_vehicle_result(string result_table, string pole_table, string cdstatus_table, int EM_amount, int start_number, vector<int> empty_pole, int sample_time)
{
	int x = 0, type_num = 0;
	int empty_pole_amount = empty_pole.size();
	
	// current timeblock will enter 'EM_amount' new EM and number
	for (int i = 0; i < EM_amount; i++)
	{
		if (empty_pole_amount > 0)
		{
			int start_timeblock = sample_time;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `BAT_CAP` FROM %s WHERE `start_timeblock` = '%d' AND `number` = '%d'", result_table.c_str(), sample_time, start_number+i+1);
			float BAT_capacity = turn_value_to_float(0);
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `Start_SOC` FROM %s WHERE `start_timeblock` = '%d' AND `number` = '%d'", result_table.c_str(), sample_time, start_number+i+1);
			float SOC = turn_value_to_float(0);
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `Original_departure_timeblock` FROM %s WHERE `start_timeblock` = '%d' AND `number` = '%d'", result_table.c_str(), sample_time, start_number+i+1);
			int departure_timeblock = turn_value_to_int(0);
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `wait` FROM %s WHERE `start_timeblock` = '%d' AND `number` = '%d'", result_table.c_str(), sample_time, start_number+i+1);
			int wait = turn_value_to_int(0);
			enter_charging_pole(pole_table, cdstatus_table, start_number+i+1, wait, SOC, BAT_capacity, start_timeblock, departure_timeblock, empty_pole[i]);

			empty_pole_amount--;
		}
	}
}

// Contained in 'enter_newEMInfo_inPole'
void enter_charging_pole(string pole_table, string cdstatus_table, int number, int wait, float SOC, float BAT_capacity, int start_timeblock, int departure_timeblock, int pole_id)
{
	if (BAT_capacity != 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `%s` SET `number` = '%d', `sure` = '1', `wait` = '%d', `SOC` = '%.2f', `BAT_CAP` = '%.2f', `Start_timeblock` = '%d', `Departure_timeblock` = '%d' WHERE `Pole_ID` = %d;", pole_table.c_str(), number, wait, SOC, BAT_capacity, start_timeblock, departure_timeblock, pole_id);
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `%s` (`user_number`) VALUES ('%d')", cdstatus_table.c_str(), number);
		sent_query();
	}
}

// Contained in 'enter_newEMInfo_inPole'
void insert_vehicle_result(string table, int Pole_ID, int number, int type, float BAT_CAP, int start_timeblock, float start_SOC, int original_departure_timeblock, int wait)
{
	if (BAT_CAP != 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `%s` (`Pole_ID`, `number`, `type`, `BAT_CAP`, `Start_timeblock`, `Start_SOC`, `Original_departure_timeblock`, `wait`) VALUES ('%d', '%d', '%d', '%.2f', '%d', '%.2f', '%d', '%d');", table.c_str(), Pole_ID, number, type, BAT_CAP, start_timeblock, start_SOC, original_departure_timeblock, wait);
		sent_query();
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `%s` (`Pole_ID` ,`number`, `type` , `BAT_CAP` , `Start_timeblock` ,`Start_SOC`, `Original_departure_timeblock` , `wait`) VALUES ('%d', '%d', NULL, NULL, '%d', '%.2f', '%d', '%d');", table.c_str(), Pole_ID, number, start_timeblock, start_SOC, original_departure_timeblock, wait);
		sent_query();
	}
}