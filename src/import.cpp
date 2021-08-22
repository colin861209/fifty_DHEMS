#include "SQL.hpp"
#include "import.hpp"

import::import(std::string iP, std::string name, std::string passwd, std::string database)
{	
	sql.connect(iP, name, passwd, database);
	// import::ENERGYMANAGESYSTEM::cems = "CEMS";
	// import::ENERGYMANAGESYSTEM::hems = "HEMS";
}

// int import::find_variableName_position(vector<string> variableNameArray, string target)
// {
// 	auto it = find(variableNameArray.begin(), variableNameArray.end(), target);

// 	// If element was found
// 	if (it != variableNameArray.end())
// 		return (it - variableNameArray.begin());
// 	else
// 		return -1;
// }

// void import::messagePrint(int lineNum, const char *message, char contentSize, float content, char tabInHeader) {
	
// 	// tap 'Y' or 'N' means yes or no
// 	if (tabInHeader == 'Y')
// 		printf("\t");
// 	// tap 'I' or 'F' means int or float, otherwise no contents be showed.
// 	switch (contentSize)
// 	{
// 	case 'I':
// 		printf("LINE %d: %s%d\n", lineNum, message, (int)content);
// 		break;
// 	case 'F':
// 		printf("LINE %d: %s%f\n", lineNum, message, content);
// 		break;
// 	default:
// 		printf("LINE %d: %s\n", lineNum, message);
// 	}
// }

bool import::determine_distributedGroup_status(string condition)
{
	sql.operate("SELECT "+ condition +" FROM `distributed_group`");
	return sql.turnValueToInt();
}

vector<int> import::split_array(string timearray)
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

bool import::get_continuityLoad_flag(string load_type, int offset_num)
{
	sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE equip_name LIKE '"+ load_type +"%' AND household_id = "+ to_string(bp.real_household_id) +" LIMIT 1 OFFSET "+ to_string(offset_num));
	vector<int> flag_status = sql.turnArrayToInt();
	flag_status.erase(flag_status.begin()+bp.next_simulate_timeblock, flag_status.end());
	return accumulate(flag_status.begin(), flag_status.end(), 0);
}

int import::get_already_operate_time(string load_type, int offset_num)
{
	if (bp.next_simulate_timeblock == 0)
	{
		return 0;
	}
	else
	{
		sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE household_id = "+ to_string(bp.real_household_id) +" AND equip_name LIKE '"+ load_type +"%' LIMIT 1 OFFSET "+ to_string(offset_num));
		vector<int> already_ot_time = sql.turnArrayToInt();
		already_ot_time.erase(already_ot_time.begin()+bp.next_simulate_timeblock, already_ot_time.end());
		return accumulate(already_ot_time.begin(), already_ot_time.end(), 0);
	}
}

int import::get_remain_ot_time(int ot, int already)
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

int import::get_remain_ot_time(int ot, int already, int flag)
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

int import::determine_change_end_time(int ot, int already, int remain_time, int flag)
{
	
	if (flag && (ot - already < ot) && (ot - already > 0) && remain_time != 0)
	{
		return bp.next_simulate_timeblock + remain_time;
	}
	return -1;
}

// =-=-=- demand response -=-=-= //
void import::get_dr_mode()
{
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'dr_mode'");
	fg.dr_mode = sql.turnValueToInt();
}

void import::get_demand_response()
{
	if (fg.dr_mode != 0)
	{
		sql.operate("SELECT start_timeblock, end_timeblock, min_decrease_power, feedback_price, customer_baseLine FROM `demand_response` WHERE mode = " + to_string(fg.dr_mode));
		dr.demand_info = sql.turnArrayToInt();
		dr.startTime		 	= dr.demand_info[0];
		dr.endTime 				= dr.demand_info[1];
		dr.minDecrease_power 	= dr.demand_info[2];
		dr.feedback_price 		= dr.demand_info[3];
		dr.customer_baseLine 	= dr.demand_info[4];
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to dr_mode = 0" << std::endl;
	}
}

void import::get_Pgrid_max_array()
{
	if (fg.dr_mode != 0)
	{
		for (int i = 0; i < bp.time_block - bp.Global_next_simulate_timeblock; i++)
		{
			sql.operate("SELECT SUM(A"+ to_string(i + bp.Global_next_simulate_timeblock) +") FROM `LHEMS_control_status` WHERE equip_name = 'dr_alpha'");
		}
		float dr_weighting_sumOfAlpha = sql.turnValueToFloat();
		dr.Pgrid_max_array.push_back(bp.Pgrid_max / bp.householdAmount * dr_weighting_sumOfAlpha);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to dr_mode = 0" << std::endl;
	}
}

// =-=-=- common parameter -=-=-= //
void import::get_experimental_parameters(string ems_name)
{
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'time_block'");
	bp.time_block = sql.turnValueToInt();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'householdAmount'");
	bp.householdAmount = sql.turnValueToInt();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'householdDistributed'");
	bp.householdDistributed = sql.turnValueToInt();

	bp.distributed_householdAmount = bp.householdAmount/bp.householdDistributed;

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Vsys'");
	bp.Vsys = sql.turnValueToInt();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Cbat'");
	bp.Cbat = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'SOCmin'");
	bp.SOC_min = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'SOCmax'");
	bp.SOC_max = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'SOCthres'");
	bp.SOC_threshold = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pbatmin'");
	bp.Pbat_min = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pbatmax'");
	bp.Pbat_max = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pgridmax'");
	bp.Pgrid_max = sql.turnValueToFloat();
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Psellmax'");
	bp.Psell_max = sql.turnValueToFloat();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Pfcmax'");
	bp.Pfc_max = sql.turnValueToFloat();

	bp.divide = bp.time_block/24;
	
	bp.delta_T = 1/bp.divide;
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_weather'");
	bp.simulate_weather = sql.turnValueToString();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price'");
	bp.simulate_price = sql.turnValueToString();

	if (ems_name == "CEMS")
	{
		pl.load_num = get_publicLoad_num();
		bp.Vsys *= bp.householdAmount;
		bp.Pbat_min *= bp.householdAmount;
		bp.Pbat_max *= bp.householdAmount;
		bp.Pgrid_max *= bp.householdAmount;
		bp.Psell_max *= bp.householdAmount;
		bp.Pfc_max *= bp.householdAmount;
		
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_next_simulate_timeblock'");
		bp.Global_next_simulate_timeblock = sql.turnValueToInt();
	}
	else
	{
		irl.load_num = get_interrupt_num();
		uirl.load_num = get_uninterrupt_num();
		varl.load_num = get_varying_num();
		bp.app_count = irl.load_num + uirl.load_num + varl.load_num;
	}
}

void import::get_allDay_price()
{
	for (int i = 0; i < bp.time_block; i++)
	{
		sql.operate("SELECT "+ bp.simulate_price +" FROM price WHERE price_period = "+ to_string(i));
		bp.price.push_back(sql.turnValueToFloat());
	}	
}

void import::getOrUpdate_SolarInfo_ThroughSampleTime()
{
	if (bp.Global_next_simulate_timeblock == 0)
	{
		for (int i = 0; i < bp.time_block; i++)
		{
			sql.operate("SELECT "+ bp.simulate_weather +" FROM `solar_data` WHERE time_block = "+ to_string(i));
			bp.weather.push_back(sql.turnValueToFloat());

			sql.operate("UPDATE `solar_day` SET value = "+ to_string(bp.weather[i]) +" WHERE time_block = "+ to_string(i));
		}
	}
	else
	{
		for (int i = 0; i < bp.time_block; i++)
		{
			sql.operate("SELECT value FROM `solar_data` WHERE time_block = "+ to_string(i));
			bp.weather.push_back(sql.turnValueToFloat());	
		}
	}
}

void import::determine_GHEMS_realTimeOrOneDayMode_andGetSOC()
{
	if (fg.Global_real_time)
	{
		sql.operate("TRUNCATE TABLE GHEMS_real_status");
		sql.operate("SELECT value FROM `BaseParameter` WHERE `parameter_name` = 'now_SOC'");
		bp.SOC_ini = sql.turnValueToFloat();
		if (bp.SOC_ini > 100)
			bp.SOC_ini = 99.8;
		std::cout << "Real time mode" << std::endl;
	}
	else
	{
		sql.operate("TRUNCATE TABLE GHEMS_control_status");
		sql.operate("TRUNCATE TABLE GHEMS_real_status");
		sql.operate("TRUNCATE TABLE cost");
		
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC'");
		bp.SOC_ini = sql.turnValueToFloat();
		
		fg.Global_real_time = 1;
		bp.Global_next_simulate_timeblock = 0;
		sql.operate("UPDATE BaseParameter SET value = "+ to_string(fg.Global_real_time) +" WHERE parameter_name = 'Global_real_time'");
		sql.operate("UPDATE BaseParameter SET value = "+ to_string(bp.Global_next_simulate_timeblock) +" WHERE parameter_name = 'Global_next_simulate_timeblock'");
		std::cout << "First time block" << std::endl;
	}
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_next_simulate_timeblock'");
	bp.Global_next_simulate_timeblock = sql.turnValueToFloat();
	if (bp.Global_next_simulate_timeblock + 1 == 97)
	{
		std::cout << "Time Block to the end" << std::endl;
		exit(0);
	}
}

void import::get_totalLoad_power()
{
	for (int i = 0; i < bp.time_block; i++)
	{
		sql.operate("SELECT totalLoad FROM `totalLoad_model` WHERE time_block = "+ to_string(i));
		bp.load_model.push_back(sql.turnValueToFloat());
		if (fg.uncontrollable_load)
		{
			sql.operate("SELECT totalLoad FROM LHEMS_uncontrollable_load WHERE time_block = "+ to_string(i));
			bp.load_model[i] += sql.turnValueToFloat();
		}		
	}
}

void import::get_distributedGroup_householdAndSampleTime(int group_num)
{
	sql.operate("SELECT `household_id` FROM `distributed_group` WHERE `group_id` = "+ to_string(group_num));
	
	bp.distributed_household_id = sql.turnValueToInt();
	bp.real_household_id = (group_num-1)*bp.distributed_householdAmount+bp.distributed_household_id;

	sql.operate("SELECT `next_simulate_timeblock` FROM `distributed_group` WHERE `group_id` = "+ to_string(group_num));
	bp.next_simulate_timeblock = sql.turnValueToInt();
}

void import::determine_LHEMS_realTimeOrOneDayMode_andGetSOC(int group_num)
{
	if (fg.real_time)
	{
		if (determine_distributedGroup_status("COUNT(group_id) = SUM(household_id)"))
		{
			sql.operate("TRUNCATE TABLE `LHEMS_real_status`");
		}
		if (fg.Pess)
		{
			sql.operate("SELECT A"+ to_string(bp.next_simulate_timeblock - 1) +" FROM LHEMS_control_status WHERE equip_name = 'SOC' and household_id = "+ to_string(bp.real_household_id));
			bp.SOC_ini = sql.turnValueToFloat();
		}
	}
	else
	{
		if (determine_distributedGroup_status("COUNT(group_id) = SUM(household_id)"))
		{
			bp.next_simulate_timeblock = 0;
			sql.operate("TRUNCATE TABLE `LHEMS_control_status`");
			sql.operate("TRUNCATE TABLE `LHEMS_real_status`");
			sql.operate("UPDATE `distributed_group` SET `real_time` = '"+ to_string(fg.real_time) +"' WHERE `group_id` = "+ to_string(group_num));
			sql.operate("UPDATE `distributed_group` SET `next_simulate_timeblock` = '"+ to_string(bp.next_simulate_timeblock) +"' WHERE `group_id` = "+ to_string(group_num));
		}
		if (fg.Pess)
		{
			sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'ini_SOC'");
			bp.SOC_ini = sql.turnValueToFloat();
		}
		if (bp.distributed_household_id == bp.distributed_householdAmount)
		{
			fg.real_time = 1;
			sql.operate("UPDATE `distributed_group` SET `real_time` = '"+ to_string(fg.real_time) +"' WHERE `group_id` = "+ to_string(group_num));
			if (determine_distributedGroup_status("COUNT(group_id) = SUM(real_time)"))
			{
				sql.operate("UPDATE BaseParameter SET value = "+ to_string(fg.real_time) +" WHERE parameter_name = 'real_time'");
			}
		}
	}
	if (bp.next_simulate_timeblock + 1 == 97)
	{
		std::cout << "Time Block to the end" << std::endl;
		exit(0);
	}
}

void import::init_totalLoad_tableAndFlag(int group_num)
{
	if (bp.distributed_household_id == 1)
	{
		sql.operate("UPDATE `distributed_group` SET `total_load_flag` = '0' WHERE `group_id` = "+ to_string(group_num));
		if (bp.next_simulate_timeblock == 0)
		{
			for (int i = 0; i < bp.distributed_householdAmount; i++)
			{
				sql.operate("UPDATE `totalLoad_model` SET `household"+ to_string(bp.real_household_id+i) +"` = '0'");
			}
		}
		sql.operate("UPDATE `totalLoad_model` SET `totalLoad` = '0'");
	}
}

// =-=-=- flag -=-=-= //
void import::get_flag(string ems_name)
{
	if (ems_name == "CEMS")
	{
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_real_time'");
		fg.Global_real_time = sql.turnValueToInt();

		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'publicLoad'");
		fg.publicLoad = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Pgrid'");
		fg.Pgrid = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'mu_grid'");
		fg.mu_grid = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Psell'");
		fg.Psell = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Pess'");
		fg.Pess = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'SOC_change'");
		fg.SOCchange = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `GHEMS_flag` WHERE `variable_name` = 'Pfc'");
		fg.Pfc = sql.turnValueToInt();
	}
	else
	{
		sql.operate("SELECT value FROM `BaseParameter` WHERE `parameter_name` = 'comfortLevel_flag'");
		fg.comfortLevel = sql.turnValueToInt();
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'real_time'");
		fg.real_time = sql.turnValueToInt();

		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'uncontrollable_load_flag'");
		fg.uncontrollable_load = sql.turnValueToInt();

		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'interrupt'");
		fg.interrupt = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'uninterrupt'");
		fg.uninterrupt = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'varying'");
		fg.varying = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'Pgrid'");
		fg.Pgrid = sql.turnValueToInt();
		sql.operate("SELECT flag FROM `LHEMS_flag` WHERE `variable_name` = 'Pess'");
		fg.Pess = sql.turnValueToInt();
	}
}

void import::create_variable_name(string ems_name)
{
	if (ems_name == "CEMS")
	{
		if (fg.publicLoad)
		{
			for (int i = 0; i < pl.load_num; i++)
				variable_name.push_back("publicLoad" + to_string(i + 1));
		}
		if (fg.Pgrid)
			variable_name.push_back("Pgrid");
		if (fg.mu_grid)
			variable_name.push_back("mu_grid");
		if (fg.Psell)
			variable_name.push_back("Psell");
		if (fg.Pess)
		{
			variable_name.push_back("Pess");
			variable_name.push_back("Pcharge");
			variable_name.push_back("Pdischarge");
			variable_name.push_back("SOC");
			variable_name.push_back("Z");
			if (fg.SOCchange)
			{
				variable_name.push_back("SOC_change");
				variable_name.push_back("SOC_increase");
				variable_name.push_back("SOC_decrease");
				variable_name.push_back("SOC_Z");
			}
		}
		if (fg.Pfc)
		{
			variable_name.push_back("Pfc");
			variable_name.push_back("Pfct");
			variable_name.push_back("PfcON");
			variable_name.push_back("PfcOFF");
			variable_name.push_back("muFC");
			for (int i = 0; i < bp.piecewise_num; i++)
				variable_name.push_back("zPfc" + to_string(i + 1));
			for (int i = 0; i < bp.piecewise_num; i++)
				variable_name.push_back("lambda_Pfc" + to_string(i + 1));
		}
	}
	else
	{
		if (fg.interrupt)
		{
			for (int i = 0; i < irl.load_num; i++)
				variable_name.push_back("interrupt" + to_string(i + 1));
		}
		if (fg.uninterrupt)
		{
			for (int i = 0; i < uirl.load_num; i++)
				variable_name.push_back("uninterrupt" + to_string(i + 1));
		}
		if (fg.varying)
		{
			for (int i = 0; i < varl.load_num; i++)
				variable_name.push_back("varying" + to_string(i + 1));
		}
		if (fg.Pgrid)
			variable_name.push_back("Pgrid");
		if (fg.Pess)
		{
			variable_name.push_back("Pess");
			variable_name.push_back("Pcharge");
			variable_name.push_back("Pdischarge");
			variable_name.push_back("SOC");
			variable_name.push_back("Z");
		}
		if (fg.dr_mode != 0)
			variable_name.push_back("dr_alpha");
		if (fg.uninterrupt)
		{
			for (int i = 0; i < uirl.load_num; i++)
				variable_name.push_back("uninterDelta" + to_string(i + 1));
		}
		if (fg.varying)
		{
			for (int i = 0; i < varl.load_num; i++)
				variable_name.push_back("varyingDelta" + to_string(i + 1));
			for (int i = 0; i < varl.load_num; i++)
				variable_name.push_back("varyingPsi" + to_string(i + 1));
		}
	}
	
	bp.variable_num = variable_name.size();
}

// =-=-=- public load -=-=-= //
int import::get_publicLoad_num()
{
	if (fg.publicLoad)
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

void import::get_publicLoad_info()
{
	if (fg.publicLoad)
	{
		
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to public load flag = 0" << std::endl;
	}
}

// =-=-=- interrupt load -=-=-= //
int import::get_interrupt_num()
{
	if (fg.interrupt)
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

void import::get_interrupt_info()
{
	if (fg.interrupt)
	{
		vector<int> start, end, ot, reot;
		for (int i = 0; i < irl.load_num; i++)
		{
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 1 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			irl.power.push_back(sql.turnValueToFloat());

			sql.operate("SELECT household"+ to_string(bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 1 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);

			int count = get_already_operate_time("interrupt", i);
			int reot_time = get_remain_ot_time(result[2], count);

			start.push_back(result[0]);
			end.push_back(result[1] - 1);
			ot.push_back(result[2]);
			reot.push_back(reot_time);
		}
		irl.time_info.push_back(start);
		irl.time_info.push_back(end);
		irl.time_info.push_back(ot);
		irl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to interrupt load flag = 0" << std::endl;
	}
}

// =-=-=- uninterrupt load -=-=-= //
int import::get_uninterrupt_num()
{
	if (fg.uninterrupt)
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

void import::get_uninterrupt_info()
{
	if (fg.uninterrupt)
	{
		vector<int> start, end, ot, reot;
		for (int i = 0; i < uirl.load_num; i++)
		{
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 2 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			uirl.power.push_back(sql.turnValueToFloat());

			sql.operate("SELECT household"+ to_string(bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 2 LIMIT "+ to_string(i) +", "+ to_string(i+1));
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
		uirl.time_info.push_back(start);
		uirl.time_info.push_back(end);
		uirl.time_info.push_back(ot);
		uirl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to uninterrupt load flag = 0" << std::endl;
	}
}

// =-=-=- varying load -=-=-= //
int import::get_varying_num()
{
	if (fg.varying)
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

void import::get_varying_info()
{
	if (fg.varying)
	{
		vector<int> start, end, ot, reot;
		for (int i = 0; i < varl.load_num; i++)
		{
			sql.operate("SELECT power1, power2, power3 FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			varl.power.push_back(sql.turnArrayToFloat());
			
			sql.operate("SELECT block1, block2, block3 FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			varl.block.push_back(sql.turnArrayToInt());

			sql.operate("SELECT household"+ to_string(bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
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
		varl.time_info.push_back(start);
		varl.time_info.push_back(end);
		varl.time_info.push_back(ot);
		varl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to varying load flag = 0" << std::endl;	
	}
}