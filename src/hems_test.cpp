#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glpk.h>
#include <mysql.h>
#include <iostream>

// #include "SQL.hpp"
#include "sqlAction.hpp"

int main(int argc, const char** argv) {
    
	SQLACTION act("140.124.42.65","root", "fuzzy314", "DHEMS_fiftyHousehold");
	int group_id = 1;
	// flag
	act.get_flag();
	act.get_experimental_parameters();
	act.get_distributedGroup_householdAndSampleTime(group_id);
	
	act.determine_LHEMS_realTimeOrOneDayMode_andGetSOC(group_id);
	// base parm
	act.create_variable_name();
	act.get_allDay_price();
	act.init_totalLoad_tableAndFlag(group_id);
	
	// load
	act.get_interrupt_info();
	act.get_uninterrupt_info();
	act.get_varying_info();
	// dr info
	act.get_dr_mode();
	act.get_demand_response();
	act.get_Pgrid_max_array();
}