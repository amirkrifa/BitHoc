/*

  Copyright (C) 2008  INRIA, Planète Team

  Authors: 
  ------------------------------------------------------------
  Amir Krifa			:  Amir.Krifa@sophia.inria.fr
  Mohamed Karim Sbai	:  Mohamed_Karim.Sbai@sophia.inria.fr
  Chadi Barakat			:  Chadi.Barakat@sophia.inria.fr
  Thierry Turletti		:  Thierry.Turletty@sophia.inria.fr
  ------------------------------------------------------------

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*!
* \file ConfigManager.cpp
* \author Amir Krifa, krifa_amir@hotmail.com
* \brief Implementation of the functions used to manage the configuration file.
**/

#include "StdAfx.h"
#include "ConfigManager.h"
#include <windows.h>
#include <sstream>
#include <fstream>
#include <iostream>

ConfigManager::ConfigManager(string file)
{
	this->sConfigFileAbsoluePath.assign(file);
}

ConfigManager::~ConfigManager(void)
{
	// Clear the Config Map
	this->mConfigMap.clear();
}



bool ConfigManager::LoadConfigFile()
{
	if(this->IsReadable())
	{
		// Config File is readable
		ifstream ConfigFile(this->sConfigFileAbsoluePath.c_str());
		string line;
		while(getline(ConfigFile,line))
		{
			// Parsing The Line
			string sItem;
			string sValue;
			sItem=line.substr(0,line.find_first_of("="));
			sValue=line.substr(line.find_first_of("=")+1);
			this->AddItemToConfigMap(sItem,sValue);
		}
		return true;
	}
	return false;

}
bool ConfigManager::IsReadable()
{
	ifstream file(this->sConfigFileAbsoluePath.c_str());
	return !file.fail();
}
void ConfigManager::AddItemToConfigMap(string &item,string &value)
{
	map<string,string>::iterator ConfigIter=this->mConfigMap.find(item);
	if(ConfigIter==this->mConfigMap.end())
	{
		this->mConfigMap.insert(make_pair<string,string>(item,value));
	}else{
		this->mConfigMap[item]=value;
	}
	this->iNumberOfItems++;
}

string ConfigManager::GetItemValue(string &item)
{
	map<string,string>::iterator ConfigIter=this->mConfigMap.find(item);
	if(ConfigIter!=this->mConfigMap.end())
	{
		// Item Founded
		return ConfigIter->second;
	}else return NULL;

}