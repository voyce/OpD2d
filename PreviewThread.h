#if !defined(AFX_PREVIEWTHREAD_H__C891D711_A927_4570_A0E6_DFF6E1C80C03__INCLUDED_)
#define AFX_PREVIEWTHREAD_H__C891D711_A927_4570_A0E6_DFF6E1C80C03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewThread.h : header file
//

#include "PreviewWnd.h"
#include "OpD2dDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CPreviewThread thread

class CPreviewThread : public CWinThread
{
	DECLARE_DYNCREATE(CPreviewThread)
protected:
	CPreviewThread();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	void Stop();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPreviewThread();

	// Generated message map functions
	//{{AFX_MSG(CPreviewThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	CPreviewWnd m_wnd;
	CThreadInfo m_data;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWTHREAD_H__C891D711_A927_4570_A0E6_DFF6E1C80C03__INCLUDED_)
