#ifndef SQLFunction_H
#define SQLFunction_H

#include <string>
#include <vector>
using namespace std;

extern MYSQL *mysql_con;
extern MYSQL_RES *mysql_result;
extern MYSQL_ROW mysql_row;
extern int row_totalNum, col_totalNum;
extern char sql_buffer[2000];

typedef struct 
{
	bool Pgrid_flag;
	int time_block;
	int sample_time;
	int remain_timeblock;
	int divide;
	int variable;
	float delta_T;
	float Pgrid_max;
	float* price;
	float *solar;
	vector<float> Pgrid_max_array;
	vector<string> variable_name;
	string str_Pgrid = "Pgrid";
	// GLPK
	int coef_row_num = 0;
	int bnd_row_num = 1;
	// GHEMS
	bool mu_grid_flag;
	bool Psell_flag;
	bool Pfc_flag;
	bool SOC_change_flag;
	float Psell_max;
	float Delta_battery;
	float Pfc_max;
	int point_num;
	int piecewise_num;
	float* load_model;
	string str_mu_grid = "mu_grid";
	string str_Psell = "Psell";
	string str_Pfc = "Pfc";
	string str_Pfct = "Pfct";
	string str_PfcON = "PfcON";
	string str_PfcOFF = "PfcOFF";
	string str_muFC = "muFC";
	string str_zPfc = "zPfc";
	string str_lambda_Pfc = "lambda_Pfc";
	string str_SOC_change = "SOC_change";
	string str_SOC_increase = "SOC_increase";
	string str_SOC_decrease = "SOC_decrease";
	string str_SOC_Z = "SOC_Z";
	// LHEMS
	int app_count;
	int householdTotal;
	int household_id;
	int distributed_householdTotal;
	int distributed_household_id;
} BASEPARAMETER;

typedef struct 
{
	bool flag;
	float capacity;
	float battery_rate;
	float INIT_SOC;
	float MIN_SOC;
	float MAX_SOC;
	float threshold_SOC;
	float MIN_power;
	float MAX_power;
	string str_Pess = "Pess";
	string str_Pcharge = "Pcharge";
	string str_Pdischarge = "Pdischarge";
	string str_SOC = "ESS_SOC";
	string str_Z = "ESS_Z";
} ENERGYSTORAGESYSTEM;

typedef struct 
{
	int mode;
	int startTime;
	int endTime;
	int minDecrease_power;
	int feedback_price;
	int customer_baseLine;
	float household_CBL;
} DEMANDRESPONSE;

typedef struct 
{
	bool flag;
	bool generate_flag;
	int number;
	float* power_array;
}UNCONTROLLABLELOAD;

int connect_mysql(string DB_name);
int fetch_row_value();
void sent_query();
int turn_int(int col_num);
float turn_float(int col_num);
int turn_value_to_int(int col_num);
float turn_value_to_float(int col_num);
char* turn_value_to_string(int col_num);
void messagePrint(int lineNum, const char *message, char contentSize = 'S', float content = 0,  char tabInHeader = 'N');
void functionPrint(const char* functionName);
int find_variableName_position(vector<string> variableNameArray, string target);
int *demand_response_info(int );
int flag_receive(string table_name, string table_variable_name);
int value_receive(string table_name, string condition_col_name, string condition_col_target);
int value_receive(string table_name, string condition_col_name, int condition_col_number);
float value_receive(string table_name, string condition_col_name, string condition_col_target, char target_dataType);
float value_receive(string table_name, string condition_col_name, int condition_col_number, char target_dataType);
float *get_allDay_price(int time_block, string col_name);
void insert_status_into_MySQLTable(string table_name, char *status_name, float *status_value, string col_name1 = "", string col_value1 = "", string col_name2 = "", int col_value2 = -1);
void update_status_to_MySQLTable(string table_name, float *status_value, string condition_col_name1, string condition_col_target1, string conjunction = "", string condition_col_name2 = "", int condition_col_target2 = -1);
float **getPublicLoad(int group_number, int publicLoad_num);
void *new2d(int, int, int);
#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))
#endif 