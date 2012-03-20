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
* \file WM_ADHOC_TRACKER_PEERS_LIST.cpp
* \brief Implementation of the Dialog class used to get the current peers list managed by the tracker.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
#include "WM-ADHOC-TRACKER.h"
#include "WM_ADHOC_TRACKER_PEERS_LIST.h"
#include "WM_ADHOC_TRACKER_MENU.h"
#include "PeersManager.h"
#include "WM-ADHOC-TRACKERDlg.h"
#include "TorrentManager.h"

// WM_ADHOC_TRACKER_PEERS_LIST dialog

IMPLEMENT_DYNAMIC(WM_ADHOC_TRACKER_PEERS_LIST, CDialog)

WM_ADHOC_TRACKER_PEERS_LIST::WM_ADHOC_TRACKER_PEERS_LIST(CWnd* pParent /*=NULL*/)
	: CDialog(WM_ADHOC_TRACKER_PEERS_LIST::IDD, pParent)
	, CString_Peers_List(_T(""))
{
		
}

WM_ADHOC_TRACKER_PEERS_LIST::~WM_ADHOC_TRACKER_PEERS_LIST()
{
}

void WM_ADHOC_TRACKER_PEERS_LIST::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PEERS_LIST, CString_Peers_List);
}


BEGIN_MESSAGE_MAP(WM_ADHOC_TRACKER_PEERS_LIST, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GET_PEERS_LIST, &WM_ADHOC_TRACKER_PEERS_LIST::OnBnClickedButtonGetPeersList)
END_MESSAGE_MAP()


// WM_ADHOC_TRACKER_PEERS_LIST message handlers

void WM_ADHOC_TRACKER_PEERS_LIST::OnBnClickedButtonGetPeersList()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_MENU * papa=(WM_ADHOC_TRACKER_MENU * )this->GetParent();
	
	std::string pl;
	CWMADHOCTRACKERDlg * gp= (CWMADHOCTRACKERDlg * )papa->GetParent();
	
	gp->ptr_tm->DeleteOutOfDatePeers();
	pl.assign(gp->ptr_pm->get_peers_list());
		if(pl.length()>0)
			CString_Peers_List=CString(pl.c_str());
		else 
			CString_Peers_List=CString("Peers Number = 0 !")+CString("\r\n");
	
	UpdateData(FALSE);
}
