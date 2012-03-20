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
* \file WM_ADHOC_TRACKER_MENU.cpp
* \brief Implementation of the Dialog class used to manage the tracker configuration dialog.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
#include "WM-ADHOC-TRACKER.h"
#include "WM_ADHOC_TRACKER_MENU.h"
#include "WM_ADHOC_TRACKER_STATUS.h"
#include "WM_ADHOC_TRACKER_PEERS_LIST.h"
#include "WM_ADHOC_TRACKER_TORRENTS_LIST.h"
#include "WM-ADHOC-TRACKERDlg.h"
#include "WM_ADHOC_TRACKER_PING.h"


// WM_ADHOC_TRACKER_MENU dialog

IMPLEMENT_DYNAMIC(WM_ADHOC_TRACKER_MENU, CDialog)

WM_ADHOC_TRACKER_MENU::WM_ADHOC_TRACKER_MENU(CWnd* pParent /*=NULL*/)
	: CDialog(WM_ADHOC_TRACKER_MENU::IDD, pParent)
	, CString_TS(_T(""))
{
	//pDlg=(CWMADHOCTRACKERDlg * )pParent;

}

WM_ADHOC_TRACKER_MENU::~WM_ADHOC_TRACKER_MENU()
{

}

void WM_ADHOC_TRACKER_MENU::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TS, CString_TS);
}


BEGIN_MESSAGE_MAP(WM_ADHOC_TRACKER_MENU, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &WM_ADHOC_TRACKER_MENU::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_PEERS_LIST, &WM_ADHOC_TRACKER_MENU::OnBnClickedButtonPeersList)
	ON_BN_CLICKED(IDC_BUTTON_TORRENTS_LIST, &WM_ADHOC_TRACKER_MENU::OnBnClickedButtonTorrentsList)
	ON_BN_CLICKED(IDC_BUTTON_TRACKER_STATUS, &WM_ADHOC_TRACKER_MENU::OnBnClickedButtonTrackerStatus)
	ON_BN_CLICKED(IDC_BUTTON_PING, &WM_ADHOC_TRACKER_MENU::OnBnClickedButtonPing)
END_MESSAGE_MAP()


// WM_ADHOC_TRACKER_MENU message handlers

void WM_ADHOC_TRACKER_MENU::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
WM_ADHOC_TRACKER_STATUS routing_trace(this);
routing_trace.DoModal();
}

void WM_ADHOC_TRACKER_MENU::OnBnClickedButtonPeersList()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_PEERS_LIST PL(this);
	PL.DoModal();
}

void WM_ADHOC_TRACKER_MENU::OnBnClickedButtonTorrentsList()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_TORRENTS_LIST tl(this);
	tl.DoModal();	
}



void WM_ADHOC_TRACKER_MENU::OnEnChangeEditTrackerStatus()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void WM_ADHOC_TRACKER_MENU::OnBnClickedButtonTrackerStatus()
{
	// TODO: Add your control notification handler code here
	
	CWMADHOCTRACKERDlg * papa=(CWMADHOCTRACKERDlg * )this->GetParent();
	
	CString_TS=CString(papa->get_tracker_state().c_str());
	UpdateData(FALSE);
}

void WM_ADHOC_TRACKER_MENU::OnBnClickedButtonPing()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_PING ping_dlg;
	ping_dlg.DoModal();
}
