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
	// TODO : ajoutez ici du code de construction,
	
	// Placez toutes les initialisations significatives dans InitInstance
}


// Seul et unique objet CWMADHOCTRACKERApp
CWMADHOCTRACKERApp theApp;

// initialisation de CWMADHOCTRACKERApp

BOOL CWMADHOCTRACKERApp::InitInstance()
{
    // SHInitExtraControls doit être appelé une fois lors de l'initialisation de votre application afin d'initialiser
    // l'un des contrôles spécifiques à Windows Mobile, tels que CAPEDIT et SIPPREF.
    SHInitExtraControls();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Initialisation standard
	// Si vous n'utilisez pas ces fonctionnalités et que vous souhaitez réduire la taille
	// de votre exécutable final, vous devez supprimer ci-dessous
	// les routines d'initialisation spécifiques dont vous n'avez pas besoin.
	// Changez la clé de Registre sous laquelle nos paramètres sont enregistrés
	// TODO : modifiez cette chaîne avec des informations appropriées,
	// telles que le nom de votre société ou organisation
	SetRegistryKey(_T("Applications locales générées par AppWizard"));

	CWMADHOCTRACKERDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	
	if (nResponse == IDOK)
	{
		// TODO : placez ici le code définissant le comportement lorsque la boîte de dialogue est
		//  fermée avec OK
	}

	// Lorsque la boîte de dialogue est fermée, retourner FALSE afin de quitter
	//  l'application, plutôt que de démarrer la pompe de messages de l'application.
	return FALSE;
}
