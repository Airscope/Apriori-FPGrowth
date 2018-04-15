#ifndef _APRIORI_H_
#define _APRIORI_H_
#include <set>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
using std::set;
using std::vector;
using std::map;
using std::pair;

template<class T>
class Apriori {
public:
	float minSupp;
	float minConf;
	set<set<T> > dataSet;
public:
	Apriori(float minSupp, float minConf, const set<set<T> >& dataSet);
	void freqSetsMining(vector<vector<set<T> > >& resList, map<set<T>, float>& suppData);
	void rulesMining(const vector<vector<set<T> > >& inResList, const map<set<T>, float>& inSuppData, map<pair<set<T>, set<T> >, float>& outBigRulesList);
private:
	void _create1stCandidateSet(set<set<T> >& candidate1st);
	void _scanDataSet(const set<set<T> >& inCandidateKth, vector<set<T> >& outResList, map<set<T>, float>& outSuppData);
	void _aprioriGen(const vector<set<T> >& inListKth, const int& k, set<set<T> >& outCandidateKplus1th);
	bool _unionSetFilter(const set<T>& unionSet, const vector<set<T>>& list);
	void _rulesFromConsequence(const set<T>& freqSet, const vector<set<T>>& h, const map<set<T>, float>& suppData, map<pair<set<T>, set<T>>, float>& brl);
	vector<set<T>> _calculateConf(const set<T>& freqSet, const vector<set<T>>& h, const map<set<T>, float>& suppData, map<pair<set<T>, set<T>>, float>& brl);
};


template<class T>
Apriori<T>::Apriori(float minSupp, float minConf, const set<set<T> >& dataSet) {
	this->minSupp = minSupp;
	this->minConf = minConf;
	this->dataSet = dataSet;
}

template<class T>
void Apriori<T>::freqSetsMining(vector<vector<set<T> > >& resList, map<set<T>, float>& suppData) {
	set<set<T> > candidate1st;
	_create1stCandidateSet(candidate1st);
	vector<set<T> > list1st;
	map<set<T>, float> localSuppData;
	_scanDataSet(candidate1st, list1st, localSuppData);
	resList.push_back(list1st);
	int k = 2;
	while (resList[k - 2].size() > 0) {
		set<set<T> > candidateKth;
		_aprioriGen(resList[k - 2], k, candidateKth);
		vector<set<T> > listKth;
		_scanDataSet(candidateKth, listKth, localSuppData);
		for (auto pair : localSuppData) {
			suppData[pair.first] = pair.second;
		}
		resList.push_back(listKth);
		k++;
	}
}

template<class T>
void Apriori<T>::rulesMining(const vector<vector<set<T>>>& inResList, const map<set<T>, float>& inSuppData, map<pair<set<T>, set<T>>, float>& outBigRulesList) {
	for (int i = 1; i < inResList.size(); i++) {
		for (auto freqSet : inResList[i]) {
			vector<set<T>> h1;
			for (auto item : freqSet) {
				set<T> tempSet;
				tempSet.insert(item);
				h1.push_back(tempSet);
			}
			if (i > 1) {
				_rulesFromConsequence(freqSet, h1, inSuppData, outBigRulesList);
			}
			else {
				_calculateConf(freqSet, h1, inSuppData, outBigRulesList);
			}
		}
	}
}

template<class T>
void Apriori<T>::_create1stCandidateSet(set<set<T> >& candidate1st) {
	for (auto transaction : this->dataSet) {
		for (auto item : transaction) {
			set<T> tempSet;
			tempSet.insert(item);
			if (candidate1st.find(tempSet) == candidate1st.end()) // not in outCandidate1st
				candidate1st.insert(tempSet);
		}
	}
}

template<class T>
void Apriori<T>::_scanDataSet(const set<set<T>>& inCandidateKth, vector<set<T>>& outResList, map<set<T>, float>& outSuppData) {
	map<set<T>, int> setSuppCnt;
	for (auto transaction : this->dataSet) {
		for (auto candidate : inCandidateKth) {
			if (std::includes(transaction.begin(), transaction.end(), candidate.begin(), candidate.end())) {
				// candidate is subset of transaction
				if (setSuppCnt.find(candidate) == setSuppCnt.end())
					setSuppCnt[candidate] = 1;
				else
					setSuppCnt[candidate] += 1;
			}
		}
	}
	float transcationsNum = (float)(this->dataSet.size());
	for (auto pair : setSuppCnt) {
		set<T> key = pair.first;
		float supp = setSuppCnt[key] / transcationsNum;
		if (supp >= this->minSupp)
			outResList.push_back(key);
		outSuppData[key] = supp;
	}
}

template<class T>
void Apriori<T>::_aprioriGen(const vector<set<T>>& inListKth, const int& k, set<set<T> >& outCandidateKplus1th) {
	int listKthLen = inListKth.size();
	for (int i = 0; i < listKthLen; i++) {
		for (int j = i + 1; j < listKthLen; j++) {
			vector<T> temp1;
			vector<T> temp2;
			std::copy(inListKth[i].begin(), inListKth[i].end(), std::back_insert_iterator<vector<T> >(temp1));
			std::copy(inListKth[j].begin(), inListKth[j].end(), std::back_insert_iterator<vector<T> >(temp2));
			temp1.pop_back();
			temp2.pop_back();
			std::sort(temp1.begin(), temp1.end());
			std::sort(temp2.begin(), temp2.end());
			if (temp1 == temp2) {
				set<T> unionSet;
				for (auto item1 : inListKth[i]) {
					unionSet.insert(item1);
				}
				for (auto item2 : inListKth[j]) {
					unionSet.insert(item2);
				}
				if (_unionSetFilter(unionSet, inListKth))
					outCandidateKplus1th.insert(unionSet);
			}
		}
	}
}

template<class T>
bool Apriori<T>::_unionSetFilter(const set<T>& unionSet, const vector<set<T>>& list) {
	for (auto item : unionSet) {
		set<T> temp = unionSet;
		temp.erase(item);
		bool in = false;
		for (int i = 0; i < list.size(); i++) {
			if (list[i] == temp)
				in = true;
		}
		if (!in)
			return false;
	}
	return true;
}

template<class T>
void Apriori<T>::_rulesFromConsequence(const set<T>& freqSet, const vector<set<T>>& h, const map<set<T>, float>& suppData, map<pair<set<T>, set<T>>, float>& brl) {
	int m = h[0].size();
	if (freqSet.size() > m + 1) {
		set<set<T>> tempGenSet;
		_aprioriGen(h, m + 1, tempGenSet);
		vector<set<T>> hmplus1;
		std::copy(tempGenSet.begin(), tempGenSet.end(), std::back_inserter(hmplus1));
		hmplus1 = _calculateConf(freqSet, hmplus1, suppData, brl);
		if (hmplus1.size() > 1)
			_rulesFromConsequence(freqSet, hmplus1, suppData, brl);
	}

}

template<class T>
vector<set<T>> Apriori<T>::_calculateConf(const set<T>& freqSet, const vector<set<T>>& h, const map<set<T>, float>& suppData, map<pair<set<T>, set<T>>, float>& brl) {
	vector<set<T>> outPrunedH;
	for (auto consequece : h) {
		set<T> diffSet;
		std::set_difference(freqSet.begin(), freqSet.end(), consequece.begin(), consequece.end(), std::inserter(diffSet, diffSet.begin()));
		float conf = suppData.at(freqSet) / suppData.at(diffSet);
		if (conf >= this->minConf) {
			pair<set<T>, set<T>> key(diffSet, consequece);
			brl[key] = conf;
			outPrunedH.push_back(consequece);
		}
	}
	return outPrunedH;
}

#endif // !_APRIORI_H_
