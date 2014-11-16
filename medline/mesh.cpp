#include"basic.h"
#include"mesh.h"
using namespace std;

MeshRecord::MeshRecord()
{
	mTuid = 0;
	mUid = 0;
	mName = NULL;
	mNumTreeTerm = 0;
	mTreeList = NULL;
}

MeshRecord::~MeshRecord()
{
	Clear();
}

int MeshRecord::Clear()
{
	int rtn = 0;
	rtn = SmartFree(mName);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumTreeTerm; ++i)
	{
		rtn = SmartFree(mTreeList[i].mPath);
		CHECK_RTN(rtn);
	}
	mTuid = 0;
	mUid = 0;
	mNumTreeTerm = 0;
	mTreeList = NULL;
	return 0;
}

int MeshRecord::Save(FILE* outFile)
{
	int rtn = 0;
	rtn = Write(outFile, mTuid);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mUid);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mName);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mNumTreeTerm);
	CHECK_RTN(rtn);

	if (mNumTreeTerm)
	{
		for (int i = 0; i < mNumTreeTerm; i++)
		{
			rtn = Write(outFile, mTreeList[i].mPath);
			CHECK_RTN(rtn);
		}
	}
	return 0;
}

int MeshRecord::Save(const char* const fileName)
{
	int rtn = 0;
	FILE* outFile = fopen(fileName, "wb");
	rtn = Save(outFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}


int MeshRecord::Load(FILE* inFile)
{
	int rtn = 0;
	rtn = Read(inFile, mTuid);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mUid);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mName);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mNumTreeTerm);
	CHECK_RTN(rtn);
	if (mNumTreeTerm)
	{
		mTreeList = Malloc(TreeTerm, mNumTreeTerm);
		for (int i = 0; i < mNumTreeTerm; i++)
		{
			rtn = Read(inFile, mTreeList[i].mPath);
			CHECK_RTN(rtn);
		}
	}
	else
		mTreeList = NULL;
	return 0;
}

int MeshRecord::Load(const char* const fileName)
{
	int rtn = 0;
	FILE* inFile = fopen(fileName, "rb");
	rtn = Load(inFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int MeshRecord::PrintText(FILE* outFile)
{
	fprintf(outFile, "UID = \"%c%010d\"\n", mTuid, mUid);
	fprintf(outFile, "Name = \"%s\"\n", mName);
	fprintf(outFile, "NumberTreeList = \"%d\"\n", mNumTreeTerm);
	for (int i = 0; i < mNumTreeTerm; i++)
	{
		fprintf(outFile, "%s\n", mTreeList[i].mPath);
	}
	fprintf(outFile, "\n");
	return 0;
}

int MeshRecord::PrintText(const char* const fileName)
{
	int rtn = 0;
	FILE* outFile = fopen(fileName, "w");
	rtn = PrintText(outFile);
	CHECK_RTN(rtn);
	return 0;
}

MeshRecordSet::MeshRecordSet()
{
	mMesh.clear();
	mStringMap.clear();
}

MeshRecordSet::~MeshRecordSet()
{
	for (map<int, MeshRecord*>::iterator it = mMesh.begin(); it != mMesh.end(); it++)
		delete it->second;
	mMesh.clear();
}

int MeshRecordSet::GetMeshCount()
{
	return (int)mMesh.size();
}

MeshRecord* MeshRecordSet::operator[](int index)
{
	if (mMesh.count(index) > 0)
		return mMesh[index];
	return NULL;
}

MeshRecord* MeshRecordSet::operator[](string index)
{
	if (mStringMap.count(index) > 0)
		return mStringMap[index];
	return NULL;
}

int MeshRecordSet::Save(FILE* outFile)
{
	int rtn = 0;
	for (map<int, MeshRecord*>::iterator it = mMesh.begin(); it != mMesh.end(); it++)
	{
		rtn = it->second->Save(outFile);
		CHECK_RTN(rtn);
	}
	return 0;
}

int MeshRecordSet::Save(const char* const fileName)
{
	int rtn;
	FILE* outFile = fopen(fileName, "wb");
	rtn = Save(outFile);
	CHECK_RTN(rtn);
	return 0;
}

int MeshRecordSet::Load(FILE* inFile)
{
	int rtn = 0;
	while (!feof(inFile))
	{
		MeshRecord* bufferMeshRecord = new MeshRecord;
		rtn = bufferMeshRecord->Load(inFile);
		if (feof(inFile))
		{
			delete bufferMeshRecord;
			return 0;
		}
		CHECK_RTN(rtn);

		mMesh[bufferMeshRecord->mUid] = bufferMeshRecord;
		mStringMap[bufferMeshRecord->mName] = bufferMeshRecord;
	}
	return 0;
}

int MeshRecordSet::Load(const char* const fileName)
{
	int rtn = 0;
	FILE * inFile = fopen(fileName, "rb");
	rtn = Load(inFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int MeshRecordSet::PrintText(FILE* outFile)
{
	int rtn = 0;
	for (map<int, MeshRecord*>::iterator it = mMesh.begin(); it != mMesh.end(); it++)
	{
		rtn = it->second->PrintText(outFile);
		CHECK_RTN(rtn);
	}
	return 0;
}

int MeshRecordSet::PrintText(const char* const fileName)
{
	int rtn = 0;
	FILE* outFile = fopen(fileName, "w");
	rtn = PrintText(outFile);
	fclose(outFile);
	CHECK_RTN(rtn);
	return 0;
}

int MeshRecordSet::MeshListDepth(string meshPath, int &depth)
{
	depth = ((int)meshPath.length() + 1) / 4 + 1;
	return 0;
}

int MeshRecordSet::MaxCommonDepth(string meshPath1, string meshPath2, int &maxCommonDepth)
{
	int minLength = (int)min(meshPath1.length(), meshPath2.length());
	int commonLength = 0;
	while (commonLength < minLength && meshPath1[commonLength] == meshPath2[commonLength])
		commonLength++;

	if (commonLength == 2)
		maxCommonDepth = 1;
	else if (commonLength < 2)
		maxCommonDepth = 0;
	else 
		maxCommonDepth = commonLength / 4 + 1;
	return 0;
}

int MeshRecordSet::GetMeshSimilarity(int mesh1, int mesh2, double& similarity)
{
	int rtn = 0;
	similarity = 0;
	if (mMesh.count(mesh1) < 1)
	{
		cerr << "Mesh Number " << mesh1 << "Not Found" << endl;
		return 1;
	}

	if (mMesh.count(mesh2) < 1)
	{
		cerr << "Mesh Number " << mesh2 << "Not Found" << endl;
		return 1;
	}

	if (mesh1 == mesh2)
	{
		similarity = 1;
	}
	else
	{
		MeshRecord* mesh1Ptr = mMesh[mesh1];
		MeshRecord* mesh2Ptr = mMesh[mesh2];
		for (size_t i = 0; i < mesh1Ptr->mNumTreeTerm; i++)
		for (size_t j = 0; j < mesh2Ptr->mNumTreeTerm; j++)
		{
			int depth1 = 1;
			rtn = MeshListDepth(mesh1Ptr->mTreeList[i].mPath, depth1);
			CHECK_RTN(rtn);
			int depth2 = 1;
			rtn = MeshListDepth(mesh2Ptr->mTreeList[j].mPath, depth2);
			CHECK_RTN(rtn);
			int commonDepth = 0;
			rtn = MaxCommonDepth(mesh1Ptr->mTreeList[i].mPath, mesh2Ptr->mTreeList[j].mPath, commonDepth);
			CHECK_RTN(rtn);
			double nowSimilarity = (double)commonDepth * 2 / (depth1 + depth2);
			similarity = max(similarity, nowSimilarity);
		}
	}
	return 0;
}

int MeshRecordSet::GetMeshSimilarity(int meshQuery, set<int> &meshSet, double& similarity)
{
	int rtn = 0;
	similarity = 0;
	for (set<int>::iterator it = meshSet.begin(); it != meshSet.end(); it++)
	{
		double similarityNow = 0;
		rtn = GetMeshSimilarity(meshQuery, *it, similarityNow);
		CHECK_RTN(rtn);
		similarity = max(similarity, similarityNow);
	}
	return 0;
}