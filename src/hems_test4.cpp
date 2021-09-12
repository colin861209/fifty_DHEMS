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
	int group_id = 4;
	ENERGYMANAGESYSTEM ems_type = ENERGYMANAGESYSTEM::HEMS;
	// flag
	act.get_flag(ems_type);
	act.get_dr_mode();
	act.get_experimental_parameters(ems_type);
	act.get_distributedGroup_householdAndSampleTime(group_id);
	
	act.determine_LHEMS_realTimeOrOneDayMode_andGetSOC(group_id);

	// dr info
	act.get_demand_response();
	act.get_household_participation();
	// base parm
	act.create_variable_name(ems_type);
	act.get_allDay_price();
	act.init_totalLoad_tableAndFlag(group_id);
	
	// load
	act.get_interrupt_info();
	act.get_uninterrupt_info();
	act.get_varying_info();
	// op.ipt = act.ipt;
	optimize op(act.ipt, OBJECTIVETARGET::MINIMUM, ems_type, group_id);
	op.setting_hems_coefficient();
	op.setting_hems_objectiveFunction();
	
	if (op.verify_solution_after_sovle_GLPK(ENERGYMANAGESYSTEM::HEMS) != -1)
	{
		op.saving_result(ems_type);
		act.get_GLPK_solve_result(op.solve_result);
		act.insertOrUpdate_control_status(ems_type);
	}
	act.update_new_load_model(group_id);
	act.update_next_simulate_timeblock(group_id);
	act.update_household_id(group_id);
}