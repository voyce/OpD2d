#if !defined(AFX_SAMPLEFORMATDLG_H__32D43E98_2FC8_45F8_8A50_C867C3B627D0__INCLUDED_)
#define AFX_SAMPLEFORMATDLG_H__32D43E98_2FC8_45F8_8A50_C867C3B627D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SampleFormatDlg.h : header file
//

enum EFormat
{
	WAV, MP3
};

struct CFormat
{
	CFormat():format(MP3),uiBitRate(192)
	{
		HINSTANCE hBladeDLL = LoadLibrary("bladeenc.dll");
		if(!hBladeDLL)	
			format = WAV;
	}
	EFormat format;
	USHORT uiBitRate;

	DWORD dwRate;
	DWORD dwChannels;
	DWORD dwBits;
	DWORD SamplesPerMinute()
	{
		return (dwRate * dwChannels * (dwBits/8)) * 60;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CSampleFormatDlg dialog

class CSampleFormatDlg : public CDialog
{
// Construction
public:
	CSampleFormatDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSampleFormatDlg)
	enum { IDD = IDD_FORMAT_DIALOG };
	int		m_comboRate;
	int		m_comboChannels;
	int		m_comboBits;
	CString	m_bitrate;
	CString	m_version;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSampleFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void GetEncoderVersion();

	// Generated message map functions
	//{{AFX_MSG(CSampleFormatDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CFormat m_format;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAMPLEFORMATDLG_H__32D43E98_2FC8_45F8_8A50_C867C3B627D0__INCLUDED_)
