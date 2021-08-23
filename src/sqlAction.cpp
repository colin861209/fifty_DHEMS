#include "sqlAction.hpp"

SQLACTION::SQLACTION(std::string iP, std::string name, std::string passwd, std::string database)
{	
	sql.connect(iP, name, passwd, database);
}

bool SQLACTION::determine_distributedGroup_status(string condition)
{
	sql.operate("SELECT "+ condition +" FROM `distributed_group`");
	return sql.turnValueToInt();
}

vector<int> SQLACTION::split_array(string timearray)
{
	string split_result;
	istringstream in(timearray);
	vector<int> result;
	while (getline(in, split_result, '~'))
	{
		result.push_back(stoi(split_result));
	}
	return result;
}

bool SQLACTION::get_continuityLoad_flag(string load_type, int offset_num)
{
	sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE equip_name LIKE '"+ load_type +"%' AND household_id = "+ to_string(ipt.bp.real_household_id) +" LIMIT 1 OFFSET "+ to_string(offset_num));
	vector<int> flag_status = sql.turnArrayToInt();
	flag_status.erase(flag_status.begin()+ipt.bp.next_simulate_timeblock, flag_status.end());
	return accumulate(flag_status.begin(), flag_status.end(), 0);
}

int SQLACTION::get_already_operate_time(string load_type, int offset_num)
{
	if (ipt.bp.next_simulate_timeblock == 0)
	{
		return 0;
	}
	else
	{
		sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE household_id = "+ to_string(ipt.bp.real_household_id) +" AND equip_name LIKE '"+ load_type +"%' LIMIT 1 OFFSET "+ to_string(offset_num));
		vector<int> already_ot_time = sql.turnArrayToInt();
		already_ot_time.erase(already_ot_time.begin()+ipt.bp.next_simulate_timeblock, already_ot_time.end());
		return accumulate(already_ot_time.begin(), already_ot_time.end(), 0);
	}
}

int SQLACTION::get_remain_ot_time(int ot, int already)
{
	int remain_time;
	if (ot - already == ot)
	{
		remain_time = ot;
	}
	else if ((ot - already < ot) && (ot - already) > 0)
	{
		remain_time = ot - already;
	}
	else if (ot - already <= 0)
	{
		remain_time = 0;
	}
	return remain_time;
}

int SQLACTION::get_remain_ot_time(int ot, int already, int flag)
{
	int remain_time;
	if (flag)
	{
		if ((ot - already < ot) && (ot - already > 0))
		{
			remain_time = ot - already;
		}
		else if (ot - already <= 0)
		{
			remain_time = 0;
		}
	}
	else
	{
		remain_time = ot;
	}
	return remain_time;
}

int SQLACTION::determine_change_end_time(int ot, int already, int remain_time, int flag)
{
	
	if (flag && (ot - already < ot) && (ot - already > 0) && remain_time != 0)
	{
		return ipt.bp.next_simulate_timeblock + remain_time;
	}
	return -1;
}

// =-=-=- demand response -=-=-= //
void SQLACTION::get_dr_mode()
{
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'dr_mode'");
	ipt.fg.dr_mode = sql.turnValueToInt();
	
}

void SQLACTION::get_demand_response()
{
	if (ipt.fg.dr_mode != 0)
	{
		sql.operate("SELECT start_timeblock, end_timeblock, min_decrease_power, feedback_price, customer_baseLine FROM `demand_response` WHERE mode = " + to_string(ipt.fg.dr_mode));
		ipt.dr.demand_info = sql.turnArrayToInt();
		ipt.dr.startTime		 	= ipt.dr.demand_info[0];
		ipt.dr.endTime 				= ipt.dr.demand_info[1];
		ipt.dr.minDecrease_power 	= ipt.dr.demand_info[2];
		ipt.dr.feedback_price 		= ipt.dr.demand_info[3];
		ipt.dr.customer_baseLine 	= ipt.dr.demand_info[4];
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to dr_mode = 0" << std::endl;
	}
}

void SQLACTION::get_Pgrid_max_array()
{
	if (ipt.fg.dr_mode != 0)
	{
		for (int i = 0; i < ipt.bp.time_block - ipt.bp.Global_next_simulate_timeblock; i++)
		{
			sql.operate("SELECT SUM(A"+ to_string(i + ipt.bp.Global_next_simulate_timeblock) +") FROM `LHEMS_control_status` WHERE equip_name = 'dr_alpha'");
		}
		float dr_weighting_sumOfAlpha = sql.turnValueToFloat();
		ipt.dr.Pgrid_max_array.push_back(ipt.bp.Pgrid_max / ipt.bp.householdAmount * dr_weighting_sumOfAlpha);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to dr_mode = 0" << std::endl;
	}
}

// =-=-=- common parameter -=-=-= //
void SQLACTION::get_experimental_parameters(ENERGYMANAGESYSTEM ems_name)
{
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'time_block'");
	ipt.bp.time_block = sql.turnValueToInt();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'householdAmount'");
	ipt.bp.householdAmount = sql.turnValueToInt();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'householdDistributed'");
	ipt.bp.householdDistributed = sql.turnValueToInt();

	ipt.bp.distributed_householdAmount = ipt.bp.householdAmount/ipt.bp.householdDistributed;

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Vsys'");
	ipt.bp.Vsys = sql.turnValueToInt();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Cbat'");
	ipt.bp.Cbat = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'SOCmin'");
	ipt.bp.SOC_min = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'SOCmax'");
	ipt.bp.SOC_max = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'SOCthres'");
	ipt.bp.SOC_threshold = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pbatmin'");
	ipt.bp.Pbat_min = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pbatmax'");
	ipt.bp.Pbat_max = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pgridmax'");
	ipt.bp.Pgrid_max = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Psellmax'");
	ipt.bp.Psell_max = sql.turnValueToFloat();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pfcmax'");
	ipt.bp.Pfc_max = sql.turnValueToFloat();

	ipt.bp.divide = ipt.bp.time_block/24;
	
	ipt.bp.delta_T = 1/ipt.bp.divide;
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_weather'");
	ipt.bp.simulate_weather = sql.turnValueToString();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price'");
	ipt.bp.simulate_price = sql.turnValueToString();

	if (ems_name == ENERGYMANAGESYSTEM::CEMS)
	{
		ipt.pl.load_num = get_publicLoad_num();
		ipt.bp.Vsys *= ipt.bp.householdAmount;
		ipt.bp.Pbat_min *= ipt.bp.householdAmount;
		ipt.bp.Pbat_max *= ipt.bp.householdAmount;
		ipt.bp.Pgrid_max *= ipt.bp.householdAmount;
		ipt.bp.Psell_max *= ipt.bp.householdAmount;
		ipt.bp.Pfc_max *= ipt.bp.householdAmount;
		
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_next_simulate_timeblock'");
		ipt.bp.Global_next_simulate_timeblock = sql.turnValueToInt();
	}
	else
	{
		ipt.irl.load_num = get_interrupt_num();
		ipt.uirl.load_num = get_uninterrupt_num();
		ipt.varl.load_num = get_varying_num();
		ipt.bp.app_count = ipt.irl.load_num + ipt.uirl.load_num + ipt.varl.load_num;
	}
}

void SQLACTION::get_allDay_price()
{
	for (int i = 0; i < ipt.bp.time_block; i++)
	{
		sql.operate("SELECT "+ ipt.bp.simulate_price +" FROM price WHERE price_period = "+ to_string(i));
		ipt.bp.price.push_back(sql.turnValueToFloat());
	}	
}

void SQLACTION::getOrUpdate_SolarInfo_ThroughSampleTime()
{
	if (ipt.bp.Global_next_simulate_timeblock == 0)
	{
		for (int i = 0; i < ipt.bp.time_block; i++)
		{
			sql.operate("SELECT "+ ipt.bp.simulate_weather +" FROM `solar_data` WHERE time_block = "+ to_string(i));
			ipt.bp.weather.push_back(sql.turnValueToFloat());

			sql.operate("UPDATE `solar_day` SET value = "+ to_string(ipt.bp.weather[i]) +" WHERE time_block = "+ to_string(i));
		}
	}
	else
	{
		for (int i = 0; i < ipt.bp.time_block; i++)
		{
			sql.operate("SELECT value FROM `solar_data` WHERE time_block = "+ to_string(i));
			ipt.bp.weather.push_back(sql.turnValueToFloat());	
		}
	}
}

void SQLACTION::determine_GHEMS_realTimeOrOneDayMode_andGetSOC()
{
	if (ipt.fg.Global_real_time)
	{
		sql.operate("TRUNCATE TABLE GHEMS_real_status");
		sql.operate("SELECT value FROM `BaseParameter` WHERE `parameter_name` = 'now_SOC'");
		ipt.bp.SOC_ini = sql.turnValueToFloat();
		if (ipt.bp.SOC_ini > 100)
			ipt.bp.SOC_ini = 99.8;
		std::cout << "Real time mode" << std::endl;
	}
	else
	{
		sql.operate("TRUNCATE TABLE GHEMS_control_status");
		sql.operate("TRUNCATE TABLE GHEMS_real_status");
		sql.operate("TRUNCATE TABLE cost");
		
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC'");
		ipt.bp.SOC_ini = sql.turnValueToFloat();
		
		ipt.fg.Global_real_time = 1;
		ipt.bp.Global_next_simulate_timeblock = 0;
		sql.operate("UPDATE BaseParameter SET value = "+ to_string(ipt.fg.Global_real_time) +" WHERE parameter_name = 'Global_real_time'");
		sql.operate("UPDATE BaseParameter SET value = "+ to_string(ipt.bp.Global_next_simulate_timeblock) +" WHERE parameter_name = 'Global_next_simulate_timeblock'");
		std::cout << "First time block" << std::endl;
	}
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_next_simulate_timeblock'");
	ipt.bp.Global_next_simulate_timeblock = sql.turnValueToFloat();
	if (ipt.bp.Global_next_simulate_timeblock + 1 == 97)
	{
		std::cout << "Time Block to the end" << std::endl;
		exit(0);
	}
}

void SQLACTION::get_totalLoad_power()
{
	for (int i = 0; i < ipt.bp.time_block; i++)
	{
		sql.operate("SELECT totalLoad FROM `totalLoad_model` WHERE time_block = "+ to_string(i));
		ipt.bp.load_model.push_back(sql.turnValueToFloat());
		if (ipt.fg.uncontrollable_load)
		{
			sql.operate("SELECT totalLoad FROM LHEMS_uncontrollable_load WHERE time_block = "+ to_string(i));
			ipt.bp.load_model[i] += sql.turnValueToFloat();
		}		
	}
}

void SQLACTION::get_distributedGroup_householdAndSampleTime(int group_num)
{
	sql.operate("SELECT `household_id` FROM `distributed_group` WHERE `group_id` = "+ to_string(group_num));
	
	ipt.bp.distributed_household_id = sql.turnValueToInt();
	ipt.bp.real_household_id = (group_num-1)*ipt.bp.distributed_householdAmount+ipt.bp.distributed_household_id;

	sql.operate("SELECT `next_simulate_timeblock` FROM `distributed_group` WHERE `group_id` = "+ to_string(group_num));
	ipt.bp.next_simulate_timeblock = sql.turnValueToInt();
}

void SQLACTION::determine_LHEMS_realTimeOrOneDayMode_andGetSOC(int group_num)
{
	if (ipt.fg.real_time)
	{
		if (determine_distributedGroup_status("COUNT(group_id) = SUM(household_id)"))
		{
			sql.operate("TRUNCATE TABLE `LHEMS_real_status`");
		}
		if (ipt.fg.Pess)
		{
			sql.operate("SELECT A"+ to_string(ipt.bp.next_simulate_timeblock - 1) +" FROM LHEMS_control_status WHERE equip_name = 'SOC' and household_id = "+ to_string(ipt.bp.real_household_id));
			ipt.bp.SOC_ini = sql.turnValueToFloat();
		}
	}
	else
	{
		if (determine_distributedGroup_status("COUNT(group_id) = SUM(household_id)"))
		{
			ipt.bp.next_simulate_timeblock = 0;
			sql.operate("TRUNCATE TABLE `LHEMS_control_status`");
			sql.operate("TRUNCATE TABLE `LHEMS_real_status`");
			sql.operate("UPDATE `distributed_group` SET `real_time` = '"+ to_string(ipt.fg.real_time) +"' WHERE `group_id` = "+ to_string(group_num));
			sql.operate("UPDATE `distributed_group` SET `next_simulate_timeblock` = '"+ to_string(ipt.bp.next_simulate_timeblock) +"' WHERE `group_id` = "+ to_string(group_num));
		}
		if (ipt.fg.Pess)
		{
			sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC'");
			ipt.bp.SOC_ini = sql.turnValueToFloat();
		}
		if (ipt.bp.distributed_household_id == ipt.bp.distributed_householdAmount)
		{
			ipt.fg.real_time = 1;
			sql.operate("UPDATE `distributed_group` SET `real_time` = '"+ to_string(ipt.fg.real_time) +"' WHERE `group_id` = "+ to_string(group_num));
			if (determine_distributedGroup_status("COUNT(group_id) = SUM(real_time)"))
			{
				sql.operate("UPDATE BaseParameter SET value = "+ to_string(ipt.fg.real_time) +" WHERE parameter_name = 'real_time'");
			}
		}
	}
	if (ipt.bp.next_simulate_timeblock + 1 == 97)
	{
		std::cout << "Time Block to the end" << std::endl;
		exit(0);
	}
}

void SQLACTION::init_totalLoad_tableAndFlag(int group_num)
{
	if (ipt.bp.distributed_household_id == 1)
	{
		sql.operate("UPDATE `distributed_group` SET `total_load_flag` = '0' WHERE `group_id` = "+ to_string(group_num));
		if (ipt.bp.next_simulate_timeblock == 0)
		{
			for (int i = 0; i < ipt.bp.distributed_householdAmount; i++)
			{
				sql.operate("UPDATE `totalLoad_model` SET `household"+ to_string(ipt.bp.real_household_id+i) +"` = '0'");
			}
		}
		sql.operate("UPDATE `totalLoad_model` SET `totalLoad` = '0'");
	}
}

// =-=-=- flag -=-=-= //
void SQLACTION::get_flag(ENERGYMANAGESYSTEM ems_name)
{
	if (ems_name == ENERGYMANAGESYSTEM::CEMS)
	{
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_real_time'");
		ipt.fg.Global_real_time = sql.turnValueToInt();

		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'publicLoad'");
		ipt.fg.publicLoad = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Pgrid'");
		ipt.fg.Pgrid = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'mu_grid'");
		ipt.fg.mu_grid = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Psell'");
		ipt.fg.Psell = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Pess'");
		ipt.fg.Pess = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'SOC_change'");
		ipt.fg.SOCchange = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Pfc'");
		ipt.fg.Pfc = sql.turnValueToInt();
	}
	else
	{
		sql.operate("SELECT value FROM `BaseParameter` WHERE `parameter_name` = 'comfortLevel_flag'");
		ipt.fg.comfortLevel = sql.turnValueToInt();
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'real_time'");
		ipt.fg.real_time = sql.turnValueToInt();

		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'uncontrollable_load_flag'");
		ipt.fg.uncontrollable_load = sql.turnValueToInt();

		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'interrupt'");
		ipt.fg.interrupt = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'uninterrupt'");
		ipt.fg.uninterrupt = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'varying'");
		ipt.fg.varying = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'Pgrid'");
		ipt.fg.Pgrid = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'Pess'");
		ipt.fg.Pess = sql.turnValueToInt();
	}
}

void SQLACTION::create_variable_name(ENERGYMANAGESYSTEM ems_name)
{
	if (ems_name == ENERGYMANAGESYSTEM::CEMS)
	{
		if (ipt.fg.publicLoad)
		{
			for (int i = 0; i < ipt.pl.load_num; i++)
				ipt.variable_name.push_back("publicLoad" + to_string(i + 1));
		}
		if (ipt.fg.Pgrid)
			ipt.variable_name.push_back("Pgrid");
		if (ipt.fg.mu_grid)
			ipt.variable_name.push_back("mu_grid");
		if (ipt.fg.Psell)
			ipt.variable_name.push_back("Psell");
		if (ipt.fg.Pess)
		{
			ipt.variable_name.push_back("Pess");
			ipt.variable_name.push_back("Pcharge");
			ipt.variable_name.push_back("Pdischarge");
			ipt.variable_name.push_back("SOC");
			ipt.variable_name.push_back("Z");
			if (ipt.fg.SOCchange)
			{
				ipt.variable_name.push_back("SOC_change");
				ipt.variable_name.push_back("SOC_increase");
				ipt.variable_name.push_back("SOC_decrease");
				ipt.variable_name.push_back("SOC_Z");
			}
		}
		if (ipt.fg.Pfc)
		{
			ipt.variable_name.push_back("Pfc");
			ipt.variable_name.push_back("Pfct");
			ipt.variable_name.push_back("PfcON");
			ipt.variable_name.push_back("PfcOFF");
			ipt.variable_name.push_back("muFC");
			for (int i = 0; i < ipt.bp.piecewise_num; i++)
				ipt.variable_name.push_back("zPfc" + to_string(i + 1));
			for (int i = 0; i < ipt.bp.piecewise_num; i++)
				ipt.variable_name.push_back("lambda_Pfc" + to_string(i + 1));
		}
	}
	else
	{
		if (ipt.fg.interrupt)
		{
			for (int i = 0; i < ipt.irl.load_num; i++)
				ipt.variable_name.push_back("interrupt" + to_string(i + 1));
		}
		if (ipt.fg.uninterrupt)
		{
			for (int i = 0; i < ipt.uirl.load_num; i++)
				ipt.variable_name.push_back("uninterrupt" + to_string(i + 1));
		}
		if (ipt.fg.varying)
		{
			for (int i = 0; i < ipt.varl.load_num; i++)
				ipt.variable_name.push_back("varying" + to_string(i + 1));
		}
		if (ipt.fg.Pgrid)
			ipt.variable_name.push_back("Pgrid");
		if (ipt.fg.Pess)
		{
			ipt.variable_name.push_back("Pess");
			ipt.variable_name.push_back("Pcharge");
			ipt.variable_name.push_back("Pdischarge");
			ipt.variable_name.push_back("SOC");
			ipt.variable_name.push_back("Z");
		}
		if (ipt.fg.dr_mode != 0)
			ipt.variable_name.push_back("dr_alpha");
		if (ipt.fg.uninterrupt)
		{
			for (int i = 0; i < ipt.uirl.load_num; i++)
				ipt.variable_name.push_back("uninterDelta" + to_string(i + 1));
		}
		if (ipt.fg.varying)
		{
			for (int i = 0; i < ipt.varl.load_num; i++)
				ipt.variable_name.push_back("varyingDelta" + to_string(i + 1));
			for (int i = 0; i < ipt.varl.load_num; i++)
				ipt.variable_name.push_back("varyingPsi" + to_string(i + 1));
		}
	}
	
	ipt.bp.variable_num = ipt.variable_name.size();
}

// =-=-=- public load -=-=-= //
int SQLACTION::get_publicLoad_num()
{
	if (ipt.fg.publicLoad)
	{
		sql.operate("SELECT COUNT(*) FROM `load_list` WHERE group_id = 5");
		return sql.turnValueToInt();
	}
	else
	{
		std::cout << "Public load flag = 0" << std::endl;
		return -1;
	}
}

void SQLACTION::get_publicLoad_info()
{
	if (ipt.fg.publicLoad)
	{
		
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to public load flag = 0" << std::endl;
	}
}

// =-=-=- interrupt load -=-=-= //
int SQLACTION::get_interrupt_num()
{
	if (ipt.fg.interrupt)
	{
		sql.operate("SELECT COUNT(*) FROM load_list WHERE group_id = 1");
		return sql.turnValueToInt();
	}
	else
	{
		std::cout << "Interrupt load flag = 0" << std::endl;
		return -1;
	}
}

void SQLACTION::get_interrupt_info()
{
	if (ipt.fg.interrupt)
	{
		vector<int> start, end, ot, reot;
		for (int i = 0; i < ipt.irl.load_num; i++)
		{
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 1 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			ipt.irl.power.push_back(sql.turnValueToFloat());

			sql.operate("SELECT household"+ to_string(ipt.bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 1 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);

			int count = get_already_operate_time("interrupt", i);
			int reot_time = get_remain_ot_time(result[2], count);

			start.push_back(result[0]);
			end.push_back(result[1] - 1);
			ot.push_back(result[2]);
			reot.push_back(reot_time);
		}
		ipt.irl.time_info.push_back(start);
		ipt.irl.time_info.push_back(end);
		ipt.irl.time_info.push_back(ot);
		ipt.irl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to interrupt load flag = 0" << std::endl;
	}
}

// =-=-=- uninterrupt load -=-=-= //
int SQLACTION::get_uninterrupt_num()
{
	if (ipt.fg.uninterrupt)
	{
		sql.operate("SELECT COUNT(*) FROM load_list WHERE group_id = 2");
		return sql.turnValueToInt();
	}
	else
	{
		std::cout << "Uninterrupt load flag = 0" << std::endl;
		return -1;
	}	
}

void SQLACTION::get_uninterrupt_info()
{
	if (ipt.fg.uninterrupt)
	{
		vector<int> start, end, ot, reot;
		for (int i = 0; i < ipt.uirl.load_num; i++)
		{
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 2 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			ipt.uirl.power.push_back(sql.turnValueToFloat());

			sql.operate("SELECT household"+ to_string(ipt.bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 2 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);
			
			int count = get_already_operate_time("uninterrupt", i);
			bool flag = get_continuityLoad_flag("uninterDelta", i);
			int reot_time = get_remain_ot_time(result[2], count, flag);
			int modify_end_time = determine_change_end_time(result[2], count, reot_time, flag);

			start.push_back(result[0]);
			if (modify_end_time != -1)
			{
				end.push_back(modify_end_time - 1);
			}
			else
			{
				end.push_back(result[1] - 1);
			}
			ot.push_back(result[2]);
			reot.push_back(reot_time);
		}
		ipt.uirl.time_info.push_back(start);
		ipt.uirl.time_info.push_back(end);
		ipt.uirl.time_info.push_back(ot);
		ipt.uirl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to uninterrupt load flag = 0" << std::endl;
	}
}

// =-=-=- varying load -=-=-= //
int SQLACTION::get_varying_num()
{
	if (ipt.fg.varying)
	{
		sql.operate("SELECT COUNT(*) FROM load_list WHERE group_id = 3");
		return sql.turnValueToInt();
	}
	else
	{
		std::cout << "Varying load flag = 0" << std::endl;
		return -1;
	}
}

void SQLACTION::get_varying_info()
{
	if (ipt.fg.varying)
	{
		vector<int> start, end, ot, reot;
		for (int i = 0; i < ipt.varl.load_num; i++)
		{
			sql.operate("SELECT power1, power2, power3 FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			ipt.varl.power.push_back(sql.turnArrayToFloat());
			
			sql.operate("SELECT block1, block2, block3 FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			ipt.varl.block.push_back(sql.turnArrayToInt());

			sql.operate("SELECT household"+ to_string(ipt.bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);
			
			int count = get_already_operate_time("varying", i);
			bool flag = get_continuityLoad_flag("varyingDelta", i);
			int reot_time = get_remain_ot_time(result[2], count, flag);
			int modify_end_time = determine_change_end_time(result[2], count, reot_time, flag);

			start.push_back(result[0]);
			if (modify_end_time != -1)
			{
				end.push_back(modify_end_time - 1);
			}
			else
			{
				end.push_back(result[1] - 1);
			}
			ot.push_back(result[2]);
			reot.push_back(reot_time);
		}
		ipt.varl.time_info.push_back(start);
		ipt.varl.time_info.push_back(end);
		ipt.varl.time_info.push_back(ot);
		ipt.varl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to varying load flag = 0" << std::endl;	
	}
}

void SQLACTION::update_new_load_model()
{
	std::cout << ipt.bp.Vsys << std::endl;
}