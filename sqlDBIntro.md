# DB DHEMS_fiftyHousehold Introduction

###### tags: `DHEMS`

##### backup_BaseParameter
+ Import table `BaseParameter`
##### backup_EM_Parameter
+ Import table `EM_Parameter`
##### backup_EM_user_number
+ Import table `EM_user_number`
##### backup_EM_user_result
+ Import table `EM_user_result`
##### backup_EV_Parameter
+ Import table `EV_Parameter`
##### backup_EV_user_number
+ Import table `EV_user_number`
##### backup_EV_user_result
+ Import table `EV_user_result`
##### backup_GHEMS
+ Import table `GHEMS_control_status`
##### backup_GHEMS_uncontrollable_load
+ Import table `GHEMS_uncontrollable_load`
##### backup_LHEMS
+ Import table `LHEMS_control_status`
##### backup_LHEMS_cost
+ Import table `LHEMS_cost`
##### backup_LHEMS_uncontrollable_load
+ Import table `LHEMS_uncontrollable_load`
##### backup_totalLoad
+ Import table `totalLoad_model`
##### BaseParameter
+ Store some basic parameter, more detail in [README.md](https://github.com/colin861209/fifty_DHEMS/blob/master/README.md)
##### cost
+ Store each timeblock power and price, **In column `cost_name` saving**
    1. Power of public load 
    1. Price of public load 
    1. Power of total load  `(Includes 50 household controllable / uncontrollable power, EM & EV charge / discharge power, public load power)`
    1. Price of total load
    1. Price of grid purchased
    1. Price of grid sell
    1. Price of fuel cell
    1. Consumption of hydrogen
    1. Feedback price of demand response
##### demand_response
+ Store each cases information of demand response, **Columns Intro like below**
    1. Cases number
    2. Start timeblock
    3. End timeblock
    4. Minimize power CEMS have to decrease
    5. Feedback price (NTD/kWh)
    6. Power of customer base line
    7. reduce ratio interval ***is not using***
##### distributed_group
+ Store 10 groups' there `real_time`, `household_id from 1 to 5`, `next_simulate_timeblock`, `total_load_flag`, `uncontrollable_load_flag`
##### EM_chargingOrDischarging_status
+ Store charge / discharge status from each EM who want to charge in the parking lot 
##### EM_motor_type
+ Store different EM `names`, `capacity`, `voltage`, `power`, `how many percentage to do simulation`, pole number ***is not using***
##### EM_Parameter
+ Store some EM parameter, more detail in [README.md](https://github.com/colin861209/fifty_DHEMS/blob/master/README.md)
##### EM_Parameter_of_randomResult
+ Store some parameter of EM normal distribution, more detail in [README.md](https://github.com/colin861209/fifty_DHEMS/blob/master/README.md)
##### EM_Pole
+ Store how many charging poles and each EM user's information: `number`, `charging_status`, `discharge_status`, `full (EM stop to charge)`, `wait (timeblock which user will stay after stop charging)`, `SOC`, `EM battery capacity`, `P_charging_pole (power of charging pole)`, `Start timeblock`, `Departure timeblock` 
##### EM_user_number
+ Store how many EM users will insert into parking lot and `charging total power / discharge total power` after optimize in each timeblock
##### EM_user_result
+ Store all EM's information when EM users leaving in whole day
##### EM_wholeDay_userChargingNumber
+ Store each EM type users will insert into parking lot in each timeblock
##### EV_chargingOrDischarging_status
+ Store charge / discharge status from each EV who want to charge in the parking lot 
##### EV_motor_type
+ Store different EV `names`, `capacity(kWh)`, `power`, `how many percentage to do simulation`
##### EV_Parameter
+ Store some EV parameter, more detail in [README.md](https://github.com/colin861209/fifty_DHEMS/blob/master/README.md)
##### EV_Parameter_of_randomResult
+ Store some parameter of EV normal distribution, more detail in [README.md](https://github.com/colin861209/fifty_DHEMS/blob/master/README.md)
##### EV_Pole
+ Store how many charging poles and each EV user's information: `number`, `charging_status`, `discharge_status`, `full (EM stop to charge)`, `wait (timeblock which user will stay after stop charging)`, `SOC`, `EM battery capacity`, `P_charging_pole (power of charging pole)`, `Start timeblock`, `Departure timeblock` 
##### EV_user_number
+ Store how many EV users will insert into parking lot and `charging total power / discharge total power` after optimize in each timeblock
##### EV_user_result
+ Store all EV's information when EV users leaving in whole day
##### EV_wholeDay_userChargingNumber
+ Store each EV type users will insert into parking lot in each timeblock
##### GHEMS_control_status
+ Store optimize result from `timeblock 0~95` and there correspond `variable name` in GLPK
##### GHEMS_flag
+ Store flags which use to set secnario CEMS
##### GHEMS_uncontrollable_load
+ Store each / total power of CEMS uncontrollable load in each timeblock
##### GHEMS_variable
+ Store the information of ESS, Grid sell FC when do the new simulation
##### LHEMS_comfort_level
+ Store 23 controllable load's comfort level in each household, must join the table `load_list_select` to make sure how many amount of loads that household have
##### LHEMS_control_status
+ Store all household's optimize result from `timeblock 0~95` and there correspond `variable name` in GLPK
##### LHEMS_cost
+ Store all household's `price consumption of grid`, `price consumption after CEMS real cost` etc..., ***ALL INFORMATION is not using***
##### LHEMS_demand_response_CBL
+ Store all household's power consumption which have / haven't comfort level, purpose to find the household's customer base line in demand response
##### LHEMS_demand_response_participation
+ Store all household's which timeblock they want to participate in demand response, `0 means don't want to paritcipate, bigger than 0 means want to paritcipate`
##### LHEMS_flag
+ Store flags which use to set secnario HEMS
##### LHEMS_uncontrollable_load
+ Store each household / total power of HEMS uncontrollable load in each timeblock
##### load_list
+ Store 23 controllable load's `start, end, operation time` in each household, must join the table `load_list_select` to make sure how many amount of loads that household have
##### load_list_select
+ Store each household having which controllable load (interrupt/uninterrupt/varying)
##### price
+ Store 96 timeblock price value
    1. price_value (Ku's three level price)
    2. summer_price (summer price from Taipower)
    3. not_summer_price (not summer price from Taipower)
    4. comed price (someday's real time price from USA ComEd company)
##### solar_data
+ Store 96 timeblock solar power
    1. Sunny
    2. Big sunny
    3. Cloudy
##### solar_day
+ Store the solar power which we choose from the table `solar_data`, ***this table isn't necessary***
##### totalLoad_model
+ Store each household's / total power after optimize in each timeblock