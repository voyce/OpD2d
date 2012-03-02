// PreviewThread.cpp : implementation file
//

#include "stdafx.h"
#include "OpD2d.h"
#include "PreviewThread.h"
#include "OpD2dDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern UINT PreviewThreadProc(LPVOID pParam);

/////////////////////////////////////////////////////////////////////////////
// CPreviewThread

IMPLEMENT_DYNCREATE(CPreviewThread, CWinThread)

CPreviewThread::CPreviewThread()
{
}

CPreviewThread::~CPreviewThread()
{
}

BOOL CPreviewThread::InitInstance()
{
	m_wnd.CreateEx(WS_EX_TOOLWINDOW, 
		AfxRegisterWndClass(CS_DBLCLKS, NULL, NULL, NULL), 
		"Preview", 
		WS_VISIBLE|WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, 250, 250, 
		NULL, 0, NULL);
	
	m_data.m_nDeviceID = 0;
	m_data.m_hParentWnd = m_wnd.GetSafeHwnd();
	m_data.m_bStop = false;
	
	m_data.m_format.dwBits = 16;
	m_data.m_format.dwChannels = 2;
	m_data.m_format.dwRate = 44100;
	m_data.m_format.uiBitRate = 192;

	m_wnd.m_pThread = AfxBeginThread( PreviewThreadProc, &m_data);
	return TRUE;
}

int CPreviewThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CPreviewThread, CWinThread)
	//{{AFX_MSG_MAP(CPreviewThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPreviewThread::Stop()
{
	m_data.m_bStop = true;
	AfxEndThread(0);
}

/////////////////////////////////////////////////////////////////////////////
// CPreviewThread message handlers
