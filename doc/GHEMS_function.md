# GHEMS_function

## Struct
|| Struct content | Introduction |
| :--------: | :--------: | :--------: |
||||
| **Public Load**|||
|| {public load}_group_id | See DB setting id |
|| flag | See DB setting |
|| {public load}_number | Amount from DB `load_list` |
|| {public load}_start | load's *Start time* |
|| {public load}_end | load's *End time* |
|| {public load}_operation_time | load's *Already operation time* |
|| {public load}_remain_operation_time | load's *Remain operation time* |
|| {public load}_power | load's *Power* |
||||
| **EMEV** |||
|| flag | See DB setting |
|| generate_result_flag | See DB setting |
|| can_discharge | See DB setting |
|| total_charging_pole | Charging pole amount in parking lot |
|| normal_charging_pole | Charging pole amount in parking lot |
|| can_charge_amount | EMEV users which not leaving |
|| normal_charging_power / charging_power | Charging pole power |
|| MAX_SOC | See DB setting |
|| MIN_SOC | See DB setting |
|| threshold_SOC | See DB setting |
|| Pole_ID | Parking lot id |
|| start_timeblock | EMEV users arrived time to charge |
|| departure_timeblock | EMEV users leave time |
|| number | EMEV user id |
|| now_SOC | EMEV user current SOC value |
|| start_SOC | EMEV user first SOC value |
|| battery_capacity | EMEV battery capacity |
||||
| **GLPK in need** || **Name** |
|| str_publicLoad |publicLoad |
|| str_stoppable_publicLoad |stoppable_publicLoad |
|| str_deferrable_publicLoad |deferrable_publicLoad |
|| str_charging | {EMEV}_charging |
|| str_discharging | {EMEV}_discharging |

## Function
### determine_realTimeOrOneDayMode_andGetSOC
+ Case `real_time is 0`
  + Truncate table `GHEMS_control_status, GHEMS_real_status, cost, {EMEV}_chargingOrDischarging_status`
  + Initial DB table `{EMEV}_user_result`
  + Initial DB table `{EMEV}_Pole` same means there's no users using 
  + Truncate or Initial users information from DB table `{EMEV}_user_result` 
  + Init `ESS SOC` in new simulation first time block
  + Update `real_time to 1` in new simulation first time block
+ Case `real_time is 1`
  + Truncate table `GHEMS_real_status`
  + Get `current ESS SOC`

### getOrUpdate_SolarInfo_ThroughSampleTime
+ Get weather data

### updateTableCost
+ Insert & Update information related to CEMS consumption from DB table `cost, BaseParameter`
  1. Public load (kWh)
  2. Public load Spend(threeLevelPrice) (NTD)
  3. Total load (kWh)
  4. Load spend(taipowerPrice) (NTD)
  5. Load spend(threeLevelPrice) (NTD)
  6. Real grid purchase (NTD)
  7. Maximum sell (NTD)
  8. Fuel Cell spend (NTD)
  9. Hydrogen consumption (g)
  10. Demand response feedback price (NTD)

### optimization
+ Preprocessing
  + public load remain operation time
  + EMEV users information which need to do optimization
+ GLPK setting 
  + rows & cols & variable boundary & constraint function & objective function
+ GLPK output
  + Set EMEV users who `can charge or discharge`
  + Save result to table `GHEMS_control_status`

### setting_GLPK_columnBoundary
+ Set GLPK variable upper & lower boundary

### calculateCostInfo
+ Calculate CEMS cost 

### updateSingleHouseholdCost
+ Update each household real payment after CEMS optimization

### insert_GHEMS_variable
+ Insert CEMS power supply information

### getPrevious_battery_dischargeSOC
+ Get `ESS SOC` already discharge how many percentage

### get_totalLoad_power
+ Get HEMS loads consumption from DB table `totalLoad_model`

### *countPublicLoads_AlreadyOpenedTimes
+ Count `operation time` from `public load`

### count_publicLoads_RemainOperateTime
+ Count `remain operation time` from `public load`

### Global_UCload_rand_operationTime
+ Random CEMS uncontrollable load through DB table `load_list`

## Major Function IN electric vehicle & motor
### update_fullSOC_or_overtime_EM_inPole
+ Update EM users SOC through charge / discharge flag in DB table `EM_Pole`
+ In the day of the end, set parking lot all pole to empty and record all users information
+ When user leaving, set parking lot correspond pole to empty and record users information
+ Update each time block power consumption to DB table `EM_user_result`

### update_fullSOC_or_overtime_EV_inPole
+ Update EV users SOC through charge / discharge flag in DB table `EV_Pole`
+ In the day of the end, set parking lot all pole to empty and record all users information
+ When user leaving, set parking lot correspond pole to empty and record users information
+ Update each time block power consumption to DB table `EV_user_result`

### enter_newEMInfo_inPole
+ Count empty pole amount so that new users can get in
+ Generate new users information or get old information
+ Return final users amount

### enter_newEVInfo_inPole
+ Count empty pole amount so that new users can get in
+ Generate new users information or get old information
+ Return final users amount

## Minor Function IN electric vehicle & motor
### record_vehicle_result
+ Record users information to DB table `{EMEV}_user_result`

### empty_charging_pole
+ Set parking lot all pole to empty from DB table `{EMEV}_Pole`

### generate_vehicle_result
+ Generate EMEV users information through normal distribution from DB table `{EMEV}_Parameter_of_randomResult`

### fetch_vehicle_result
+ Get old EMEV users information from DB table `{EMEV}_user_reuslt`

### enter_charging_pole
+ Insert new user information to DB table `{EMEV}_Pole`

### insert_vehicle_result
+ Insert new user information to DB table `EM_user_result`