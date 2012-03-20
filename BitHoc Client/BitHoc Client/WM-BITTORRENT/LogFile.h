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
* \file LogFile.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#ifndef _LogFile_H
#define _LogFile_H
#pragma once
#include <string>

using namespace std;
/*!
* \class LogFile
* \brief Used by the tracker to update and manage generated log files.
*/
class LogFile
{
public:
	LogFile(std::string FileName);
	/*!
	* \fn void add_line(std::string line, bool SetTime)
	* \brief Method used to add a line to the log file.
	*/
	void add_line(std::string line, bool SetTime);
	/*!
	* \fn void close()
	* \brief Method used to close a log file.
	*/
	void close();
	/*!
	* \fn bool file_exists(string file_name)
	* \brief Function used to verify if the log file already exists.
	*/
	bool file_exists(string file_name);
	~LogFile(void);

private:
	/*!
	* \brief A ptr to the log file.
	*/
	FILE * log_file;
	/*!
	* \brief The log file name.
	*/
	string file_name;
	/*!
	* \brief Critical session associated to the log file.
	*/	
	CRITICAL_SECTION log_sync;
};
#endif _LogFile_H
