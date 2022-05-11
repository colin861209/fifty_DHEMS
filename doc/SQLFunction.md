# SQLFunction

## Struct
|| Struct content | Introduction |
| :--------: | :--------: | :--------: |
||||
| **BaseParameter** |||
| **Common** | Pgrid_flag ||
|| time_block | Total time interval |
|| sample_time | Current time block |
|| remain_timeblock | time_block - sample_time |
|| divide | Split how many interval in an hour |
|| variable | Length of variable name |
|| delta_T | 1 / divide |
|| Pgrid_max | Grid power upper limit |
|| price ||
|| solar ||
|| Pgrid_max_array | Grid poewr upper limit when DR |
|| variable_name | Save variables inside which GLPK in need |
| **GLPK** | coef_row_num | start from 0 |
|| bnd_row_num | start from 1 |
| **CEMS** | mu_grid_flag | See DB setting |
|| Psell_flag | See DB setting |
|| Pfc_flag | See DB setting |
|| SOC_change_flag | See DB setting |
|| Psell_max | See DB setting |
|| Delta_battery | Not use |
|| Pfc_max | See DB setting |
|| point_num | See DB setting |
|| piecewise_num | See DB setting |
|| load_model | HEMS total load model |
| **HEMS** | app_count | Three load amount |
|| householdTotal | 50 |
|| household_id | 1~50 |
|| distributed_householdTotal | 50/10 |
|| distributed_household_id | 1~50/10 |
||||
| **SQL in need** |	str_sql_allTimeblock | A0,...,A95 |
||||
| **GLPK in need** |||
| **Common** | str_Pgrid | Pgrid |
| **GHEMS** | str_mu_grid | mu_grid |
|| str_Psell | Psell |
|| str_Pfc | Pfc |
|| str_Pfct | Pfct |
|| str_PfcON | PfcON |
|| str_PfcOFF | PfcOFF |
|| str_muFC | muFC |
|| str_zPfc | zPfc |
|| str_lambda_Pfc | lambda_Pfc |
|| str_SOC_change | SOC_change |
|| str_SOC_increase | SOC_increase |
|| str_SOC_decrease | SOC_decrease |
|| str_SOC_Z | SOC_Z |

|| Struct content | Introduction |
| :--------: | :--------: | :--------: |
||||
| **ESS** |||
|| flag | See DB setting |
|| capacity | unit (kWh) |
|| battery_rate ||
|| INIT_SOC ||
|| MIN_SOC ||
|| MAX_SOC ||
|| threshold_SOC ||
|| MIN_power | capacity*battery_rate |
|| MAX_power | capacity*battery_rate | 
||||
| **GLPK in need** |||
|| str_Pess | Pess |
|| str_Pcharge | Pcharge |
|| str_Pdischarge | Pdischarge |
|| str_SOC | ESS_SOC |
|| str_Z | ESS_Z |


|| Struct content | Introduction |
| :--------: | :--------: | :--------: |
||||
| **Demand Response** |||
| **Common** | mode ||
|| startTime ||
|| endTime ||
|| feedback_price ||
| **CEMS** | minDecrease_power ||
|| customer_baseLine ||
| **HEMS** | participate_array ||
|| household_CBL | each household power baseline  |

|| Struct content | Introduction |
| :--------: | :--------: | :--------: |
||||
| **Uncontrollable Load** |||
|| hems_group_id | 4 |
|| cems_group_id | 7 |
|| flag ||
|| generate_flag ||
|| number ||
|| power_array ||

## Function
### connect_mysql
+ Connect MySQL

### fetch_row_value
+ Sucess get result from DB will return `0`
+ Fail get result from DB will return `-1` 

### sent_query
+ Sent SQL query

### turn_int
+ Turn result from DB to `integer`
+ Fail to get result from DB will return `-999`

### turn_float
+ Turn result from DB to `float`
+ Fail to get result from DB will return `-999`

### turn_value_to_int
+ Include fetch_row_value() and turn result from DB to `integer`
+ Fail to get result from DB will return `-404`

### turn_value_to_float
+ Include fetch_row_value() and turn result from DB to `float`
+ Fail to get result from DB will return `-404`

### turn_value_to_string
+ Include fetch_row_value() and turn result from DB to `string`
+ Fail to get result from DB will return `-404`

### messagePrint

### functionPrint

### find_variableName_position
+ Use to find the variable position in GLPK

### demand_response_info
+ Return information from DB table `demand_response`

### flag_receive
+ Use to get flags from DB table `{LHEMSGHEMS}_flag` which column name is `flag`

### value_receive
+ Use to get values from DB table `BaseParameter etc...` which column name is `value`
+ Can choose integer or float to return

### get_allDay_price

### insert_status_into_MySQLTable

### update_status_to_MySQLTable

### getPublicLoad
+ Get public load information from DB table `load_list`

