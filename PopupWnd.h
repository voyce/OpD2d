#if !defined(AFX_POPUPWND_H__C881E45D_17D7_49EC_83FA_A60A16F8EC52__INCLUDED_)
#define AFX_POPUPWND_H__C881E45D_17D7_49EC_83FA_A60A16F8EC52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PopupWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupWnd window

class CPopupWnd : public CWnd
{
// Construction
public:
	CPopupWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPopupWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPopupWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg UINT OnNcHitTest(CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	HBITMAP m_hBmp;
	CFont m_font;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPWND_H__C881E45D_17D7_49EC_83FA_A60A16F8EC52__INCLUDED_)
