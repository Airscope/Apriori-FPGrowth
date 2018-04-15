#ifndef _FPGROWTH_H_
#define _FPGROWTH_H_
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include "Apriori.h"
using std::set;
using std::vector;
using std::map;
using std::pair;

template<class T>
class TreeNode {
public:
	T value;
	int count;
	TreeNode<T>* parent;
	TreeNode<T>* link;
	map<T, TreeNode<T>*> children;
public:
	TreeNode(T value, int count, TreeNode<T>* parent);
	void inc(int numOccur);
};

template<class T>
TreeNode<T>::TreeNode(T value, int count, TreeNode<T>* parent) {
	this->value = value;
	this->count = count;
	this->parent = parent;
	this->link = NULL;
}

template<class T>
void TreeNode<T>::inc(int numOccur) {
	this->count += numOccur;
}

template<class T>
class FpGrowth {
public:
	int minSuppCnt;
	set<set<T> > dataSet;
private:
	vector<TreeNode<T>*> _garbageCollector;
public:
	FpGrowth(int minSuppCnt, const set<set<T>>& dataSet);
	void freqSetsMining(vector<set<T>>& freqSetsList);
	void rulesMining(const float& inMinConf, const vector<set<T>>& inFreqSets, 
			vector<vector<set<T>>>& outFreqSetsList, map<set<T>, float>& outSuppData, 
			map<pair<set<T>, set<T>>, float>& outBigRulesList);
private:
	void _dataSet2Map(map<set<T>, int>& dataMap);
	TreeNode<T>* _createTree(map<set<T>, int>& dataMap, map<T, pair<int, TreeNode<T>*>>& headerTable);
	void _updateTree(const vector<T>& items, const int& itemsStartIndex, TreeNode<T>* root, 
			map<T, pair<int, TreeNode<T>*> >& headerTable, const int& transCnt);
	void _ascendTree(TreeNode<T>* leafNode, vector<T>& prefixPath);
	void _findPrefixPath(const T& basePattern, TreeNode<T>* inTreeNode, map<set<T>, int>& outCondPattern);
	void _mineTree(TreeNode<T>* inTree, map<T, pair<int, TreeNode<T>*> >& headerTable, 
			const set<T>& prefix, vector<set<T> >& freqItemSetsList);
	void _collect(TreeNode<T>* root);
	void _free(TreeNode<T>* root);
	void _distroy();
};


template<class T>
FpGrowth<T>::FpGrowth(int minSuppCnt, const set<set<T>>& dataSet) {
	this->minSuppCnt = minSuppCnt;
	this->dataSet = dataSet;
}

template<class T>
void FpGrowth<T>::freqSetsMining(vector<set<T>>& freqSetsList) {
	map<set<T>, int> dataMap;
	_dataSet2Map(dataMap);
	map<T, pair<int, TreeNode<T>*>> headerTable;
	TreeNode<T>* fpTreeRoot = _createTree(dataMap, headerTable);
	_mineTree(fpTreeRoot, headerTable, set<T>(), freqSetsList);
	_distroy();
}

// call Apriori's member func to mine rules
template<class T>
void FpGrowth<T>::rulesMining(const float& inMinConf, const vector<set<T>>& inFreqSets, 
		vector<vector<set<T>>>& outFreqSetsList, map<set<T>, float>& outSuppData, 
		map<pair<set<T>, set<T>>, float>& outBigRulesList) {
	// gen FreqSetsList
	int maxSetLen = 0;
	for (auto set : inFreqSets) {
		if (set.size() > maxSetLen)
			maxSetLen = set.size();
	}
	for (int i = 0; i < maxSetLen; i++) {
		outFreqSetsList.push_back(vector<set<T>>());
	}
	for (auto set : inFreqSets) {
		outFreqSetsList[set.size() - 1].push_back(set);
	}
	
	// gen suppData
	for (auto transaction : this->dataSet) {
		for (auto set : inFreqSets) {
			if (std::includes(transaction.begin(), transaction.end(), set.begin(), set.end())) {
				if (outSuppData.find(set) == outSuppData.end())
					outSuppData[set] = 1.0f;
				else {
					outSuppData[set] += 1.0f;
				}
			}
		}
	}

	for (auto pair : outSuppData) {
		outSuppData[pair.first] = pair.second / (this->dataSet.size());
	}


	Apriori<T>* apr = new Apriori<T>((float)minSuppCnt / dataSet.size(), inMinConf, this->dataSet);
	apr->rulesMining(outFreqSetsList, outSuppData, outBigRulesList);
	delete apr;
}

template<class T>
void FpGrowth<T>::_dataSet2Map(map<set<T>, int>& dataMap) {
	for (auto trans : this->dataSet) {
		if (dataMap.find(trans) == dataMap.end()) {
			dataMap[trans] = 1;
		}
		else {
			dataMap[trans] ++;
		}
	}
}

template<class T>
TreeNode<T>* FpGrowth<T>::_createTree(map<set<T>, int>& dataMap, map<T, pair<int, TreeNode<T>*>>& headerTable) {
	// calculate and prune every item's freq count
	// outHeaderTabel : {item => (freq_count, linknode)}
	for (auto transactionPair : dataMap) {
		for (auto item : transactionPair.first) {
			if (headerTable.find(item) == headerTable.end()) {
				headerTable[item] = pair<int, TreeNode<T>*>(transactionPair.second, NULL);
			}
			else {
				headerTable[item].first += transactionPair.second;
			}
		}
	}
	set<T> freqItemSet; // {freq_item, ...}
	vector<T> eraseKeys;
	for (auto pair : headerTable) {
		T key = pair.first;
		if (headerTable[key].first < this->minSuppCnt) {
			eraseKeys.push_back(key);
		}
	}
	for (auto item : eraseKeys) {
		headerTable.erase(item);
	}
	for (auto pair : headerTable) {
		freqItemSet.insert(pair.first);
	}
	if (freqItemSet.size() == 0) {
		return NULL;
	}
	TreeNode<T>* retTree = new TreeNode<T>(T(), -1, NULL);
	this->_collect(retTree);

	for (auto transactionPair : dataMap) {
		map<T, int> local;
		vector<T> orderedItems;
		for (auto item : transactionPair.first) {
			if (freqItemSet.find(item) != freqItemSet.end()) {
				local[item] = headerTable[item].first;
				orderedItems.push_back(item);
			}
		}
		if (orderedItems.size() > 0) {
			std::sort(orderedItems.begin(), orderedItems.end(), [&headerTable](const T& x, const T& y) {
				return headerTable[x].first > headerTable[y].first;
			});
			_updateTree(orderedItems, 0, retTree, headerTable, transactionPair.second);
		}
	}
	return retTree;
}

template<class T>
void FpGrowth<T>::_updateTree(const vector<T>& items, const int & itemsStartIndex, TreeNode<T>* root, map<T, pair<int, TreeNode<T>*>>& headerTable, const int & transCnt) {
	T curItem = items[itemsStartIndex];
	auto findRes = (root->children).find(curItem);
	if (findRes != (root->children).end()) { // in it
		findRes->second->inc(transCnt);
	}
	else {
		root->children[curItem] = new TreeNode<T>(curItem, transCnt, root);
		TreeNode<T>* next = root->children[curItem];
		if (headerTable[curItem].second == NULL) { // link head to first
			headerTable[curItem].second = next;
		}
		else {
			// link all same items
			TreeNode<T>* cur = headerTable[curItem].second; // begin
			while (cur->link != NULL) {
				cur = cur->link;
			}
			cur->link = next;
		}
	}
	if (itemsStartIndex < items.size() - 1) {
		_updateTree(items, itemsStartIndex + 1, root->children[curItem], headerTable, transCnt);
	}
}

template<class T>
void FpGrowth<T>::_ascendTree(TreeNode<T>* leafNode, vector<T>& prefixPath) {
	TreeNode<T>* cur = leafNode;
	while (cur->parent != NULL){
		prefixPath.push_back(cur->value);
		cur = cur->parent;
	}
}

template<class T>
void FpGrowth<T>::_findPrefixPath(const T & basePattern, TreeNode<T>* inTreeNode, map<set<T>, int>& outCondPattern) {
	while (inTreeNode != NULL) {
		vector<T> prefixPath;
		_ascendTree(inTreeNode, prefixPath);
		if (prefixPath.size() > 1) {
			set<T> keySet;
			for (int i = 1; i < prefixPath.size(); i++)
				keySet.insert(prefixPath[i]);
			outCondPattern[keySet] = inTreeNode->count;
		}
		inTreeNode = inTreeNode->link;
	}
}

template<class T>
void FpGrowth<T>::_mineTree(TreeNode<T>* inTree, map<T, pair<int, TreeNode<T>*>>& headerTable, const set<T>& prefix, vector<set<T>>& freqItemSetsList) {
	vector<T> bigl;
	for (auto pair : headerTable) {
		bigl.push_back(pair.first);
	}
	std::sort(bigl.begin(), bigl.end(), [&headerTable](const T& x, const T& y)->bool {
		return (headerTable[x].first < headerTable[y].first);
	});
	for (auto basePattern : bigl) {
		set<T> newFreqSet = prefix;
		newFreqSet.insert(basePattern);
		newFreqSet.insert(basePattern);
		freqItemSetsList.push_back(newFreqSet);
		map<set<T>, int> condPatternBases;
		_findPrefixPath(basePattern, headerTable[basePattern].second, condPatternBases);
		map<T, pair<int, TreeNode<T>*> > head;
		TreeNode<T>* condTree = _createTree(condPatternBases, head);
		if (head.size() != 0) {
			_mineTree(condTree, head, newFreqSet, freqItemSetsList);
		}
	}
}

template<class T>
void FpGrowth<T>::_collect(TreeNode<T>* root) {
	this->_garbageCollector.push_back(root);
}

template<class T>
inline void FpGrowth<T>::_free(TreeNode<T>* root) {
	if (root->children.size() == 0) {
		delete root;
		return;
	}
	else {
		for (auto pair : root->children) {
			_free(pair.second);
		}
	}
}

template<class T>
void FpGrowth<T>::_distroy() {
	for (int i = 0; i < this->_garbageCollector.size(); i++) {
		_free(this->_garbageCollector[i]);
	}
}


#endif // _FPGROWTH_H_