#include "variable.hpp"
#include "new2D.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <glpk.h>
#include "scheduling_parameter.hpp"

using namespace std;

class optimize
{
private:

    IMPORT rxipt;
    glp_prob* mip;
    float **coefficient;
    int rowTotal;
    int colTotal;
    int coef_row_num = 0, bnd_row_num = 1;
    int variable_num;
    int sample_time;
    vector<string> variable_name;
    int remain_timeblock;//time block - sample time    
public:
    
    optimize(IMPORT ipt, OBJECTIVETARGET maxmin, ENERGYMANAGESYSTEM ems_type=ENERGYMANAGESYSTEM::CEMS, int group_id = -1);
    ~optimize();
    // common
    vector<vector<float>> solve_result;
    void saving_result(ENERGYMANAGESYSTEM ems_type=ENERGYMANAGESYSTEM::CEMS);
    int verify_solution_after_sovle_GLPK(ENERGYMANAGESYSTEM ems_type=ENERGYMANAGESYSTEM::CEMS);
    // hems
    void setting_hems_coefficient();
    void setting_hems_objectiveFunction();
    // cems
    void setting_cems_coefficient();
    void setting_cems_objectiveFunction();
private:
    
    void set_name(ENERGYMANAGESYSTEM ems_type=ENERGYMANAGESYSTEM::CEMS, int group_id=-1);
    void get_remainTimeblock_and_sampleTime(ENERGYMANAGESYSTEM ems_type=ENERGYMANAGESYSTEM::CEMS);
    void set_rows_total();
    void set_cols_total();
    void init_coef_matrix();
    void set_columnBoundary(ENERGYMANAGESYSTEM ems_type=ENERGYMANAGESYSTEM::CEMS);
    int find_variableName_position(vector<string> variableNameArray, string target);

// constraint
private:
    // common
    void previousSOCPlusSummationPessTransToSOC_biggerThan_SOCthreshold(int row_num_maxAddition);
    void previousSOCPlusPessTransToSOC_equalTo_currentSOC(int row_num_maxAddition);
    void pessPositive_smallerThan_zMultiplyByPchargeMax(int row_num_maxAddition);
    void pessNegative_smallerThan_oneMinusZMultiplyByPdischargeMax(int row_num_maxAddition);
    void pessPositiveMinusPessNegative_equalTo_Pess(int row_num_maxAddition);
    // hems  
    void summation_interruptLoadRa_biggerThan_Qa(int row_num_maxAddition);
    void pgrid_smallerThan_alphaPgridMax(int row_num_maxAddition);
    void alpha_between_oneminusDu_and_one(int row_num_maxAddition);
    void pgridMinusPess_equalTo_ploadPlusPuncontrollLoad(int row_num_maxAddition);
    void summation_uninterruptDelta_equalTo_one(int row_num_maxAddition);
    void uninterruptRajToN_biggerThan_uninterruptDelta(int row_num_maxAddition);
    void summation_varyingDelta_equalTo_one(int row_num_maxAddition);
    void varyingRajToN_biggerThan_varyingDelta(int row_num_maxAddition);
    void varyingPSIajToN_biggerThan_varyingDeltaMultiplyByPowerModel(int row_num_maxAddition);
    // cems 
    void summation_publicLoadRa_biggerThan_QaMinusD(int row_num_maxAddition);
    void pgrid_smallerThan_muGridMultiplyByPgridMaxArray(int row_num_maxAddition);
    void psell_smallerThan_oneMinusMuGridMultiplyByPsellMax(int row_num_maxAddition);
    void psell_smallerThan_PfuelCellPlusPsolar(int row_num_maxAddition);
    void pgridPlusPfuelCellPlusPsolarMinusPessMinusPsell_equalTo_summationPloadPlusPpublicLoad(int row_num_maxAddition);
    void targetLoadReduction_smallerThan_summationPcustomerBaseLineMinusPgridMultiplyByTs(int row_num_maxAddition);
    /* temporary won't use
    // fuel cell
    void pfcOnPlusPfcOff_equalTo_pfuelCell(int row_num_maxAddition);
    void pfcOn_smallerThan_mufcMultiplyByPfcMax(int row_num_maxAddition);
    void pfcOn_biggerThan_mufcMultiplyByPfcMin(int row_num_maxAddition);
    void pfcOff_smallerThan_oneMinusMufcMultiplyByPfcShutDown(int row_num_maxAddition);
    void pfuelCell_equalTo_xoneMultiplyByZonePlusXtwoMinusXoneMultiplyByLambdaOne_etc(int row_num_maxAddition);
    void pfuelCell_equalTo_yoneMultiplyByZonePlusYtwoMinusYoneMultiplyByLambdaOne_etc(int row_num_maxAddition);
    void zPfcOnePlusZPfcTwo_etc_equalTo_one(int row_num_maxAddition);
    void lambdaPfc_smallerThan_zpfc(int row_num_maxAddition);
    // soc change
    void SOCPositiveMinusSOCNegative_equalTo_SOCchange(int row_num_maxAddition);
    void SOCPositive_smallerThan_SOCZMultiplyByPchargeMaxTransToSOC(int row_num_maxAddition);
    void SOCNegative_smallerThan_oneMinusSOCZMultiplyByPdischargeMaxTransToSOC(int row_num_maxAddition);
    void SOCchange_equalTo_PessTransToSOC(int row_num_maxAddition);
    void summation_SOCNegative_biggerThan_targetDischargeSOC(int row_num_maxAddition);
    */
};
