#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
// #include "SQLFunction.hpp"
// use function 'find_variableName_position' needs

#include "sqlAction.hpp"
#include "optimize.hpp"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, const char** argv) {
	// IMPORT ipt;
	SQLACTION act("140.124.42.65","root", "fuzzy314", "DHEMS_fiftyHousehold");
	ENERGYMANAGESYSTEM ems_name = ENERGYMANAGESYSTEM::CEMS;
	
	act.get_flag(ems_name);
	act.get_experimental_parameters(ems_name);
	
	act.determine_GHEMS_realTimeOrOneDayMode_andGetSOC();
	
	act.create_variable_name(ems_name);
	act.get_allDay_price();
	act.getOrUpdate_SolarInfo_ThroughSampleTime();
	act.get_totalLoad_power();
	
	// dr info
	act.get_dr_mode();
	act.get_demand_response();
	act.get_Pgrid_max_array();
	
	// public load
	act.get_publicLoad_info();

	// op.ipt = act.ipt;
	optimize op(act.ipt);
	// act.result = op.result;
	op.print();

	act.calculate_table_cost_info();
	act.update_table_cost_info();
	act.calculate_table_BaseParameter_total_cost_info();
	act.update_table_BaseParameter_total_cost_info();
	act.update_new_SOC();
	act.update_Global_next_simulate_timeblock();
}