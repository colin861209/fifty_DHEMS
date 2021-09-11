#ifndef LHEMS_constraint_H
#define LHEMS_constraint_H

// extern float **coefficient;
extern int coef_row_num, bnd_row_num;
// extern glp_prob *mip;
// extern int *interrupt_start, *interrupt_end, *interrupt_ot, *interrupt_reot, *uninterrupt_start, *uninterrupt_end, *uninterrupt_ot, *uninterrupt_reot, *varying_start, *varying_end, *varying_ot, *varying_reot, **varying_t_pow;
// extern float *interrupt_p, *uninterrupt_p, **varying_p_pow;
// extern bool *uninterrupt_flag, *varying_flag;

void summation_interruptLoadRa_biggerThan_Qa(int *interrupt_start, int *interrupt_end, int *interrupt_reot, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pgrid_smallerThan_alphaPgridMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void alpha_between_oneminusDu_and_one(int *participate_array, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(int *interrupt_start, int *interrupt_end, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, float *uninterrupt_p, int *varying_start, int *varying_end, float *uncontrollable_load, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusPessTransToSOC_equalTo_currentSOC(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositive_smallerThan_zMultiplyByPchargeMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositiveMinusPessNegative_equalTo_Pess(float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_uninterruptDelta_equalTo_one(int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_varyingDelta_equalTo_one(int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void uninterruptRajToN_biggerThan_uninterruptDelta(int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void varyingRajToN_biggerThan_varyingDelta(int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, int **varying_t_d, float **varying_p_d, int *buff, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void setting_LHEMS_objectiveFunction(float* price, int *participate_array, float **comfortLevelWeighting, glp_prob *mip);

#endif