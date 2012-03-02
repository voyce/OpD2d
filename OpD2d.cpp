// OpD2d.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OpD2d.h"
#include "OpD2dDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpD2dApp

BEGIN_MESSAGE_MAP(COpD2dApp, CWinApp)
	//{{AFX_MSG_MAP(COpD2dApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpD2dApp construction

COpD2dApp::COpD2dApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only COpD2dApp object

COpD2dApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COpD2dApp initialization

BOOL COpD2dApp::InitInstance()
{
	SetRegistryKey(_T("Opcode"));
	AfxEnableControlContainer();

	COpD2dDlg dlg;
	m_pMainWnd = &dlg;

	if (_tcslen(m_lpCmdLine) != 0)
	{
		// Parse command line
		enum ArgState
		{
			Filename, Size, Done
		};
		ArgState state = Filename;
		TCHAR *szArg = _tcstok(m_lpCmdLine, _T(" "));
		while (szArg)
		{
			switch(state)
			{
			case Filename:
				dlg.SetCmdlineFilename(szArg);
				state = Size;
				break;
			case Size:
				dlg.SetCmdlineSizeDuration(szArg);
				state = Done;
				break;
			default:
				AfxMessageBox("Invalid command line arguments. Expected filename and recording size/duration.");
				return FALSE;
			}
			if (state == Done)
			{
				dlg.EnableAutoMode();
				break;
			}
			szArg = _tcstok(NULL, _T(" "));
		}
	}

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
