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
* \file ConfigManager.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \brief Class manage the configuration file.
**/

#ifndef _CONFIG_MANAGER_H
#define _CONFIG_MANAGER_H

#pragma once

#include <string>
#include <map>
using namespace std;


class ConfigManager
{
public:
	ConfigManager(string sConfigFileAbsoluePath);
	~ConfigManager(void);
	bool LoadConfigFile();
	void AddItemToConfigMap(string &item,string &value);
	string GetItemValue(string &item);
	bool IsReadable();
private:
	map<string,string> mConfigMap;
	string sConfigFileAbsoluePath;
	int iNumberOfItems;

};

#endif _CONFIG_MANAGER_H