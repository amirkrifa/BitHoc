/*

  Copyright (C) 2008  INRIA, Plan�te Team

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
* \file WM_ADHOC_TRACKER_MENU.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _WM_ADHOC_TRACKER_MENU_H
#define _WM_ADHOC_TRACKER_MENU_H

#pragma once



// WM_ADHOC_TRACKER_MENU dialog

class WM_ADHOC_TRACKER_MENU : public CDialog
{
	DECLARE_DYNAMIC(WM_ADHOC_TRACKER_MENU)

public:
	WM_ADHOC_TRACKER_MENU(CWnd* pParent);   // standard constructor
	virtual ~WM_ADHOC_TRACKER_MENU();
	

// Dialog Data
	enum { IDD = IDD_POCKETPC_PORTRAIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnBnClickedButtonPeersList();
public:
	afx_msg void OnBnClickedButtonTorrentsList();
public:
	afx_msg void OnBnClickedButton3();
public:
	afx_msg void OnEnChangeEditTrackerStatus();
public:
	afx_msg void OnBnClickedButtonTrackerStatus();
public:
	CString CString_TS;
public:
	afx_msg void OnBnClickedButtonPing();
};

#endif _WM_ADHOC_TRACKER_MENU_H