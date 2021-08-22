#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glpk.h>
#include <mysql.h>
#include <iostream>

// #include "SQL.hpp"
#include "import.hpp"

int main(int argc, const char** argv) {
    
	IMPORT ipt("140.124.42.65","root", "fuzzy314", "DHEMS_fiftyHousehold");
	int group_id = 1;
	string ems_name = "HEMS";
	// flag
	ipt.get_flag(ems_name);
	ipt.get_experimental_parameters(ems_name);
	ipt.get_distributedGroup_householdAndSampleTime(group_id);
	
	ipt.determine_LHEMS_realTimeOrOneDayMode_andGetSOC(group_id);
	// base parm
	ipt.create_variable_name(ems_name);
	ipt.get_allDay_price();
	ipt.init_totalLoad_tableAndFlag(group_id);
	
	// load
	ipt.get_interrupt_info();
	ipt.get_uninterrupt_info();
	ipt.get_varying_info();
	// dr info
	ipt.get_dr_mode();
	ipt.get_demand_response();
	ipt.get_Pgrid_max_array();
	
}