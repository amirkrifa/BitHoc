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
* \file WM_ADHOC_TRACKER_TORRENTS_LIST.cpp
* \brief Implementation of the Torrents List Dialog class.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
#include "WM-ADHOC-TRACKER.h"
#include "WM_ADHOC_TRACKER_TORRENTS_LIST.h"
#include "WM-ADHOC-TRACKERDlg.h"
#include "TorrentManager.h"
#include "WM_ADHOC_TRACKER_MENU.h"

// WM_ADHOC_TRACKER_TORRENTS_LIST dialog

IMPLEMENT_DYNAMIC(WM_ADHOC_TRACKER_TORRENTS_LIST, CDialog)

WM_ADHOC_TRACKER_TORRENTS_LIST::WM_ADHOC_TRACKER_TORRENTS_LIST(CWnd* pParent /*=NULL*/)
	: CDialog(WM_ADHOC_TRACKER_TORRENTS_LIST::IDD, pParent)
	, CString_Torrents_List(_T(""))
{

}

WM_ADHOC_TRACKER_TORRENTS_LIST::~WM_ADHOC_TRACKER_TORRENTS_LIST()
{
}

void WM_ADHOC_TRACKER_TORRENTS_LIST::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TORRENTS_LIST, CString_TL);
	DDX_Text(pDX, IDC_EDIT_TORRENTS_LIST, CString_Torrents_List);
}


BEGIN_MESSAGE_MAP(WM_ADHOC_TRACKER_TORRENTS_LIST, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GET_TORRENTS_LIST, &WM_ADHOC_TRACKER_TORRENTS_LIST::OnBnClickedButtonGetTorrentsList)
END_MESSAGE_MAP()


// WM_ADHOC_TRACKER_TORRENTS_LIST message handlers

void WM_ADHOC_TRACKER_TORRENTS_LIST::OnBnClickedButtonGetTorrentsList()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_MENU * papa=(WM_ADHOC_TRACKER_MENU * )this->GetParent();
	
	std::string tl;
	CWMADHOCTRACKERDlg * gp= (CWMADHOCTRACKERDlg * )papa->GetParent();
	tl.assign(gp->ptr_tm->get_torrents_list());
		if(tl.length()>0)
			CString_Torrents_List=CString(tl.c_str());
		else 
			CString_Torrents_List=CString("Torrents Number = 0 !")+CString("\r\n");
	
	UpdateData(FALSE);

}

