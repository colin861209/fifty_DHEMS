#ifndef GHEMS_constraint_H
#define GHEMS_constraint_H

extern float Hydro_Cons, Hydro_Price;

void summation_forceToStopPublicLoadRa_biggerThan_QaMinusD(BASEPARAMETER &bp, DEMANDRESPONSE dr, PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_interruptPublicLoadRa_biggerThan_Qa(BASEPARAMETER &bp, DEMANDRESPONSE dr, PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_periodicPublicLoadRa_biggerThan_Qa(BASEPARAMETER &bp, DEMANDRESPONSE dr, PUBLICLOAD pl, float **coefficient, glp_prob *mip, int row_num_maxAddition);

void pgrid_smallerThan_muGridMultiplyByPgridMaxArray(BASEPARAMETER &bp, int dr_mode, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// sell
void psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void psell_smallerThan_PfuelCellPlusPsolar(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// battery
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusPessTransToSOC_equalTo_currentSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositive_smallerThan_zMultiplyByPchargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositiveMinusPessNegative_equalTo_Pess(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// balance function
void pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoadPlusPchargingEMPlusPchargingEV(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, PUBLICLOAD pl, ELECTRICMOTOR em, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// demand response
void targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(BASEPARAMETER &bp, DEMANDRESPONSE dr, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_EMEVPcharge_smallerThan_PgridPlusPessPlusPpv(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, ELECTRICMOTOR em, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// fuel cell 
void pfcOnPlusPfcOff_equalTo_pfuelCell(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfcOn_smallerThan_mufcMultiplyByPfcMax(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfcOn_biggerThan_mufcMultiplyByPfcMin(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(BASEPARAMETER &bp, float *P_power, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(BASEPARAMETER &bp, float *P_power_all, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void zPfcOnePlusZPfcTwo_etc_equalTo_one(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void lambdaPfc_smallerThan_zpfc(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// SOC change 
void SOCPositiveMinusSOCNegative_equalTo_SOCchange(BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void SOCchange_equalTo_PessTransToSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_SOCNegative_biggerThan_targetDischargeSOC(BASEPARAMETER &bp, float target_dischargeSOC, float already_dischargeSOC, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// EM
void EM_Rcharging_smallerThan_mu(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EM_Rdischarging_smallerThan_oneMinusMu(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EM_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EM_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ELECTRICMOTOR em, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// EV
void EV_Rcharging_smallerThan_mu(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EV_Rdischarging_smallerThan_oneMinusMu(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EV_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EV_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ELECTRICVEHICLE ev, float **coefficient, glp_prob *mip, int row_num_maxAddition);

void setting_GHEMS_ObjectiveFunction(BASEPARAMETER bp, DEMANDRESPONSE dr, ELECTRICMOTOR em, ELECTRICVEHICLE ev, float* price, glp_prob *mip);
#endif
