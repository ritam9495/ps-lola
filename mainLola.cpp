#include<cstdio>
#include<iostream>
#include<fstream>
#include <utility>
#include<vector>
#include<random>
#include<time.h>
#include<cstdlib>
//#include<omp.h>
//#include<z3++.h>

using namespace std;
//using namespace z3;

class SystemTrace
{
public:
    SystemTrace();
    SystemTrace(string, int, int);
    void getTrace(int, int);
    void distributeTrace(int);
    vector<vector<vector<int> > > eventDataMon;
    vector<vector<vector<int> > > eventTimeMon;

private:
    string traceFileName;
    void readEntireFile();
    void readRACEData();
    void readSwATData();
    void readPowerData();
    vector<vector<int> > eventDataAll;
    vector<vector<int> > eventTimeAll;
    vector<vector<int> > eventData;
    vector<vector<int> > eventTime;
};

/*
*
*/
SystemTrace::SystemTrace()
{
    traceFileName = "sampleTrace.txt";
    int numProcess = 2;
    int numMonitors = 5;
    for(int i = 0; i < numProcess; i++)
    {
        eventDataAll.emplace_back(vector<int>());
        eventTimeAll.emplace_back(vector<int>());
        eventData.emplace_back(vector<int>());
        eventTime.emplace_back(vector<int>());
    }

    for(int i = 0; i < numMonitors; i++)
    {
        eventDataMon.emplace_back(vector<vector<int> >());
        eventTimeMon.emplace_back(vector<vector<int> >());
        for(int j = 0; j < numProcess; j++)
        {
            eventDataMon[i].emplace_back(vector<int>());
            eventTimeMon[i].emplace_back(vector<int>());
        }
    }
}

/*
*
*/
SystemTrace::SystemTrace(string fileName, int numProcess, int numMonitors)
{
    traceFileName = fileName;

    for(int i; i < numProcess; i++)
    {
        eventDataAll.emplace_back(vector<int>());
        eventTimeAll.emplace_back(vector<int>());
        eventData.emplace_back(vector<int>());
        eventTime.emplace_back(vector<int>());
    }

    for(int i = 0; i < numMonitors; i++)
    {
        eventDataMon.emplace_back(vector<vector<int> >());
        eventTimeMon.emplace_back(vector<vector<int> >());
        for(int j = 0; j < numProcess; j++)
        {
            eventDataMon[i].emplace_back(vector<int>());
            eventTimeMon[i].emplace_back(vector<int>());
        }
    }

    readEntireFile();
}

/*
*
*/
void SystemTrace::readEntireFile()
{
    ifstream myFile(traceFileName);
    string line;
    int i = -1;

    while(getline(myFile, line))
    {
        if(line.find("Process") != -1)
        {
            i++;
        }
        else
        {
            int a = line.find(',');
            // cout << line << ":" << line.substr(0, a) << ", " << line.substr(a+2, line.length()-a-1) << endl;
            eventTimeAll[i].push_back(stoi(line.substr(0, a)));
            eventDataAll[i].push_back(stoi(line.substr(a+2, line.length()-a-1)));
        }
    }
    myFile.close();
}

/*
 *
 */
void SystemTrace::readRACEData()
{

}

/*
 *
 */
void SystemTrace::readSwATData()
{

}

/*
 *
 */
void SystemTrace::readPowerData()
{

}

/*
*
*/
void SystemTrace::getTrace(int fromTime, int toTime)
{
    readEntireFile();

    for(int i = 0; i < eventDataAll.size(); i++)
    {
        for(int j = 0; j < eventDataAll[i].size(); j++)
        {
            if(eventTimeAll[i][j] >= fromTime && eventTimeAll[i][j] <= toTime)
            {
                eventData[i].push_back(eventDataAll[i][j]);
                eventTime[i].push_back(eventTimeAll[i][j]);
            }
        }
    }
}

/*
*
*/
void SystemTrace::distributeTrace(int numMon)
{
    random_device gen;
    bernoulli_distribution dist(1);
    vector<vector<int> > flag(eventData.size(), vector<int>(eventData[0].size(), 0));

    for(int mon = 0; mon < numMon; mon++)
    {
        for(int st = 0; st < eventData.size(); st++)
        {
            for(int ev = 0 ; ev < eventData[st].size(); ev++)
            {
                if(dist(gen))
                {
                    eventDataMon[mon][st].push_back(eventData[st][ev]);
                    eventTimeMon[mon][st].push_back(eventTime[st][ev]);
                    cout << mon << ": " << st << ", " << ev << endl;
                    flag[st][ev] = 1;
                }
            }
        }
    }

    //making sure every event is read by at least one monitor
    for(int st = 0; st < eventData.size(); st++)
    {
        for(int ev = 0; ev < eventData[st].size(); ev++)
        {
            if(flag[st][ev] == 0)
            {
                int mon = random() % numMon;
                cout << "Flag:" << mon << " : " << st << ", " << ev << endl;
                eventDataMon[mon][st].insert(eventDataMon[mon][st].begin()+ev, eventData[st][ev]);
                eventTimeMon[mon][st].insert(eventTimeMon[mon][st].begin()+ev, eventTime[st][ev]);
            }
        }
    }
}

class Monitor
{
public:
    Monitor();
    Monitor(int);
    void setCurrentData(vector<vector<int> >, vector<vector<int> >);
    void getEvaluation(int, int, int);

private:
    vector<vector<int> > eventDataHistory;
    vector<vector<int> > evalResults;
    void smtSolver(string);
    void iterativeSolution(string, int, int, int);
    void generateComb(vector<vector<int> >&, vector<int>, vector<vector<int> >, int);
    void getAllCombination(vector<vector<int> >&, vector<vector<int> >);
};

Monitor::Monitor()
{
    for(int  i = 0; i < 10; i++) {
        eventDataHistory.emplace_back(vector<int>());
        evalResults.emplace_back(vector<int>());
    }
}

Monitor::Monitor(int traceLength)
{
    for(int  i = 0; i < traceLength; i++)
    {
        eventDataHistory.emplace_back(vector<int>());
        evalResults.emplace_back(vector<int>());
    }
}

void Monitor::generateComb(vector<vector<int> >& allComb, vector<int> present, vector<vector<int> > words, int i)
{
    if(i == words.size())
    {
        allComb.emplace_back(present);
        return;
    }

    for(int n : words[i])
    {
        present[i] = n;
        // cout << i << " " << n << endl;
        generateComb(allComb, present, words, i+1);
    }
}

void Monitor::getAllCombination(vector<vector<int> >& allComb, vector<vector<int> > words)
{
    vector<int> present(words.size(), 0);

    generateComb(allComb, present, words, 0);
}

void Monitor::setCurrentData(vector<vector<int> > data, vector<vector<int> > time)
{
    for(int i = 0; i < data.size(); i++)
    {
        for(int j = 0; j < data[i].size(); j++)
        {
            int timeOcc = time[i][j];
            eventDataHistory[timeOcc].push_back(data[i][j]);
        }
    }
}

void Monitor::getEvaluation(int epsilon, int startTime, int endTime)
{
    vector<string> outputFormula = {"a+b", "a+1"};

    for(string str : outputFormula)
    {
        iterativeSolution(str, epsilon, max(0, startTime - epsilon + 1), endTime);
    }
}

void Monitor::iterativeSolution(string formula, int epsilon, int startTime, int endTime)
{
    vector<vector<int> > resultValue(endTime - startTime, vector<int>);
    smtSolver(resultValue, formula, epsilon, startTime, endTime);

    for(int i = startTime; i < endTime; i++)
    {
        for(int val : resultValue[i])
        {
            if(find(evalResults[i].begin(), evalResult[i].end(), val) == evalResults[i].end())
                evalResults[i].push_back(val);
        }
    }
}

void Monitor::smtSolver(vector<vector<int> >& resultValue, string formula, int epsilon, int startTime, int endTime)
{

    context c;
    solver s(c);

    int totNumberEvents = endTime - startTime;

    expr_vector eventValue1(c);

    for(int i = 0; i < totNumberEvents; i++)
    {
        string str = "eventVal1" + to_string(i);
        eventValue1.push_back(c.int_const());

        expr_vector eventValue1All(c);
        for(int j = max(0, i - epsilon + 1); j < min(currTime, i + epsilon) ; j++)
        {
            eventValue1All.push_back(eventValue1[i] == eventDataHistory[0][j]);
        }
        s.add(mk_or(eventValue1All));
    }

    expr_vector eventValue2(c);

    for(int i = 0; i < totNumberEvents; i++)
    {
        string str = "eventVal2" + to_string(i);
        eventValue2.push_back(c.int_const());

        expr_vector eventValue2All(c);
        for(int j = max(0, i - epsilon + 1); j < min(currTime, i + epsilon) ; j++)
        {
            eventValue2All.push_back(eventValue2[i] == eventDataHistory[1][j]);
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

    while(s.check() != unsat)
    {
        model m = s.get_model();
        
        for(int i = 0; i < totNumberEvents; i++)
        {
            int val = m.eval(eventValueFinal[i]).get_numeral_int();
            // int val = m.eval(eventValueFinal2[i]).is_true();
            
            if(find(resultValue[i].begin(), resultValue[i].end(), val) == resultValue[i].end())
                resultValue[i].push_back(val);
            // cout << val << " ";
        }

        vector<vector<int> > allComb;
        getAllCombination(allComb, resultValue);

        for(vector<int> comb : allComb)
        {
            expr_vector negateResults(c);

            for(int i = 0; i < totNumberEvents; i++)
            {
                negateResults.push_back(eventValueFinal[i] != comb[i]);
            }

            s.add(mk_or(negateResults));
        }
    }
}

int main()
{
    cout << "Welcome to P-LOLA!" << endl;

    int numMonitors = 5;
    int timeStep = 5, maxTime = 5, epsilon = 2;

    SystemTrace st;
    vector<Monitor> allMonitors(numMonitors);

    time_t startTime = time(NULL);

    for(int currTime = 1; currTime < maxTime; currTime += timeStep)
    {
        st.getTrace(max(0, currTime - epsilon), min(maxTime, currTime + timeStep));
        st.distributeTrace(numMonitors);

//         checking if all the data is really consistently (manually)
//        for(int i = 0; i < st.eventDataMon.size(); i++)
//        {
//            for(int j = 0; j < st.eventDataMon[i].size(); j++)
//            {
//                for(int k = 0; k < st.eventDataMon[i][j].size(); k++)
//                {
//                    cout << i << " : " << st.eventDataMon[i][j][k] << ", " << st.eventTimeMon[i][j][k] << endl;
//                }
//            }
//        }

        /*
         * Local Computation for Monitors
         */
        for(int mon = 0; mon < numMonitors; mon++)
        {
            allMonitors.emplace_back(Monitor());
            cout << "Monitor: " << mon << endl;

            allMonitors[mon].setCurrentData(st.eventDataMon[mon], st.eventTimeMon[mon]);

            allMonitors[mon].getEvaluation(epsilon, currTime, currTime + timeStep);
        }

        /*
         * Communication
         */
        // for(int mon1 = 0; mon1 < numMonitors; mon1++)
        // {
        //     for(int mon2 = 0; mon2 < numMonitors; mon2++)
        //     {
        //         if(mon1 == mon2)
        //             continue;

        //         allMonitors[mon1].otherEvaluations(allMonitors[mon2].);
        //     }
        // }
    }

    time_t endTime = time(NULL);

    auto exeTime = (double)(endTime - startTime);

    cout << "Execution Time: " << exeTime << endl;

    return 1;
}
