#ifndef fifty_LHEMS_function_H
#define fifty_LHEMS_function_H

#include <vector>
#include <string>
using namespace std;


typedef struct 
{
    bool flag;
    int comfortLevel = 4;
    int total_timeInterval = 3;
	float **weighting;

} COMFORTLEVEL;

int determine_realTimeOrOneDayMode_andGetSOC(BASEPARAMETER &bp, ENERGYSTORAGESYSTEM &ess, int real_time, int distributed_group_num);
void countUninterruptAndVaryingLoads_Flag(BASEPARAMETER bp, bool *uninterrupt_flag, bool *varying_flag);
void countLoads_AlreadyOpenedTimes(BASEPARAMETER bp, int *buff);
void count_interruptLoads_RemainOperateTime(int interrupt_num, int *interrupt_ot, int *interrupt_reot, int *buff);
void count_uninterruptAndVaryingLoads_RemainOperateTime(BASEPARAMETER bp, int group_id, int loads_total, int *total_operateTime, int *remain_operateTime, int *end_time, bool *flag, int *buff);
void init_VaryingLoads_OperateTimeAndPower(BASEPARAMETER bp, int **varying_t_d, float **varying_p_d, int *varying_ot);
void putValues_VaryingLoads_OperateTimeAndPower(BASEPARAMETER bp, int **varying_t_d, float **varying_p_d, int **varying_t_pow, float **varying_p_pow, int *varying_start, int *varying_end, float *varying_p_max);
void optimization(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, COMFORTLEVEL comlv, int *interrupt_start, int *interrupt_end, int *interrupt_ot, int *interrupt_reot, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_ot, int *uninterrupt_reot, float *uninterrupt_p, bool *uninterrupt_flag, int *varying_start, int *varying_end, int *varying_ot, int *varying_reot, bool *varying_flag, int **varying_t_pow, float **varying_p_pow, float *uncontrollable_load,int distributed_group_num);
void update_loadModel(BASEPARAMETER bp, float *interrupt_p, float *uninterrupt_p, int distributed_group_num);
float *rand_operationTime(BASEPARAMETER bp, int distributed_group_num);
float *household_alpha_upperBnds(BASEPARAMETER bp, DEMANDRESPONSE dr, int distributed_group_num);
int *household_participation(DEMANDRESPONSE dr, int household_id, string table);
int truncate_table_flag();
int get_distributed_group(string target, string condition_col = "", int condition_num = -1);
void update_distributed_group(string target, int target_value, string condition_col, int condition_num);
void init_totalLoad_flag_and_table(BASEPARAMETER bp, int distributed_group_num);
void setting_LHEMS_columnBoundary(BASEPARAMETER bp, ENERGYSTORAGESYSTEM ess, DEMANDRESPONSE dr, glp_prob *mip, float* varying_p_max);
vector<vector<int>> get_comfortLevel_timeInterval(int household_id, int app_count, int total_timeInterval, int comfort_level);
float **calculate_comfortLevel_weighting(BASEPARAMETER bp, vector<vector<vector<int>>> comfortLevel_startEnd, int comfortLevel, int total_timeInterval);
void calculateCostInfo(BASEPARAMETER bp);
#endif
