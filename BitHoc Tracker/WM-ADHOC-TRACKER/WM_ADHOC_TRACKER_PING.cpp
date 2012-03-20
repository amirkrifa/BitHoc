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
* \file WM_ADHOC_TRACKER_PING.cpp
* \brief Implementation of the Dialog class used to send, receive and interpret a ping request to a given destination.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
#include "WM-ADHOC-TRACKER.h"
#include "WM_ADHOC_TRACKER_PING.h"
#include "Ping.h"
#include <string>
#include <sstream>
#include <iostream>
#include "TuningFunctions.h"


typedef std::basic_string<TCHAR> tstring;  
  

// WM_ADHOC_TRACKER_PING dialog

IMPLEMENT_DYNAMIC(WM_ADHOC_TRACKER_PING, CDialog)

WM_ADHOC_TRACKER_PING::WM_ADHOC_TRACKER_PING(CWnd* pParent /*=NULL*/)
	: CDialog(WM_ADHOC_TRACKER_PING::IDD, pParent)
	, CString_IP(_T(""))
	, CString_PR(_T(""))
{

}

WM_ADHOC_TRACKER_PING::~WM_ADHOC_TRACKER_PING()
{
}

void WM_ADHOC_TRACKER_PING::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_IP, CString_IP);
	DDX_Text(pDX, IDC_EDIT_PING_RESULT, CString_PR);
}


BEGIN_MESSAGE_MAP(WM_ADHOC_TRACKER_PING, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEND_PING, &WM_ADHOC_TRACKER_PING::OnBnClickedButtonSendPing)
END_MESSAGE_MAP()



// WM_ADHOC_TRACKER_PING message handlers


void WM_ADHOC_TRACKER_PING::OnBnClickedButtonSendPing()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	Ping send_ping;
	string ip;
	ip=toStlString(CString_IP);
	string pr(send_ping.send_a_ping((char *)ip.c_str()));
	CString_PR=pr.c_str();
	UpdateData(FALSE);
}
