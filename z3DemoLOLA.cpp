#include<vector>
#include<cstdio>
#include<iostream>
#include<z3++.h>

using namespace std;
using namespace z3;

void SMTSolver()
{
    context c;
    solver s(c);

    int totNumberEvents = 5;
    int epsilon = 2;
    vector<vector<int> > eventValue = {{1, 5, 5, 7, 7}, {2, 3, 4, 4, 4}};

    expr_vector eventValue1(c);

    for(int i = 0; i < totNumberEvents; i++)
    {
        string str = "eventVal1" + to_string(i);
        eventValue1.push_back(c.int_const());

        expr_vector eventValue1All(c);
        for(int j = max(0, i - epsilon + 1); j < min(5, i + epsilon - 1) ; j++)
        {
            eventValue1All.push_back(eventValue1[i] == eventValue[0][i]);
        }
        s.add(mk_or(eventValue1All));
    }

    expr_vector eventValue2(c);

    for(int i = 0; i < totNumberEvents; i++)
    {
        string str = "eventVal2" + to_string(i);
        eventValue2.push_back(c.int_const());

        expr_vector eventValue2All(c);
        for(int j = max(0, i - epsilon + 1); j < min(5, i + epsilon - 1) ; j++)
        {
            eventValue2All.push_back(eventValue2[i] == eventValue[1][i]);
        }
        s.add(mk_or(eventValue2All));
    }

    expr_vector eventValueFinal(c);

    for(int i = 0; i < totNumberEvents; i++)
    {
        string str = "eventValFinal" + to_string(i);
        eventValueFinal.push_back(c.int_const());

        s.add(eventValueFinal[i] = eventValue1[i] + eventValue2[i]);
    }
    
    cout << s.check() << endl;
    cout << s.get_model() << endl;
}

int main()
{
    SMTSolver();

    return 1;
}