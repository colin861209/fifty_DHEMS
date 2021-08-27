#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glpk.h>
#include <mysql.h>
#include <iostream>

#include "optimize.hpp"
#include "sqlAction.hpp"

int main(int argc, const char** argv) {
    
	SQLACTION act("140.124.42.65","root", "fuzzy314", "DHEMS_fiftyHousehold");
	int group_id = 1;
	ENERGYMANAGESYSTEM ems_type = ENERGYMANAGESYSTEM::HEMS;
	// flag
	act.get_flag(ems_type);
	act.get_experimental_parameters(ems_type);
	act.get_distributedGroup_householdAndSampleTime(group_id);
	
	act.determine_LHEMS_realTimeOrOneDayMode_andGetSOC(group_id);
	// base parm
	act.create_variable_name(ems_type);
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
	act.get_household_participation();
	// op.ipt = act.ipt;
	optimize op(act.ipt, OBJECTIVETARGET::MINIMUM, ems_type, group_id);
	// act.result = op.result;
	op.print();

	act.update_new_load_model(group_id);
	act.update_household_id(group_id);
	act.update_next_simulate_timeblock(group_id);
}