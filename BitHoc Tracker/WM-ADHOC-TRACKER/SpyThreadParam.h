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
* \file SpyThreadParam.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _SPY_THREAD_PARAM_H
#define _SPY_THREAD_PARAM_H
#pragma once
#include <string>

using namespace std;

class WMRoutingTable;
class NeighborsManager;
/*!
* \class SpyThreadParam
* \brief Class used to transmit the input from the tracker to the thread which is tracking changes in the device routing table and the output from this thread to the tracker. 
*/

class SpyThreadParam
{
public:
	SpyThreadParam(WMRoutingTable * rt);
	/*!
	* \fn bool get_rt_state()
	* \brief Function used to return the device routing table status. 
	*/
	inline bool get_rt_state(){return rt_was_modified;};
	/*!
	* \fn void initiate_rt_state()
	* \brief Method used to initiate the routing table status. 
	*/
	inline void initiate_rt_state(){rt_was_modified=false;};
	/*!
	* \fn void rt_is_modified()
	* \brief Method used to track and report any changes in the device routing table. 
	*/
	void rt_is_modified();
	/*!
	* \fn void set_tracking_error(char * er)
	* \brief Method used to report any error that occurs while tracker the device routing table. 
	*/
	void set_tracking_error(char * er){this->error.assign(er);};
	WMRoutingTable * wmrt;
	NeighborsManager * ptr_nm;
	~SpyThreadParam(void);
private:

	/*!
	* \brief Used to indicate the routing table status. 
	*/
	bool rt_was_modified;
	/*!
	* \brief Used to indicate if there was an error while tracking changes in the routing table. 
	*/
	std::string error;
};

#endif _SPY_THREAD_PARAM_H