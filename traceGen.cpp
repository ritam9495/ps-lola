#include<iostream>
#include<fstream>
// #include<stdio>

using namespace std;

void generateTrace(string fileName, int numProcess, int traceLength)
{
	ofstream myFile;
	myFile.open(fileName);
	for(int i = 0; i < numProcess; i++)
	{
		int time = 0, value = 0;
		myFile << "Process " << (i+1) << endl;
		for(int j = 0; j < traceLength; j++)
		{
			time = time + rand() % 2 + 1;
			value = rand() % 10 + 1;
			myFile << time << ", " << value << endl;
		}
	}
	myFile.close();
}

int main()
{
	generateTrace("sampleTrace.txt", 2, 10);
}