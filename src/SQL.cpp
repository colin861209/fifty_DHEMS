#include "SQL.hpp"

void SQL::connect(const std::string ip,const std::string name,const std::string passwd,const std::string database)
{
	server_ip=ip;
    user_name=name;
    password=passwd;
    database_name=database;

    //初始化mysql   
    mysql_conn=mysql_init(NULL);

   //連線mysql  
    if(!mysql_real_connect(mysql_conn,server_ip.c_str(),user_name.c_str(),password.c_str(),database_name.c_str(),0,NULL,0)) //後面三個引數分別是port,unix_socket,client_flag.
    {
        std::cout<<"fail to connect mysql"<<std::endl;
        exit(1);
    }else{
        std::cout <<"success to connect mysql"<<std::endl;
    }
}

void SQL::disconnect()
{
    mysql_close(mysql_conn);
}

bool SQL::operate(const std::string &operation)
{
	if(mysql_query(mysql_conn,operation.c_str()))
    {
        std::cout<<"mysql操作失敗"<<std::endl;
        return false;
    }
    //將操作結果儲存在結果集  
    mysql_res=mysql_use_result(mysql_conn);
    return true;
}

MYSQL_ROW SQL::getRow(void)
{
    return mysql_fetch_row(mysql_res);
}

int SQL::getColNum(void)
{
    return mysql_num_fields(mysql_res);
}

void SQL::print_result(void)
{
    int num=mysql_num_fields(mysql_res);
    while((mysql_row = getRow()) != NULL) {
        for(int i = 0; i < num; i++) {
            std::cout << mysql_row[i]<<" ";
        }
        std::cout << std::endl;
    }
}

float SQL::turnValueToFloat()
{
	mysql_row = getRow();
	float result = atof(mysql_row[0]);
	free_result();
	return result;
}

vector<float> SQL::turnArrayToFloat()
{
	int colNum = getColNum();
	// float *result = new float[colNum];
    vector<float> result;
	while((mysql_row = getRow()) != NULL) {
        
		for(int i = 0; i < colNum; i++)
			result.push_back(atof(mysql_row[i]));
    }
	free_result();
	return result;
}

int SQL::turnValueToInt()
{
	mysql_row = getRow();
	int result = atoi(mysql_row[0]);
	free_result();
	return result;
}

vector<int> SQL::turnArrayToInt()
{	
	int colNum = getColNum();
	// int *result = new int[colNum];
    vector<int> result;
	while((mysql_row = getRow()) != NULL) {
        
        for(int i = 0; i < colNum; i++)
			result.push_back(atoi(mysql_row[i]));
    }
	free_result();
	
	return result;
}

string SQL::turnValueToString()
{
	mysql_row = getRow();
	string result = mysql_row[0];
	free_result();
	return result;
}

void SQL::free_result(void)   
{
	mysql_free_result(mysql_res);
}

