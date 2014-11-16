/* 
 * File:   tools.h
 * Author: Aris Kosmopoulos
 *
 * Created on November 9, 2011, 4:03 PM
 */

#ifndef TOOLS_H
#define	TOOLS_H
#include <vector>
#include <set>
#include <sstream>
#include <iostream>
//using namespace std;

static std::vector<std::string> split(std::string line, char s=' ') {
  std::vector<std::string> ret;
  std::string word = "";
  for(size_t i=0;i<line.length();i++)
  {
    if (line[i] == s)
    {
      ret.push_back(word);
      word = "";
    }
    else
    {
      word += line[i];
    }
  }
  if (word != "")
    ret.push_back(word);
  return ret;
}

static int stringToInt (std::string s) {
  std::stringstream ss;
  ss << s;
  int ret;
  ss >> ret;
  return ret;
}

static std::string intToString (int i) {
  std::stringstream ss;
  ss << i;
  std::string ret;
  ss >> ret;
  return ret;
}
static std::set<int> fillSet (std::string line) {
    std::vector<std::string> v = split(line);
    std::set<int> ret;
    for (size_t i=0;i<v.size();i++)
        ret.insert(stringToInt(v[i]));
    return ret;
}

static std::set<int> getSubsetMinusCommon (std::set<int>& target, std::set<int>& other) {
    std::set<int>::iterator t_iter, o_iter;
    std::set<int> ret;
    for (t_iter=target.begin();t_iter!=target.end();t_iter++) {
        int n = *t_iter;
        o_iter =  other.find(n);
        if (o_iter == other.end())
            ret.insert(n);
    }
    return ret;
}
static std::set<int> addSets (std::set<int>& s1, std::set<int>& s2) {
    std::set<int> ret;
    if (s1.size() > s2.size()) {
        ret = s1;
        std::set<int>::iterator s_iter;
        for (s_iter=s2.begin();s_iter!=s2.end();s_iter++)
                ret.insert(*s_iter);
    }
    else {
        ret = s2;
        std::set<int>::iterator s_iter;
        for (s_iter=s1.begin();s_iter!=s1.end();s_iter++)
                ret.insert(*s_iter);
    }
    return ret;
}

static std::set<int> getIntrOfSets (std::set<int>& s1, std::set<int>& s2) {
    std::set<int> ret;
    std::set<int>::iterator s_iter1,s_iter2;
    for (s_iter1=s1.begin();s_iter1!=s1.end();s_iter1++) {
        s_iter2 = s2.find(*s_iter1);
        if (s_iter2 != s2.end())
                ret.insert(*s_iter2);
    }
    return ret;
}


static void printSet (std::set<int>& s) {
    std::set<int>::iterator iter;
    for (iter=s.begin();iter!=s.end();iter++)
        std::cout << *iter << std::endl;
}

static void printVector (std::vector<int>& v) {    
    for (size_t i=0;i!=v.size();i++)
        std::cout << "   " << v[i] << std::endl;
}
#endif	/* TOOLS_H */

