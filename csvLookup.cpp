#include <iomanip>
#include <iostream>
#include <fstream>

#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

template <typename Out>
void split(const string &s, char delim, Out result){
	istringstream iss(s);
	string item;
	while (getline(iss, item, delim)){
		*result++ = item;
	}
}

vector<string> split(const string &s, char delim){
	vector<string> elems;
	split(s, delim, back_inserter(elems));
	return elems;
}

int main(int argc, char** argv){
	ifstream cidCsv, wrkspcCsv;
	ofstream outCsv;
	string line, cidFName, wrkspcFName, outFName, cidSearch;
	vector<string> v1(argv + 1, argv + argc), v2, v3;
	map<string, vector<string>> cidM;
	map<int, int> dTest;

	int fColumn = 0, fRow = 0, fLoc = 0, cidIndex = 0, wrkIndex = 0;
	bool debug = false, fLocSpec = false, fLocSearch = false;

	for(auto i = v1.begin(); i != v1.end(); i++){
		if(*i == "-h" || *i == "--help"){
			cout << "Syntax: -p <primaryTable.csv> -l <LookUpTable.csv> -o <outputTable.csv> -d <debug> -f <pre post inline>\n";
			return 0;
		}
		else if(*i == "-p"){
			wrkspcFName = *++i;
			wrkspcCsv.open(wrkspcFName);
			if(wrkspcCsv.fail()){
				cout << "File: " << wrkspcFName << " has set failbit andor badbit flags\n";
				return 1;
			} 
		}
		else if(*i == "-l"){
			cidFName = *++i;
			cidCsv.open(cidFName);
			if(cidCsv.fail()){
				cout << "File: " << cidFName << " has set failbit andor badbit flags\n";
				return 1;
			}
		}
		else if(*i == "-o"){
			outFName = *++i;
			outCsv.open(outFName);
		}
		else if(*i == "-d"){
			debug = true;
		}
		else if(*i == "-f"){
			for(auto j = i; j != v1.end(); j++){
				if(*j == "inline")
					fLocSearch = true;
				else if(fLocSpec == false){
					if(*j == "pre"){
						fLoc = -1;
						fLocSpec = true;
					}
					else if(*j == "post"){
						fLoc = -2;
						fLocSpec = true;
					}
				}

			}
		}
	}
	
	getline(cidCsv, line);
	line.erase(remove(line.begin(), line.end(), '\n'), line.end());
	line.erase(remove(line.begin(), line.end(), '\r'), line.end());
	v1 = split(line, ',');

	getline(wrkspcCsv, line);
	line.erase(remove(line.begin(), line.end(), '\n'), line.end());
	line.erase(remove(line.begin(), line.end(), '\r'), line.end());
	v2 = split(line, ',');

	fRow = v2.size();

	v3.resize(v1.size() + v2.size());
	for(auto i = v1.begin(); i != v1.end(); i++)
    	if(find(v2.begin(), v2.end(), *i) != v2.end()) v3.push_back(*i);
    
	v3.erase(remove(v3.begin(), v3.end(), ""), v3.end());
	if(fLocSearch == true) for(const auto &i : v3) dTest.insert(pair<int, int>(distance(v1.begin(), find(v1.begin(), v1.end(), i)), distance(v2.begin(), find(v2.begin(), v2.end(), i))));

	if(fLocSpec == true) for(const auto &i :v1) dTest.insert(pair<int, int>(distance(v1.begin(), find(v1.begin(), v1.end(), i)), fLoc));

	getline(cidCsv, line);
	v1 = split(line, ',');

	getline(wrkspcCsv, line);
	v2 = split(line, ',');

	for(const auto &i : dTest){
		if(v2.at(i.second) != ""){
			cidIndex = i.first;
			wrkIndex = i.second;
			dTest.erase(i.first);
		}
	}

	cidCsv.clear();
	cidCsv.seekg(0);
	wrkspcCsv.clear();
	wrkspcCsv.seekg(0);

	while(getline(cidCsv, line)){
		fColumn++;

		line.erase(remove(line.begin(), line.end(), '\n'), line.end());
		line.erase(remove(line.begin(), line.end(), '\r'), line.end());

		vector<string> curCidData = split(line, ',');

		string cidKey = curCidData.at(cidIndex);

		try{
			cidM.insert(pair<string, vector<string>>(cidKey, curCidData));
		}
		catch(exception& e){
			if(debug == true) cout << "Check line " << fColumn << " in " << cidFName << " "  << e.what() << "\n";
		}		
	}

	cidCsv.close();

	fColumn = 0;

	while(getline(wrkspcCsv, line)){
		fColumn++;

		line.erase(remove(line.begin(), line.end(), '\n'), line.end());
		line.erase(remove(line.begin(), line.end(), '\r'), line.end());

		vector<string> curWrkspcData = split(line, ',');

		map<string, vector<string>>::iterator cidIt = cidM.find(curWrkspcData.at(wrkIndex));

		try{
			for(const auto &i : dTest){
				if(i.second >= 0)
					curWrkspcData[i.second] = cidIt->second[i.first];
				else if(i.second == -1)
					curWrkspcData.insert(curWrkspcData.begin(), cidIt->second.at(i.first));
				else if(i.second == -2){
					curWrkspcData.resize(fRow);
					curWrkspcData.push_back(cidIt->second.at(i.first));					
				}

			}
		}
		catch(exception& e){
			if(debug == true) cout << "Check line " << fColumn << " in " << wrkspcFName << " "  << e.what() << "\n";
		}
		for(const auto &i : curWrkspcData) outCsv << i << ",";
		outCsv << "\n";
	}
}