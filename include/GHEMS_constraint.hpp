#ifndef GHEMS_constraint_H
#define GHEMS_constraint_H

extern int coef_row_num, bnd_row_num;
extern float Hydro_Cons, Hydro_Price;

void summation_publicLoadRa_biggerThan_QaMinusD(int *public_start, int *public_end, int *public_reot, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pgrid_smallerThan_muGridMultiplyByPgridMaxArray(int dr_mode, vector<float> Pgrid_max_array, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// sell
void psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void psell_smallerThan_PfuelCellPlusPsolar(bool Pfc_flag, float *solar2, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// battery
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusPessTransToSOC_equalTo_currentSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositive_smallerThan_zMultiplyByPchargeMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositiveMinusPessNegative_equalTo_Pess(float **coefficient, glp_prob *mip, int row_num_maxAddition);

// balance function
void pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoadPlusPchargingEM(int *public_start, int *public_end, float *public_p, float *solar2, float *load_model, vector<int> departure_timeblock, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// demand response
void targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(float **coefficient, glp_prob *mip, int row_num_maxAddition);

// fuel cell 
void pfcOnPlusPfcOff_equalTo_pfuelCell(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfcOn_smallerThan_mufcMultiplyByPfcMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfcOn_biggerThan_mufcMultiplyByPfcMin(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(float *P_power, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(float *P_power_all, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void zPfcOnePlusZPfcTwo_etc_equalTo_one(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void lambdaPfc_smallerThan_zpfc(float **coefficient, glp_prob *mip, int row_num_maxAddition);

// SOC change 
void SOCPositiveMinusSOCNegative_equalTo_SOCchange(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void SOCchange_equalTo_PessTransToSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_SOCNegative_biggerThan_targetDischargeSOC(float target_dischargeSOC, float already_dischargeSOC, float **coefficient, glp_prob *mip, int row_num_maxAddition);

// EM
void EM_Rcharging_smallerThan_mu(vector<int> departure_timeblock, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EM_Rdischarging_smallerThan_oneMinusMu(vector<int> departure_timeblock, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EM_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin(vector<int> departure_timeblock, vector<float> EM_now_SOC, vector<float> battery_capacity, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void EM_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold(vector<int> departure_timeblock, vector<float> EM_now_SOC, vector<int> start_timeblock, vector<float> EM_start_SOC, vector<float> battery_capacity, float **coefficient, glp_prob *mip, int row_num_maxAddition);

void setting_GHEMS_ObjectiveFunction(float* price, glp_prob *mip);
#endif
