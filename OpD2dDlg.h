// OpD2dDlg.h : header file
//

#if !defined(AFX_OPD2DDLG_H__55A7D52C_A2C7_482F_9471_D16D0E09F002__INCLUDED_)
#define AFX_OPD2DDLG_H__55A7D52C_A2C7_482F_9471_D16D0E09F002__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>
#include "PopupWnd.h"
#include "SampleFormatDlg.h"

class CThreadInfo
{
public:
	CThreadInfo():m_nDeviceID(0),m_bStop(true),m_hParentWnd(NULL),m_szFilename(0),m_hwi(0)
		{}
	UINT m_nDeviceID;
	bool m_bStop;
	HWND m_hParentWnd;
	LPCTSTR m_szFilename;
	HWAVEIN m_hwi;
	CFormat m_format;
};

#define WM_RECORDTIME WM_USER+1
#define WM_RECORDABORT WM_USER+2

/////////////////////////////////////////////////////////////////////////////
// COpD2dDlg dialog

class COpD2dDlg : public CDialog
{
// Construction
public:
	COpD2dDlg(CWnd* pParent = NULL);	// standard constructor
	static HRGN BitmapToRegion(HBITMAP hBmp);

	void EnableAutoMode() { m_bAutoMode = true; }
	void SetCmdlineSizeDuration(TCHAR *szArg);
	void SetCmdlineFilename(TCHAR *szArg);

// Dialog Data
	//{{AFX_DATA(COpD2dDlg)
	enum { IDD = IDD_OPD2D_DIALOG };
	CString	m_strPath;
	CString	m_strProgress;
	DWORD	m_dwLength;
	int		m_nUnits;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpD2dDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL CheckDiskSpace(DWORD dwRecordingSize, unsigned __int64 *piFree);
	HICON m_hIcon;
	HBITMAP m_hBmp;
	
	CString m_strFilenameArg;
	CString m_strSizeDurationArg;
	bool m_bAutoMode;

	DWORD m_dwLengthMinutes;
	DWORD m_dwLengthBytes;
	CTime m_timeStart;

	CThreadInfo m_data;
	HANDLE m_hThread;

	bool m_bShowHelp;
	CPopupWnd m_wndHelp;
	CSampleFormatDlg m_dlgFormat;

	// Generated message map functions
	//{{AFX_MSG(COpD2dDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnRecordBtn();
	afx_msg void OnStopBtn();
	afx_msg void OnUpdateButtons(CCmdUI *pCmdUI);
	afx_msg void OnBrowseBtn();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelendokUnitsCombo();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHelpBtn();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnFormatButton();
	afx_msg void OnPreviewBtn();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnRecordTime(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRecordAbort(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	EFormat m_format;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPD2DDLG_H__55A7D52C_A2C7_482F_9471_D16D0E09F002__INCLUDED_)
