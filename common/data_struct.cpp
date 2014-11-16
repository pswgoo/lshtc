#include "common_basic.h"
#include "data_struct.h"
using namespace std;

int Instance::SaveInstances(const char* const fileName, vector<Instance> &instances)
{
	FILE *outFile = fopen(fileName, "wb");
	for (unsigned i = 0; i < instances.size(); ++i)
	{
		instances[i].Save(outFile);
	}
	fclose(outFile);
	return 0;
}

int Instance::LoadInstances(const char* const fileName, vector<Instance> &instances)
{
	FileBuffer buffer(fileName);
	instances.clear();

	Instance tmpInstance;
	while (tmpInstance.Load(buffer) == 0)
	{
		instances.push_back(tmpInstance);
	}
	return 0;
}

Instance::Instance()
{
	mFeatures.clear();
	mLabels.clear();
}

Instance::~Instance()
{

}

int Instance::Load(FileBuffer &buffer)
{
	if (buffer.Eof())
		return -1;

	mFeatures.clear();
	mLabels.clear();

	int numFeature;
	buffer.GetNextData(numFeature);
	if (buffer.Eof())
		return -1;

	for (int i = 0; i < numFeature; ++i)
	{
		if (buffer.Eof())
		{
			cerr << "Error: Load Instance error,FileBuffer empty unexpected!" << endl;
			return -1;
		}
		int key;
		double value;
		buffer.GetNextData(key);
		buffer.GetNextData(value);
		mFeatures[key] = value;
	}

	int numLabel;
	buffer.GetNextData(numLabel);
	for (int i = 0; i < numLabel; ++i)
	{
		if (buffer.Eof())
		{
			cerr << "Error: Load Instance error,FileBuffer empty unexpected!" << endl;
			return -1;
		}
		int labelId;
		buffer.GetNextData(labelId);
		mLabels.insert(labelId);
	}
	return 0;
}

int Instance::Save(FILE *outFile)
{
	if (outFile == NULL)
	{
		cerr << "Error: Load inFile is NULL in Instance's Load()!" << endl;
		return -1;
	}

	Write(outFile, (int)mFeatures.size());
	for (Feature::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
	{
		Write(outFile, it->first);
		Write(outFile, it->second);
	}

	Write(outFile, (int)mLabels.size());
	for (set<int>::iterator it = mLabels.begin(); it != mLabels.end(); ++it)
		Write(outFile, *it);
	return 0;
}

int Instance::PrintText(FILE* outFile)
{
	if (outFile == NULL)
	{
		cerr << "Error: outFile is NULL in PrintText() of Instance!" << endl;
		return -1;
	}
	for (Feature::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
	{
		if (it != mFeatures.begin())
			fprintf(outFile, " ");
		fprintf(outFile, "%d:%lf", it->first, it->second);
	}
	fprintf(outFile, "\n");
	for (set<int>::iterator it = mLabels.begin(); it != mLabels.end(); ++it)
	{
		if (it != mLabels.begin())
			fprintf(outFile, " ");
		fprintf(outFile, "%d", *it);
	}
	return 0;
}

FeatureSet::FeatureSet()
{
	mMaxIndex = 0;
}

FeatureSet::~FeatureSet()
{

}

FeatureSet::FeatureSet(int maxIndex)
{
	mMaxIndex = maxIndex;
}

Feature& FeatureSet::operator[](int idx)
{
	return mFeatures[idx];
}

int FeatureSet::Size() const
{
	return (int)mFeatures.size();
}

int FeatureSet::AddInstance(Feature &feature)
{
	mFeatures.push_back(feature);

	Feature::reverse_iterator rit = feature.rbegin();
	if (mMaxIndex < rit->first)
		mMaxIndex = rit->first;
	return 0;
}

int FeatureSet::Merge(FeatureSet& anotherFeatureSet)
{
	if (Size() != anotherFeatureSet.Size())
	{
		cerr << "Error: the sizes of two FeatureSets are not equal.Can't merge!" << endl;
		return -1;
	}

	for (int i = 0; i < Size(); ++i)
	{
		for (Feature::iterator it = anotherFeatureSet.mFeatures[i].begin(); it != anotherFeatureSet.mFeatures[i].end(); ++it)
		{
			mFeatures[i][mMaxIndex + 1 + it->first] = it->second;
		}
	}
	mMaxIndex += anotherFeatureSet.mMaxIndex + 1;

	return 0;
}

int FeatureSet::Normalize(Feature& feature)
{
	double squareLength = 0;
	for (map<int, double>::iterator it = feature.begin(); it != feature.end(); it++)
	{
		squareLength += (it->second) * (it->second);
	}
	double length = sqrt(squareLength);
	for (map<int, double>::iterator it = feature.begin(); it != feature.end(); it++)
	{
		it->second /= length;
	}
	return 0;
}

int FeatureSet::Normalize()
{
	int rtn = 0;
	for (int i = 0; i < mFeatures.size(); i++)
	{
		rtn = Normalize(mFeatures[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int FeatureSet::UpdateMaxIndex()
{
	mMaxIndex = 0;
	for (size_t i = 0; i < mFeatures.size(); ++i)
		mMaxIndex = max(mMaxIndex, mFeatures[i].rbegin()->first);
	return 0;
}

int FeatureSet::SaveFeature(const string& fileName, vector<string>* vecInfo)
{
	FILE* outFile = fopen(fileName.c_str(), "w");
	if (outFile == NULL)
		return -1;
	for (size_t i = 0; i < mFeatures.size(); ++i)
	{
		for (Feature::iterator it = mFeatures[i].begin(); it != mFeatures[i].end(); ++it)
		{
			fprintf(outFile, "%d:%lf ", it->first, it->second);
		}
		if (vecInfo != NULL)
			fprintf(outFile, "# %s\n", (*vecInfo)[i].c_str());
		else
			fprintf(outFile, "\n");
	}
	fclose(outFile);
	return 0;
}

int FeatureSet::SaveSvmFormat(const std::vector<double>& labels, std::string fileName, std::vector<std::string>* vecInfo)
{
	if (labels.size() != mFeatures.size())
		return -1;
	FILE* outFile = fopen(fileName.c_str(), "w");
	if (outFile == NULL)
		return -1;
	for (size_t i = 0; i < mFeatures.size(); ++i)
	{
		fprintf(outFile, "%lf", labels[i]);
		for (Feature::iterator it = mFeatures[i].begin(); it != mFeatures[i].end(); ++it)
		{
			fprintf(outFile, " %d:%lf", it->first, it->second);
		}
		if (vecInfo != NULL)
			fprintf(outFile, " # %s\n", (*vecInfo)[i].c_str());
		else
			fprintf(outFile, "\n");
	}
	fclose(outFile);
	return 0;
}

int FeatureSet::SaveFeature(Feature& feature, const char* const fileName)
{
	FILE* outFile = fopen(fileName, "wb");
	int rtn = Write(outFile, (int)feature.size());
	CHECK_RTN(rtn);
	for (Feature::iterator it = feature.begin(); it != feature.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, it->second);
		CHECK_RTN(rtn);
	}
	fclose(outFile);
	return 0;
}

int FeatureSet::LoadFeature(const char* const fileName, Feature& feature)
{
	feature.clear();
	FileBuffer buffer(fileName);
	int rtn = 0;
	int len;
	rtn = buffer.GetNextData(len);
	CHECK_RTN(rtn);
	for (int i = 0; i < len; ++i)
	{
		int key;
		double value;
		rtn = buffer.GetNextData(key);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(value);
		CHECK_RTN(rtn);
		feature[key] = value;
	}
	return 0;
}

int FeatureSet::SaveMatrix(FILE *OutFile, vector<Feature>& features)
{
	/*Save features Matrix to text file that can read by matlab*/
	if (OutFile == NULL)
		return -1;
	Feature::iterator it;
	for (size_t i = 0; i < features.size(); ++i)
	{
		for (it = features[i].begin(); it != features[i].end(); ++it)
		{
			fprintf(OutFile, "%d %d %lf\n", i + 1, it->first, it->second);
		}
	}
	return 0;
}

int FeatureSet::SaveMatrix(const char* const fileName, vector<Feature>& features)
{
	if (fileName == NULL)
		return -1;
	FILE *fp = fopen(fileName, "w");
	if (fp == NULL)
	{
		cerr << "Error: Can't open file in FeatureSet::SaveMatrix()" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = SaveMatrix(fp, features);
	CHECK_RTN(rtn);
	fclose(fp);
	return 0;
}

QueryTable::QueryTable()
{
	mQueryTable.clear();
}

QueryTable::QueryTable(const vector<pair<double, double>>& table)
{
	Initialize(table);
}

QueryTable::~QueryTable()
{
	mQueryTable.clear();
}

int QueryTable::Interpolate(const pair<double, double>& front, const pair<double, double>& rear, double key, double& value)
{
	if (key < front.first || key > rear.first)
		return -1;

	if (front.first == rear.first)
		value = (front.second + rear.second) / 2.0;
	else
		value = front.second + (rear.second - front.second) * (key - front.first) / (rear.first - front.first);
	return 0;
}

int QueryTable::Initialize(const vector<pair<double, double>>& table)
{
	mQueryTable = table;
	sort(mQueryTable.begin(), mQueryTable.end());
	return 0;
}

int QueryTable::BinarySearch(double key, int &index)
{
	index = 0;
	int left = 0, right = (int)mQueryTable.size() - 1;
	while (left <= right)
	{
		int mid = (left + right) >> 1;
		if (mQueryTable[mid].first <= key)
			left = mid + 1;
		else
			right = mid - 1;
	}
	index = right;
	return 0;
}

int QueryTable::Query(const double key, double& value)
{
	value = 0.0;
	int rtn = 0;
	int index = 0;
	rtn = BinarySearch(key, index);
	CHECK_RTN(rtn);
	if (index >= mQueryTable.size() || (index == mQueryTable.size() - 1 && mQueryTable[index].first != key))
		return -1;
	else if (index == mQueryTable.size() - 1 && mQueryTable[index].first == key)
		value = mQueryTable[index].second;
	else if (index < mQueryTable.size() - 1)
	{
		rtn = Interpolate(mQueryTable[index], mQueryTable[index + 1], key, value);
		CHECK_RTN(rtn);
	}
	return 0;
}

double QueryTable::operator[](const double key)
{
	double value;
	Query(key, value);
	return value;
}

int QueryTable::Size()
{
	return (int)mQueryTable.size();
}

int QueryTable::GetElement(int index, pair<double, double>& ele)
{
	if (index < Size())
		ele = mQueryTable[index];
	else
		return -1;
	return 0;
}

int QueryTable::Save(const string& fileName, int printLog)
{
	int rtn = 0;
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	rtn = Save(outFile, printLog);
	fclose(outFile);
	CHECK_RTN(rtn);
	return 0;
}

int QueryTable::Save(FILE* outFile, int printLog)
{
	if (outFile == NULL)
		return -1;
	int rtn = 0;
	int cnt = 0;
	rtn = Write(outFile, (size_t)mQueryTable.size());
	CHECK_RTN(rtn);
	for (size_t i = 0; i < mQueryTable.size(); ++i)
	{
		rtn = Write(outFile, mQueryTable[i].first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, mQueryTable[i].second);
		CHECK_RTN(rtn);
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total save " << cnt << " querytable instances" << endl;
	return 0;
}

int QueryTable::Load(const string& fileName, int printLog)
{
	mQueryTable.clear();
	FILE* inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Load(inFile, printLog);
	fclose(inFile);
	CHECK_RTN(rtn);
	return 0;
}

int QueryTable::Load(FILE* inFile, int printLog)
{
	mQueryTable.clear();
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	FileBuffer buffer(inFile);
	rtn = Load(buffer, printLog);
	CHECK_RTN(rtn);
	return 0;
}

int QueryTable::Load(FileBuffer& buffer, int printLog)
{
	int rtn = 0;
	int cnt = 0;
	size_t len = 0;
	buffer.GetNextData(len);
	for (size_t i = 0; i < len; ++i)
	{
		if (buffer.Eof())
			return -1;
		double key, value;
		rtn = buffer.GetNextData(key);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(value);
		CHECK_RTN(rtn);
		mQueryTable.push_back(make_pair(key, value));
		++cnt;
	}
	sort(mQueryTable.begin(), mQueryTable.end());
	if (printLog != SILENT)
		clog << "Total Load " << cnt << " querytable instances" << endl;
	return 0;
}

TrieGraph::TrieGraph()
{
	mNodes.clear();
}

TrieGraph::~TrieGraph()
{

}

TrieGraph::TrieGraph(map<string, int> &stringMap)
{
	Build(stringMap);
}

TrieGraph::TrieGraph(const char* const fileName)
{
	Load(fileName);
}

int TrieGraph::Clear()
{
	mNodes.clear();
	return 0;
}

int TrieGraph::Load(FILE* binFile)
{
	int rtn = 0;
	rtn = Read(binFile, mNodes);
	CHECK_RTN(rtn);
	return 0;
}

int TrieGraph::Load(const char* const fileName)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "rb");
	rtn = Load(binFile);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int TrieGraph::Save(FILE* binFile)
{
	int rtn = 0;
	rtn = Write(binFile, mNodes);
	CHECK_RTN(rtn);
	return 0;
}

int TrieGraph::Save(const char* const fileName)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "wb");
	rtn = Save(binFile);
	CHECK_RTN(rtn);
	return 0;
}

int TrieGraph::AddInTrieTree(std::string word, int id)
{
	size_t nowPos = 0;
	for (size_t i = 0; i < word.length(); i++)
	{
		if ((char)word[i] < (char)32 || (char)word[i] > (char)127)
			continue;
		char nextLink = word[i] - (char)32;
		if (mNodes[nowPos].mLink[nextLink] == 0)
		{
			mNodes.push_back(TrieNode());
			mNodes.back().mFather = nowPos;
			mNodes.back().mDepth = mNodes[nowPos].mDepth + 1;
			mNodes.back().mPath = nextLink;
			mNodes[nowPos].mLink[nextLink] = mNodes.size() - 1;
		}
		nowPos = mNodes[nowPos].mLink[nextLink];
	}
	if (nowPos > 0)
	{
		mNodes[nowPos].mId = id;
		mNodes[nowPos].mDanger = true;
	}
	return 0;
}

int TrieGraph::CompleteGraph()
{
	vector<pair<int, int> > depthOrder;
	depthOrder.resize(mNodes.size());
	for (size_t i = 0; i < mNodes.size(); i++)
		depthOrder[i] = make_pair((int)i, (int)mNodes[i].mDepth);
	sort(depthOrder.begin(), depthOrder.end(), CmpPairBySmallerSecond<int, int>);
	for (size_t i = 0; i < depthOrder.size(); i++)
	{
		size_t index = depthOrder[i].first;
		if (mNodes[index].mDepth > 1)
			mNodes[index].mSuffix = mNodes[mNodes[mNodes[index].mFather].mSuffix].mLink[mNodes[index].mPath];
		mNodes[index].mDanger |= mNodes[mNodes[index].mSuffix].mDanger;
		for (unsigned int i = 0; i < PRINTABLE_CHAR; i++)
		if (mNodes[index].mLink[i] == 0)
			mNodes[index].mLink[i] = mNodes[mNodes[index].mSuffix].mLink[i];
	}
	return 0;
}

int TrieGraph::Build(map<string, int> &stringMap)
{
	int rtn = 0;
	mNodes.push_back(TrieNode());
	for (map<string, int>::iterator it = stringMap.begin(); it != stringMap.end(); it++)
	{
		rtn = AddInTrieTree(it->first, it->second);
		CHECK_RTN(rtn);
	}
	rtn = CompleteGraph();
	CHECK_RTN(rtn);
	return 0;
}

int TrieGraph::PhaseText(string &origText, vector<TrieDetected> &matches)
{
	int rtn = 0;
	size_t nowPos = 0;
	matches.clear();
	for (size_t i = 0; i < origText.length(); i++)
	{
		if ((char)origText[i] < (char)32 || (char)origText[i] > (char)127)
			continue;
		char nextLink = origText[i] - (char)32;
		nowPos = mNodes[nowPos].mLink[nextLink];
		if (mNodes[nowPos].mDanger)
		{
			size_t source = nowPos;
			while (mNodes[source].mDanger)
			{
				if (mNodes[source].mId > 0)
				{
					TrieDetected detected;
					detected.mFoundId = mNodes[source].mId;
					detected.mStart = i + 1 - mNodes[source].mDepth;
					detected.mEnd = i;
					matches.push_back(detected);
					break;
				}
				source = mNodes[source].mSuffix;
			}
		}
	}
	return 0;
}


EntryMap::EntryMap()
{
	mStringIndex.clear();
	mTrie.Clear();
	mHasTrie = false;
	mMode = WORD_MODE;
}

EntryMap::EntryMap(bool mode)
{
	mStringIndex.clear();
	mTrie.Clear();
	mHasTrie = false;
	mMode = mode;
}

EntryMap::EntryMap(map<string, int>& entryMap)
{
	mStringIndex = entryMap;
	mTrie.Clear();
	mHasTrie = false;
}

EntryMap::~EntryMap()
{
	mStringIndex.clear();
	mTrie.Clear();
	mHasTrie = false;
}

int EntryMap::SetMode(bool mode)
{
	mMode = mode;
	return 0;
}

int EntryMap::Clear()
{
	int rtn = 0;
	mStringIndex.clear();
	rtn = mTrie.Clear();
	CHECK_RTN(rtn);
	mHasTrie = false;
	return 0;
}

int EntryMap::Load(FILE* inFile)
{
	int rtn = 0;
	mStringIndex.clear();
	rtn = mTrie.Clear();
	CHECK_RTN(rtn);
	mHasTrie = false;
	rtn = Read(inFile, mStringIndex);
	CHECK_RTN(rtn);
	return 0;
}

int EntryMap::Load(string fileName)
{
	int rtn = 0;
	FILE* inFile = fopen(fileName.c_str(), "rb");
	rtn = Load(inFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int EntryMap::Save(FILE* outFile)
{
	int rtn = 0;
	rtn = Write(outFile, mStringIndex);
	CHECK_RTN(rtn);
	return 0;
}

int EntryMap::Save(string fileName)
{
	int rtn = 0;
	FILE* outFile = fopen(fileName.c_str(), "wb");
	rtn = Save(outFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int EntryMap::BuildFromMeshXML(FILE* inFile)
{
	int rtn = 0;
	mStringIndex.clear();
	while (Pending(inFile, "DescriptorRecord") == 0)
	{
		rtn = Pending(inFile, "DescriptorUI");
		CHECK_RTN(rtn);
		char* ui = NULL;
		rtn = GetContent(inFile, ui);
		CHECK_RTN(rtn);
		int uiNum;
		sscanf(ui + 1, "%d", &uiNum);
		rtn = SmartFree(ui);
		CHECK_RTN(rtn);

		rtn = Pending(inFile, "DescriptorName");
		CHECK_RTN(rtn);
		rtn = Pending(inFile, "String");
		CHECK_RTN(rtn);
		char* meshName = NULL;
		rtn = GetContent(inFile, meshName);
		CHECK_RTN(rtn);
		if (meshName == NULL)
		{
			cerr << "ERROR : Invalid MeSH name found!" << endl;
			return -1;
		}
		mStringIndex[string(meshName)] = uiNum;
		rtn = SmartFree(meshName);
		CHECK_RTN(rtn);

		char* element = NULL;
		while (element == NULL || strcmp(element, "/DescriptorRecord"))
		{
			rtn = SmartFree(element);
			CHECK_RTN(rtn);
			GetFirstElement(inFile, element);
			CHECK_RTN(rtn);
			if (strcmp(element, "TermUI") == 0)
			{
				rtn = Pending(inFile, "String");
				CHECK_RTN(rtn);
				char* termName = NULL;
				rtn = GetContent(inFile, termName);
				CHECK_RTN(rtn);
				mStringIndex[string(termName)] = uiNum;
				rtn = SmartFree(termName);
				CHECK_RTN(rtn);
			}
		}
		rtn = SmartFree(element);
		CHECK_RTN(rtn);
	}
	return 0;
}

int EntryMap::BuildFromMeshXML(string fileName)
{
	int rtn = 0;
	FILE* inFile = fopen(fileName.c_str(), "r");
	rtn = BuildFromMeshXML(inFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int EntryMap::BuildTrie()
{
	int rtn = 0;
	rtn = mTrie.Clear();
	CHECK_RTN(rtn);
	if (mMode = WORD_MODE)
	{
		map<string, int> lower;
		lower.clear();
		for (map<string, int>::iterator it = mStringIndex.begin(); it != mStringIndex.end(); it++)
		{
			string lowerWord = it->first;
			rtn = ToLow(lowerWord);
			lowerWord = lowerWord;
			lower[lowerWord] = it->second;
		}
		rtn = mTrie.Build(lower);
	}
	else
		rtn = mTrie.Build(mStringIndex);
	CHECK_RTN(rtn);
	mHasTrie = true;
	return 0;
}

int EntryMap::PhaseText(string origText, map<int, double> &termCount)
{
	
	int rtn = 0;

	if (mHasTrie == false)
	{
		cerr << "Trie Graph Has not Build" << endl;
		return 1;
	}
	if (mMode = WORD_MODE)
	{
		rtn = ToLow(origText);
		CHECK_RTN(rtn);
	}

	vector<TrieDetected> detected;
	rtn = mTrie.PhaseText(origText, detected);
	CHECK_RTN(rtn);
	termCount.clear();
	for (size_t i = 0; i < detected.size(); i++)
	if ((detected[i].mStart == 0 || (IsPunctuation(origText[detected[i].mStart - 1]) == true)) && 
		(detected[i].mEnd == origText.length() || (IsPunctuation(origText[detected[i].mEnd + 1]) == true)))
	{
		if (termCount.count(detected[i].mFoundId) == 0)
			termCount[detected[i].mFoundId] = 1;
		else
			termCount[detected[i].mFoundId] += 1;
	}
	return 0;
}

vector<double> ShortFloat::mTable = ShortFloat::InitTable();

vector<double> ShortFloat::InitTable()
{
	vector<double> table;
	for (size_t i = 0; i <= C_MAX_INTEGER; ++i)
	{
		table.push_back(double(i) / double(C_MAX_INTEGER));
	}
	return table;
}

ShortFloat::ShortFloat()
{
	mValue = (FloatType)0;
}

ShortFloat::ShortFloat(double value)
{
	this->operator=(value);
}

ShortFloat ShortFloat::operator=(const double& value)
{
	double tmpValue = value;
	if (tmpValue > 1.0)
		tmpValue = 1.0;
	if (tmpValue < 0.0)
		tmpValue = 0.0;
	mValue = (FloatType)round(tmpValue * C_MAX_INTEGER);
	return *this;
}

double ShortFloat::Value() const
{
	return mTable[mValue];
}
