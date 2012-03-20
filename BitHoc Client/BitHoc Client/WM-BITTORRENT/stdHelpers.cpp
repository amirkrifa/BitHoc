
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
* \file stdHelpers.cpp
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#include "stdafx.h"
#include "stdHelpers.h"
#include <algorithm>
#include <cctype>

std::string ReplaceInStr(const std::string& in, const std::string& search_for, const std::string& replace_with) {

  std::string ret = in;

  std::string::size_type pos = ret.find(search_for);

  while (pos != std::string::npos) {
    ret = ret.replace(pos, search_for.size(), replace_with);

    pos =  pos - search_for.size() + replace_with.size() + 1;

    pos = ret.find(search_for, pos);
  }

  return ret;
}

void ToUpper(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), toupper);
}

void ToLower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), tolower);
}
