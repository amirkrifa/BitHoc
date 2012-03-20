#include "StdAfx.h"
#include "BlackIpList.h"

void BlackIpList::InitBlackList(string rl)
{
	// Format Ip1#Ip2#Ip3#Ip4
	if(rl.length() > 0)
	{
		int index = rl.find_first_of('#');
		if(index == -1 && rl.length() > 1)
		{
			// Just one Ip @
			blackList.push_back(rl);
		}else if(index != -1)
		{
			string ip;
			while(index != -1 || rl.length() > 1 )
			{
				if(index != -1)
				{
					ip = rl.substr(0,index);
					blackList.push_back(ip);
					rl = rl.substr(index+1);
					index = rl.find_first_of('#');
				}else
				{
					// The last Ip in the list
					ip = rl;
					blackList.push_back(ip);
					break;
				}
			}
		}
	}else
	{
		blackList.clear();
		tmpBlackList.clear();
	}
}

string BlackIpList::GetFormattedList()
{
	formattedList.clear();
	if(tmpBlackList.empty() && blackList.empty())
		return string("");

	// Merge the two lists
	list<string>::iterator mIter = tmpBlackList.begin();
	while(mIter!=tmpBlackList.end())
	{
		blackList.push_back(*mIter);
		mIter ++;
	}
	tmpBlackList.clear();

	if(blackList.size() > 0)
	{
		list<string>::iterator iter = blackList.begin();
		while(iter != blackList.end())
		{
			formattedList.append(*iter);
			iter++;
			if(iter != blackList.end())
				formattedList.append("#");
			
		}
		
		return formattedList;

	}else 
	{
		return string("");
	}
}
