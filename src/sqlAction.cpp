#include "sqlAction.hpp"

SQLACTION::SQLACTION(std::string iP, std::string name, std::string passwd, std::string database)
{	
	sql.connect(iP, name, passwd, database);
}

SQLACTION::~SQLACTION()
{
	sql.disconnect();
}

// =-=-=- private function -=-=-= //
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

int SQLACTION::get_already_operate_time(string load_type, int offset_num, ENERGYMANAGESYSTEM ems_type)
{
	switch (ems_type)
	{
	case HEMS:
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
	
	default:
		if (ipt.bp.Global_next_simulate_timeblock == 0)
		{
			return 0;
		}
		else
		{
			sql.operate("SELECT "+ sql.column +" FROM GHEMS_control_status WHERE equip_name LIKE '"+ load_type +"%' LIMIT 1 OFFSET "+ to_string(offset_num));
			vector<int> already_ot_time = sql.turnArrayToInt();
			already_ot_time.erase(already_ot_time.begin()+ipt.bp.Global_next_simulate_timeblock, already_ot_time.end());
			return accumulate(already_ot_time.begin(), already_ot_time.end(), 0);
		}
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

// hems
bool SQLACTION::determine_distributedGroup_status(string condition)
{
	sql.operate("SELECT "+ condition +" FROM `distributed_group`");
	return sql.turnValueToInt();
}

bool SQLACTION::get_continuityLoad_flag(string load_type, int offset_num)
{
	sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE equip_name LIKE '"+ load_type +"%' AND household_id = "+ to_string(ipt.bp.real_household_id) +" LIMIT 1 OFFSET "+ to_string(offset_num));
	vector<int> flag_status = sql.turnArrayToInt();
	flag_status.erase(flag_status.begin()+ipt.bp.next_simulate_timeblock, flag_status.end());
	return accumulate(flag_status.begin(), flag_status.end(), 0);
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

vector<float> SQLACTION::convert_real_power_array(vector<float> power, vector<int> block)
{
	vector<float> real_power;
	for (int j = 0; j < block[0]; j++)
		real_power.push_back(power[0]);

	for (int j = 0; j < block[1]; j++)
		real_power.push_back(power[1]);
	
	for (int j = 0; j < block[2]; j++)
		real_power.push_back(power[2]);
	
	return real_power;
}

vector<int> SQLACTION::convert_real_block_array(int start, int end)
{
	vector<int> real_block;
	real_block.assign(ipt.bp.time_block-ipt.bp.next_simulate_timeblock, 0);
	if ((end - ipt.bp.next_simulate_timeblock) >= 0)
	{
		if ((start - ipt.bp.next_simulate_timeblock) >= 0)
		{
			for (int i = (start - ipt.bp.next_simulate_timeblock); i <= (end - ipt.bp.next_simulate_timeblock); i++)
			{
				real_block[i] = 1;
			}
		}
		else if ((start - ipt.bp.next_simulate_timeblock) < 0)
		{
			for (int i = 0; i <= (end - ipt.bp.next_simulate_timeblock); i++)
			{
				real_block[i] = 1;
			}
		}
	}
	return real_block;
}

float SQLACTION::find_varyingLoad_max_power(vector<float> power)
{
	vector<float>::iterator result;
	result = max_element(power.begin(), power.end());
	float index = distance(power.begin(), result);
	return power[index];
}

void SQLACTION::insert_table_cost(string cost_name, vector<float> cost)
{
	sql.operate("INSERT INTO cost (cost_name, "+ sql.column +") VALUES('"+ cost_name +"','"
	+ to_string(cost[0]) +"','"+ to_string(cost[1]) +"','"+ to_string(cost[2]) +"','"+ to_string(cost[3]) +"','"+ to_string(cost[4]) +"','"+ to_string(cost[5]) +"','"+ to_string(cost[6]) +"','"+ to_string(cost[7]) +"','"+ to_string(cost[8]) +"','"+ to_string(cost[9]) +"','"+ to_string(cost[10]) +"','"+ to_string(cost[11]) +"','"+ to_string(cost[12]) +"','"+ to_string(cost[13]) +"','"+ to_string(cost[14]) +"','"+ to_string(cost[15]) +"','"+ to_string(cost[16]) +"','"+ to_string(cost[17]) +"','"+ to_string(cost[18]) +"','"+ to_string(cost[19]) +"','"+ to_string(cost[20]) +"','"+ to_string(cost[21]) +"','"+ to_string(cost[22]) +"','"+ to_string(cost[23]) +"','"+ to_string(cost[24]) +"','"+ to_string(cost[25]) +"','"+ to_string(cost[26]) +"','"+ to_string(cost[27]) +"','"+ to_string(cost[28]) +"','"+ to_string(cost[29]) +"','"+ to_string(cost[30]) +"','"+ to_string(cost[31]) +"','"+ to_string(cost[32]) +"','"+ to_string(cost[33]) +"','"+ to_string(cost[34]) +"','"+ to_string(cost[35]) +"','"+ to_string(cost[36]) +"','"+ to_string(cost[37]) +"','"+ to_string(cost[38]) +"','"+ to_string(cost[39]) +"','"+ to_string(cost[40]) +"','"+ to_string(cost[41]) +"','"+ to_string(cost[42]) +"','"+ to_string(cost[43]) +"','"+ to_string(cost[44]) +"','"+ to_string(cost[45]) +"','"+ to_string(cost[46]) +"','"+ to_string(cost[47]) +"','"+ to_string(cost[48]) +"','"+ to_string(cost[49]) +"','"+ to_string(cost[50]) +"','"+ to_string(cost[51]) +"','"+ to_string(cost[52]) +"','"+ to_string(cost[53]) +"','"+ to_string(cost[54]) +"','"+ to_string(cost[55]) +"','"+ to_string(cost[56]) +"','"+ to_string(cost[57]) +"','"+ to_string(cost[58]) +"','"+ to_string(cost[59]) +"','"+ to_string(cost[60]) +"','"+ to_string(cost[61]) +"','"+ to_string(cost[62]) +"','"+ to_string(cost[63]) +"','"+ to_string(cost[64]) +"','"+ to_string(cost[65]) +"','"+ to_string(cost[66]) +"','"+ to_string(cost[67]) +"','"+ to_string(cost[68]) +"','"+ to_string(cost[69]) +"','"+ to_string(cost[70]) +"','"+ to_string(cost[71]) +"','"+ to_string(cost[72]) +"','"+ to_string(cost[73]) +"','"+ to_string(cost[74]) +"','"+ to_string(cost[75]) +"','"+ to_string(cost[76]) +"','"+ to_string(cost[77]) +"','"+ to_string(cost[78]) +"','"+ to_string(cost[79]) +"','"+ to_string(cost[80]) +"','"+ to_string(cost[81]) +"','"+ to_string(cost[82]) +"','"+ to_string(cost[83]) +"','"+ to_string(cost[84]) +"','"+ to_string(cost[85]) +"','"+ to_string(cost[86]) +"','"+ to_string(cost[87]) +"','"+ to_string(cost[88]) +"','"+ to_string(cost[89]) +"','"+ to_string(cost[90]) +"','"+ to_string(cost[91]) +"','"+ to_string(cost[92]) +"','"+ to_string(cost[93]) +"','"+ to_string(cost[94]) +"','"+ to_string(cost[95]) +"')");
}

void SQLACTION::update_table_cost(string cost_name, vector<float> cost)
{
	sql.operate("UPDATE cost set A0 = '"+ to_string(cost[0]) +"', A1 = '"+ to_string(cost[1]) +"', A2 = '"+ to_string(cost[2]) +"', A3 = '"+ to_string(cost[3]) +"', A4 = '"+ to_string(cost[4]) +"', A5 = '"+ to_string(cost[5]) +"', A6 = '"+ to_string(cost[6]) +"', A7 = '"+ to_string(cost[7]) +"', A8 = '"+ to_string(cost[8]) +"', A9 = '"+ to_string(cost[9]) +"', A10 = '"+ to_string(cost[10]) +"', A11 = '"+ to_string(cost[11]) +"', A12 = '"+ to_string(cost[12]) +"', A13 = '"+ to_string(cost[13]) +"', A14 = '"+ to_string(cost[14]) +"', A15 = '"+ to_string(cost[15]) +"', A16 = '"+ to_string(cost[16]) +"', A17 = '"+ to_string(cost[17]) +"', A18 = '"+ to_string(cost[18]) +"', A19 = '"+ to_string(cost[19]) +"', A20 = '"+ to_string(cost[20]) +"', A21 = '"+ to_string(cost[21]) +"', A22 = '"+ to_string(cost[22]) +"', A23 = '"+ to_string(cost[23]) +"', A24 = '"+ to_string(cost[24]) +"', A25 = '"+ to_string(cost[25]) +"', A26 = '"+ to_string(cost[26]) +"', A27 = '"+ to_string(cost[27]) +"', A28 = '"+ to_string(cost[28]) +"', A29 = '"+ to_string(cost[29]) +"', A30 = '"+ to_string(cost[30]) +"', A31 = '"+ to_string(cost[31]) +"', A32 = '"+ to_string(cost[32]) +"', A33 = '"+ to_string(cost[33]) +"', A34 = '"+ to_string(cost[34]) +"', A35 = '"+ to_string(cost[35]) +"', A36 = '"+ to_string(cost[36]) +"', A37 = '"+ to_string(cost[37]) +"', A38 = '"+ to_string(cost[38]) +"', A39 = '"+ to_string(cost[39]) +"', A40 = '"+ to_string(cost[40]) +"', A41 = '"+ to_string(cost[41]) +"', A42 = '"+ to_string(cost[42]) +"', A43 = '"+ to_string(cost[43]) +"', A44 = '"+ to_string(cost[44]) +"', A45 = '"+ to_string(cost[45]) +"', A46 = '"+ to_string(cost[46]) +"', A47 = '"+ to_string(cost[47]) +"', A48 = '"+ to_string(cost[48]) +"', A49 = '"+ to_string(cost[49]) +"', A50 = '"+ to_string(cost[50]) +"', A51 = '"+ to_string(cost[51]) +"', A52 = '"+ to_string(cost[52]) +"', A53 = '"+ to_string(cost[53]) +"', A54 = '"+ to_string(cost[54]) +"', A55 = '"+ to_string(cost[55]) +"', A56 = '"+ to_string(cost[56]) +"', A57 = '"+ to_string(cost[57]) +"', A58 = '"+ to_string(cost[58]) +"', A59 = '"+ to_string(cost[59]) +"', A60 = '"+ to_string(cost[60]) +"', A61 = '"+ to_string(cost[61]) +"', A62 = '"+ to_string(cost[62]) +"', A63 = '"+ to_string(cost[63]) +"', A64 = '"+ to_string(cost[64]) +"', A65 = '"+ to_string(cost[65]) +"', A66 = '"+ to_string(cost[66]) +"', A67 = '"+ to_string(cost[67]) +"', A68 = '"+ to_string(cost[68]) +"', A69 = '"+ to_string(cost[69]) +"', A70 = '"+ to_string(cost[70]) +"', A71 = '"+ to_string(cost[71]) +"', A72 = '"+ to_string(cost[72]) +"', A73 = '"+ to_string(cost[73]) +"', A74 = '"+ to_string(cost[74]) +"', A75 = '"+ to_string(cost[75]) +"', A76 = '"+ to_string(cost[76]) +"', A77 = '"+ to_string(cost[77]) +"', A78 = '"+ to_string(cost[78]) +"', A79 = '"+ to_string(cost[79]) +"', A80 = '"+ to_string(cost[80]) +"', A81 = '"+ to_string(cost[81]) +"', A82 = '"+ to_string(cost[82]) +"', A83 = '"+ to_string(cost[83]) +"', A84 = '"+ to_string(cost[84]) +"', A85 = '"+ to_string(cost[85]) +"', A86 = '"+ to_string(cost[86]) +"', A87 = '"+ to_string(cost[87]) +"', A88 = '"+ to_string(cost[88]) +"', A89 = '"+ to_string(cost[89]) +"', A90 = '"+ to_string(cost[90]) +"', A91 = '"+ to_string(cost[91]) +"', A92 = '"+ to_string(cost[92]) +"', A93 = '"+ to_string(cost[93]) +"', A94 = '"+ to_string(cost[94]) +"', A95 = '"+ to_string(cost[95]) +"', `datetime` = CURRENT_TIMESTAMP WHERE cost_name = '"+ cost_name +"'");
}

int SQLACTION::calculate_publicLoad_decrease_operate_time(int public_start, int public_end)
{
	int start, end;
	if (ipt.fg.dr_mode != 0)
	{
		if (public_end > ipt.dr.startTime)
		{
			if (public_start <= ipt.dr.startTime)
				start = ipt.dr.startTime;
			else
				start = public_start;
			
			if (public_end >= ipt.dr.endTime)
				end = ipt.dr.endTime;
			else
				end = public_end;
			
			return end - start;
		}
	}
	else
	{
		return 0;
	}
}

// import function
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

void SQLACTION::get_experimental_parameters(ENERGYMANAGESYSTEM ems_type)
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

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'hydrogen_price'");
	ipt.bp.hydro_price = sql.turnValueToFloat();

	ipt.bp.divide = ipt.bp.time_block/24;
	
	ipt.bp.delta_T = 1/ipt.bp.divide;
	
	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_weather'");
	ipt.bp.simulate_weather = sql.turnValueToString();

	sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'simulate_price'");
	ipt.bp.simulate_price = sql.turnValueToString();

	switch (ems_type)
	{
	case HEMS:
		ipt.irl.load_num = get_interrupt_num();
		ipt.uirl.load_num = get_uninterrupt_num();
		ipt.varl.load_num = get_varying_num();
		ipt.bp.app_count = ipt.irl.load_num + ipt.uirl.load_num + ipt.varl.load_num;
		break;
	
	default:
		ipt.pl.load_num = get_publicLoad_num();
		ipt.bp.Vsys *= ipt.bp.householdAmount;
		ipt.bp.Pbat_min *= ipt.bp.householdAmount;
		ipt.bp.Pbat_max *= ipt.bp.householdAmount;
		ipt.bp.Pgrid_max *= ipt.bp.householdAmount;
		ipt.bp.Psell_max *= ipt.bp.householdAmount;
		ipt.bp.Pfc_max *= ipt.bp.householdAmount;
		
		sql.operate("SELECT value FROM BaseParameter WHERE parameter_name = 'Global_next_simulate_timeblock'");
		ipt.bp.Global_next_simulate_timeblock = sql.turnValueToInt();
	
		break;
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
			sql.operate("SELECT value FROM `solar_day` WHERE time_block = "+ to_string(i));
			ipt.bp.weather.push_back(sql.turnValueToFloat());	
		}
	}
}

// cems
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

// hems
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

void SQLACTION::get_flag(ENERGYMANAGESYSTEM ems_type)
{
	switch (ems_type)
	{
	case HEMS:
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
		break;
	
	default:
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
		break;
	}
}

void SQLACTION::create_variable_name(ENERGYMANAGESYSTEM ems_type)
{
	switch (ems_type)
	{
	case HEMS:
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
		break;
	
	default:
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
		break;
	}
	
	ipt.bp.variable_num = ipt.variable_name.size();
}

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
		vector<int> start, end, ot, reot;
		for (int i = 0; i < ipt.pl.load_num; i++)
		{
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 5 LIMIT 1 OFFSET "+ to_string(i));
			ipt.pl.power.push_back(sql.turnValueToFloat());
			
			sql.operate("SELECT public_loads FROM load_list WHERE group_id = 5 LIMIT 1 OFFSET "+ to_string(i));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);
			
			int decrease_ot = calculate_publicLoad_decrease_operate_time(result[0], result[1]);
			int count = get_already_operate_time("publicLoad", i, ENERGYMANAGESYSTEM::CEMS);
			int reot_time = get_remain_ot_time(result[2]-decrease_ot, count);
			start.push_back(result[0]);
			end.push_back(result[1]-1);
			ot.push_back(result[2]-decrease_ot);
			reot.push_back(reot_time);
		}
		ipt.pl.time_info.push_back(start);
		ipt.pl.time_info.push_back(end);
		ipt.pl.time_info.push_back(ot);
		ipt.pl.time_info.push_back(reot);
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to public load flag = 0" << std::endl;
	}
}

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
		for (int i = 0; i < ipt.irl.load_num; i++)
		{
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 1 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			ipt.irl.power.push_back(sql.turnValueToFloat());

			sql.operate("SELECT household"+ to_string(ipt.bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 1 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);

			int count = get_already_operate_time("interrupt", i);
			int reot_time = get_remain_ot_time(result[2], count);

			ipt.irl.start.push_back(result[0]);
			ipt.irl.end.push_back(result[1] - 1);
			ipt.irl.ot.push_back(result[2]);
			ipt.irl.reot.push_back(reot_time);
		}
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to interrupt load flag = 0" << std::endl;
	}
}

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

			ipt.uirl.start.push_back(result[0]);
			if (modify_end_time != -1)
			{
				ipt.uirl.end.push_back(modify_end_time - 1);
			}
			else
			{
				ipt.uirl.end.push_back(result[1] - 1);
			}
			ipt.uirl.ot.push_back(result[2]);
			ipt.uirl.reot.push_back(reot_time);
		}
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to uninterrupt load flag = 0" << std::endl;
	}
}

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
		for (int i = 0; i < ipt.varl.load_num; i++)
		{
			vector<float> power_tmp;
			vector<int> block_tmp;
			sql.operate("SELECT power1, power2, power3 FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			power_tmp = sql.turnArrayToFloat();
			
			sql.operate("SELECT block1, block2, block3 FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			block_tmp = sql.turnArrayToInt();

			sql.operate("SELECT household"+ to_string(ipt.bp.real_household_id) +"_startEndOperationTime FROM load_list WHERE group_id = 3 LIMIT "+ to_string(i) +", "+ to_string(i+1));
			string timearray = sql.turnValueToString();
			vector<int> result = split_array(timearray);
			
			int count = get_already_operate_time("varying", i);
			bool flag = get_continuityLoad_flag("varyingDelta", i);
			int reot_time = get_remain_ot_time(result[2], count, flag);
			int modify_end_time = determine_change_end_time(result[2], count, reot_time, flag);

			ipt.varl.start.push_back(result[0]);
			if (modify_end_time != -1)
			{
				ipt.varl.end.push_back(modify_end_time - 1);
			}
			else
			{
				ipt.varl.end.push_back(result[1] - 1);
			}
			ipt.varl.ot.push_back(result[2]);
			ipt.varl.reot.push_back(reot_time);
			ipt.varl.power.push_back(convert_real_power_array(power_tmp, block_tmp));
			ipt.varl.block.push_back(convert_real_block_array(ipt.varl.start[i], ipt.varl.end[i]));
			ipt.varl.max_power.push_back(find_varyingLoad_max_power(power_tmp));
		}
	}
	else
	{
		std::cout << "Function "<< __func__ <<" don't work, due to varying load flag = 0" << std::endl;	
	}
}


void SQLACTION::get_household_participation()
{
	if (ipt.fg.dr_mode != 0)
	{
		for (int i = 0; i < ipt.dr.endTime-ipt.dr.startTime; i++)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM `LHEMS_demand_response_participation` WHERE `household_id` = "+to_string(ipt.bp.real_household_id));
			ipt.dr.participate_status.push_back(sql.turnValueToInt());
		}
	}
}

// export
// cems
void SQLACTION::calculate_table_cost_info()
{
	for (int i = 0; i < ipt.bp.Global_next_simulate_timeblock; i++)
	{
		sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_totalLoad +"'");
		ept.ci.totalLoad.push_back(sql.turnValueToFloat());
		
		sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_cost_of_totalLoad +"'");
		ept.ci.cost_of_totalLoad.push_back(sql.turnValueToFloat());

		if (ipt.fg.Pgrid)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_cost_of_gridOnly +"'");
			ept.ci.cost_of_gridOnly.push_back(sql.turnValueToFloat());
			// wrong to calculate dr feedback, can not dr baseline - grid price
			if (ipt.fg.dr_mode != 0)
			{
				sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_feedback_of_dr +"'");
				ept.ci.feedback_of_dr.push_back(sql.turnValueToFloat());
			}
			else
			{
				ept.ci.feedback_of_dr.push_back(0);
			}
		}
		else
		{
			ept.ci.cost_of_gridOnly.push_back(0);
		}
		
		if (ipt.fg.Psell)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_feedback_of_sellGrid +"'");
			ept.ci.feedback_of_sellGrid.push_back(sql.turnValueToFloat());
		}
		else
		{
			ept.ci.feedback_of_sellGrid.push_back(0);
		}
		
		if (ipt.fg.Pfc)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_cost_of_fuelCell +"'");
			ept.ci.cost_of_fuelCell.push_back(sql.turnValueToFloat());
			sql.operate("SELECT A"+ to_string(i) +" FROM cost WHERE cost_name = '"+ ept.ci.name_hydrogen_comsuption +"'");
			ept.ci.hydrogen_comsuption.push_back(sql.turnValueToFloat());
		}
		else
		{
			ept.ci.cost_of_fuelCell.push_back(0);
			ept.ci.hydrogen_comsuption.push_back(0);
		}
		
	}
	
	for (int i = ipt.bp.Global_next_simulate_timeblock; i < ipt.bp.time_block; i++)
	{
		sql.operate("SELECT totalLoad FROM totalLoad_model WHERE time_block = "+ to_string(i));
		ept.ci.totalLoad.push_back(sql.turnValueToFloat());
		for (int j = 0; j < ipt.pl.load_num; j++)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM GHEMS_control_status WHERE equip_name = 'publicLoad"+ to_string(j+1) +"'");
			int status = sql.turnValueToInt();
			sql.operate("SELECT power1 FROM load_list WHERE group_id = 5 LIMIT 1 OFFSET "+ to_string(j));
			float power = sql.turnValueToFloat();
			ept.ci.totalLoad[i] += status*power;
		}
		ept.ci.cost_of_totalLoad.push_back(ept.ci.totalLoad[i]*ipt.bp.price[i]*ipt.bp.delta_T);

		if (ipt.fg.Pgrid)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM GHEMS_control_status WHERE equip_name = 'Pgrid'");
			float power_tmp = sql.turnValueToFloat();
			ept.ci.cost_of_gridOnly.push_back(power_tmp * ipt.bp.price[i] * ipt.bp.delta_T);
			if (ipt.fg.dr_mode != 0 && i >= ipt.dr.startTime && i < ipt.dr.endTime)
			{
				float feedback_price= ipt.dr.feedback_price * (ipt.dr.customer_baseLine-power_tmp) * ipt.bp.delta_T;
				ept.ci.feedback_of_dr.push_back(feedback_price);
			}
			else
			{
				ept.ci.feedback_of_dr.push_back(0);
			}
		}
		else
		{
			ept.ci.cost_of_gridOnly.push_back(0);
		}

		if (ipt.fg.Psell)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM GHEMS_control_status WHERE equip_name = 'Psell'");
			ept.ci.feedback_of_sellGrid.push_back(sql.turnValueToFloat()*ipt.bp.price[i]*ipt.bp.delta_T);
		}
		else
		{
			ept.ci.feedback_of_sellGrid.push_back(0);
		}

		if (ipt.fg.Pfc)
		{
			sql.operate("SELECT A"+ to_string(i) +" FROM GHEMS_control_status WHERE equip_name = 'Pfct'");
			float fc_tmp = sql.turnValueToFloat();
			ept.ci.cost_of_fuelCell.push_back(fc_tmp*ipt.bp.hydro_price/ipt.bp.hydro_cons*ipt.bp.delta_T);
			ept.ci.hydrogen_comsuption.push_back(fc_tmp/ipt.bp.hydro_cons*ipt.bp.delta_T);
		}
		else
		{
			ept.ci.cost_of_fuelCell.push_back(0);
			ept.ci.hydrogen_comsuption.push_back(0);
		}
	}
}

void SQLACTION::update_table_cost_info()
{
	if (ipt.bp.Global_next_simulate_timeblock == 0)
	{
		insert_table_cost(ept.ci.name_totalLoad, ept.ci.totalLoad);
		insert_table_cost(ept.ci.name_cost_of_totalLoad, ept.ci.cost_of_totalLoad);
		insert_table_cost(ept.ci.name_cost_of_gridOnly, ept.ci.cost_of_gridOnly);
		insert_table_cost(ept.ci.name_feedback_of_sellGrid, ept.ci.feedback_of_sellGrid);
		insert_table_cost(ept.ci.name_cost_of_fuelCell, ept.ci.cost_of_fuelCell);
		insert_table_cost(ept.ci.name_hydrogen_comsuption, ept.ci.hydrogen_comsuption);
		insert_table_cost(ept.ci.name_feedback_of_dr, ept.ci.feedback_of_dr);
	}
	else
	{
		update_table_cost(ept.ci.name_totalLoad, ept.ci.totalLoad);
		update_table_cost(ept.ci.name_cost_of_totalLoad, ept.ci.cost_of_totalLoad);
		update_table_cost(ept.ci.name_cost_of_gridOnly, ept.ci.cost_of_gridOnly);
		update_table_cost(ept.ci.name_feedback_of_sellGrid, ept.ci.feedback_of_sellGrid);
		update_table_cost(ept.ci.name_cost_of_fuelCell, ept.ci.cost_of_fuelCell);
		update_table_cost(ept.ci.name_hydrogen_comsuption, ept.ci.hydrogen_comsuption);
		update_table_cost(ept.ci.name_feedback_of_dr, ept.ci.feedback_of_dr);
	}
}

void SQLACTION::calculate_table_BaseParameter_total_cost_info()
{
	ept.ci.summation_totalLoad = accumulate(ept.ci.totalLoad.begin(), ept.ci.totalLoad.end(), 0.0);
	ept.ci.summation_cost_of_totalLoad = accumulate(ept.ci.cost_of_totalLoad.begin(), ept.ci.cost_of_totalLoad.end(), 0.0);
	ept.ci.summation_cost_of_gridOnly = accumulate(ept.ci.cost_of_gridOnly.begin(), ept.ci.cost_of_gridOnly.end(), 0.0);
	ept.ci.summation_feedback_of_realSell = accumulate(ept.ci.feedback_of_sellGrid.begin(), ept.ci.feedback_of_sellGrid.end(), 0.0);
	ept.ci.summation_cost_of_fuelCell = accumulate(ept.ci.cost_of_fuelCell.begin(), ept.ci.cost_of_fuelCell.end(), 0.0);
	ept.ci.summation_hydrogen_comsuption = accumulate(ept.ci.hydrogen_comsuption.begin(), ept.ci.hydrogen_comsuption.end(), 0.0);
	ept.ci.summation_feedback_of_dr = accumulate(ept.ci.feedback_of_dr.begin(), ept.ci.feedback_of_dr.end(), 0.0);
	
	if (ept.ci.summation_totalLoad <= (120.0/30.0))
		ept.ci.summation_taipowercost_of_totalLoad = ept.ci.summation_totalLoad*1.63;

	else if ((ept.ci.summation_totalLoad > (120.0 / 30.0)) & (ept.ci.summation_totalLoad <= 330.0 / 30.0))
		ept.ci.summation_taipowercost_of_totalLoad = (120.0*1.63 + (ept.ci.summation_totalLoad*30.0 - 120.0)*2.38) / 30.0;

	else if ((ept.ci.summation_totalLoad > (330.0 / 30.0)) & (ept.ci.summation_totalLoad <= 500.0 / 30.0))
		ept.ci.summation_taipowercost_of_totalLoad = (120.0*1.63 + (330.0-120.0)*2.38 + (ept.ci.summation_totalLoad*30.0-330.0)*3.52) / 30.0;

	else if ((ept.ci.summation_totalLoad > (500.0 / 30.0)) & (ept.ci.summation_totalLoad <= 700.0 / 30.0))
		ept.ci.summation_taipowercost_of_totalLoad = (120.0*1.63 + (330.0-120.0)*2.38 + (500.0-330.0)*3.52 + (ept.ci.summation_totalLoad*30.0-500.0)*4.80) / 30.0;

	else if ((ept.ci.summation_totalLoad > (700.0 / 30.0)) & (ept.ci.summation_totalLoad <= 1000.0 / 30.0))
		ept.ci.summation_taipowercost_of_totalLoad = (120.0*1.63 + (330.0-120.0)*2.38 + (500.0-330.0)*3.52 + (700.0-500.0)*4.80 + (ept.ci.summation_totalLoad*30.0-700.0)*5.66) / 30.0;

	else if (ept.ci.summation_totalLoad > (1000.0 / 30.0))
		ept.ci.summation_taipowercost_of_totalLoad = (120.0*1.63 + (330.0-120.0)*2.38 + (500.0-330.0)*3.52 + (700.0-500.0)*4.80 + (1000.0-700.0)*5.66 + (ept.ci.summation_totalLoad*30.0-1000.0)*6.41) / 30.0;
}

void SQLACTION::update_table_BaseParameter_total_cost_info()
{
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_totalLoad) +" WHERE parameter_name = 'totalLoad' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_taipowercost_of_totalLoad) +" WHERE parameter_name = 'LoadSpend(taipowerPrice)' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_cost_of_totalLoad) +" WHERE parameter_name = 'LoadSpend(threeLevelPrice)' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_cost_of_gridOnly) +" WHERE parameter_name = 'realGridPurchase' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_feedback_of_realSell) +" WHERE parameter_name = 'maximumSell' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_cost_of_fuelCell) +" WHERE parameter_name = 'fuelCellSpend' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_hydrogen_comsuption) +" WHERE parameter_name = 'hydrogenConsumption(g)' ");
	sql.operate("UPDATE BaseParameter SET value = "+ to_string(ept.ci.summation_feedback_of_dr) +" WHERE parameter_name = 'demandResponse_feedbackPrice(g)' ");
}

void SQLACTION::update_new_SOC()
{
	sql.operate("UPDATE `BaseParameter` SET value = (SELECT A"+ to_string(ipt.bp.Global_next_simulate_timeblock) +" FROM GHEMS_control_status where equip_name = 'SOC') WHERE parameter_name = 'now_SOC'");
}

void SQLACTION::update_Global_next_simulate_timeblock()
{
	std::cout << "Global_next_simulate_timeblock: " << ipt.bp.Global_next_simulate_timeblock << std::endl;
	std::cout << "next Global_next_simulate_timeblock: " << ipt.bp.Global_next_simulate_timeblock+1 << std::endl;
	sql.operate("UPDATE BaseParameter SET value = '"+ to_string(ipt.bp.Global_next_simulate_timeblock+1) +"' WHERE  parameter_name = 'Global_next_simulate_timeblock'");
}

// hems
void SQLACTION::update_new_load_model(int group_num)
{
	vector<float> power_tmp;
	vector<float> power;
	
	power.assign(ipt.bp.time_block, 0);
	for (int i = 0; i < ipt.irl.load_num; i++)
	{
		sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE equip_name = 'interrupt"+ to_string(i+1) +"' and household_id = "+ to_string(ipt.bp.real_household_id));
		power_tmp = sql.turnArrayToFloat();
		for (int j = 0; j < power_tmp.size(); j++)
		{
			power_tmp[j] *= ipt.irl.power[i];
			power[j] += power_tmp[j];
		}
	}
	for (int i = 0; i < ipt.uirl.load_num; i++)
	{
		sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE equip_name = 'uninterrupt"+ to_string(i+1) +"' and household_id = "+ to_string(ipt.bp.real_household_id));
		power_tmp = sql.turnArrayToFloat();
		for (int j = 0; j < power_tmp.size(); j++)
		{
			power_tmp[j] *= ipt.uirl.power[i];
			power[j] += power_tmp[j];
		}
	}
	for (int i = 0; i < ipt.varl.load_num; i++)
	{
		sql.operate("SELECT "+ sql.column +" FROM LHEMS_control_status WHERE equip_name = 'varyingPsi"+ to_string(i+1) +"' and household_id = "+ to_string(ipt.bp.real_household_id));
		power_tmp = sql.turnArrayToFloat();
		for (int j = 0; j < power_tmp.size(); j++)
		{
			power[j] += power_tmp[j];
		}
	}
	power_tmp.clear();
	power.erase(power.begin(), power.begin()+ipt.bp.next_simulate_timeblock);

	for (int i = 0; i < ipt.bp.time_block - ipt.bp.next_simulate_timeblock; i++)
	{
		sql.operate("UPDATE `totalLoad_model` SET `household"+ to_string(ipt.bp.real_household_id) +"` = '"+ to_string(power[i]) +"' WHERE `totalLoad_model`.`time_block` = "+ to_string(i+ipt.bp.next_simulate_timeblock));
	}
	if (ipt.bp.distributed_household_id == ipt.bp.distributed_householdAmount)
	{
		sql.operate("UPDATE `distributed_group` SET `total_load_flag` = '1' WHERE `group_id` = "+ to_string(group_num));
		if (determine_distributedGroup_status("COUNT(group_id) = SUM(total_load_flag)"))
		{
			for (int i = 0; i < ipt.bp.time_block; i++)
			{
				float power_total = 0.0;
				for (int j = 0; j < ipt.bp.householdAmount; j++)
				{
					sql.operate("SELECT household"+ to_string(j+1) +" FROM totalLoad_model WHERE time_block = "+ to_string(i));
					power_total += sql.turnValueToFloat();
				}
				sql.operate("UPDATE `totalLoad_model` SET `totalLoad` = '"+ to_string(power_total) +"', `time` = CURRENT_TIMESTAMP WHERE `totalLoad_model`.`time_block` = "+ to_string(i));
			}	
		}	
	}
}

void SQLACTION::update_next_simulate_timeblock(int group_num)
{
	std::cout << "next_simulate_timeblock: " << ipt.bp.next_simulate_timeblock << std::endl;
	if (ipt.bp.distributed_household_id == ipt.bp.distributed_householdAmount)
	{
		std::cout << "next next_simulate_timeblock: " << ipt.bp.next_simulate_timeblock+1 << std::endl;
		sql.operate("UPDATE `distributed_group` SET `next_simulate_timeblock` = '"+ to_string(ipt.bp.next_simulate_timeblock+1) +"' WHERE `group_id` = "+ to_string(group_num));
		if (determine_distributedGroup_status("SUM(next_simulate_timeblock)/COUNT(group_id) = "+ to_string(ipt.bp.next_simulate_timeblock+1)))
		{
			sql.operate("UPDATE `BaseParameter` SET value = "+ to_string(ipt.bp.next_simulate_timeblock+1) +" WHERE `parameter_name` = 'next_simulate_timeblock' ");
		}
	}
}

void SQLACTION::update_household_id(int group_num)
{
	if (ipt.bp.distributed_household_id < ipt.bp.distributed_householdAmount)
	{
		ipt.bp.distributed_household_id++;
	}
	else
	{
		ipt.bp.distributed_household_id = 1;
	}
	std::cout << "next distributed_household_id = " << ipt.bp.distributed_household_id << std::endl;
	std::cout << "next real_household_id = " << (group_num - 1) * ipt.bp.distributed_householdAmount + ipt.bp.distributed_household_id << std::endl;
	sql.operate("UPDATE `distributed_group` SET `household_id` = '"+ to_string(ipt.bp.distributed_household_id) +"' WHERE `group_id` = "+ to_string(group_num));
}