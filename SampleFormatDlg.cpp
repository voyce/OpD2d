// SampleFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OpD2d.h"
#include "SampleFormatDlg.h"
#include "bladedll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSampleFormatDlg dialog


CSampleFormatDlg::CSampleFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSampleFormatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSampleFormatDlg)
	m_comboRate = -1;
	m_comboChannels = -1;
	m_comboBits = -1;
	m_bitrate = _T("");
	m_version = _T("");
	//}}AFX_DATA_INIT
}


void CSampleFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSampleFormatDlg)
	DDX_CBIndex(pDX, IDC_RATE_COMBO, m_comboRate);
	DDX_CBIndex(pDX, IDC_CHANNELS_COMBO, m_comboChannels);
	DDX_CBIndex(pDX, IDC_BITS_COMBO, m_comboBits);
	DDX_CBString(pDX, IDC_MP3_BITRATE_COMBO, m_bitrate);
	DDX_Text(pDX, IDC_MP3_VERSION_TEXT, m_version);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSampleFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CSampleFormatDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSampleFormatDlg message handlers

BOOL CSampleFormatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	switch (m_format.dwRate)
	{
	case 44100:
		m_comboRate = 0;
		break;
	case 22050:
		m_comboRate = 1;
		break;
	case 11025:
		m_comboRate = 2;
		break;
	}
	m_comboBits = (m_format.dwBits/8)-1;
	m_comboChannels = m_format.dwChannels-1;
	
	m_version = "Unknown";

	GetEncoderVersion();

	CheckRadioButton(IDC_WAV_RADIO, IDC_MP3_RADIO,
		m_format.format == WAV?IDC_WAV_RADIO:IDC_MP3_RADIO);

	char sz[32];
	m_bitrate = ltoa( m_format.uiBitRate, sz, 10);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSampleFormatDlg::OnOK() 
{
	if (!UpdateData(TRUE))
		return;

	switch (m_comboRate)
	{
	case 0:
		m_format.dwRate = 44100;
		break;
	case 1:
		m_format.dwRate = 22050;
		break;
	case 2:
		m_format.dwRate = 11025;
		break;
	}
	m_format.dwBits = (m_comboBits+1)*8;
	m_format.dwChannels = m_comboChannels+1;

	m_format.uiBitRate = (USHORT)atol( (LPCTSTR)m_bitrate);
	m_format.format = 
		GetCheckedRadioButton(IDC_WAV_RADIO, IDC_MP3_RADIO) == IDC_WAV_RADIO?
		WAV:MP3;
	
	CDialog::OnOK();
}

// Get version number from bladeenc, if available
void CSampleFormatDlg::GetEncoderVersion()
{
	char szPath[MAX_PATH] = "";
	/*
	GetCurrentDirectory(sizeof(szPath), szPath);
	if (szPath[strlen(szPath)-1] != '\\')
		strcat(szPath, "\\");
	*/
	strcat(szPath, "bladeenc.dll");
	
	HINSTANCE hBladeDLL = LoadLibrary(szPath);
	if(!hBladeDLL)	
	{
		char szErr[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
			GetLastError(), 0, szErr, sizeof(szErr), NULL);
		
		CString str;
		str.Format("Could not load MP3 encoder DLL %s.\n"
			"%s\nPlease refer to http://ww.opcode.co.uk/OpD2d for more information",
			szPath, szErr);
		AfxMessageBox( str );
		return;
	}

	// Get Interface
	BEVERSION beVersion = (BEVERSION)GetProcAddress(hBladeDLL, TEXT_BEVERSION);
	if(!beVersion)	
	{
		AfxMessageBox("Unable to get BladeEnc interface");
		return;
	}

	// Get Version
	BE_VERSION	Version;
	beVersion(&Version);

	char sz[8192];
	_snprintf( sz, sizeof(sz),
		"BladeEnc.dll version %u.%02u (%u/%u/%u)\n"
		"BladeEnc Engine %u.%02u",
		Version.byDLLMajorVersion, Version.byDLLMinorVersion,
		Version.byDay, Version.byMonth, Version.wYear,
		Version.byMajorVersion, Version.byMinorVersion);
	m_version = sz;
	UpdateData(FALSE);
}
