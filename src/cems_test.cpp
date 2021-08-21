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
#include "SQL.hpp"
#include "import.hpp"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, const char** argv) {
	
	import ipt("140.124.42.65","root", "fuzzy314", "DHEMS_fiftyHousehold");
	string ems_name = "CEMS";
	
	ipt.get_flag(ems_name);
	ipt.get_experimental_parameters(ems_name);
	
	ipt.determine_GHEMS_realTimeOrOneDayMode_andGetSOC();
	
	ipt.create_variable_name(ems_name);
	ipt.get_allDay_price();
	ipt.getOrUpdate_SolarInfo_ThroughSampleTime();
	ipt.get_totalLoad_power();
	
	// dr info
	ipt.get_dr_mode();
	ipt.get_demand_response();
	ipt.get_Pgrid_max_array();
	
	// public load
	ipt.get_publicLoad_info();
}