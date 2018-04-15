#include <iostream>
#include <set>
#include <map>
#include <vector>
#include "Apriori.h"
#include "FpGrowth.h"
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::set;
using std::pair;

void loadTestIntDataSet(set<set<int>>& dataSet) {
	const int COLUMNS = 10;
	const int DATA_ARR[][COLUMNS] = { { 1, 3, 4 },{ 2, 3, 5 },{ 1, 2, 3, 5 },{ 2, 5 } };
	const int ROWS = sizeof(DATA_ARR) / sizeof(*DATA_ARR);
	//cout << row << endl;
	for (int i = 0; i < ROWS; i++) {
		set<int> s;
		for (int j = 0; j < COLUMNS; j++) {
			if (DATA_ARR[i][j] != 0) {
				s.insert(DATA_ARR[i][j]);
			}
		}
		dataSet.insert(s);
	}
}

template<class T>
void printResults(const set<set<T>>& dataSet, const float& minSupp,
	const float& minConf, const vector<vector<set<T>>>& res,
	const map<set<T>, float>& suppData, const map <pair<set<T>, set<T>>, float>& brl) {

	// print min supp & min conf
	cout << "min supp: " << minSupp
		<< " |  min conf: " << minConf << "\n";

	// print data set
	cout << "data set: {";
	for (auto trans : dataSet) {
		cout << "{";
		for (auto item : trans) {
			cout << item << " ";
		}
		cout << "}";
	}

	cout << "}\n----------------------------------" << endl;

	// print freq sets
	cout << "frequent sets:" << endl;
	for (int i = 0; i < res.size(); i++) {
		cout << "list " << i + 1 << " [";
		for (auto set : res[i]) {
			cout << "{";
			for (auto item : set) {
				cout << item << " ";
			}
			cout << "}";
		cout << " : " << suppData.at(set) << ", ";
		}
		cout << "]" << endl;
	}

	cout << "----------------------------------" << endl;

	// print big rules
	cout << "big rules:" << endl;
	for (auto pair : brl) {
		cout << "(";
		cout << "{";
		for (auto item : pair.first.first) {
			cout << item << " ";
		}
		cout << "}";
		cout << " --> ";
		cout << "{";
		for (auto item : pair.first.second) {
			cout << item << " ";
		}
		cout << "}";
		cout << ": conf = " << brl.at(pair.first) << ")" << endl;
	}

	cout << "----------------------------------" << endl;
}

void loadTestCharDataSet(set<set<char>>& dataSet) {
	const int COLUMNS = 10;
	char data[][COLUMNS] = {
		{ 'r', 'z', 'h', 'j', 'p' },
		{ 'z', 'y', 'x', 'w', 'v', 'u', 't', 's' },
		{ 'z' },
		{ 'r', 'x', 'n', 'o', 's' },
		{ 'y', 'r', 'x', 'z', 'q', 't', 'p' },
		{ 'y', 'z', 'x', 'e', 'q', 's', 't', 'm' }
	};
	const int ROWS = sizeof(data) / sizeof(*data);
	for (int i = 0; i < ROWS; i++) {
		std::set<char> s;
		for (int j = 0; j < COLUMNS; j++) {
			if ('a' <= data[i][j] && data[i][j] <= 'z') {
				s.insert(data[i][j]);
			}
		}
		dataSet.insert(s);
	}
}

template<class T>
void printResults(const set<set<T>>& dataSet, const int& minSuppCnt, const vector<set<T>>& freqSets) {
	// print min supp cnt
	cout << "min supp cnt: " << minSuppCnt << endl;

	// print data set
	cout << "data set: {";
	for (auto trans : dataSet) {
		cout << "{";
		for (auto item : trans) {
			cout << item << " ";
		}
		cout << "}";
	}

	cout << "}\n----------------------------------" << endl;

	// print freq sets
	cout << "freq sets : [\n";
	for (auto set : freqSets) {
		cout << "{";
		for (auto item : set) {
			cout << item << ", ";
		}
		cout << "}, ";
		cout << endl;
	}
	cout << "]" << endl;
}
void aprioriTest() {

	const float MIN_SUPPORT = 0.5f;
	const float MIN_CONFIDENCE = 0.5f;

	// -----------------------------------
	// 0.You need these objects to recv mining results
	vector<vector<set<int>>> res;
	map<set<int>, float> suppData;
	map <pair<set<int>, set<int>>, float> brl;
	
	// -----------------------------------
	// 1.Load data 
	set<set<int>> dataSet;
	loadTestIntDataSet(dataSet);

	// ------------------------------------
	// 2.Init an Aprioiri instance
	Apriori<int> apr(MIN_SUPPORT, MIN_CONFIDENCE, dataSet);

	// ------------------------------------
	// 3.Mining freq sets
	apr.freqSetsMining(res, suppData);

	// ------------------------------------
	// 4.Mining rules
	apr.rulesMining(res, suppData, brl);

	// ------------------------------------
	// 5.Print results
	cout << ">>> apriori" << endl;
	printResults<int>(dataSet, MIN_SUPPORT, MIN_CONFIDENCE, res, suppData, brl);
}

void fpGrowthTest() {
	const int MIN_SUPP_COUNT = 2;

	// -----------------------------------
	// 0.You need it to recv mining results
	vector<set<int>> freqSets;

	// -----------------------------------
	// 1.Load data 
	set<set<int>> dataSet;
	loadTestIntDataSet(dataSet);

	// ------------------------------------
	// 2.Init an Aprioiri instance
	FpGrowth<int> fp(MIN_SUPP_COUNT, dataSet);
	
	// ------------------------------------
	// 3.Mining freq sets
	fp.freqSetsMining(freqSets);

	// ------------------------------------
	// *4. Use Apriori's member func to gen rules
	vector<vector<set<int>>> freqSetsList;
	map<set<int>, float> suppData;
	map <pair<set<int>, set<int>>, float> brl;
	// Implemented by calling Apriori's member function
	fp.rulesMining(0.5f, freqSets, freqSetsList, suppData, brl);
	
	// ------------------------------------
	// 5.Print results
	cout << ">>> fp-growth" << endl;
	printResults<int>(dataSet, (float)MIN_SUPP_COUNT / dataSet.size(), 0.5f, freqSetsList, suppData, brl);

}

int main() {
	aprioriTest();
	fpGrowthTest();
	return 0;
}
