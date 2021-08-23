#include "variable.hpp"
#include <string>
#include <iostream>
using namespace std;

class optimize
{
private:
    
    IMPORT ipt_data;
public:
    optimize(IMPORT ipt);
    ~optimize();
    void print();
};

optimize::optimize(IMPORT ipt)
{
    ipt_data = ipt;
}

optimize::~optimize()
{
}

void optimize::print()
{
    cout << ipt_data.bp.Vsys << endl;
    cout << ipt_data.bp.price[5] << endl;
}