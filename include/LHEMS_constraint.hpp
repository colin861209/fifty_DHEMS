#ifndef LHEMS_constraint_H
#define LHEMS_constraint_H

// interrupt load
void summation_interruptLoadRa_biggerThan_Qa(INTERRUPTLOAD irl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
// balanced equation
void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float *uncontrollable_load, float **coefficient, glp_prob *mip, int row_num_maxAddition);
// ess
void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void previousSOCPlusPessTransToSOC_equalTo_currentSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositive_smallerThan_zMultiplyByPchargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void pessPositiveMinusPessNegative_equalTo_Pess(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM ess, float **coefficient, glp_prob *mip, int row_num_maxAddition);
// uniinterrupt& varying load
void summation_uninterruptDelta_equalTo_one(UNINTERRUPTLOAD uirl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void summation_varyingDelta_equalTo_one(VARYINGLOAD varl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void uninterruptRajToN_biggerThan_uninterruptDelta(UNINTERRUPTLOAD uirl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void varyingRajToN_biggerThan_varyingDelta(VARYINGLOAD varl, BASEPARAMETER &bp, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(VARYINGLOAD varl, BASEPARAMETER &bp, int *buff, int buff_shift_length, float **coefficient, glp_prob *mip, int row_num_maxAddition);
void setting_LHEMS_objectiveFunction(INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, BASEPARAMETER bp, DEMANDRESPONSE dr, COMFORTLEVEL comlv, glp_prob *mip);

#endif