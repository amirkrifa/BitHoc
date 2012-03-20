
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

// WM-ADHOC-TRACKER.h : fichier d'en-tête principal pour l'application PROJECT_NAME
//
/*!
* \file WM-ADHOC-TRACKER.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/


#ifndef _WM_ADHOC_TRACKER_H
#define _WM_ADHOC_TRACKER_H

#pragma once

#ifndef __AFXWIN_H__
	#error "incluez 'stdafx.h' avant d'inclure ce fichier pour PCH"
#endif

#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 

// CWMADHOCTRACKERApp:
// Consultez WM-ADHOC-TRACKER.cpp pour l'implémentation de cette classe
//

class CWMADHOCTRACKERApp : public CWinApp
{
public:
	CWMADHOCTRACKERApp();
	
// Substitutions
public:
	virtual BOOL InitInstance();

// Implémentation

	DECLARE_MESSAGE_MAP()
};

extern CWMADHOCTRACKERApp theApp;

#endif _WM_ADHOC_TRACKER_H