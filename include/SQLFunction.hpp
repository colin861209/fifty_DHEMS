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
	// SQL 
	char str_sql_allTimeblock[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";
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
	int feedback_price;
	// GHEMS 
	int minDecrease_power;
	int customer_baseLine;
	// LHEMS
	float *participate_array;
	float household_CBL;
} DEMANDRESPONSE;

typedef struct 
{
    int hems_group_id = 4;
    int cems_group_id = 7;
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