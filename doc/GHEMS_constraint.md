# GHEMS_constraint

Detail formula show in **thesis_mathFormula.md** & **allCase_mathFormula.md**, part *Community Energy Management System*

### Related to *Public Load*
+ summation_stoppablePublicLoadRa_biggerThan_QaMinusD
+ summation_deferrablePublicLoadRa_biggerThan_Qa

### Related to *Grid when Demand Response*
+ pgrid_smallerThan_muGridMultiplyByPgridMaxArray

### Related to *Sell*
+ psell_smallerThan_oneMinusMuGridMultiplyByPsellMax
+ psell_smallerThan_PfuelCellPlusPsolar

### Related to *ESS*
+ previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold
+ previousSOCPlusPessTransToSOC_equalTo_currentSOC
+ pessPositive_smallerThan_zMultiplyByPchargeMax
+ pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax
+ pessPositiveMinusPessNegative_equalTo_Pess

### Related to *Balance Equation*
+ pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoadPlusPchargingEMPlusPchargingEV

### Related to *Demand Response*
+ targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs
+ summation_EMEVPcharge_smallerThan_PgridPlusPessPlusPpv

### Related to *Fuel Cell*
+ pfcOnPlusPfcOff_equalTo_pfuelCell
+ pfcOn_smallerThan_mufcMultiplyByPfcMax
+ pfcOn_biggerThan_mufcMultiplyByPfcMin
+ pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown
+ pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc
+ pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc
+ zPfcOnePlusZPfcTwo_etc_equalTo_one
+ lambdaPfc_smallerThan_zpfc

### Related to *SOC change*
+ SOCPositiveMinusSOCNegative_equalTo_SOCchange
+ SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC
+ SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC
+ SOCchange_equalTo_PessTransToSOC
+ summation_SOCNegative_biggerThan_targetDischargeSOC

### Related to *Electric Motor*
+ EM_Rcharging_smallerThan_mu
+ EM_Rdischarging_smallerThan_oneMinusMu
+ EM_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin
+ EM_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold

### Related to *Electric Vehicle*
+ EV_Rcharging_smallerThan_mu
+ EV_Rdischarging_smallerThan_oneMinusMu
+ EV_previousSOCPlusPchargeMinusPdischargeTransToSOC_biggerThan_SOCmin
+ EV_previousSOCPlusSummationPchargeMinusPdischargeTransToSOC_biggerThan_SOCthreshold

### Related to *GLPK Objective Function*
+ setting_GHEMS_ObjectiveFunction