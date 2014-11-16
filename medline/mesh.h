#ifndef MESH_H
#define MESH_H
#include <cstdio>
#include <string>
#include <map>

struct TreeTerm
{
	char* mPath;
};

class MeshRecord
{
public:
	char mTuid;
	int mUid;
	char* mName;
	int mNumTreeTerm;
	TreeTerm* mTreeList;

	MeshRecord();
	~MeshRecord();

	int Clear();

	int Save(FILE* outFile);

	int Save(const char* const fileName);

	int Load(FILE* inFile);

	int Load(const char* const fileName);

	int PrintText(FILE* outfile);

	int PrintText(const char* const fileName);
};

class MeshRecordSet
{
private:
	std::map<std::string, MeshRecord*> mStringMap;
public:
	std::map<int, MeshRecord*> mMesh;

	MeshRecordSet();
	~MeshRecordSet();

	MeshRecord* operator[](int index);

	MeshRecord* operator[](std::string index);

	int GetMeshCount();

	int Save(FILE* outFile);

	int Save(const char* const fileName);

	int Load(FILE* inFile);

	int Load(const char* const fileName);

	int PrintText(FILE* outfile);

	int PrintText(const char* const fileName);

	static int MeshListDepth(std::string meshPath, int &depth);

	static int MaxCommonDepth(std::string meshPath1, std::string meshPath2, int &maxCommonDepth);

	int GetMeshSimilarity(int mesh1, int mesh2, double& similarity);

	int GetMeshSimilarity(int meshQuery, std::set<int> &meshSet, double& similarity);
};

#endif //MESH_H