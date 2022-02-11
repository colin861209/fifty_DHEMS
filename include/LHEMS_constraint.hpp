#ifndef LHEMS_constraint_H
#define LHEMS_constraint_H

void summation_interruptLoadRa_biggerThan_Qa(BASEPARAMETER &bp, int *interrupt_start, int *interrupt_end, int *interrupt_reot, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pgrid_smallerThan_alphaPgridMax(BASEPARAMETER &bp, DEMANDRESPONSE dr, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void alpha_between_oneminusDu_and_one(BASEPARAMETER &bp, DEMANDRESPONSE dr, int *participate_array, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, int *interrupt_start, int *interrupt_end, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, float *uninterrupt_p, int *varying_start, int *varying_end, float *uncontrollable_load, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusPessTransToSOC_equalTo_currentSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositive_smallerThan_zMultiplyByPchargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositiveMinusPessNegative_equalTo_Pess(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_uninterruptDelta_equalTo_one(BASEPARAMETER &bp, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_varyingDelta_equalTo_one(BASEPARAMETER &bp, int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void uninterruptRajToN_biggerThan_uninterruptDelta(BASEPARAMETER &bp, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_reot, bool *uninterrupt_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void varyingRajToN_biggerThan_varyingDelta(BASEPARAMETER &bp, int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(BASEPARAMETER &bp, int *varying_start, int *varying_end, int *varying_reot, bool *varying_flag, int **varying_t_d, float **varying_p_d, int *buff, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void setting_LHEMS_objectiveFunction(BASEPARAMETER bp, DEMANDRESPONSE dr, COMFORTLEVEL comlv, float* price, int *participate_array, glp_prob *mip);

#endif