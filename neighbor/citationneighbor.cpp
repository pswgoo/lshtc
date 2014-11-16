#include "citationneighbor.h"
#include "evaluation/tools.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
using namespace std;

int MeshNeighbor::SaveMeshNeighborBinary(map<int, MeshNeighbor>& gra, const char* fileName, int EdgeNum)
{
	if (EdgeNum == -1)
	{
		for (map<int, MeshNeighbor>::iterator wit = gra.begin(); wit != gra.end(); ++wit)
		{
			EdgeNum += (int)wit->second.mNeighbors.size();
		}
	}
	FILE *outFile = fopen(fileName, "wb"); //"meshneighbor_gra.bin"
	Write(outFile, EdgeNum);
	for (map<int, MeshNeighbor>::iterator wit = gra.begin(); wit != gra.end(); ++wit)
	{
		MeshNeighbor &nei = wit->second;
		for (map<int, int>::iterator it = nei.mNeighbors.begin(); it != nei.mNeighbors.end(); ++it)
		{
			Write(outFile, nei.mMeshId);
			Write(outFile, it->first);
			Write(outFile, it->second);
		}
	}
	fclose(outFile);
	return 0;
}

int MeshNeighbor::LoadMeshNeighborBinary(map<int, MeshNeighbor>& gra, const char* fileName)
{
	FileBuffer buffer(fileName);
	gra.clear();
	int edgeNum;
	buffer.GetNextData(edgeNum);
	for (int i = 0; i < edgeNum; ++i)
	{
		int u, v, w;
		buffer.GetNextData(u);
		buffer.GetNextData(v);
		buffer.GetNextData(w);
		gra[u].mMeshId = u;
		gra[u].AddEdge(v, w);
	}
	return 0;
}

MeshNeighbor::MeshNeighbor(int mId)
{
	mNeighbors.clear();
	mMeshId = mId;
	mTotWeight = 0;
}

int MeshNeighbor::AddNeighbor(int neiId)
{
	if (mNeighbors.count(neiId) == 0)
		mNeighbors[neiId] = 0;
	mNeighbors[neiId]++;
	mTotWeight++;
	return 0;
}

int MeshNeighbor::AddEdge(int neiId, int weight)
{
	mNeighbors[neiId] = weight;
	mTotWeight += weight;
	return 0;
}

int MeshNeighbor::GetNeighbor(vector<int>& meshs, int rank)
{
	meshs.clear();
	vector<pair<int, double> > edges;
	edges.reserve(mNeighbors.size());
	for (map<int, int>::iterator it = mNeighbors.begin(); it != mNeighbors.end(); ++it)
		edges.push_back(make_pair(it->first, it->second));
	sort(edges.begin(), edges.end(), CmpScore);
	for (int i = 0; i < rank && i < (int)edges.size(); ++i)
		meshs.push_back(edges[i].first);
	return 0;
}

const char CitationNeighbor::mNeighborPath[] = "pmidlinks";//d:/bioasq1a/
const char CitationNeighbor::mUrlDownPath[] = "urldowndata";
map<int, int> CitationNeighbor::mCntFindPmids = map<int, int>();
CitationSet CitationNeighbor::mCitationSet = CitationSet();

//map<int, std::vector<std::string> > CitationNeighbor::mCitationMeshs = map<int, std::vector<std::string> >();

int CitationNeighbor::DownloadLinkFiles(const std::set<int> &pmids, string linkPath)
{
	if (pmids.empty())
		return 0;

	string listPath("linkpmidlist.txt");
	FILE *outFile = fopen(listPath.c_str(), "w");
	if (outFile == NULL)
	{
		cerr << "Error: Can't open path \"" << listPath << "\"" << endl;
		return -1;
	}
	for (set<int>::iterator it = pmids.begin(); it != pmids.end(); ++it)
	{
		fprintf(outFile, "%d\n", *it);
	}
	fclose(outFile);

	string pyPath("python scraper.py");
	string cmd = pyPath + " " + listPath + " " + linkPath;
	system(cmd.c_str());
	return 0;
}

int CitationNeighbor::DownloadRowDataFiles(const std::set<int> &pmids, std::string urlPath)
{
	if (pmids.empty())
		return 0;

	string listPath("urldownlist.txt");
	const int segSize = 100;
	const char url[] = "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=pubmed&id=";//eutils.ncbi.nlm.nih.gov -> [2607:f220:41e:4290::110]
	int index = 1;
	set<int>::iterator it = pmids.begin();
	ofstream fout(listPath);
	while (it != pmids.end())
	{
		fout << url;
		int cnt = 0;
		set<int>::iterator start = it;
		for (; it != pmids.end(); ++it)
		{
			if (it != start)
				fout << ',';
			fout << *it;
			cnt++;
			if (cnt >= segSize)
			{
				++it;
				break;
			}
		}
		fout << endl;
	}
	fout.close();

	string pyPath("python urltofile.py");
	string cmd = pyPath + " " + listPath + " " + urlPath;
	system(cmd.c_str());
	return 0;
}

int CitationNeighbor::AddCitations(const char* const dir, int start, int end, string journalSetFile, CitationSet &tarSet, std::set<int> delPmids, int printLog)
{
#define CHECK_RTN_FILE(n) if((n)!=0){ fclose(inFile);return (n);}

	if (dir == NULL)
		return -1;

	int rtn = 0;
	JournalSet journalSet;
	rtn = journalSet.LoadJournalSet(journalSetFile, printLog);
	CHECK_RTN(rtn);

	const int MAX_NAME_LEN = 500;
	const int STRING_BUFFER_LEN = 1000000;
	char fileName[MAX_NAME_LEN];
	char strBuffer[STRING_BUFFER_LEN];
	string bracket;
	bracket.reserve(200000);

	int totalLoad = 0;
	for (int i = start; i <= end; ++i)
	{
		if (printLog == FULL_LOG)
			clog << "\rloading " << i << ".txt";
		sprintf(fileName, "%s/%d.txt", dir, i);
		FILE *inFile = fopen(fileName, "r");
		if (inFile == NULL)
		{
			cerr << "Warning: Can't open file " << fileName << endl;
			fclose(inFile);
			return 0;
		}

		int cnt = 0;
		while (fscanf(inFile, "%s", strBuffer) != EOF)
		{
			if (0 != strcmp(strBuffer, "Pubmed-entry"))
				continue;

			char ch = 0;
			while ((ch = fgetc(inFile)) != EOF)
			if (ch == '{') break;
			if (ch == EOF)
			{
				fclose(inFile);
				return -1;
			}
			fscanf(inFile, "%s", strBuffer);
			if (0 != strcmp(strBuffer, "pmid"))
			{
				cerr << "Error: Can't parse \"pmid\"!" << endl;
				fclose(inFile);
				return -1;
			}
			int pmid;
			fscanf(inFile, "%d", &pmid);

			if (tarSet.mCitations.count(pmid) > 0)
				continue;

			++cnt;
			tarSet.mCitations[pmid] = new Citation;
			Citation *nwCita = tarSet.mCitations[pmid];
			nwCita->mPmid = pmid;

			rtn = NextBracket(inFile, '{', '}', bracket);
			CHECK_RTN_FILE(rtn);

			string::size_type found = 0;
			found = bracket.find("em std {");
			if (found == bracket.npos)
			{
				cerr << "Can't find datecreated" << endl;
				fclose(inFile);
				return -1;
			}
			int dateLeft, dateRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), dateLeft, dateRight);
			CHECK_RTN(rtn);
			int yearfound = (int)bracket.find("year ", dateLeft);
			if (yearfound > dateRight)
			{
				cerr << "Can't find year in em std {}" << endl;
				fclose(inFile);
				return -1;
			}
			string strYear = bracket.substr(yearfound + 5, 4);

			int monthfound = (int)bracket.find("month ", dateLeft);
			int commfound = (int)bracket.find(",", monthfound);
			if (monthfound > dateRight)
			{
				cerr << "Can't find month in em std {}" << endl;
				fclose(inFile);
				return -1;
			}
			string strMonth = bracket.substr(monthfound + 6, commfound - monthfound - 6);

			int dayfound = (int)bracket.find("day ", dateLeft);
			commfound = (int)bracket.find("}", dayfound);
			if (dayfound > dateRight)
			{
				cerr << "Can't find day in em std {}" << endl;
				fclose(inFile);
				return -1;
			}
			string strDay = bracket.substr(dayfound + 4, commfound - dayfound - 4);

			nwCita->mDateCreated = stringToInt(strYear) * 10000 + stringToInt(strMonth) * 100 + stringToInt(strDay);

			found = bracket.find("cit {");
			if (found == bracket.npos)
			{
				cerr << "Can't find 'cit {'" << endl;
				fclose(inFile);
				return -1;
			}
			int citLeft, citRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), citLeft, citRight);
			CHECK_RTN_FILE(rtn);
			int titFound = (int)bracket.find("title {", citLeft);
			int titLeft, titRight;
			rtn = NextBracketRange(bracket, (int)titFound, (int)bracket.size(), titLeft, titRight);
			CHECK_RTN_FILE(rtn);
			int nameFound = (int)bracket.find("name \"", titLeft);
			if (nameFound < titRight && titRight<citRight)
			{
				int tpos = nameFound + 4;
				string articleTitle;
				rtn = NextQuotePair(bracket, tpos, titRight, articleTitle);
				CHECK_RTN_FILE(rtn);
				if (articleTitle.size() > 0)
				{
					nwCita->mArticleTitle = Malloc(char, articleTitle.size() + 1);
					strcpy(nwCita->mArticleTitle, articleTitle.c_str());
				}
			}

			int authFound = (int)bracket.find("authors {", citLeft);
			if (authFound != (int)bracket.npos)
			{
				int authLeft, authRight;
				rtn = NextBracketRange(bracket, (int)authFound, (int)bracket.size(), authLeft, authRight);
				int namestdFound = (int)bracket.find("names std {");
				if (namestdFound < authRight && namestdFound > authLeft)
				{
					int namestdLeft, namestdRight;
					rtn = NextBracketRange(bracket, (int)namestdFound, authRight, namestdLeft, namestdRight);
					int termLeft, termRight;
					int termPos = namestdLeft + 1;
					vector<string> vecAuths;
					while (NextBracketRange(bracket, termPos, namestdRight, termLeft, termRight) >= 0)
					{
						termPos = termRight + 1;
						int termFound = (int)bracket.find("name ", termLeft);
						if (termFound + 5 >= termRight)
						{
							clog << "Warning: Can't find valid author name" << endl;
							continue;
						}

						int tpos = termFound + 7;
						string auth;
						rtn = NextQuotePair(bracket, tpos, termRight, auth);
						CHECK_RTN_FILE(rtn);
						if (auth.empty())
						{
							cerr << "Error: Read a empty author!" << endl;
							fclose(inFile);
							return -1;
						}
						vecAuths.push_back(auth);
					}
					nwCita->mNumberAuthor = (int)vecAuths.size();
					nwCita->mAuthorList = Malloc(Author, nwCita->mNumberAuthor);
					for (unsigned j = 0; j < vecAuths.size(); ++j)
					{
						memset(nwCita->mAuthorList + j, 0, sizeof(Author));
						if (vecAuths[j].size() > 0)
						{
							nwCita->mAuthorList[j].mForeName = Malloc(char, vecAuths[j].size() + 1);
							strcpy(nwCita->mAuthorList[j].mForeName, vecAuths[j].c_str());
						}
					}
				}
			}

			int jourFound = (int)bracket.find("from journal {", citLeft);
			if (jourFound != (int)bracket.npos && jourFound < citRight)
			{
				int jourLeft, jourRight;
				rtn = NextBracketRange(bracket, (int)jourFound, (int)bracket.size(), jourLeft, jourRight);
				CHECK_RTN_FILE(rtn);
				int mlFound = (int)bracket.find("ml-jta \"", jourLeft);
				if (mlFound < jourRight && jourRight < citRight)
				{
					int tpos = mlFound + 4;
					string medAbbr;
					rtn = NextQuotePair(bracket, tpos, jourRight, medAbbr);
					CHECK_RTN_FILE(rtn);
					string jourTitle = journalSet.GetFirstJournalTitle(medAbbr);
					if (jourTitle.empty())
						jourTitle = medAbbr;
					if (jourTitle.size() > 0)
					{
						nwCita->mJournalTitle = Malloc(char, jourTitle.size() + 1);
						strcpy(nwCita->mJournalTitle, jourTitle.c_str());
					}
				}
			}

			int absFound = (int)bracket.find("abstract \"");
			if (absFound != bracket.npos && (absFound < citLeft || absFound > citRight))
			{
				int tpos = absFound + 4;
				string abst;
				rtn = NextQuotePair(bracket, tpos, (int)bracket.size(), abst);
				CHECK_RTN_FILE(rtn);
				if (abst.size() > 0)
				{
					nwCita->mNumberAbstract = 1;
					nwCita->mAbstract = Malloc(Abstract, 1);
					memset(nwCita->mAbstract, 0, sizeof(Abstract));
					nwCita->mAbstract[0].mText = Malloc(char, abst.size() + 1);
					strcpy(nwCita->mAbstract[0].mText, abst.c_str());
				}
			}

			int pos = 0;
			found = bracket.find("mesh {", pos);
			if (found == bracket.npos)
				continue;

			int meshLeft, meshRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), meshLeft, meshRight);
			CHECK_RTN_FILE(rtn);
			int termLeft = 0, termRight = 0;
			int termPos = (int)found + 6;
			typedef pair<string, bool> meshEntry;
			vector<meshEntry> vecMeshs;
			vector<vector<meshEntry>> vecQualifiers;
			while (NextBracketRange(bracket, termPos, meshRight, termLeft, termRight) >= 0)
			{
				vector<meshEntry> qualifiers;
				int qualLeft = termLeft, qualRight = termLeft;
				size_t qualFound = bracket.find("qual {", termLeft);
				if (qualFound != bracket.npos && qualFound < (size_t)termRight)
				{
					rtn = NextBracketRange(bracket, qualFound, termRight, qualLeft, qualRight);
					CHECK_RTN_FILE(rtn);
					int subPos = (int)qualFound + 6;
					int subLeft = 0, subRight = 0;

					while (NextBracketRange(bracket, subPos, qualRight, subLeft, subRight) >= 0)
					{
						int subFound = (int)bracket.find("subh", subLeft);
						if (subFound + 4 > subRight)
						{
							cerr << "Error: Can't find valid \"subh\"!" << endl;
							fclose(inFile);
							return -1;
						}
						int tpos = subFound + 4;
						string subh;
						rtn = NextQuotePair(bracket, tpos, subRight, subh);
						CHECK_RTN_FILE(rtn);
						if (subh.empty())
						{
							cerr << "Error: Read a empty subh!" << endl;
							fclose(inFile);
							return -1;
						}

						meshEntry entry(subh, false);
						size_t majorFound = bracket.find("mp TRUE,", subLeft);
						if (majorFound != bracket.npos && majorFound < subRight)
							entry.second = true;
						qualifiers.push_back(entry);
						subPos = subRight + 1;
					}
				}

				int termFound = (int)bracket.find("term", termLeft);
				if (termFound + 4 > termRight)
				{
					cerr << "Error: Can't find valid \"term\"!" << endl;
					fclose(inFile);
					return -1;
				}
				int tpos = termFound + 4;
				string mesh;
				rtn = NextQuotePair(bracket, tpos, termRight, mesh);
				CHECK_RTN_FILE(rtn);
				if (mesh.empty())
				{
					cerr << "Error: Read a empty mesh!" << endl;
					fclose(inFile);
					return -1;
				}

				meshEntry entry(mesh, false);
				size_t majorFound = bracket.find("mp TRUE,", termLeft);
				if (majorFound != bracket.npos && majorFound < termRight && (majorFound < qualLeft || majorFound > qualRight))
					entry.second = true;
				vecMeshs.push_back(entry);
				vecQualifiers.push_back(qualifiers);

				termPos = termRight + 1;
			}
			nwCita->mNumberMesh = (int)vecMeshs.size();
			nwCita->mMeshHeadingList = Malloc(Mesh, nwCita->mNumberMesh);
			for (unsigned j = 0; j < vecMeshs.size(); ++j)
			{
				memset(nwCita->mMeshHeadingList + j, 0, sizeof(Mesh));
				if (vecMeshs[j].first.size() > 0)
				{
					Mesh& mesh = nwCita->mMeshHeadingList[j];
					mesh.mDescriptorName.mIsMajor = vecMeshs[j].second;
					mesh.mDescriptorName.mText = Malloc(char, vecMeshs[j].first.size() + 1);
					strcpy(mesh.mDescriptorName.mText, vecMeshs[j].first.c_str());
					mesh.mNumQualifierName = (int)vecQualifiers[j].size();
					mesh.mQualifierName = Malloc(MeshTerm, mesh.mNumQualifierName);
					for (size_t k = 0; k < vecQualifiers[j].size(); ++k)
					{
						memset(mesh.mQualifierName + k, 0, sizeof(MeshTerm));
						mesh.mQualifierName[k].mIsMajor = vecQualifiers[j][k].second;
						if (vecQualifiers[j][k].first.size() > 0)
						{
							mesh.mQualifierName[k].mText = Malloc(char, vecQualifiers[j][k].first.size() + 1);
							strcpy(mesh.mQualifierName[k].mText, vecQualifiers[j][k].first.c_str());
						}
					}
				}
				else
				{
					cerr << "Error: Empty mesh " << endl;
					fclose(inFile);
					return -1;
				}
			}
		}
		fclose(inFile);
		totalLoad += cnt;
	}
	if (printLog == FULL_LOG)
	{
		clog << endl;
	}
	if (printLog != SILENT)
		clog << totalLoad << " Row Citations loaded" << endl;
	return 0;
}

int CitationNeighbor::AddCitations(const char* const dir, int start, int end, string journalSetFile, std::set<int>& pmids, CitationSet &tarSet, int printLog)
{
#define CHECK_RTN_FILE(n) if((n)!=0){ fclose(inFile);return (n);}

	if (dir == NULL)
		return -1;

	int rtn = 0;
	JournalSet journalSet;
	rtn = journalSet.LoadJournalSet(journalSetFile, printLog);
	CHECK_RTN(rtn);

	const int MAX_NAME_LEN = 500;
	const int STRING_BUFFER_LEN = 1000000;
	char fileName[MAX_NAME_LEN];
	char strBuffer[STRING_BUFFER_LEN];
	string bracket;
	bracket.reserve(200000);

	int totalLoad = 0;
	for (int i = start; i <= end; ++i)
	{
		if (printLog == FULL_LOG)
			clog << "\rloading " << i << ".txt";
		sprintf(fileName, "%s/%d.txt", dir, i);
		FILE *inFile = fopen(fileName, "r");
		if (inFile == NULL)
		{
			cerr << "Warning: Can't open file " << fileName << endl;
			fclose(inFile);
			return 0;
		}

		int cnt = 0;
		while (fscanf(inFile, "%s", strBuffer) != EOF)
		{
			if (0 != strcmp(strBuffer, "Pubmed-entry"))
				continue;

			char ch = 0;
			while ((ch = fgetc(inFile)) != EOF)
			if (ch == '{') break;
			if (ch == EOF)
			{
				fclose(inFile);
				return -1;
			}
			fscanf(inFile, "%s", strBuffer);
			if (0 != strcmp(strBuffer, "pmid"))
			{
				cerr << "Error: Can't parse \"pmid\"!" << endl;
				fclose(inFile);
				return -1;
			}
			int pmid;
			fscanf(inFile, "%d", &pmid);
			if (pmids.count(pmid) == 0)
				continue;

			if (tarSet.mCitations.count(pmid) > 0)
				continue;

			++cnt;
			tarSet.mCitations[pmid] = new Citation;
			Citation *nwCita = tarSet.mCitations[pmid];
			nwCita->mPmid = pmid;

			rtn = NextBracket(inFile, '{', '}', bracket);
			CHECK_RTN_FILE(rtn);

			string::size_type found = 0;
			found = bracket.find("em std {");
			if (found == bracket.npos)
			{
				cerr << "Can't find datecreated" << endl;
				fclose(inFile);
				return -1;
			}
			int dateLeft, dateRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), dateLeft, dateRight);
			CHECK_RTN_FILE(rtn);
			int yearfound = (int)bracket.find("year ", dateLeft);
			if (yearfound > dateRight)
			{
				cerr << "Can't find year in em std {}" << endl;
				fclose(inFile);
				return -1;
			}
			string strYear = bracket.substr(yearfound + 5, 4);

			int monthfound = (int)bracket.find("month ", dateLeft);
			int commfound = (int)bracket.find(",", monthfound);
			if (monthfound > dateRight)
			{
				cerr << "Can't find month in em std {}" << endl;
				fclose(inFile);
				return -1;
			}
			string strMonth = bracket.substr(monthfound + 6, commfound - monthfound - 6);

			int dayfound = (int)bracket.find("day ", dateLeft);
			commfound = (int)bracket.find("}", dayfound);
			if (dayfound > dateRight)
			{
				cerr << "Can't find day in em std {}" << endl;
				fclose(inFile);
				return -1;
			}
			string strDay = bracket.substr(dayfound + 4, commfound - dayfound - 4);

			nwCita->mDateCreated = stringToInt(strYear) * 10000 + stringToInt(strMonth) * 100 + stringToInt(strDay);

			found = bracket.find("cit {");
			if (found == bracket.npos)
			{
				cerr << "Can't find 'cit {'" << endl;
				fclose(inFile);
				return -1;
			}
			int citLeft, citRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), citLeft, citRight);
			CHECK_RTN_FILE(rtn);
			int titFound = (int)bracket.find("title {", citLeft);
			int titLeft, titRight;
			rtn = NextBracketRange(bracket, (int)titFound, (int)bracket.size(), titLeft, titRight);
			CHECK_RTN_FILE(rtn);
			int nameFound = (int)bracket.find("name \"", titLeft);
			if (nameFound < titRight && titRight<citRight)
			{
				int tpos = nameFound + 4;
				string articleTitle;
				rtn = NextQuotePair(bracket, tpos, titRight, articleTitle);
				CHECK_RTN_FILE(rtn);
				if (articleTitle.size() > 0)
				{
					nwCita->mArticleTitle = Malloc(char, articleTitle.size() + 1);
					strcpy(nwCita->mArticleTitle, articleTitle.c_str());
				}
			}

			int authFound = (int)bracket.find("authors {", citLeft);
			if (authFound != (int)bracket.npos)
			{
				int authLeft, authRight;
				rtn = NextBracketRange(bracket, (int)authFound, (int)bracket.size(), authLeft, authRight);
				int namestdFound = (int)bracket.find("names std {");
				if (namestdFound < authRight && namestdFound > authLeft)
				{
					int namestdLeft, namestdRight;
					rtn = NextBracketRange(bracket, (int)namestdFound, authRight, namestdLeft, namestdRight);
					int termLeft, termRight;
					int termPos = namestdLeft + 1;
					vector<string> vecAuths;
					while (NextBracketRange(bracket, termPos, namestdRight, termLeft, termRight) >= 0)
					{
						termPos = termRight + 1;
						int termFound = (int)bracket.find("name ", termLeft);
						if (termFound + 5 >= termRight)
						{
							clog << "Warning: Can't find valid author name" << endl;
							continue;
						}

						int tpos = termFound + 7;
						string auth;
						rtn = NextQuotePair(bracket, tpos, termRight, auth);
						CHECK_RTN_FILE(rtn);
						if (auth.empty())
						{
							cerr << "Error: Read a empty author!" << endl;
							fclose(inFile);
							return -1;
						}
						vecAuths.push_back(auth);
					}
					nwCita->mNumberAuthor = (int)vecAuths.size();
					nwCita->mAuthorList = Malloc(Author, nwCita->mNumberAuthor);
					for (unsigned j = 0; j < vecAuths.size(); ++j)
					{
						memset(nwCita->mAuthorList + j, 0, sizeof(Author));
						if (vecAuths[j].size() > 0)
						{
							nwCita->mAuthorList[j].mForeName = Malloc(char, vecAuths[j].size() + 1);
							strcpy(nwCita->mAuthorList[j].mForeName, vecAuths[j].c_str());
						}
					}
				}
			}

			int jourFound = (int)bracket.find("from journal {", citLeft);
			if (jourFound != (int)bracket.npos && jourFound < citRight)
			{
				int jourLeft, jourRight;
				rtn = NextBracketRange(bracket, (int)jourFound, (int)bracket.size(), jourLeft, jourRight);
				CHECK_RTN_FILE(rtn);
				int mlFound = (int)bracket.find("ml-jta \"", jourLeft);
				if (mlFound < jourRight && jourRight < citRight)
				{
					int tpos = mlFound + 4;
					string medAbbr;
					rtn = NextQuotePair(bracket, tpos, jourRight, medAbbr);
					CHECK_RTN_FILE(rtn);
					string jourTitle = journalSet.GetFirstJournalTitle(medAbbr);
					if (jourTitle.empty())
						jourTitle = medAbbr;
					if (jourTitle.size() > 0)
					{
						nwCita->mJournalTitle = Malloc(char, jourTitle.size() + 1);
						strcpy(nwCita->mJournalTitle, jourTitle.c_str());
					}
				}
			}

			int absFound = (int)bracket.find("abstract \"");
			if (absFound != (int)bracket.npos && (absFound < citLeft || absFound > citRight))
			{
				int tpos = absFound + 4;
				string abst;
				rtn = NextQuotePair(bracket, tpos, (int)bracket.size(), abst);
				CHECK_RTN_FILE(rtn);
				if (abst.size() > 0)
				{
					nwCita->mNumberAbstract = 1;
					nwCita->mAbstract = Malloc(Abstract, 1);
					memset(nwCita->mAbstract, 0, sizeof(Abstract));
					nwCita->mAbstract[0].mText = Malloc(char, abst.size() + 1);
					strcpy(nwCita->mAbstract[0].mText, abst.c_str());
				}
			}

			int pos = 0;
			found = bracket.find("mesh {", pos);
			if (found == bracket.npos)
				continue;

			int meshLeft, meshRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), meshLeft, meshRight);
			CHECK_RTN_FILE(rtn);
			int termLeft = 0, termRight = 0;
			int termPos = (int)found + 6;
			typedef pair<string, bool> meshEntry;
			vector<meshEntry> vecMeshs;
			vector<vector<meshEntry>> vecQualifiers;
			while (NextBracketRange(bracket, termPos, meshRight, termLeft, termRight) >= 0)
			{
				vector<meshEntry> qualifiers;
				int qualLeft = termLeft, qualRight = termLeft;
				size_t qualFound = bracket.find("qual {", termLeft);
				if (qualFound !=bracket.npos && qualFound < (size_t)termRight)
				{
					rtn = NextBracketRange(bracket, qualFound, termRight, qualLeft, qualRight);
					CHECK_RTN_FILE(rtn);
					int subPos = (int)qualFound + 6;
					int subLeft = 0, subRight = 0;
					
					while (NextBracketRange(bracket, subPos, qualRight, subLeft, subRight) >= 0)
					{
						int subFound = (int)bracket.find("subh", subLeft);
						if (subFound + 4 > subRight)
						{
							cerr << "Error: Can't find valid \"subh\"!" << endl;
							fclose(inFile);
							return -1;
						}
						int tpos = subFound + 4;
						string subh;
						rtn = NextQuotePair(bracket, tpos, subRight, subh);
						CHECK_RTN_FILE(rtn);
						if (subh.empty())
						{
							cerr << "Error: Read a empty subh!" << endl;
							fclose(inFile);
							return -1;
						}

						meshEntry entry(subh, false);
						size_t majorFound = bracket.find("mp TRUE,", subLeft);
						if (majorFound != bracket.npos && majorFound < subRight)
							entry.second = true;
						qualifiers.push_back(entry);
						subPos = subRight + 1;
					}
				}

				int termFound = (int)bracket.find("term", termLeft);
				if (termFound + 4 > termRight)
				{
					cerr << "Error: Can't find valid \"term\"!" << endl;
					fclose(inFile);
					return -1;
				}
				int tpos = termFound + 4;
				string mesh;
				rtn = NextQuotePair(bracket, tpos, termRight, mesh);
				CHECK_RTN_FILE(rtn);
				if (mesh.empty())
				{
					cerr << "Error: Read a empty mesh!" << endl;
					fclose(inFile);
					return -1;
				}

				meshEntry entry(mesh,false);
				size_t majorFound = bracket.find("mp TRUE,", termLeft);
				if (majorFound != bracket.npos && majorFound < termRight && (majorFound < qualLeft || majorFound > qualRight))
					entry.second = true;
				vecMeshs.push_back(entry);
				vecQualifiers.push_back(qualifiers);

				termPos = termRight + 1;
			}
			nwCita->mNumberMesh = (int)vecMeshs.size();
			nwCita->mMeshHeadingList = Malloc(Mesh, nwCita->mNumberMesh);
			for (unsigned j = 0; j < vecMeshs.size(); ++j)
			{
				memset(nwCita->mMeshHeadingList + j, 0, sizeof(Mesh));
				if (vecMeshs[j].first.size() > 0)
				{
					Mesh& mesh = nwCita->mMeshHeadingList[j];
					mesh.mDescriptorName.mIsMajor = vecMeshs[j].second;
					mesh.mDescriptorName.mText = Malloc(char, vecMeshs[j].first.size() + 1);
					strcpy(mesh.mDescriptorName.mText, vecMeshs[j].first.c_str());
					mesh.mNumQualifierName = (int)vecQualifiers[j].size();
					mesh.mQualifierName = Malloc(MeshTerm, mesh.mNumQualifierName);
					for (size_t k = 0; k < vecQualifiers[j].size(); ++k)
					{
						memset(mesh.mQualifierName + k, 0, sizeof(MeshTerm));
						mesh.mQualifierName[k].mIsMajor = vecQualifiers[j][k].second;
						if (vecQualifiers[j][k].first.size() > 0)
						{
							mesh.mQualifierName[k].mText = Malloc(char, vecQualifiers[j][k].first.size() + 1);
							strcpy(mesh.mQualifierName[k].mText, vecQualifiers[j][k].first.c_str());
						}
					}
				}
				else
				{
					cerr << "Error: Empty mesh " << endl;
					fclose(inFile);
					return -1;
				}
			}
		}
		fclose(inFile);
		totalLoad += cnt;
	}
	if (printLog == FULL_LOG)
	{
		clog << endl;
	}
	if (printLog != SILENT)
		clog << totalLoad << " Row Citations loaded" << endl;
	return 0;
}

int CitationNeighbor::LoadCitations(const char* const fileName, CitationSet &tarSet, int printLog)
{
	if (fileName == NULL)
		return -1;
	tarSet.mCitations.clear();
	int rtn = tarSet.Load(fileName);
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "Load " << tarSet.Size() << " citations from citationSet" << endl;
	return 0;
}

int CitationNeighbor::LoadCitations(const char* const fileName, std::set<int>& pmids, CitationSet &tarSet, int printLog)
{
	if (fileName == NULL)
		return -1;
	tarSet.mCitations.clear();
	int rtn = tarSet.Load(fileName, pmids);
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "Load " << tarSet.Size() << " citations from citationSet" << endl;
	return 0;
}

int CitationNeighbor::LoadCitationMeshs(const char* const dir, int start, int end, set<int> testpmids, int printLog)
{
	if (dir == NULL)
		return -1;

	const int FILENAME_SIZE = 1000;
	const int STRING_BUFFER_SIZE = 1000000;
	int rtn = 0;
	char fileName[FILENAME_SIZE];
	char strBuffer[STRING_BUFFER_SIZE];
	string bracket;
	bracket.reserve(200000);

	int totalLoad = 0;
	for (int i = start; i <= end; ++i)
	{
		if (printLog != SILENT)
			clog << "\rloading " << i << ".txt";
		sprintf(fileName, "%s/%d.txt", dir, i);
		if (!FileExist(fileName))
			break;

		FILE *inFile = fopen(fileName, "r");
		if (inFile == NULL)
		{
			cerr << "Can't open file " << fileName << endl;
			return -1;
		}

		int cnt = 0;
		while (fscanf(inFile, "%s", strBuffer) != EOF)
		{
			if (0 != strcmp(strBuffer, "Pubmed-entry"))
				continue;

			char ch = 0;
			while ((ch = fgetc(inFile)) != EOF)
			if (ch == '{') break;
			if (ch == EOF)
				return -1;
			fscanf(inFile, "%s", strBuffer);
			if (0 != strcmp(strBuffer, "pmid"))
			{
				cerr << "Error: Can't find \"pmid\"!" << endl;
				return -1;
			}
			int pmid;
			fscanf(inFile, "%d", &pmid);
			if (mCitationSet.mCitations.count(pmid) > 0)
				continue;
			Citation* nwCita = Malloc(Citation, 1);
			memset(nwCita, 0, sizeof(Citation));
			mCitationSet.mCitations[pmid] = nwCita;
			vector<string> vecMeshs;
			++cnt;
			if (testpmids.count(pmid) > 0)
				continue;

			rtn = NextBracket(inFile, '{', '}', bracket);
			CHECK_RTN(rtn);

			string::size_type found = 0;
			int pos = 0;
			found = bracket.find("mesh {", pos);
			if (found == bracket.npos)
				continue;

			int meshLeft, meshRight;
			rtn = NextBracketRange(bracket, (int)found, (int)bracket.size(), meshLeft, meshRight);
			CHECK_RTN(rtn);
			int termLeft = 0, termRight = 0;
			int termPos = (int)found + 6;
			while (NextBracketRange(bracket, termPos, meshRight, termLeft, termRight) >= 0)
			{
				int termFound = (int)bracket.find("term", termLeft);
				if (termFound + 4 > termRight)
				{
					cerr << "Error: Can't find valid \"term\"!" << endl;
					return -1;
				}
				int tpos = termFound + 4;
				bool bBegin = false;
				string mesh;
				while (tpos <= termRight)
				{
					if (bracket[tpos] == '"')
					{
						if (!bBegin)
							bBegin = true;
						else
							break;
					}
					if (bBegin && bracket[tpos] != '"' && bracket[tpos] != '\n' && bracket[tpos] != '\r')
						mesh.push_back(bracket[tpos]);
					++tpos;
				}
				if (mesh.empty())
				{
					cerr << "Error: Read a empty mesh!" << endl;
					return -1;
				}
				vecMeshs.push_back(mesh);
				termPos = termRight + 1;
			}

			nwCita->mNumberMesh = (int)vecMeshs.size();
			nwCita->mMeshHeadingList = Malloc(Mesh, nwCita->mNumberMesh);
			for (unsigned j = 0; j < vecMeshs.size(); ++j)
			{
				memset(nwCita->mMeshHeadingList + j, 0, sizeof(Mesh));
				nwCita->mMeshHeadingList[j].mDescriptorName.mText = Malloc(char, vecMeshs[j].size() + 1);
				strcpy(nwCita->mMeshHeadingList[j].mDescriptorName.mText, vecMeshs[j].c_str());
			}
		}
		fclose(inFile);
		totalLoad += cnt;
	}

	if (printLog != SILENT)
	{
		clog << endl;
		clog << totalLoad << " Row meshs loaded" << endl;
	}
	return 0;
}

int CitationNeighbor::LoadCitationMeshs(CitationSet& citationSet, std::set<int> testpmids, int printLog)
{
	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); ++it)
	{
		if (mCitationSet.mCitations.count(it->first) > 0)
			continue;
		if (testpmids.count(it->first) > 0)
			continue;
		Citation* pCita = Malloc(Citation, 1);
		memset(pCita, 0, sizeof(Citation));
		mCitationSet.mCitations[it->first] = pCita;
		pCita->mNumberMesh = it->second->mNumberMesh;
		pCita->mMeshHeadingList = Malloc(Mesh, pCita->mNumberMesh);
		for (int i = 0; i < pCita->mNumberMesh; ++i)
		{
			memset(pCita->mMeshHeadingList + i, 0, sizeof(Mesh));
			int len = (int)strlen(it->second->mMeshHeadingList[i].mDescriptorName.mText);
			pCita->mMeshHeadingList[i].mDescriptorName.mText = Malloc(char, len + 1);
			strcpy(pCita->mMeshHeadingList[i].mDescriptorName.mText, it->second->mMeshHeadingList[i].mDescriptorName.mText);
		}
	}
	return 0;
}

int CitationNeighbor::StoreCntFindPmids(const char* const fileName)
{
	FILE *outFile = fopen(fileName, "w");
	for (map<int, int>::iterator it = mCntFindPmids.begin(); it != mCntFindPmids.end(); ++it)
		fprintf(outFile, "%d,%d\n", it->first, it->second);
	fclose(outFile);
	return 0;
}

CitationNeighbor::CitationNeighbor(int pmid)
{
	mPmid = pmid;
}

CitationNeighbor::~CitationNeighbor()
{

}

int CitationNeighbor::GetNeighborNum(int& numNeighbor)
{
	numNeighbor = (int)mNeighborScores.size();
	return 0;
}

int CitationNeighbor::GetNeighborPmid(int& pmid, int rank)
{
	if ((size_t)rank > mNeighborScores.size())
		pmid = -1;
	else
		pmid = mNeighborScores[rank - 1].first;
	return 0;
}

int CitationNeighbor::GetNeighborScore(double& score, int rank)
{
	score = 0.0;
	if ((size_t)rank > mNeighborScores.size())
		score = -1.0;
	else
		score = mNeighborScores[rank - 1].second;
	return 0;
}

int CitationNeighbor::GetValidNeighborCount(int topRank, int &count)
{
	count = 0;
	if (topRank > (int)mNeighborScores.size())
		count = (int)mNeighborScores.size();
	else
		count = topRank;
	return 0;
}

int CitationNeighbor::GetValidScoreCount(int topRank, double &sumScore)
{
	sumScore = 0;
	if (topRank > (int)mNeighborScores.size())
		topRank = (int)mNeighborScores.size();
	for (int i = 0; i < topRank; i++)
		sumScore += mNeighborScores[i].second;
	return 0;
}

int CitationNeighbor::GetAllNeighborPmid(std::vector<int>& pmids)
{
	pmids.clear();
	for (size_t i = 0; i < mNeighborScores.size(); ++i)
		pmids.push_back(mNeighborScores[i].first);
	return 0;
}

int CitationNeighbor::GetNeighborMesh(std::vector<std::string>& meshs, int rank)
{
	meshs.clear();
	if ((size_t)rank > mNeighborScores.size())
		return 0;
	int pmid = mNeighborScores[rank - 1].first;
	if (mNeighbors.count(pmid) == 0)
	{
		if (mCitationSet.mCitations.count(pmid) == 0)
		{
			//cerr << "Warning: Can't find pmid " << pmid << endl;
			if (mCntFindPmids.count(pmid) == 0)
				mCntFindPmids[pmid] = 0;
			++mCntFindPmids[pmid];
			return 0;
		}
		mNeighbors[pmid] = mCitationSet.mCitations[pmid];
	}
	Citation* pCita = mNeighbors[pmid];
	for (int i = 0; i < pCita->mNumberMesh; ++i)
		meshs.push_back(pCita->mMeshHeadingList[i].mDescriptorName.mText);
	return 0;
}

int CitationNeighbor::GetNeighborMesh(map<int, vector<string> >& neighbors, int rank)
{
	neighbors.clear();
	for (size_t k = 0; k < rank && k < mNeighborScores.size(); ++k)
	{
		int pmid = mNeighborScores[k].first;
		vector<string> &meshs = neighbors[pmid];
		if (mNeighbors.count(pmid) == 0)
		{
			if (mCitationSet.mCitations.count(pmid) == 0)
			{
				//cerr << "Warning: Can't find pmid " << pmid << endl;
				if (mCntFindPmids.count(pmid) == 0)
					mCntFindPmids[pmid] = 0;
				++mCntFindPmids[pmid];
				continue;
			}
			mNeighbors[pmid] = mCitationSet.mCitations[pmid];
		}
		Citation* pCita = mNeighbors[pmid];
		for (int i = 0; i < pCita->mNumberMesh; ++i)
			meshs.push_back(pCita->mMeshHeadingList[i].mDescriptorName.mText);
	}
	return 0;
}

int CitationNeighbor::GetNeighborMesh(std::map<string, int>& meshCnt, int rank)
{
	meshCnt.clear();
	for (size_t k = 0; k < rank && k < mNeighborScores.size(); ++k)
	{
		int pmid = mNeighborScores[k].first;
		if (mNeighbors.count(pmid) == 0)
		{
			if (mCitationSet.mCitations.count(pmid) == 0)
			{
				//cerr << "Warning: Can't find pmid " << pmid << endl;
				if (mCntFindPmids.count(pmid) == 0)
					mCntFindPmids[pmid] = 0;
				++mCntFindPmids[pmid];
				continue;
			}
			mNeighbors[pmid] = mCitationSet.mCitations[pmid];
		}
		Citation* pCita = mNeighbors[pmid];
		for (int i = 0; i < pCita->mNumberMesh; ++i)
		{
			if (meshCnt.count(pCita->mMeshHeadingList[i].mDescriptorName.mText) == 0)
				meshCnt[pCita->mMeshHeadingList[i].mDescriptorName.mText] = 0;
			meshCnt[pCita->mMeshHeadingList[i].mDescriptorName.mText]++;
		}
	}
	return 0;
}

int CitationNeighbor::GetNeighborCitation(std::map<int, Citation*>& neighbors, int rank)
{
	neighbors.clear();
	for (size_t k = 0; k < rank && k < mNeighborScores.size(); ++k)
	{
		int pmid = mNeighborScores[k].first;
		if (mNeighbors.count(pmid) == 0)
		{
			if (mCitationSet.mCitations.count(pmid) == 0)
			{
				//cerr << "Warning: Can't find pmid " << pmid << endl;
				if (mCntFindPmids.count(pmid) == 0)
					mCntFindPmids[pmid] = 0;
				++mCntFindPmids[pmid];
				continue;
			}
			mNeighbors[pmid] = mCitationSet.mCitations[pmid];
		}
		neighbors[pmid] = mNeighbors[pmid];
	}
	return 0;
}

int CitationNeighbor::GetNeighborMeshWithScore(std::set<std::string>& meshs, double score)
{
	meshs.clear();
	int res = 0;
	for (size_t k = 0; k < mNeighborScores.size(); ++k)
	{
		if (score > mNeighborScores[k].first)
			break;

		int pmid = mNeighborScores[k].first;
		if (mNeighbors.count(pmid) == 0)
		{
			if (mCitationSet.mCitations.count(pmid) == 0)
			{
				//cerr << "Warning: Can't find pmid " << pmid << endl;
				if (mCntFindPmids.count(pmid) == 0)
					mCntFindPmids[pmid] = 0;
				++mCntFindPmids[pmid];
				continue;
			}
			mNeighbors[pmid] = mCitationSet.mCitations[pmid];
		}
		Citation* pCita = mNeighbors[pmid];
		res++;
		for (int i = 0; i < pCita->mNumberMesh; ++i)
			meshs.insert(pCita->mMeshHeadingList[i].mDescriptorName.mText);
	}
	return res;
}

int CitationNeighbor::GetNeighborMeshWithRank(set<std::string>& meshs, int rank)
{
	meshs.clear();
	int res = 0;
	for (size_t k = 0; k < rank && k < mNeighborScores.size(); ++k)
	{
		int pmid = mNeighborScores[k].first;
		if (mNeighbors.count(pmid) == 0)
		{
			if (mCitationSet.mCitations.count(pmid) == 0)
			{
				//cerr << "Warning: Can't find pmid " << pmid << endl;
				if (mCntFindPmids.count(pmid) == 0)
					mCntFindPmids[pmid] = 0;
				++mCntFindPmids[pmid];
				continue;
			}
			mNeighbors[pmid] = mCitationSet.mCitations[pmid];
		}
		Citation* pCita = mNeighbors[pmid];
		res++;
		for (int i = 0; i < pCita->mNumberMesh; ++i)
			meshs.insert(pCita->mMeshHeadingList[i].mDescriptorName.mText);
	}
	return res;
}

int CitationNeighbor::GetNeighborPmidWithScore(std::vector<int>& pmids, double score)
{
	pmids.clear();
	for (size_t i = 0; i<mNeighborScores.size(); ++i)
	{
		if (score > mNeighborScores[i].second)
			break;
		pmids.push_back(mNeighborScores[i].first);
	}
	return 0;
}

int CitationNeighbor::GetNeighborPmidWithRank(std::vector<int>& pmids, int rank)
{
	pmids.clear();
	for (size_t i = 0; i < (size_t)rank && i < mNeighborScores.size(); ++i)
		pmids.push_back(mNeighborScores[i].first);
	return 0;
}

int CitationNeighbor::LoadNeighbor(string linkPath)
{
	mNeighborScores.clear();
	if (mPmid == 0)
		return -1;

	ostringstream sout;
	sout << linkPath << "/" << mPmid << ".xml";
	int rtn = LoadRowNeighbor(sout.str().c_str());
	CHECK_RTN(rtn);

	return 0;
}

int CitationNeighbor::LoadRowNeighbor(const char* const fileName)
{
	FILE *inFile = fopen(fileName, "r");
	if (inFile == NULL)
	{
		cerr << "Error: Can't open file " << fileName << endl;
		fclose(inFile);
		return -1;
	}

	int rtn = 0;
	string bracket;
	while ((rtn = NextBracket(inFile, '<', '>', bracket)) == 0)
	if (bracket == "<IdList>") break;

	if (rtn != 0)
	{
		cerr << "Error: Can't Read <IdList> in file " << fileName << endl;
		fclose(inFile);
		return -1;
	}

	if (NextBracket(inFile, '<', '>', bracket) != 0)
	{
		cerr << "Error: Can't Read <> in <IdList> in file " << fileName << endl;
		fclose(inFile);
		return -1;
	}
	if (bracket != "<Id>")
	{
		cerr << "Error: Can't Read <Id> in <IdList>" << fileName << endl;
		fclose(inFile);
		return -1;
	}

	int pmid;
	fscanf(inFile, "%d", &pmid);
	mPmid = pmid;

	bool bFind = false;
	while (NextBracket(inFile, '<', '>', bracket) == 0)
	if (bracket == "<LinkName>")
	{
		char linkName[200];
		fscanf(inFile, "%[^<]", linkName);
		if (strcmp(linkName, "pubmed_pubmed") == 0)
		{
			bFind = true;
			break;
		}
	}
	map<int, int> mapNeib;
	while (bFind && NextBracket(inFile, '<', '>', bracket) == 0)
	{
		if (bracket == "</LinkSetDb>")
			break;
		if (bracket == "<Link>")
		{
			if (NextBracket(inFile, '<', '>', bracket) != 0)
			{
				cerr << "Error: Can't Read <> in <Link>" << fileName << endl;
				fclose(inFile);
				return -1;
			}
			if (bracket != "<Id>")
			{
				cerr << "Error: Can't Read <Id> in <Link>" << fileName << endl;
				fclose(inFile);
				return -1;
			}
			int id, score;
			fscanf(inFile, "%d", &id);

			fscanf(inFile, "%*s");

			if (NextBracket(inFile, '<', '>', bracket) != 0)
			{
				cerr << "Error: Can't Read <> in <Link> after <id>" << fileName << endl;
				fclose(inFile);
				return -1;
			}
			//cout << bracket << endl;
			if (bracket != "<Score>")
			{
				cerr << "Error: Can't Read <Score> in <Link> " << fileName << endl;
				fclose(inFile);
				return -1;
			}
			fscanf(inFile, "%d", &score);
			if (mapNeib.count(id) == 0)
				mapNeib[id] = score;
			else if (mapNeib[id] < score)
				mapNeib[id] = score;
		}
	}
	mNeighborScores.clear();
	for (map<int, int>::iterator it = mapNeib.begin(); it != mapNeib.end(); ++it)
		mNeighborScores.push_back(make_pair(it->first, double(it->second)));
	sort(mNeighborScores.begin(), mNeighborScores.end(), CmpScore);
	fclose(inFile);
	return 0;
}

int CitationNeighbor::RemoveEmptyNeigbbor()
{
	if (mCitationSet.mCitations.empty())
	{
		cerr << "Warning: CitationNeighbor::mCitationSet is empty, not allow remove!" << endl;
		return 0;
	}
	vector<pair<int, double>> leftNeighbors;
	for (size_t i = 0; i < mNeighborScores.size(); ++i)
	{
		if (mCitationSet.mCitations.count(mNeighborScores[i].first) > 0)
			leftNeighbors.push_back(mNeighborScores[i]);
	}
	mNeighborScores.clear();
	mNeighborScores = leftNeighbors;
	return 0;
}

CitationNeighborSet::CitationNeighborSet()
{
	mNeighborSet.clear();
}

int CitationNeighborSet::Load(const char* const linkPath, const vector<int>& vecPmids)
{
	if (linkPath == NULL)
		return -1;
	mNeighborSet.clear();
	int rtn = 0;
	for (unsigned i = 0; i < vecPmids.size(); ++i)
	{
		mNeighborSet[vecPmids[i]].mPmid = vecPmids[i];
		rtn = mNeighborSet[vecPmids[i]].LoadNeighbor(linkPath);
		CHECK_RTN(rtn);
	}
	return 0;
}

int CitationNeighborSet::Load(const char* const linkPath, const char* const testSetPath)
{
	mNeighborSet.clear();
	int rtn = 0;
	TestCitationSet testSet;
	rtn = testSet.LoadTestSet(testSetPath);
	CHECK_RTN(rtn);
	for (size_t i = 0; i < testSet.mTestSet.size(); ++i)
	{
		mNeighborSet[testSet.mTestSet[i].mPmid].mPmid = testSet.mTestSet[i].mPmid;
		rtn = mNeighborSet[testSet.mTestSet[i].mPmid].LoadNeighbor(linkPath);
		CHECK_RTN(rtn);
	}
	return 0;
}

int CitationNeighborSet::Load(const char* const linkPath, const char* const citationSetPath, const vector<int>& vecPmids)
{
	mNeighborSet.clear();
	int rtn = Load(linkPath, vecPmids);
	CHECK_RTN(rtn);
	rtn = CitationNeighbor::LoadCitations(citationSetPath);
	CHECK_RTN(rtn);
	for (map<int, CitationNeighbor>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); ++it)
		it->second.RemoveEmptyNeigbbor();
	return 0;
}

int CitationNeighborSet::Load(const char* const linkPath, const CitationSet& citationSet, const vector<int>& vecPmids)
{
	mNeighborSet.clear();
	int rtn = Load(linkPath, vecPmids);
	CHECK_RTN(rtn);
	CitationNeighbor::mCitationSet = citationSet;
	for (map<int, CitationNeighbor>::iterator it = mNeighborSet.begin(); it != mNeighborSet.end(); ++it)
		it->second.RemoveEmptyNeigbbor();
	return 0;
}

int CitationNeighborSet::GetNeighbor(int queryPmid, int queryRank, double& score, std::vector<std::string>& meshs)
{
	meshs.clear();
	if (mNeighborSet.count(queryPmid) == 0)
	{
		score = -1.0;
		return 0;
	}
	mNeighborSet[queryPmid].GetNeighborMesh(meshs, queryRank);
	mNeighborSet[queryPmid].GetNeighborScore(score, queryRank);
	return 0;
}

int CitationNeighborSet::GetNeighborNum(int queryPmid, int& numNeighbor)
{
	numNeighbor = -1;
	if (mNeighborSet.count(queryPmid) == 0)
		return 0;

	mNeighborSet[queryPmid].GetNeighborNum(numNeighbor);
	return 0;
}

CitationNeighbor* CitationNeighborSet::operator[](int index)
{
	if (mNeighborSet.count(index) == 0)
		return NULL;
	return &mNeighborSet[index];
}

int NextBracket(FILE *pFile, const char left, const char right, std::string& seq)
{
	if (pFile == NULL)
		return -1;
	seq.clear();
	char ch;
	while ((ch = fgetc(pFile)) != EOF)
	if (ch == left) break;
	if (ch == EOF)
		return -1;

	seq += ch;
	int cnt = 1;
	while (cnt != 0)
	{
		if ((ch = fgetc(pFile)) == EOF)
		{
			seq.clear();
			return -1;
		}
		seq += ch;
		if (ch == left)
			++cnt;
		if (ch == right)
			--cnt;
	}
	return 0;
}

int NextBracketRange(const std::string& str, const int startPos, const int endPos, int& leftPos, int& rightPos, const char left, const char right, std::string* pStr)
{
	int cnt = 0;
	bool bLeft = false;
	int i = 0;
	for (i = startPos; i < endPos; ++i)
	{
		if (str[i] == left)
		{
			bLeft = true;
			if (0 == cnt)
				leftPos = i;
			++cnt;
		}
		if (str[i] == right)
		{
			--cnt;
			if (cnt == 0)
			{
				rightPos = i;
				break;
			}
		}
		if (pStr != NULL && bLeft && i != leftPos)
			pStr->push_back(str[i]);
	}
	if (bLeft && cnt == 0)
		return 0;
	else
		return -1;
}

int NextQuotePair(const std::string& bracket, int start, int end, std::string& tar)
{
	bool bBegin = false;
	int cur = start;
	while (cur <= end)
	{
		if (bracket[cur] == '"')
		{
			if (!bBegin)
				bBegin = true;
			else
			{
				if (cur >= end || bracket[cur + 1] != '"')
					break;
				else
				{
					tar.push_back('"');
					++cur;
				}
			}
		}
		if (bBegin && bracket[cur] != '"' && bracket[cur] != '\n' && bracket[cur] != '\r')//
			tar.push_back(bracket[cur]);
		++cur;
	}
	return 0;
}
