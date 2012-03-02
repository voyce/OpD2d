#if !defined(AFX_PREVIEWWND_H__3D33B1E4_9FF1_43EF_94BC_A8B13F188D40__INCLUDED_)
#define AFX_PREVIEWWND_H__3D33B1E4_9FF1_43EF_94BC_A8B13F188D40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewWnd.h : header file
//

#define WM_RECORDBUFFER WM_USER+0x234
#define WM_PREVIEWSTOP WM_USER+0x235

/////////////////////////////////////////////////////////////////////////////
// CPreviewWnd window

class CPreviewWnd : public CWnd
{
// Construction
public:
	CPreviewWnd();
	CWinThread *m_pThread;
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPreviewWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPreviewWnd)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	LRESULT OnRecordBuffer(WPARAM wParam, LPARAM lParam);
	WORD *m_pBuffer;
	DWORD m_dwBufferLen;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWWND_H__3D33B1E4_9FF1_43EF_94BC_A8B13F188D40__INCLUDED_)
