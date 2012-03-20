#pragma once
#ifndef _BLACK_IP_LIST_
#define _BLACK_IP_LIST_
#include <list>
using namespace std;

class BlackIpList
{
public:
	BlackIpList(void)
	{

	}
	~BlackIpList(void)
	{
		if(blackList.size())
			blackList.clear();
		if(tmpBlackList.size())
			tmpBlackList.clear();
	}
	
	void AddIpToBlackList(string & ip)
	{
		// Verify in the main black List
		list<string>::iterator iter = blackList.begin();
		bool found = false;
		while(iter != blackList.end())
		{
			if(ip.compare(*iter) == 0)
			{
				found = true;
				break;
			}
			iter++;
		}

		// Verify in the tmpBlackList
		list<string>::iterator tmpIter = tmpBlackList.begin();
		bool tmpFound = false;
		while(tmpIter != tmpBlackList.end())
		{
			if(ip.compare(*tmpIter) == 0)
			{
				tmpFound = true;
				break;
			}
			tmpIter++;
		}

		if(!found && !tmpFound)
		{
			// Add the ip
			tmpBlackList.push_back(ip);
		}
	}

	bool IsIpInTheBlackList(string ip)
	{
		if(blackList.empty())
			return false;
		list<string>::iterator iter = blackList.begin();
		bool found = false;
		while(iter != blackList.end())
		{
			if(ip.compare(*iter) == 0)
			{
				found = true;
				break;
			}
			iter++;
		}

		return found;
	}

	void InitBlackList(string receivedList);
	string GetFormattedList();
	
	BlackIpList * Clone()
	{
		BlackIpList * tmp = new BlackIpList();
		tmp->CopyBlackList(this->blackList);
		tmp->CopyTmpBlackList(this->tmpBlackList);
		return tmp;
	}



private:
	void CopyBlackList(list<string> &l)
	{
		this->blackList.clear();
		list<string>::iterator iter = l.begin();
		while(iter != l.end())
		{
			this->blackList.push_back(*iter);
			iter++;
		}
	}
	void CopyTmpBlackList(list<string> &l)
	{
		this->tmpBlackList.clear();
		list<string>::iterator iter = l.begin();
		while(iter != l.end())
		{
			this->tmpBlackList.push_back(*iter);
			iter++;
		}
	}
	
	list<string> blackList;
	list<string> tmpBlackList;
	string formattedList;
};

#endif _BLACK_IP_LIST_