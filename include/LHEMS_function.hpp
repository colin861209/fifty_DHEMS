#ifndef fifty_LHEMS_function_H
#define fifty_LHEMS_function_H

#include <vector>
#include <string>
using namespace std;

typedef struct 
{
    int group_id = 1;
    bool flag;
    int number;
    int *start;
    int *end;
    int *ot;
    int *reot;
    float *power;
	string str_interrupt = "interrupt";
} INTERRUPTLOAD;

typedef struct 
{
    int group_id = 2;
    bool flag;
    int number;
    int *start;
    int *end;
    int *ot;
    int *reot;
    float *power;
    bool *continuous_flag;
	string str_uninterrupt = "uninterrupt";
	string str_uninterDelta = "uninterDelta";
} UNINTERRUPTLOAD;

typedef struct 
{
    int group_id = 3;
    bool flag;
    int number;
    int *start;
    int *end;
    int *ot;
    int *reot;
    int **block;
    float **power;
    float *max_power;
    bool *continuous_flag;
    int **block_tmp;
    float **power_tmp;
	string str_varying = "varying";
	string str_varyingDelta = "varyingDelta";
	string str_varyingPsi = "varyingPsi";
} VARYINGLOAD;

typedef struct 
{
    bool flag;
    int comfortLevel = 4;
    int total_timeInterval = 3;
	float **weighting;

} COMFORTLEVEL;

int determine_realTimeOrOneDayMode_andGetSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM &ess, int real_time, int distributed_group_num);
// Load's function
void getLoads_startEndOperationTime_and_power(INTERRUPTLOAD &irl, BASEPARAMETER bp);
void getLoads_startEndOperationTime_and_power(UNINTERRUPTLOAD &uirl, BASEPARAMETER bp);
void getLoads_startEndOperationTime_and_power(VARYINGLOAD &varl, BASEPARAMETER bp);
void countUninterruptAndVaryingLoads_Flag(BASEPARAMETER bp, UNINTERRUPTLOAD &uirl, VARYINGLOAD &varl);
void countLoads_AlreadyOpenedTimes(BASEPARAMETER bp, int *buff);
void count_interruptLoads_RemainOperateTime(INTERRUPTLOAD &irl, int *buff);
void count_uninterruptLoads_RemainOperateTime(BASEPARAMETER bp, UNINTERRUPTLOAD &uirl, int buff_shift_length, int *buff);
void count_varyingLoads_RemainOperateTime(BASEPARAMETER bp, VARYINGLOAD &varl, int buff_shift_length, int *buff);
void init_VaryingLoads_OperateTimeAndPower(BASEPARAMETER bp, VARYINGLOAD &varl);
void putValues_VaryingLoads_OperateTimeAndPower(BASEPARAMETER bp, VARYINGLOAD &varl);
void optimization(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, COMFORTLEVEL comlv, INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, float *uncontrollable_load,int distributed_group_num);
void update_loadModel(BASEPARAMETER bp, INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, int distributed_group_num);
void HEMS_UCload_rand_operationTime(BASEPARAMETER bp, UNCONTROLLABLELOAD &ucl, int distributed_group_num);
float *household_participation(DEMANDRESPONSE dr, int household_id, string table);
int truncate_table_flag();
int get_distributed_group(string target, string condition_col = "", int condition_num = -1);
void update_distributed_group(string target, int target_value, string condition_col, int condition_num);
void init_totalLoad_flag_and_table(BASEPARAMETER bp, int distributed_group_num);
void setting_LHEMS_columnBoundary(INTERRUPTLOAD irl, UNINTERRUPTLOAD uirl, VARYINGLOAD varl, BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, glp_prob *mip);
vector<vector<int>> get_comfortLevel_timeInterval(int household_id, int app_count, int total_timeInterval, int comfort_level);
float **calculate_comfortLevel_weighting(BASEPARAMETER bp, vector<vector<vector<int>>> comfortLevel_startEnd, int comfortLevel, int total_timeInterval);
void calculateCostInfo(BASEPARAMETER bp);
#endif
