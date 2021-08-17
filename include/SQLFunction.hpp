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
extern vector<string> variable_name;
extern int dr_mode, dr_startTime, dr_endTime, dr_minDecrease_power, dr_feedback_price, dr_customer_baseLine;
extern bool Pgrid_flag, mu_grid_flag, Psell_flag, Pess_flag, Pfc_flag, interruptLoad_flag, uninterruptLoad_flag, varyingLoad_flag, comfortLevel_flag;

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
void insert_status_into_MySQLTable(string table_name, char *status_name, float *status_value, string col_name1 = "", string col_value1 = "", string col_name2 = "", int col_value2 = -1);
void update_status_to_MySQLTable(string table_name, float *status_value, string condition_col_name1, string condition_col_target1, string conjunction = "", string condition_col_name2 = "", int condition_col_target2 = -1);
float **getPublicLoad(bool publicLoad_flag, int publicLoad_num);
void *new2d(int, int, int);
#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))
#endif 