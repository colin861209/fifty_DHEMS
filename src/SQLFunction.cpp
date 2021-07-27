#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <algorithm>
#include "SQLFunction.hpp"

using namespace std;
MYSQL *mysql_con = mysql_init(NULL);
MYSQL_RES *mysql_result;
MYSQL_ROW mysql_row;
char sql_buffer[2000] = {'\0'};
int row_totalNum, col_totalNum, error;

int fetch_row_value() {

	sent_query();
	mysql_result = mysql_store_result(mysql_con);
	if ((mysql_row = mysql_fetch_row(mysql_result)) != NULL) {

		row_totalNum = mysql_num_rows(mysql_result);
		col_totalNum = mysql_num_fields(mysql_result);
		error = 0;
	}
	else
		error = -1;

	mysql_free_result(mysql_result);
	memset(sql_buffer, 0, sizeof(sql_buffer));
	return error;
}

void sent_query() { mysql_query(mysql_con, sql_buffer); }

int turn_int(int col_num) {	

	if (mysql_row[col_num] != NULL)
		return atoi(mysql_row[col_num]); 
	else
		return -999;		
}

float turn_float(int col_num) { 
	
	if (mysql_row[col_num] != NULL)
		return atof(mysql_row[col_num]); 
	else
		return -999;
}

float turn_value_to_float(int col_num) {
	
	if (fetch_row_value() != -1) {

		float result = turn_float(col_num);
		return result;
	}
	else
		return -404;
}

int turn_value_to_int(int col_num) {

	if (fetch_row_value() != -1) {
		
		int result = turn_int(col_num);
		return result;
	}
	else
		return -404;
}

void messagePrint(int lineNum, const char *message, char contentSize, float content, char tabInHeader) {

	// tap 'Y' or 'N' means yes or no
	if (tabInHeader == 'Y')
		printf("\t");
	// tap 'I' or 'F' means int or float, otherwise no contents be showed.
	switch (contentSize)
	{
	case 'I':
		printf("LINE %d: %s%d\n", lineNum, message, (int)content);
		break;
	case 'F':
		printf("LINE %d: %s%f\n", lineNum, message, content);
		break;
	default:
		printf("LINE %d: %s\n", lineNum, message);
	}
}

void functionPrint(const char* functionName) {

	printf("\nFunction: %s\n", functionName);
}

int find_variableName_position(vector<string> variableNameArray, string target)
{
	auto it = find(variableNameArray.begin(), variableNameArray.end(), target);

	// If element was found
	if (it != variableNameArray.end())
		return (it - variableNameArray.begin());
	else
		return -1;
}

int *demand_response_info(int dr_mode)
{
	functionPrint(__func__);

	int *result = new int[5];
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT * FROM `demand_response` WHERE mode = %d", dr_mode);
	fetch_row_value();
	for (int i = 0; i < 5; i++)
	{
		result[i] = turn_int(i + 1);
	}

	messagePrint(__LINE__, "dr start time: ", 'I', result[0], 'Y');
	messagePrint(__LINE__, "dr end time: ", 'I', result[1], 'Y');
	messagePrint(__LINE__, "dr min decrease power: ", 'I', result[2], 'Y');
	messagePrint(__LINE__, "dr min feedback price: ", 'I', result[3], 'Y');
	messagePrint(__LINE__, "dr customer base line: ", 'I', result[4], 'Y');
	return result;
}

int flag_receive(string table_name, string table_variable_name) {

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT flag FROM `%s` WHERE variable_name = '%s' ", table_name.c_str(), table_variable_name.c_str());
	int flag = turn_value_to_int(0);
	return flag;
}

int value_receive(string table_name, string condition_col_name, string condition_col_target) {

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `%s` WHERE `%s` = '%s' ", table_name.c_str(), condition_col_name.c_str(), condition_col_target.c_str());
	int reuslt = turn_value_to_int(0);
	return reuslt;
}

int value_receive(string table_name, string condition_col_name, int condition_col_number) {

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `%s` WHERE `%s` = %d ", table_name.c_str(), condition_col_name.c_str(), condition_col_number);
	int reuslt = turn_value_to_int(0);
	return reuslt;
}

float value_receive(string table_name, string condition_col_name, string condition_col_target, char target_dataType) {

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `%s` WHERE `%s` = '%s' ", table_name.c_str(), condition_col_name.c_str(), condition_col_target.c_str());
	float reuslt = turn_value_to_float(0);
	return reuslt;
}

float value_receive(string table_name, string condition_col_name, int condition_col_number, char target_dataType) {

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `%s` WHERE `%s` = %d ", table_name.c_str(), condition_col_name.c_str(), condition_col_number);
	float reuslt = turn_value_to_float(0);
	return reuslt;
}

void insert_status_into_MySQLTable(string table_name, char *status_name, float *status_value, string col_name1, string col_value1, string col_name2, int col_value2) {

	if (col_name1.empty() && col_value1.empty())
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO %s (%s) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');", table_name.c_str(), status_name, status_value[0], status_value[1], status_value[2], status_value[3], status_value[4], status_value[5], status_value[6], status_value[7], status_value[8], status_value[9], status_value[10], status_value[11], status_value[12], status_value[13], status_value[14], status_value[15], status_value[16], status_value[17], status_value[18], status_value[19], status_value[20], status_value[21], status_value[22], status_value[23], status_value[24], status_value[25], status_value[26], status_value[27], status_value[28], status_value[29], status_value[30], status_value[31], status_value[32], status_value[33], status_value[34], status_value[35], status_value[36], status_value[37], status_value[38], status_value[39], status_value[40], status_value[41], status_value[42], status_value[43], status_value[44], status_value[45], status_value[46], status_value[47], status_value[48], status_value[49], status_value[50], status_value[51], status_value[52], status_value[53], status_value[54], status_value[55], status_value[56], status_value[57], status_value[58], status_value[59], status_value[60], status_value[61], status_value[62], status_value[63], status_value[64], status_value[65], status_value[66], status_value[67], status_value[68], status_value[69], status_value[70], status_value[71], status_value[72], status_value[73], status_value[74], status_value[75], status_value[76], status_value[77], status_value[78], status_value[79], status_value[80], status_value[81], status_value[82], status_value[83], status_value[84], status_value[85], status_value[86], status_value[87], status_value[88], status_value[89], status_value[90], status_value[91], status_value[92], status_value[93], status_value[94], status_value[95]);
	}
	else if (col_name2.empty() && col_value2 == -1)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO %s (%s, %s) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%s');", table_name.c_str(), status_name, col_name1.c_str(), status_value[0], status_value[1], status_value[2], status_value[3], status_value[4], status_value[5], status_value[6], status_value[7], status_value[8], status_value[9], status_value[10], status_value[11], status_value[12], status_value[13], status_value[14], status_value[15], status_value[16], status_value[17], status_value[18], status_value[19], status_value[20], status_value[21], status_value[22], status_value[23], status_value[24], status_value[25], status_value[26], status_value[27], status_value[28], status_value[29], status_value[30], status_value[31], status_value[32], status_value[33], status_value[34], status_value[35], status_value[36], status_value[37], status_value[38], status_value[39], status_value[40], status_value[41], status_value[42], status_value[43], status_value[44], status_value[45], status_value[46], status_value[47], status_value[48], status_value[49], status_value[50], status_value[51], status_value[52], status_value[53], status_value[54], status_value[55], status_value[56], status_value[57], status_value[58], status_value[59], status_value[60], status_value[61], status_value[62], status_value[63], status_value[64], status_value[65], status_value[66], status_value[67], status_value[68], status_value[69], status_value[70], status_value[71], status_value[72], status_value[73], status_value[74], status_value[75], status_value[76], status_value[77], status_value[78], status_value[79], status_value[80], status_value[81], status_value[82], status_value[83], status_value[84], status_value[85], status_value[86], status_value[87], status_value[88], status_value[89], status_value[90], status_value[91], status_value[92], status_value[93], status_value[94], status_value[95], col_value1.c_str());
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO %s (%s, %s, %s) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%s', '%d');", table_name.c_str(), status_name, col_name1.c_str(), col_name2.c_str(), status_value[0], status_value[1], status_value[2], status_value[3], status_value[4], status_value[5], status_value[6], status_value[7], status_value[8], status_value[9], status_value[10], status_value[11], status_value[12], status_value[13], status_value[14], status_value[15], status_value[16], status_value[17], status_value[18], status_value[19], status_value[20], status_value[21], status_value[22], status_value[23], status_value[24], status_value[25], status_value[26], status_value[27], status_value[28], status_value[29], status_value[30], status_value[31], status_value[32], status_value[33], status_value[34], status_value[35], status_value[36], status_value[37], status_value[38], status_value[39], status_value[40], status_value[41], status_value[42], status_value[43], status_value[44], status_value[45], status_value[46], status_value[47], status_value[48], status_value[49], status_value[50], status_value[51], status_value[52], status_value[53], status_value[54], status_value[55], status_value[56], status_value[57], status_value[58], status_value[59], status_value[60], status_value[61], status_value[62], status_value[63], status_value[64], status_value[65], status_value[66], status_value[67], status_value[68], status_value[69], status_value[70], status_value[71], status_value[72], status_value[73], status_value[74], status_value[75], status_value[76], status_value[77], status_value[78], status_value[79], status_value[80], status_value[81], status_value[82], status_value[83], status_value[84], status_value[85], status_value[86], status_value[87], status_value[88], status_value[89], status_value[90], status_value[91], status_value[92], status_value[93], status_value[94], status_value[95], col_value1.c_str(), col_value2);
	}
	sent_query();
}

void update_status_to_MySQLTable(string table_name, float *status_value, string condition_col_name1, string condition_col_target1, string conjunction, string condition_col_name2, int condition_col_target2) {

	if (conjunction.empty())
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE %s set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE %s = '%s';", table_name.c_str(), status_value[0], status_value[1], status_value[2], status_value[3], status_value[4], status_value[5], status_value[6], status_value[7], status_value[8], status_value[9], status_value[10], status_value[11], status_value[12], status_value[13], status_value[14], status_value[15], status_value[16], status_value[17], status_value[18], status_value[19], status_value[20], status_value[21], status_value[22], status_value[23], status_value[24], status_value[25], status_value[26], status_value[27], status_value[28], status_value[29], status_value[30], status_value[31], status_value[32], status_value[33], status_value[34], status_value[35], status_value[36], status_value[37], status_value[38], status_value[39], status_value[40], status_value[41], status_value[42], status_value[43], status_value[44], status_value[45], status_value[46], status_value[47], status_value[48], status_value[49], status_value[50], status_value[51], status_value[52], status_value[53], status_value[54], status_value[55], status_value[56], status_value[57], status_value[58], status_value[59], status_value[60], status_value[61], status_value[62], status_value[63], status_value[64], status_value[65], status_value[66], status_value[67], status_value[68], status_value[69], status_value[70], status_value[71], status_value[72], status_value[73], status_value[74], status_value[75], status_value[76], status_value[77], status_value[78], status_value[79], status_value[80], status_value[81], status_value[82], status_value[83], status_value[84], status_value[85], status_value[86], status_value[87], status_value[88], status_value[89], status_value[90], status_value[91], status_value[92], status_value[93], status_value[94], status_value[95], condition_col_name1.c_str(), condition_col_target1.c_str());
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE %s set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE %s = '%s' %s %s = %d;", table_name.c_str(), status_value[0], status_value[1], status_value[2], status_value[3], status_value[4], status_value[5], status_value[6], status_value[7], status_value[8], status_value[9], status_value[10], status_value[11], status_value[12], status_value[13], status_value[14], status_value[15], status_value[16], status_value[17], status_value[18], status_value[19], status_value[20], status_value[21], status_value[22], status_value[23], status_value[24], status_value[25], status_value[26], status_value[27], status_value[28], status_value[29], status_value[30], status_value[31], status_value[32], status_value[33], status_value[34], status_value[35], status_value[36], status_value[37], status_value[38], status_value[39], status_value[40], status_value[41], status_value[42], status_value[43], status_value[44], status_value[45], status_value[46], status_value[47], status_value[48], status_value[49], status_value[50], status_value[51], status_value[52], status_value[53], status_value[54], status_value[55], status_value[56], status_value[57], status_value[58], status_value[59], status_value[60], status_value[61], status_value[62], status_value[63], status_value[64], status_value[65], status_value[66], status_value[67], status_value[68], status_value[69], status_value[70], status_value[71], status_value[72], status_value[73], status_value[74], status_value[75], status_value[76], status_value[77], status_value[78], status_value[79], status_value[80], status_value[81], status_value[82], status_value[83], status_value[84], status_value[85], status_value[86], status_value[87], status_value[88], status_value[89], status_value[90], status_value[91], status_value[92], status_value[93], status_value[94], status_value[95], condition_col_name1.c_str(), condition_col_target1.c_str(), conjunction.c_str(), condition_col_name2.c_str(), condition_col_target2);
	}
	
	sent_query();
}

float **getPublicLoad(bool publicLoad_flag, int publicLoad_num)
{
	float **info = new float *[publicLoad_num];
	for (int i = 0; i < publicLoad_num; i++)
		info[i] = new float[4];

	if (publicLoad_flag)
	{
		for (int i = 0; i < publicLoad_num; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT public_loads FROM load_list WHERE group_id = 5 LIMIT %d, %d", i, i + 1);
			fetch_row_value();
			char *seo_time = mysql_row[0];
			char *token = strtok(seo_time, "~");
			int j = 0;
			while (token != NULL)
			{
				info[i][j] = atof(token);
				j++;
				token = strtok(NULL, "~");
			}
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT power1 FROM load_list WHERE group_id = 5 LIMIT %d, %d", i, i + 1);
			info[i][j] = turn_value_to_float(0);
		}
	}
	else
	{
		for (int i = 0; i < publicLoad_num; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				info[i][j] = 0.0;
			}
		}
	}
	return info;
}