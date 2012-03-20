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
* \file WM_ADHOC_TRACKER_STATUS.cpp
* \brief Implementation of the Dialog class used to get the current Tracker status.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
#include "WM-ADHOC-TRACKER.h"
#include "WM_ADHOC_TRACKER_STATUS.h"
#include "WM_ADHOC_TRACKER_MENU.h"
#include "WMRoutingTable.h"
#include "WM-ADHOC-TRACKERDlg.h"
// WM_ADHOC_TRACKER_STATUS dialog

IMPLEMENT_DYNAMIC(WM_ADHOC_TRACKER_STATUS, CDialog)

WM_ADHOC_TRACKER_STATUS::WM_ADHOC_TRACKER_STATUS(CWnd* pParent /*=NULL*/)
	: CDialog(WM_ADHOC_TRACKER_STATUS::IDD, pParent)
	, CString_RT(_T(""))
{

}

WM_ADHOC_TRACKER_STATUS::~WM_ADHOC_TRACKER_STATUS()
{
}

void WM_ADHOC_TRACKER_STATUS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ROUTING_TABLE, CString_RT);
}


BEGIN_MESSAGE_MAP(WM_ADHOC_TRACKER_STATUS, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &WM_ADHOC_TRACKER_STATUS::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_GET_ROUTING_TABLE, &WM_ADHOC_TRACKER_STATUS::OnBnClickedButtonGetRoutingTable)
END_MESSAGE_MAP()


// WM_ADHOC_TRACKER_STATUS message handlers

void WM_ADHOC_TRACKER_STATUS::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	
}

void WM_ADHOC_TRACKER_STATUS::OnBnClickedButtonGetRoutingTable()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_MENU * papa=(WM_ADHOC_TRACKER_MENU * )this->GetParent();
	
		std::string rt;
		CWMADHOCTRACKERDlg * gp= (CWMADHOCTRACKERDlg * )papa->GetParent();
	
		rt.assign(gp->ptr_wmrt->get_routing_table_from_the_map());
		if(rt.length()>0)
			CString_RT=CString(rt.c_str());
		else 
			CString_RT=CString("ERROR while Reading the routing table!")+CString("\r\n");
	
	UpdateData(FALSE);
}
