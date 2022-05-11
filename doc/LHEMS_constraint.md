# LHEMS_constraint

Detail formula show in **thesis_mathFormula.md**, part *Home Energy Management System*

### Related to *Interrupt Load*
+ summation_interruptLoadRa_biggerThan_Qa

### Related to *Balanced Equation*
+ pgridMinusPess_equalTo_ploadPlusPuncontrollLoad

### Related to *ESS*
+ previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold
+ previousSOCPlusPessTransToSOC_equalTo_currentSOC
+ pessPositive_smallerThan_zMultiplyByPchargeMax
+ pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax
+ pessPositiveMinusPessNegative_equalTo_Pess

### Related to *Uninterrupt & Varying Load*
+ summation_uninterruptDelta_equalTo_one
+ summation_varyingDelta_equalTo_one
+ uninterruptRajToN_biggerThan_uninterruptDelta
+ varyingRajToN_biggerThan_varyingDelta
+ varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel

### Related to *GLPK Objective Function*
+ setting_LHEMS_objectiveFunction