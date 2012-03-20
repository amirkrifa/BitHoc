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
* \file WM_ADHOC_TRACKERDlg.cpp
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
#include "WM-ADHOC-TRACKER.h"
#include "WM-ADHOC-TRACKERDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWMADHOCTRACKERApp

BEGIN_MESSAGE_MAP(CWMADHOCTRACKERApp, CWinApp)
END_MESSAGE_MAP()


// construction CWMADHOCTRACKERApp
CWMADHOCTRACKERApp::CWMADHOCTRACKERApp()
	: CWinApp()
{
	// TODO�: ajoutez ici du code de construction,
	
	// Placez toutes les initialisations significatives dans InitInstance
}


// Seul et unique objet CWMADHOCTRACKERApp
CWMADHOCTRACKERApp theApp;

// initialisation de CWMADHOCTRACKERApp

BOOL CWMADHOCTRACKERApp::InitInstance()
{
    // SHInitExtraControls doit �tre appel� une fois lors de l'initialisation de votre application afin d'initialiser
    // l'un des contr�les sp�cifiques � Windows Mobile, tels que CAPEDIT et SIPPREF.
    SHInitExtraControls();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Initialisation standard
	// Si vous n'utilisez pas ces fonctionnalit�s et que vous souhaitez r�duire la taille
	// de votre ex�cutable final, vous devez supprimer ci-dessous
	// les routines d'initialisation sp�cifiques dont vous n'avez pas besoin.
	// Changez la cl� de Registre sous laquelle nos param�tres sont enregistr�s
	// TODO : modifiez cette cha�ne avec des informations appropri�es,
	// telles que le nom de votre soci�t� ou organisation
	SetRegistryKey(_T("Applications locales g�n�r�es par AppWizard"));

	CWMADHOCTRACKERDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	
	if (nResponse == IDOK)
	{
		// TODO�: placez ici le code d�finissant le comportement lorsque la bo�te de dialogue est
		//  ferm�e avec OK
	}

	// Lorsque la bo�te de dialogue est ferm�e, retourner FALSE afin de quitter
	//  l'application, plut�t que de d�marrer la pompe de messages de l'application.
	return FALSE;
}
