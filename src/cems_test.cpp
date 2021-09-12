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
	
	act.get_flag();
	act.get_dr_mode();
	act.get_experimental_parameters();
	act.determine_GHEMS_realTimeOrOneDayMode_andGetSOC();
	
	// dr info
	act.get_demand_response();
	act.get_Pgrid_max_array();
	act.get_dr_already_decrease_power();
	// base parm
	act.create_variable_name();
	act.get_allDay_price();
	act.getOrUpdate_SolarInfo_ThroughSampleTime();
	act.get_totalLoad_power();
	
	// public load
	act.get_publicLoad_info();

	// op.ipt = act.ipt;
	optimize op(act.ipt, OBJECTIVETARGET::MINIMUM);
	op.setting_cems_coefficient();
	op.setting_cems_objectiveFunction();
	
	if (op.verify_solution_after_sovle_GLPK() != -1)
	{
		op.saving_result();
		act.get_GLPK_solve_result(op.solve_result);
		act.insertOrUpdate_control_status();
	}
	act.calculate_table_cost_info();
	act.update_table_cost_info();
	act.calculate_table_BaseParameter_total_cost_info();
	act.update_table_BaseParameter_total_cost_info();
	act.update_new_SOC();
	act.update_Global_next_simulate_timeblock();
}