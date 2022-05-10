# fifty_DHEMS

> This project is about my final master drgree topic, the goal is optimization the 50 household's load consumption deployment and community energy supplying.

> Use crontab setting execute time and shell script will do the script, the script will optimize 50 household load deployment first, then feedback the total amount of load to community, final community will optimized the energy supply we have

> `DHEMS_screenshot/web.py` is automation script for 
> 1. export related tables to csv
> 2. screenshot web page
> 
>> only use on Windows OS, and related libs are in `anaconda ENV: webDriver`

> `DHEMS_screenshot/backup.py` import csv files automatically to backup tables in DB `DHEMS_fiftyHousehold` and
> + want to re-screenshot figures (Y/N)
> + want to recover old figures, please type 1
> + want to save as new figures, please type 2
>> If upload fail, please check realted tables column amount is correct.
---
## Notice & Step
We should **modify parameters** from this [Link](http://140.124.42.65/how/DHEMS_web/BaseParameter.html) first, and change `real_time` & `Global_real_time` to `0` to do new simulation.
#### Step
+ In [基本參數](http://140.124.42.65/how/DHEMS_web/BaseParameter.html) we can modify below parameter to setting different scenario.
+ Also there is something else in Table `BaseParameter`, but it's not necessary to modify.

| Parameter name | Chinese Introduction | Value |
|:---:|:---:|:---:|
| real_time | 50戶HEMS 最佳化 | 0 to start/1 |
| Global_real_time | CEMS 最佳化 | 0 to start/1 |
| dr_mode | 需量反應模式(設定or新增請至DB) | 0~5 |
| ElectricVehicle | 電動汽車 | 0/1(add to scenario) |
| ElectricMotor | 電動機車 | 0/1(add to scenario) |
| comfortLevel_flag | HEMS舒適度 | 0/1(add to scenario) |
| uncontrollable_load_flag | HEMS不可控負載 | 0/1(add to scenario) |
| Global_uncontrollable_load_flag | CEMS不可控負載 | 0/1(add to scenario) |
| EV_generate_random_user_result | 汽車重新生成資料 | 0/1(重新生成,模擬結束將為0) |
| EM_generate_random_user_result | 機車重新生成資料 | 0/1(重新生成,模擬結束將為0) |
| generate_uncontrollable_load_flag | HEMS不可控負載重新生成資料 | 0/1(重新生成,模擬結束將為0) |
| generate_Global_uncontrollable_load_flag | CEMS不可控負載重新生成資料 | 0/1(重新生成,模擬結束將為0) |
| Pgridmax | 單戶市電最大功率(kW) | 依情景設定 |
| Cbat | 單戶電池容量(kWh) | 依情景設定 |
| battery_rate | 電池轉換率 | 依情景設定 |
| simulate_weather | 模擬天氣 | big_sunny / sunny / cloudy |
| simulate_price | 模擬電價 | summer_price / not_summer_price / price_value |
| SOCmin | 電池電量下限 | 依情景設定 |
| SOCmax | 電池電量上限 | 依情景設定 |
| SOCthres | 電池電量門檻值 | 依情景設定 |
| ini_SOC | 初始電池電量 | 依情景設定 |
| hydrogen_pric | 氫氣價錢 | 依情景設定(當前無使用) |

+ In [電動汽機車參數](http://140.124.42.65/how/DHEMS_web/emevParameter.html) we can modify EM and EV parameter setting.
+ Related table: `EM_Parameter, EM_Parameter_of_randomResult, EV_Parameter, EV_Parameter_of_randomResult`

  - EM & EV基本參數

  | EM Parameter name | EM Chinese Introduction | Value | EV Parameter name | EV Chinese Introduction | Value |
  |:---:|:---:|:---:|:---:|:---:|:---:|
  | Total_Charging_Pole | 總充電樁 | 依情景設定 | Normal_Charging_Pole | 一般充電樁 | 依情景設定 |
  | EV_Upper_SOC | 電池電量上限 | 依情景設定 | EM_Upper_SOC | 電池電量上限 | 依情景設定 |
  | EV_Lower_SOC | 電池電量下限 | 依情景設定 | EM_Lower_SOC | 電池電量下限 | 依情景設定 |
  | EV_threshold_SOC | 電池電量門檻值 | 依情景設定 | EM_threshold_SOC | 電池電量門檻值 | 依情景設定 |
  | Vehicle_can_discharge | 模擬汽車可放電旗標 | 0/1(可放電) | Motor_can_discharge | 模擬機車可放電旗標 | 0/1(可放電) |
  
  - EM & EV常態分佈用戶設定
  
  | EM Parameter name | EM Chinese Introduction | Value | EV Parameter name | EV Chinese Introduction | Value |
  |:---:|:---:|:---:|:---:|:---:|:---:|
  | soc_mean | SOC中位數 | 依情景設定 | normal_soc_mean | SOC中位數| 依情景設定 |
  | soc_variance | SOC變異數 | 依情景設定 | normal_soc_variance | SOC變異數| 依情景設定 |
  | time_mean | 充電時間中位數 | 依情景設定 | normal_time_mean | 充電時間中位數| 依情景設定 |
  | time_variance | 充電時間變異數 | 依情景設定 | normal_time_variance | 充電時間變異數| 依情景設定 |
  | wait_mean | 停留時間中位數 | 依情景設定 | normal_wait_mean | 停留時間中位數| 依情景設定 |
  | wait_variance | 停留時間變異數 | 依情景設定 | normal_wait_variance | 停留時間變異數| 依情景設定 |

+ In [所有家庭負載](http://140.124.42.65/how/DHEMS_web/index.html) we can modify HEMS scenario
+ Related table: `LHEMS_flag`

| Parameter name | Chinese Introduction | Value |
|:---:|:---:|:---:|
| interrupt | 可中斷負載旗標 | 0/1(啟用) |
| uninterrupt | 不可中斷負載旗標 | 0/1(啟用) |
| varying | 變動型負載旗標 | 0/1(啟用) |
| Pgrid | 市電旗標 | 0/1(啟用) |

+ In [社區負載監控](http://140.124.42.65/how/DHEMS_web/loadFix.html`)
+ Related table: `GHEMS_flag`

| Parameter name | Chinese Introduction | Value |
|:---:|:---:|:---:|
| publicLoad | 公共設施旗標 | 0/1(啟用) |
| Pgrid | 市電旗標 | 0/1(啟用) |
| mu_grid | 買賣電(需量反應)旗標 | 0/1(啟用) |
| Psell | 賣電旗標 | 0/1(啟用) |
| Pess | 電池旗標 | 0/1(啟用) |
| SOC_change | 整日放電80%旗標 | 0/1(啟用) |
| Pfc | 燃料電池旗標 | 0/1(啟用) |

##### Final Step: Command `crontab -e` in terminal, and change execute time
> for example: if the next minute is 19:05, please type `05 19` as below
![](https://i.imgur.com/lTg0vB3.png)

#### Notice

+ 若曾經終止運行，為與防萬一，請進入140.124.42.65[資料庫](http://140.124.42.65/phpmyadmin)
  1. 請至 Table `distributed_group` 設定所有 `household_id` 為 1
     - ``` Command: UPDATE `distributed_group` SET `household_id` = 1;```
  2. 請至 Table `BaseParameter` 設定 `next_simulate_timeblock` & `Global_next_simulate_timeblock` 為0
     - ``` Command: UPDATE `BaseParameter` SET `next_simulate_timeblock` = 0, `Global_next_simulate_timeblock`= 0; ```

+ 若使用`web.py`進行自動化截圖，
請修改`def setting_screenshot_path`中的`fix_path`為自己電腦的路徑，
確定環境無問題，再執行(再次提醒：腳本須在Windows執行)

## Commit Record
### 2021/06/03

+ Commit Link [f521b6f](https://github.com/colin861209/fifty_DHEMS/commit/f521b6ffeb56e13600e8cb9f75031692987bd828)
+ **INITIAL** files from [DHEMS](https://github.com/colin861209/DHEMS) repository

---
### 2021/06/18

+ Commit Link [e5ea65a](https://github.com/colin861209/fifty_DHEMS/commit/e5ea65a307779ddf633804eefd2094d9c5e5b386)
+ Community Public Load
+ Demand Response `alpha constraint`
+ Type `bool` for flags

---
### 2021/06/25

+ Commit Link [4d08c46](https://github.com/colin861209/fifty_DHEMS/commit/4d08c4628223385aff8e2d4fdcc0d4acd275d0e9)
+ Community Public Load `decrease operation time` when demand response mode
+ Demand Response **remove** `alpha constraint`, it's not a good constraint
+ Demand Response **increase** `household participation`, household determine their time to participate.

---
### 2021/07/21

+ Commit Link [2cd9b4b](https://github.com/colin861209/fifty_DHEMS/commit/2cd9b4b2c37d3b0a5cbc1b0d2a744aa954a119e3)
+ GLPK `constraint` functionlization in GHEMS & LHEMS

---
### 2021/07/21

+ Commit Link [5f2bcb1](https://github.com/colin861209/fifty_DHEMS/commit/5f2bcb143db9db5ecb8709134763334e4165d7c9)
+ Community Public Load `force close` when demand response mode
+ Function SQL to insert & update status to DB

---
### 2021/09/11

+ Commit Link [f30a6a7](https://github.com/colin861209/fifty_DHEMS/commit/f30a6a73bfb23b6b177e3a9ca148b4376ed3f8d6)
+ Feature: Comfort level in LHEMS

---
### 2021/10/06

+ Commit Link [4dcff12](https://github.com/colin861209/fifty_DHEMS/commit/4dcff1263e80df2546c13f292b8e9f02787b1162)
+ Feature: LHEMS Cost
+ Calculate each household's cost details
  + Origin cost
  + Public cost
  + Real cost after optimize
  + Saving efficiency

---
### 2022/01/18

+ Commit Link [cdc1b19](https://github.com/colin861209/fifty_DHEMS/commit/cdc1b19d96b8fb87f4db4ab798e2a73379c15554)
+ Feature: **EM** `Charge & Discharge` case
  + Insert newEM and update SOC in `GHEMS.cpp`
  + Collect all EM which sure to charge or discharge and optimized the result in `GHEMS_function.cpp`
  + In `GHEMS_Constraint.cpp`
  1.  Each time block can't less than min SOC in function `EM_previousSOCPlusPchargeTransToSOC_biggerThan_SOCmin`
  2.  Departure SOC bigger than threshold SOC in function `EM_previousSOCPlusSummationPchargeTransToSOC_biggerThan_SOCthreshold`

---
### 2022/03/25

+ Commit Link [8dc3370](https://github.com/colin861209/fifty_DHEMS/commit/8dc33709d8fe0f5070c61091fe16a7232e98e1c3)
+ Pgrid max won't exceed than CBL power
+ LHEMS:
+ Remove dr alpha
    + New Cost
+ GHEMS:
  + Uncontrollable Load
  + Public Load
    + force to stop
    + interrupt
    + periodic
+ web.py
  + New screenshot path
  + Fix ev em flag
  + export GHEMS_UCLoad
+ backup.py
  + Target folder
  + Rescreenshot all images or not (yY/nN)
  + Recover or SaveAs all images, if choose rescreenshot the images (1/2)
