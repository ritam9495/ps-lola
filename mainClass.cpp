#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <fstream>
// #include "z3++.h"
#include <omp.h>

using namespace std;
// using namespace z3;

struct TreeNode
{
	string formula;
	int verdict; //true 1; false -1; don't know 0;
	int b1, b2; //interval
	struct TreeNode* left;
	struct TreeNode* right;

	TreeNode()
	{
		formula = "";
		verdict = 0;
		b1 = 0;
		b2 = 0;

		left = NULL;
		right = NULL;
	}

	TreeNode(string val)
	{
		formula = val;
		verdict = 0;
		b1 = 0;
		b2 = 0;

		left = NULL;
		right = NULL;
	}
};

class MakeParseTree
{
public:
	MakeParseTree();
	struct TreeNode* getRoot();
	void genTree(string);

private:
	struct TreeNode* root;
	struct TreeNode* makeTree(string, string);
};

MakeParseTree::MakeParseTree()
{
	root = NULL;
}

struct TreeNode* MakeParseTree::getRoot()
{
	return root;
}

void MakeParseTree::genTree(string formula)
{
	string form, formI;
	int flag, flagI = 0;

	stack<string> stack;

	for(int i = 0; i < formula.length(); i++)
	{
		char ch = formula[i];
		if(ch != ' ')
		{
			// cout << ch << " flag:" << flagI << endl;
			if(flagI == 1)
			{
				formI = formI + ch;
				if(ch == ']')
				{
					stack.push(formI);
					flagI = 0;
				}
			}
			else if(flagI == 0)
			{
				if(ch == 'G' || ch == 'U' || ch == 'F')
				{
					formI = ch;
					flagI = 1;
				}
				else if(ch != ')')
					stack.push(string(1, ch));
				else
				{
					form = "";
					flag = 1; //signifying unary operator
					while(!stack.empty())
					{
						string str = stack.top();
						stack.pop();
						// cout << str << endl;
						if(str == "(")
							break;
						else if(str[0] == 'U' || str[0] == '&' || str[0] == '|')
						{
							flag = 2; //signifying binary operator
							form = str + " " + form;
						}
						else
						{
							if(flag == 1)
								form = str + " " + form;
							else
								form = form + str;
						}
					}
					form = "(" + form + ")";
					stack.push(form);
				}
			}
		}
	}

	flag = 1;
	form = "";
	while(!stack.empty())
	{
		string str = stack.top();
		stack.pop();
		if(str == "(")
			break;
		else if(str[0] == 'U' || str[0] == '&' || str[0] == '|' || str[0] == 'I') //until, and, or, implies
		{
			flag = 2; //signifying binary operator
			form = str + " " + form;
		}
		else
		{
			if(flag == 1)
				form = str + " " + form;
			else
				form = form + str;
		}
	}
	// cout << form << endl;

	root = makeTree(form, "");

	// root = new TreeNode("U");
	// root->left = new TreeNode("a");
	// root->right = new TreeNode("b");
}

struct TreeNode* MakeParseTree::makeTree(string subformula, string prefix)
{
	if(subformula.length() == 1) {
		cout << "Leaf: " << prefix << " " << subformula << endl;
		struct TreeNode* node = new TreeNode(subformula);
		return node;
	}
	else if(subformula.length() == 0)
		return NULL;
	int numBrac = 0, type;
	string subForm1 = "", subForm2 = "";
	char op = subformula[0];
	string Op = subformula.substr(0, subformula.find(' '));
	if(op == 'U' || op == '|' || op == '&' || op == 'I')
		type = 2;
	else
		type = 1;

	for(int i = Op.length() + 1; i < subformula.length(); i++)
	{
		char ch = subformula[i];

		if(ch == '(')
			numBrac++;
		else if(ch == ')')
			numBrac--;

		// cout << ch << " : " << numBrac << " : " << type << endl;
		if(type == 1) {
			subForm1 = subForm1 + ch;
			if(numBrac == 0)
				break;
		}
		else if(type == 2) {
			subForm2 = subForm2 + ch;
			if(numBrac == 0) {
				type--;
				i++;
			}
		}
	}
	// cout << subformula << " : " << Op << " : " << subForm1 << " : " << subForm2 << endl;
	if(subForm1[0] == '(' && subForm1[subForm1.length() - 1] == ')')
		subForm1 = subForm1.substr(1, subForm1.length() - 2);
	if(subForm2[0] == '(' && subForm2[subForm2.length() - 1] == ')')
		subForm2 = subForm2.substr(1, subForm2.length() - 2);
	// cout << subformula << " : " << subForm1 << ", " << subForm2 << endl;

	struct TreeNode* node = new TreeNode(string(1, op));
	if(op == 'U' || op == 'F' || op == 'G')
	{
		node->b1 = stoi(Op.substr(2, Op.find(',') - Op.find('[')));
		node->b2 = stoi(Op.substr(Op.find(',') + 1));
	}
	// cout << node->b1 << " : " << node->b2 << endl;

	if(op == 'U')
	{
		node->right = makeTree(subForm1, prefix + "G");
		node->left = makeTree(subForm2, prefix + "F");
	}
	else if(op == 'G' || op == 'F')
	{
		node->right = makeTree(subForm1, prefix + op);
		node->left = makeTree(subForm2, prefix + op);
	}
	else
	{
		node->right = makeTree(subForm1, prefix);
		node->left = makeTree(subForm2, prefix);
	}
	return node;
}

class makeSMT
{
public:
	makeSMT();
	makeSMT(string, string);
	void solveSMT();

private:
	int numMonitors;
	string formula;
	string traceFileName;
	struct HLCTime
	{
		int timeVal[3];
		HLCTime()
		{
			timeVal[0] = 0;
			timeVal[1] = 0;
			timeVal[2] = 0;
		}
		HLCTime(int l, int m, int c)
		{
			timeVal[0] = l;
			timeVal[1] = m;
			timeVal[2] = c;
		}
	};
	vector<vector<string> > eventValueAll;
	vector<vector<struct HLCTime> > eventTimeAll;
	vector<vector<char> > eventTypeAll;
	int numProcess;
	vector<vector<vector<int> > > eventNumber;
	vector<vector<vector<string> > > eventValueSeg;
	vector<vector<vector<struct HLCTime> > > eventTimeSeg;
	vector<vector<vector<char> > > eventTypeSeg;

	struct hb
	{
		int procNum[2];
		hb()
		{
			procNum[0] = 0;
			procNum[1] = 0;
		}
		hb(int a, int b)
		{
			procNum[0] = a;
			procNum[1] = b;
		}
	};
	vector<struct hb> hbSet;
	void createSMT();
	int readTraceFile();
	void readTraceSeg(int, int, int);
};

makeSMT::makeSMT()
{
	formula = "G((F b) I ((! a) U b))";
	traceFileName = "trace2_20_10";

	numProcess = 0;
}

makeSMT::makeSMT(string a, string b)
{
	formula = a;
	traceFileName = b;

	numProcess = 0;
}

int makeSMT::readTraceFile()
{
	ifstream traceFile("traceFiles/" + traceFileName);
	string line;
	int maxTime = 0;

	while(getline(traceFile, line))
	{
		// cout << line << endl;
		if(line.find("System") != -1 || line.find("\\Process") != -1)
			continue;
		else if(line.find("Process") != -1) {
			eventTypeAll.push_back(vector<char>());
			eventValueAll.push_back(vector<string>());
			eventTimeAll.push_back(vector<struct HLCTime>());
			numProcess++;
		}
		else
		{
			int a = line.find("=");
			a = line.find("=", a + 1);
			a = line.find("\"", a + 1);
			int b = line.find("\"", a + 1);
			int c = line.find("\"", b + 1);
			int d = line.find("\"", c + 1);
			int e = line.find("\"", d + 1);
			int f = line.find("\"", e + 1);
			// cout << numProcess << endl;
			eventTypeAll[numProcess - 1].push_back(line.substr(a + 1, b - a - 1)[0]);
			eventValueAll[numProcess - 1].push_back(line.substr(c + 1, d - c - 1));
			string time = line.substr(e + 1, f - e - 2);
			a = time.find(",");
			b = time.find(",", a + 1);
			maxTime = max(maxTime, stoi(time.substr(1, a - 1)));
			eventTimeAll[numProcess - 1].push_back(HLCTime(stoi(time.substr(1, a - 1)), stoi(time.substr(a + 2, b - a - 2)), stoi(time.substr(b + 2))));
		}
	}
	// cout << "numProcess: " << numProcess << endl;
	// for(int i=0;i<numProcess;i++)
	// {
	// 	cout << "processNum: " << i << " eventNum: " << eventTypeAll[i].size() << endl;
	// 	for(int j=0;j<eventTypeAll[i].size();j++)
	// 	{
	// 		cout << eventTypeAll[i][j] << " : " << eventValueAll[i][j] << endl;
	// 		cout << eventTimeAll[i][j].timeVal[0] << " : " << eventTimeAll[i][j].timeVal[1] << " : " << eventTimeAll[i][j].timeVal[2] << endl;
	// 	}
	// }
	return maxTime;
}

void makeSMT::readTraceSeg(int start, int end, int eps)
{
	start = max(start - eps, 0);
	eventTypeSeg.clear();
	eventValueSeg.clear();
	eventTimeSeg.clear();
	eventNumber.clear();
	// cout << start << " : " << end << endl;
	int num = 0;
	for(int mon = 0; mon < numMonitors; mon++)
	{
		eventTypeSeg.push_back(vector<vector<char> >());
		eventValueSeg.push_back(vector<vector<string> >());
		eventTimeSeg.push_back(vector<vector<struct HLCTime> >());
		eventNumber.push_back(vector<vector<int> >());
		for(int i = 0; i < numProcess; i++)
		{
			eventTypeSeg[mon].push_back(vector<char>());
			eventValueSeg[mon].push_back(vector<string>());
			eventTimeSeg[mon].push_back(vector<struct HLCTime>());
			eventNumber[mon].push_back(vector<int>());
		}
	}

	for(int i = 0; i < numProcess; i++)
	{
		for(int j = 0; j < eventTypeAll[i].size(); j++)
		{
			if(eventTimeAll[i][j].timeVal[0] >= start && eventTimeAll[i][j].timeVal[0] <= end)
			{
				int prob1 = (int)(rand() * 3 + 1); //number of monitors reading this event
				vector<int> arr(numMonitors);
				iota(begin(arr), end(arr), 0);
				for(int k = 0; k < prob1; k++)
				{
					int mon = arr[(int)floor(rand() * arr.size())];
					arr.erase(arr.begin() + mon);
					eventNumber[mon][i].push_back(++num);
					eventTypeSeg[mon][i].push_back(eventTypeAll[i][j]);
					eventValueSeg[mon][i].push_back(eventValueAll[i][j]);
					eventTimeSeg[mon][i].push_back(HLCTime(eventTimeAll[i][j].timeVal[0], eventTimeAll[i][j].timeVal[1], eventTimeAll[i][j].timeVal[2]));
				}
			}
		}
	}

	for(int i = 0; i < numProcess; i++)
	{
		for(int j = 0; j < eventTypeSeg[i].size(); j++)
		{
			if(eventTypeSeg[i][j] == 'R')
			{
				for(int k = 0; k < numProcess; k++)
				{
					for(int l = 0; l < eventTypeSeg[k].size() && k != i; l++)
					{
						if(eventTypeSeg[k][l] == 'S' && eventTimeSeg[k][l].timeVal[0] == stoi(eventValueSeg[i][j].substr(0, eventValueSeg[i][j].find(","))))
						{
							hbSet.push_back(hb(eventNumber[k][l], eventNumber[i][j]));
							// cout << "happenBefore: " << eventNumber[k][l] << " " << eventNumber[i][j] << endl;
						}
					}
				}
			}
			for(int k = 0; k < numProcess; k++)
			{
				for(int l = 0; l < eventTypeSeg[k].size() && k != i; l++)
				{
					if(abs(eventTimeSeg[i][j].timeVal[0] - eventTimeSeg[k][l].timeVal[0]) >= eps)
					{
						hbSet.push_back(hb(eventNumber[k][l], eventNumber[i][j]));
						// cout << "happenBefore: " << eventNumber[k][l] << " " << eventNumber[i][j] << endl;
					}
				}
			}
		}
	}
}

void makeSMT::solveSMT()
{
	int maxTimeTrace = readTraceFile();

	string formula = "G[0,1] (((F[2,3] a) U[4,5] (b U[6,7] c)) & d)";
	MakeParseTree mpt;
	// cout << formula << endl;
	mpt.genTree(formula);

	int segLength = 20, eps = 15, numCores = 2, tid;

	#pragma omp parallel private(tid) num_threads(numCores)
	{
		#pragma omp parallel for
		for(int core = 0; core < numCores; core++)
		{
			tid = omp_get_thread_num();
			for(int startSeg = (maxTimeTrace / numCores) * tid; startSeg < (maxTimeTrace / numCores) * (tid + 1); startSeg += segLength)
			{
				cout << "Thread: " << tid << " ; " << startSeg << " : " << startSeg + segLength << endl;
				// readTraceSeg(startSeg, startSeg + segLength, eps);

				int totNumFormulas = 2;
				for(int numFormula = 0; numFormula < totNumFormulas; numFormula ++)
				{
					// context c;
					// solver s(c);

					// expr_vector eventList(c);

					// int totNumEvents = 0;
					// for(int i = 0; i < numProcess; i++)
					// {
					// 	totNumEvents = totNumEvents + eventTypeSeg[i].size();
					// }

					// for(int i = 0; i < pow(totNumEvents, 2); i++)
				 //    {
				 //        std::string str = "eventList" + std::to_string(i);
				 //        eventList.push_back(c.bv_const(str.c_str(), totNumEvents));
				 //        s.add(eventList[i] == i);
				 //    }

				 //    func_decl rho = z3::function("rho", c.int_sort(), c.bv_sort(totNumEvents));

				 //    s.add(rho(0) == eventList[0]);

				 //    for (int i = 0; i <= totNumEvents; i++)
				 //    {
				 //        expr_vector event_range(c);

				 //        for (int j = 0; j < pow(totNumEvents, 2); j++)
				 //        {
				 //            event_range.push_back(rho(i) == eventList[j]);
				 //        }

				 //        s.add(mk_or(event_range));
				 //    }

				 //    for(int i = 0; i < totNumEvents; i++)
				 //    {
				 //        expr_vector event_order(c);

				 //        for(int j = 1; j < pow(totNumEvents, 2); j = j * 2)
				 //            event_order.push_back(bv2int(f(i + 1), false) - bv2int(f(i), false) == j);

				 //        // s.add(f(i + 1) > f(i));
				 //        s.add(mk_or(event_order));
				 //    }

				 //    s.add(f(totNumEvents) == eventList[(int)pow(totNumEvents, 2) - 1]);

				 //    expr x = c.int_const("x");
				 //    s.add(0 <= x && x <= totNumEvents);

				 //    expr_vector b1(c);
				 //    expr_vector b2(c);
				}
			}
		}
	}
}

int main()
{
	makeSMT smt;
	smt.solveSMT();

	return 1;
}

