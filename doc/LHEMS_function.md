# LHEMS_function

## Struct
|| Struct content | Introduction |
| :--------: | :--------: | :--------: |
||||
| **Interrupt & Uninterrupt & Varying Load** |||
|| group_id | See DB setting id |
|| flag | See DB setting |
|| number | Amount from DB `load_list_select` |
|| start | load's *Start time* |
|| end | load's *End time* |
|| ot | load's *Already operation time* |
|| reot | load's *Remain operation time* |
|| power | load's *Power* |
||||
| **Uninterrupt & Varying Load** ||
|| continuous_flag | Load continue operate or not |
||||
| **Varying Load** ||
|| block | load's *each power operation time* |
|| max_power | load's *maximun power* |
||||
| **Comfort Level** ||
|| flag | See DB setting |
|| comfortLevel | Maximum level setting |
|| total_timeInterval | Maxumum interval setting |
|| weighting ||
||||
| **GLPK in need** || **Name** |
|| str_interrupt | interrupt |
|| str_uninterrupt | uninterrupt |
|| str_uninterDelta | uninterDelta |
|| str_varying | varying |
|| str_varyingDelta | varyingDelta |
|| str_varyingPsi | varyingPsi |

## Function
### determine_realTimeOrOneDayMode_andGetSOC
+ Case `real_time is 0`
  + Truncate table `LHEMS_control_status, LHEMS_real_status, LHEMS_cost`
  + Update `real_time to 1` in new simulation first time block
  + Init `ESS SOC` in new simulation first time block
+ Case `real_time is 1`
  + Truncate table `LHEMS_real_status`
  
### getLoads_startEndOperationTime_and_power
+ Through DB table `load_list & load_list_select` to get correspond `interrupt & uninterrupt & varying load` information

### countUninterruptAndVaryingLoads_Flag
+ Get `continous_flag` from `uninterrupt & varying load `

### countLoads_AlreadyOpenedTimes
+ Count `operation time` from `interrupt & uninterrupt & varying load`

### count_interruptLoads_RemainOperateTime
+ Count `remain operation time` from `interrupt load`

### count_uninterruptLoads_RemainOperateTime
+ Count `remain operation time` from `uninterrupt load`

### count_varyingLoads_RemainOperateTime
+ Count `remain operation time` from `varying load`

### init_VaryingLoads_OperateTimeAndPower
+ initial array `block & power to 0` from `varying load`
  + `Length of power` is same as the `length of block`
  + `Length of block` is same as the `remain_timeblock`

### putValues_VaryingLoads_OperateTimeAndPower
+ Set `power value` to `power array`
+ Set `1` to `block array` which between `start time` and `end time`

### optimization
+ Preprocessing
  + Inconvenience weighting
  + Uninterrupt & Varying load's continous_flag
  + Interrupt & Uninterrupt & Varying load's remain operation time
  + Varying load's power 
  + Grid power array with demand response `Power baseline (CBL)`
+ GLPK setting 
  + rows & cols & variable boundary & constraint function & objective function
+ GLPK output
  + Save result to table `LHEMS_control_status`

### update_loadModel
+ Add `interrupt & uninterrupt & varying load` power then update to each time block

### HEMS_UCload_rand_operationTime
+ Random HEMS uncontrollable load through DB table `load_list`

### household_participation
+ Get each household participation time block when demand response from DB table `LHEMS_demand_response_participation`
 
### truncate_table_flag
+ Purpose to truncate HEMS tables if `COUNT(group_id) = SUM(household_id)` from table `distributed_group` return `1`
 
### get_distributed_group
+ Get table `distributed_group` value 
 
### update_distributed_group
+ Update value to table `distributed_group`
 
### init_totalLoad_flag_and_table
+ Initial `totalLoad_flag & totalLoad to 0` from table `distributed_group & totalLoad_model`
 
### setting_LHEMS_columnBoundary
+ Set GLPK variable upper & lower boundary
 
### get_comfortLevel_timeInterval
+ Get each load's inconvenience level start time and end time
 
### calculate_comfortLevel_weighting
+ Calculate each load's inconvenience weighting
 
### calculateCostInfo
+ Calculate each household cost 