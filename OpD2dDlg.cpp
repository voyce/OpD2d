// OpD2dDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OpD2d.h"
#include "OpD2dDlg.h"
#include "WaveOutputFile.h"
#include "Mp3OutputFile.h"
#include "SampleFormatDlg.h"
#include "PreviewThread.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT PreviewThreadProc(LPVOID pParam);
static int g_nDeviceID;
extern HWND g_hwndPreview;

#define HELP_TIMER_ID 1000

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpD2dDlg dialog

COpD2dDlg::COpD2dDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpD2dDlg::IDD, pParent),m_hThread(NULL),m_bShowHelp(false),m_bAutoMode(false)
{
	//{{AFX_DATA_INIT(COpD2dDlg)
	m_strPath = _T("");
	m_strProgress = _T("");
	m_dwLength = 0;
	m_nUnits = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COpD2dDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpD2dDlg)
	DDX_Text(pDX, IDC_PATH_EDIT, m_strPath);
	DDX_Text(pDX, IDC_PROGRESS_TEXT, m_strProgress);
	DDX_Text(pDX, IDC_LENGTH_EDIT, m_dwLength);
	DDX_CBIndex(pDX, IDC_UNITS_COMBO, m_nUnits);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COpD2dDlg, CDialog)
	//{{AFX_MSG_MAP(COpD2dDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RECORD_BTN, OnRecordBtn)
	ON_BN_CLICKED(IDC_STOP_BTN, OnStopBtn)
	ON_UPDATE_COMMAND_UI(IDC_RECORD_BTN, OnUpdateButtons)
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_CBN_SELENDOK(IDC_UNITS_COMBO, OnSelendokUnitsCombo)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_HELP_BTN, OnHelpBtn)
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDC_FORMAT_BUTTON, OnFormatButton)
	ON_BN_CLICKED(IDC_PREVIEW_BTN, OnPreviewBtn)
	ON_UPDATE_COMMAND_UI(IDC_STOP_BTN, OnUpdateButtons)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(IDC_FORMAT_BUTTON, OnUpdateButtons)
	ON_UPDATE_COMMAND_UI(IDC_BROWSE_BTN, OnUpdateButtons)
	ON_MESSAGE(WM_RECORDTIME, OnRecordTime)
	ON_MESSAGE(WM_RECORDABORT, OnRecordAbort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpD2dDlg message handlers

BOOL COpD2dDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	m_hBmp = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP1));
	HRGN hRgn = BitmapToRegion((HBITMAP)m_hBmp);	
	SetWindowRgn(hRgn, TRUE);

	if (!waveInGetNumDevs())
	{
		AfxMessageBox("No audio input devices.");
	}
	// populate wave input device combo
	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_DEVICE_COMBO);
	ASSERT(pCombo);
	for (UINT n=0; n < waveInGetNumDevs(); n++)
	{
		WAVEINCAPS wic;
		MMRESULT res = waveInGetDevCaps(n, &wic, sizeof(wic));
		if (res == MMSYSERR_NOERROR)
		{
			pCombo->InsertString(n, wic.szPname);
		}
		else
		{
			char sz[128];
			waveInGetErrorText(res, sz, sizeof(sz));

			TCHAR szMsg[512];
			_stprintf_s(szMsg, _countof(szMsg), 
				"Couldn't get capabilities of audio input device #%ld\n\n%s",
				n+1, sz);
			AfxMessageBox(szMsg);
		}
	}
	pCombo->SetCurSel(0);

	m_nUnits = 0;
	m_dwLength = 0;

	m_strProgress = "Ready";

	UpdateData(FALSE);
	UpdateDialogControls(this, FALSE);
	SetWindowText("OpD2D - Direct to disk audio");

	LPCTSTR szCls = AfxRegisterWndClass(0);
	m_wndHelp.CreateEx(WS_EX_TOOLWINDOW, szCls, "!", WS_POPUP|WS_OVERLAPPED, CRect(0,0,147,200), NULL, 0);
	int nShowHelp = AfxGetApp()->GetProfileInt("Settings", "ShowHelp", 1);
	m_wndHelp.ShowWindow( nShowHelp == 1 ? SW_SHOW : SW_HIDE);
	if (nShowHelp == 1)
		SetTimer(HELP_TIMER_ID, 100, NULL);
	else
		KillTimer(HELP_TIMER_ID);

	m_dlgFormat.m_format.dwBits = 16;
	m_dlgFormat.m_format.dwChannels = 2;
	m_dlgFormat.m_format.dwRate = 44100;

	if (m_bAutoMode)
	{
		GetDlgItem(IDC_PATH_EDIT)->SetWindowTextA(m_strFilenameArg);

		m_strSizeDurationArg.MakeLower();
		CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_UNITS_COMBO);
		int n = 0;
		if (strstr(m_strSizeDurationArg, "min") != NULL &&
			sscanf_s(m_strSizeDurationArg, "%dmin", &n) == 1)
		{
			pCombo->SetCurSel(1);
		}
		else if (strstr(m_strSizeDurationArg, "mb") != NULL &&
			sscanf_s(m_strSizeDurationArg, "%dmb", &n) == 1)
		{
			pCombo->SetCurSel(0);
		}
		else
		{
			CString str;
			AfxFormatString1(str, IDS_SIZEARGUMENT_ERROR, m_strSizeDurationArg);
			AfxMessageBox(str);
			EndDialog(0);
			return TRUE;
		}
		char sz[512] = "";
		_ltoa_s(n, sz, sizeof(sz), 10);
		GetDlgItem(IDC_LENGTH_EDIT)->SetWindowTextA(sz);

		OnRecordBtn();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COpD2dDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COpD2dDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		BITMAP bm;
		GetObject(m_hBmp, sizeof(bm), &bm);

		CPaintDC dc(this);

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap *pBmp = dcMem.SelectObject(CBitmap::FromHandle(m_hBmp));
		dc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &dcMem, 0, 0, SRCCOPY);
		dcMem.SelectObject(pBmp);

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COpD2dDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

HRGN COpD2dDlg::BitmapToRegion(HBITMAP hBmp)
{
	COLORREF cTransparentColor = 0xff0000, cTolerance = 0x101010;
	HRGN hRgn = NULL;

	ASSERT(hBmp);
	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap conten
		HDC hMemDC = CreateCompatibleDC(NULL);
		ASSERT(hMemDC);
		if (hMemDC)
		{
			// Get bitmap siz
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
					sizeof(BITMAPINFOHEADER),	// biSize
					bm.bmWidth,					// biWidth;
					bm.bmHeight,				// biHeight;
					1,							// biPlanes;
					32,							// biBitCount
					BI_RGB,						// biCompression;
					0,							// biSizeImage;
					0,							// biXPelsPerMeter;
					0,							// biYPelsPerMeter;
					0,							// biClrUsed;
					0							// biClrImportant;
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			ASSERT(hbm32);
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory D
				HDC hDC = CreateCompatibleDC(hMemDC);
				ASSERT(hDC);
				if (hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits
					BITMAP bm32;
					VERIFY(GetObject(hbm32, sizeof(bm32), &bm32));
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory D
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					VERIFY(BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY));

					// For better performances, we will use the ExtCreateRegion() function to create th
					// region. This function take a RGNDATA structure on entry. We will add rectangles b
					// amount of ALLOC_UNIT number in this structure
					#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixel
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetBValue(cTolerance));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to righ
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the regio
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									VERIFY(hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE));
									pData = (RGNDATA *)GlobalLock(hData);
									ASSERT(pData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is to
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									ASSERT(h);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangle
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					ASSERT(h);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean u
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	
	}

	return hRgn;
}



void COpD2dDlg::OnClose() 
{
	if (IsWindow(g_hwndPreview))
		::SendMessage(g_hwndPreview, WM_CLOSE, 0, 0);
	CDialog::OnClose();
}


UINT RecordThreadProc(LPVOID pParam)
{
	CThreadInfo *pInfo = (CThreadInfo *)pParam;
	ASSERT(pInfo);
	
	HANDLE hEvt = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(hEvt != INVALID_HANDLE_VALUE);
	::ResetEvent(hEvt);

	HWAVEIN hwi = NULL;

	WAVEFORMATEX wfx;
	::ZeroMemory(&wfx, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (USHORT)pInfo->m_format.dwChannels;
	wfx.nSamplesPerSec = pInfo->m_format.dwRate;
	wfx.wBitsPerSample = (USHORT)pInfo->m_format.dwBits;
	wfx.nBlockAlign = (wfx.wBitsPerSample/8)*wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	COutputFile *file = NULL;
	if (pInfo->m_format.format == WAV)
	{
		file = new CWaveOutputFile;
		if (!file->Create(pInfo->m_szFilename, wfx))
		{
			::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 3L;
		}
	}
	else
	{
		file = new CMp3OutputFile;
		
		reinterpret_cast<CMp3OutputFile *>(file)->m_uiBitRate = 
			pInfo->m_format.uiBitRate;

		if (!file->Create(pInfo->m_szFilename, wfx))
		{
			::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 3L;
		}
	}
	
	MMRESULT res = waveInOpen(&hwi, 
		pInfo->m_nDeviceID, 
		&wfx, 
		(DWORD)hEvt, 
		0L, 
		CALLBACK_EVENT);
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't open audio input device.\n\n") + 
			CString(sz) );
		
		::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
		return 1L;
	}

	pInfo->m_hwi = hwi;

	const DWORD dwBufferSize = 1024*128; //128k buffers

	WAVEHDR wh[2];
	LPVOID pBuffer[2];
	for (int n=0; n < 2; n++)
	{
		// Create the buffer
		pBuffer[n] = ::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize);
		if (!pBuffer[n])
		{
			AfxMessageBox("Couldn't allocate memory for audio input buffer.");

			waveInClose(hwi);
			::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 2L;
		}

		::ZeroMemory(&wh[n], sizeof(wh[n]));
		wh[n].lpData = (char *)pBuffer[n];
		wh[n].dwBufferLength = dwBufferSize;
		res = waveInPrepareHeader(hwi, &wh[n], sizeof(wh[n]));
		if (res != MMSYSERR_NOERROR)
		{
			char sz[128];
			waveInGetErrorText(res, sz, sizeof(sz));
			AfxMessageBox(CString("Couldn't prepare input buffer.\n\n") + 
				CString(sz) );
			
			waveInClose(hwi);
			::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 2L;
		}
		
		res = waveInAddBuffer(hwi, &wh[n], sizeof(wh[n]));	
		if (res != MMSYSERR_NOERROR)
		{
			char sz[128];
			waveInGetErrorText(res, sz, sizeof(sz));
			AfxMessageBox(CString("Couldn't add input buffer.\n\n") + 
				CString(sz) );
			
			waveInClose(hwi);
			::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 2L;
		}
	}
	res = waveInStart(hwi);	
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't start recording.\n\n") + 
			CString(sz) );
		
		waveInClose(hwi);
		::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
		return 2L;
	}

	MMTIME mmt;
	int nBuffer = 1;
	bool bGotStopAlready = false;
	while (true)
	{
		// Event will be signalled when controller thinks
		// we've recorded enough (it gets WM_RECORDTIME messages)
		DWORD dw = ::WaitForSingleObject(hEvt, INFINITE);
		if (dw == WAIT_OBJECT_0)
		{
			// buffer full! write to disk and go again...
			int n = (nBuffer == 1)?0:1;
			if (wh[n].dwFlags & WHDR_DONE)
			{
				AfxTrace("buffer full\n");

				mmt.wType = TIME_BYTES;
				waveInGetPosition(hwi, &mmt, sizeof(mmt));
				::SendMessage(pInfo->m_hParentWnd, WM_RECORDTIME, (WPARAM)&mmt, (LPARAM)pInfo);

				res = waveInUnprepareHeader(hwi, &wh[n], sizeof(wh[n]));
				ASSERT( res == MMSYSERR_NOERROR);

				file->WriteData((LPBYTE)pBuffer[n], wh[n].dwBytesRecorded);

				// an incomplete buffer fill prolly means we're being stopped
				if (wh[n].dwBytesRecorded != dwBufferSize &&
					pInfo->m_bStop == true)
				{
					AfxTrace("incomplete buffer - stopping\n");
					
					n = !n;

					res = waveInReset(hwi);
					ASSERT( res == MMSYSERR_NOERROR);

					res = waveInUnprepareHeader(hwi, &wh[n], sizeof(wh[n]));
					if (res != MMSYSERR_NOERROR)
					{
						char sz[128];
						waveInGetErrorText(res, sz, sizeof(sz));
						AfxMessageBox(CString("Couldn't unprepare input buffer.\n\n") + 
							CString(sz) );
					}
					
					break;
				}

				::ZeroMemory(&wh[n], sizeof(wh[n]));
				wh[n].lpData = (char *)pBuffer[n];
				wh[n].dwBufferLength = dwBufferSize;
				res = waveInPrepareHeader(hwi, &wh[n], sizeof(wh[n]));
				ASSERT(res == MMSYSERR_NOERROR);
				
				res = waveInAddBuffer(hwi, &wh[n], sizeof(wh[n]));	
				ASSERT(res == MMSYSERR_NOERROR);

				if (nBuffer == 1)
					nBuffer = 2;
				else if (nBuffer == 2)
					nBuffer = 1;
			}
		}
	}
	file->Flush();

/*
	res = waveInUnprepareHeader(hwi, &wh[0], sizeof(wh[0]));
	//ASSERT( res == MMSYSERR_NOERROR);
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't unprepare input buffer 2.\n\n") + 
			CString(sz) );
	}
*/
	res = waveInClose(hwi);
	//ASSERT( res == MMSYSERR_NOERROR);
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't close audio input device.\n\n") + 
			CString(sz) );
	}

	::HeapFree(GetProcessHeap(), 0, pBuffer[0]);
	::HeapFree(GetProcessHeap(), 0, pBuffer[1]);

	return 0L;
}

#include "PreviewWnd.h"

// This is very similar to the recording thread
// but obviously it doesn't write to the output file
UINT PreviewThreadProc(LPVOID pParam)
{
	CThreadInfo *pInfo = (CThreadInfo *)pParam;
	ASSERT(pInfo);
		
	HANDLE hEvt = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(hEvt != INVALID_HANDLE_VALUE);
	::ResetEvent(hEvt);

	HWAVEIN hwi = NULL;

	WAVEFORMATEX wfx;
	::ZeroMemory(&wfx, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (USHORT)pInfo->m_format.dwChannels;
	wfx.nSamplesPerSec = pInfo->m_format.dwRate;
	wfx.wBitsPerSample = (USHORT)pInfo->m_format.dwBits;
	wfx.nBlockAlign = (wfx.wBitsPerSample/8)*wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	
	MMRESULT res = waveInOpen(&hwi, 
		g_nDeviceID, 
		&wfx, 
		(DWORD)hEvt, 
		0L, 
		CALLBACK_EVENT);
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't open audio input device.\n\n") + 
			CString(sz) );
		
		//::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
		return 1L;
	}

	pInfo->m_hwi = hwi;

	const DWORD dwBufferSize = 1024*4; // buffers

	WAVEHDR wh[2];
	LPVOID pBuffer[2];
	for (int n=0; n < 2; n++)
	{
		// Create the buffer
		pBuffer[n] = ::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize);
		if (!pBuffer[n])
		{
			AfxMessageBox("Couldn't allocate memory for audio input buffer.");

			waveInClose(hwi);
			//::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 2L;
		}

		::ZeroMemory(&wh[n], sizeof(wh[n]));
		wh[n].lpData = (char *)pBuffer[n];
		wh[n].dwBufferLength = dwBufferSize;
		res = waveInPrepareHeader(hwi, &wh[n], sizeof(wh[n]));
		if (res != MMSYSERR_NOERROR)
		{
			char sz[128];
			waveInGetErrorText(res, sz, sizeof(sz));
			AfxMessageBox(CString("Couldn't prepare input buffer.\n\n") + 
				CString(sz) );
			
			waveInClose(hwi);
			//::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 2L;
		}
		
		res = waveInAddBuffer(hwi, &wh[n], sizeof(wh[n]));	
		if (res != MMSYSERR_NOERROR)
		{
			char sz[128];
			waveInGetErrorText(res, sz, sizeof(sz));
			AfxMessageBox(CString("Couldn't add input buffer.\n\n") + 
				CString(sz) );
			
			waveInClose(hwi);
			//::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
			return 2L;
		}
	}
	res = waveInStart(hwi);	
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't start recording.\n\n") + 
			CString(sz) );
		
		waveInClose(hwi);
		//::SendMessage(pInfo->m_hParentWnd, WM_RECORDABORT, 0L, 0L);
		return 2L;
	}

	MMTIME mmt;
	int nBuffer = 1;
	while (pInfo->m_bStop == false)
	{
		// Event will be signalled when controller thinks
		// we've recorded enough (it gets WM_RECORDTIME messages)
		DWORD dw = ::WaitForSingleObject(hEvt, INFINITE);
		if (dw == WAIT_OBJECT_0)
		{
			// buffer full! write to disk and go again...
			int n = (nBuffer == 1)?0:1;
			if (wh[n].dwFlags & WHDR_DONE)
			{
				mmt.wType = TIME_BYTES;
				waveInGetPosition(hwi, &mmt, sizeof(mmt));
				
				::SendMessage(pInfo->m_hParentWnd, WM_RECORDBUFFER, 
					(WPARAM)wh[n].lpData, 
					(LPARAM)wh[n].dwBufferLength );

				res = waveInUnprepareHeader(hwi, &wh[n], sizeof(wh[n]));
				ASSERT( res == MMSYSERR_NOERROR);

				// an incomplete buffer fill prolly means we're being stopped
				if (wh[n].dwBytesRecorded != dwBufferSize &&
					pInfo->m_bStop == true)
				{
					AfxTrace("incomplete buffer - stopping\n");
					
					n = !n;

					res = waveInReset(hwi);
					ASSERT( res == MMSYSERR_NOERROR);

					res = waveInUnprepareHeader(hwi, &wh[n], sizeof(wh[n]));
					if (res != MMSYSERR_NOERROR)
					{
						char sz[128];
						waveInGetErrorText(res, sz, sizeof(sz));
						AfxMessageBox(CString("Couldn't unprepare input buffer.\n\n") + 
							CString(sz) );
					}
					
					break;
				}

				::ZeroMemory(&wh[n], sizeof(wh[n]));
				wh[n].lpData = (char *)pBuffer[n];
				wh[n].dwBufferLength = dwBufferSize;
				res = waveInPrepareHeader(hwi, &wh[n], sizeof(wh[n]));
				ASSERT(res == MMSYSERR_NOERROR);
				
				res = waveInAddBuffer(hwi, &wh[n], sizeof(wh[n]));	
				ASSERT(res == MMSYSERR_NOERROR);

				if (nBuffer == 1)
					nBuffer = 2;
				else if (nBuffer == 2)
					nBuffer = 1;
			}
		}
	}
	res = waveInReset(hwi);
	res = waveInClose(hwi);
	//ASSERT( res == MMSYSERR_NOERROR);
	if (res != MMSYSERR_NOERROR)
	{
		char sz[128];
		waveInGetErrorText(res, sz, sizeof(sz));
		AfxMessageBox(CString("Couldn't close audio input device.\n\n") + 
			CString(sz) );
	}

	::HeapFree(GetProcessHeap(), 0, pBuffer[0]);
	::HeapFree(GetProcessHeap(), 0, pBuffer[1]);

	return 0L;
}

void COpD2dDlg::OnRecordBtn() 
{
	if (IsWindow(g_hwndPreview))
		::SendMessage(g_hwndPreview, WM_CLOSE, 0, 0L);

	//  check for existence of recording thread
	if (m_hThread)
		return;

	if (!UpdateData(TRUE))
		return;

	if (m_dwLength == 0)
	{
		AfxMessageBox("Please specify a number greater then 0 for the recording length/size.");
		return;
	}

	if (m_nUnits == 0) // bytes
	{
		m_dwLengthBytes = m_dwLength * 1048576;
		m_dwLengthMinutes = (DWORD)((double)m_dwLengthBytes/(double)m_dlgFormat.m_format.SamplesPerMinute());
	}
	else if (m_nUnits == 1) // minutes
	{
		m_dwLengthMinutes = m_dwLength;
		m_dwLengthBytes = m_dwLength * m_dlgFormat.m_format.SamplesPerMinute();
	}

	if (m_strPath.GetLength() == 0)
	{
		AfxMessageBox("Please specify a destination file for the recorded audio.");
		return;
	}

	unsigned __int64 iFree;
	if (!CheckDiskSpace(m_dwLengthBytes, &iFree))
	{
		char szMsg[512];
		wsprintf(szMsg,
			"Recorded file size may exceed the available free space on drive.\n\n"
			"Recording size: %ld MB (WAV format, MP3 will be much smaller)\nAvailable space: %ld MB\n\n"
			"Are you sure you want to continue?", 
			m_dwLengthBytes/(1024*1024), iFree/(1024*1024));
		if (AfxMessageBox(szMsg, MB_YESNO) != IDYES)
			return;
	}

	m_strProgress = "Recording...";

	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_DEVICE_COMBO);
	m_data.m_nDeviceID = pCombo->GetCurSel();
	m_data.m_hParentWnd = m_hWnd;
	m_data.m_bStop = false;
	m_data.m_szFilename = (LPCTSTR)m_strPath;
	m_data.m_format = m_dlgFormat.m_format;

	m_timeStart = CTime::GetCurrentTime();

	m_hThread = AfxBeginThread(RecordThreadProc, &m_data);

	UpdateData(FALSE);
	UpdateDialogControls(this, FALSE);
}

void COpD2dDlg::OnStopBtn() 
{
	waveInReset(m_data.m_hwi);
	waveInStop(m_data.m_hwi);
	m_data.m_bStop = true;

	m_hThread = NULL;

	UpdateData(FALSE);
	UpdateDialogControls(this, FALSE);

	GetDlgItem(IDC_PROGRESS_TEXT)->SetWindowText("Stopped");
}

void COpD2dDlg::OnUpdateButtons(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_nID == IDC_RECORD_BTN)
		pCmdUI->Enable(m_data.m_bStop);
	if (pCmdUI->m_nID == IDC_STOP_BTN)
		pCmdUI->Enable(!m_data.m_bStop);
	if (pCmdUI->m_nID == IDC_FORMAT_BUTTON)
		pCmdUI->Enable(m_data.m_bStop);
	if (pCmdUI->m_nID == IDC_BROWSE_BTN)
		pCmdUI->Enable(m_data.m_bStop);
	if (pCmdUI->m_nID == IDC_PREVIEW_BTN)
		pCmdUI->Enable(m_data.m_bStop);
}

void COpD2dDlg::OnBrowseBtn() 
{
	UpdateData(TRUE);
	CString str;
	// Add date to recording title (e.g. "recording Friday, 6 July, 2004.mp3")
	str.Format("recording %s", (LPCTSTR)CTime::GetCurrentTime().Format("%#x"));
	CFileDialog *dlg;
	if (m_format == WAV)
		dlg = new CFileDialog(FALSE, "wav", str+".wav", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"WAV File (*.wav)|*.wav|All Files (*.*)|*.*||");
	else
		dlg = new CFileDialog(FALSE, "mp3", str+".mp3", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"MP3 File (*.mp3)|*.mp3|All Files (*.*)|*.*||");
	if (dlg->DoModal() == IDOK)
	{
		m_strPath = dlg->GetPathName();
		UpdateData(FALSE);
	}
}

LRESULT COpD2dDlg::OnRecordTime(WPARAM wParam, LPARAM lParam)
{
	MMTIME *mmt = (MMTIME *)wParam;
	if (!mmt)
		return 0L;
	CThreadInfo *pInfo = reinterpret_cast<CThreadInfo *>(lParam);
	if (pInfo->m_format.format == WAV)
	{
		if (mmt->u.cb)
		{
			m_strProgress.Format("%.2f Mb recorded", 
				(double)mmt->u.cb/(double)(1024*1024));
			GetDlgItem(IDC_PROGRESS_TEXT)->SetWindowText(m_strProgress);
		}
		if (mmt->u.cb >= m_dwLengthBytes)
		{
			OnStopBtn();
		}
	}
	else
	{
		HANDLE hFile = CreateFile(pInfo->m_szFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
			NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwLow, dwHigh;
			dwLow = GetFileSize(hFile, &dwHigh);
			CloseHandle(hFile);
			
			m_strProgress.Format("%.2f Mb recorded", 
				(double)dwLow/(double)(1024*1024));
			GetDlgItem(IDC_PROGRESS_TEXT)->SetWindowText(m_strProgress);
		
			if (dwLow >= m_dwLengthBytes)
			{
				OnStopBtn();
			}
		}
		else
			GetDlgItem(IDC_PROGRESS_TEXT)->SetWindowText("Unable to determine file size");

	}
	// Have specified number of minutes passed?
	if (m_nUnits == 1)
	{
		CTime timeNow = CTime::GetCurrentTime();

		CTimeSpan elapsed = timeNow - m_timeStart;
		if (elapsed.GetTotalMinutes() >= static_cast<long>(m_dwLengthMinutes))
		{
			OnStopBtn();
		}
	}
	
	return 0L;
}

LRESULT COpD2dDlg::OnRecordAbort(WPARAM wParam, LPARAM lParam)
{
	OnStopBtn();
	return 0L;
}

LRESULT COpD2dDlg::OnNcHitTest(CPoint point) 
{
	if (!ChildWindowFromPoint(point, CWP_SKIPINVISIBLE))
		return HTCAPTION;
	
	return CDialog::OnNcHitTest(point);
}

HBRUSH COpD2dDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetBkColor(RGB(255,255,255));
	
	// TODO: Return a different brush if the default is not desired
	return (HBRUSH)GetStockObject(WHITE_BRUSH);
	//return hbr;
}

void COpD2dDlg::OnSelendokUnitsCombo() 
{
	DWORD dwOldLength = m_dwLength;
	int nOldUnits = m_nUnits;

	UpdateData(TRUE);

	if (nOldUnits == m_nUnits)
		return;

	if (nOldUnits == 0) // bytes
	{
		// new length is old converted to minutes
		m_dwLength = (DWORD)((double)(m_dwLength * 1048576)/(double)m_dlgFormat.m_format.SamplesPerMinute());
	}
	else if (nOldUnits == 1) // minutes
	{
		// new length is old (minutes) converted to MB
		m_dwLength = (DWORD)((double)(m_dwLength * m_dlgFormat.m_format.SamplesPerMinute())/(double)1048576);
	}
	UpdateData(FALSE);
}


void COpD2dDlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == HELP_TIMER_ID)
	{
		if (GetActiveWindow() != this)
			return;

		CPoint pt;
		GetCursorPos(&pt);
		
		// Only show the popup window if the mouse
		// is over our window
		CRgn rgn;
		rgn.CreateRectRgn(0,0,1,1);
		int n = GetWindowRgn(rgn);
		ScreenToClient(&pt);
		if (rgn.PtInRegion(pt))
			m_wndHelp.ShowWindow(SW_SHOWNOACTIVATE);
		else
			m_wndHelp.ShowWindow(SW_HIDE);

		if (m_wndHelp.IsWindowVisible())
		{
			// find which control we're over
			CWnd *pWnd = ChildWindowFromPoint(pt);
			
			CString str;
			m_wndHelp.GetWindowText(str);

			DWORD dwCtrlID = pWnd?pWnd->GetDlgCtrlID():0;
			switch(dwCtrlID)
			{
			case IDC_PATH_EDIT:
				m_wndHelp.SetWindowText("Enter a path. This is where the resulting audio file is saved.");
				break;
			case IDC_BROWSE_BTN:
				m_wndHelp.SetWindowText("Click this button to select where to save the file.");
				break;
			case IDC_LENGTH_EDIT:
				m_wndHelp.SetWindowText("Enter the length of time or megabytes to record.");
				break;
			case IDC_UNITS_COMBO:
				m_wndHelp.SetWindowText("Select the units for the length - either time (minutes) or file size (megabytes).");
				break;
			case IDC_DEVICE_COMBO:
				m_wndHelp.SetWindowText("Select your preferred sound recording device.");
				break;
			case IDC_RECORD_BTN:
				if (pWnd->IsWindowEnabled())
					m_wndHelp.SetWindowText("Press this button to start recording.");
				else
					m_wndHelp.SetWindowText("Press this button to start recording. It's currently disabled because recording is in progress.");
				break;
			case IDC_STOP_BTN:
				if (pWnd->IsWindowEnabled())
					m_wndHelp.SetWindowText("Press this button to stop recording.");
				else
					m_wndHelp.SetWindowText("Press this button to stop recording. It's currently disabled because no recording is in progress.");
				break;
			default:
				m_wndHelp.SetWindowText("Move the mouse over a control for help. Hit the Escape key to quit. Click the ? button to toggle this help window.");
				break;
			case IDC_FORMAT_BUTTON:
				m_wndHelp.SetWindowText("Select your preferred recording format (WAV or MP3) and quality settings.");
				break;
			}
			
			// force a redraw if text has changed
			CString strNew;
			m_wndHelp.GetWindowText(strNew);
			if (str != strNew)
				m_wndHelp.Invalidate(FALSE);

			ClientToScreen(&pt);
			m_wndHelp.SetWindowPos(&wndTopMost, pt.x-25,pt.y-120,0,0,SWP_NOSIZE|SWP_NOACTIVATE);
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void COpD2dDlg::OnHelpBtn() 
{
	if (m_bShowHelp)
	{
		m_wndHelp.ShowWindow(SW_HIDE);
		KillTimer(HELP_TIMER_ID);
		m_bShowHelp = false;
		AfxGetApp()->WriteProfileInt("Settings", "ShowHelp", 0);
	}
	else
	{
		m_wndHelp.ShowWindow(SW_SHOWNOACTIVATE);
		SetTimer(HELP_TIMER_ID, 100, NULL);
		m_bShowHelp = true;
		AfxGetApp()->WriteProfileInt("Settings", "ShowHelp", 1);
	}
}

void COpD2dDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	if (nState == WA_INACTIVE)
	{
		m_wndHelp.ShowWindow(SW_HIDE);
	}
	else
	{
		if (m_bShowHelp)
			m_wndHelp.ShowWindow(SW_SHOWNOACTIVATE);
	}
}

void COpD2dDlg::OnFormatButton() 
{
	if (m_dlgFormat.DoModal() == IDOK)
	{
		m_format = m_dlgFormat.m_format.format;
	}

}

BOOL COpD2dDlg::CheckDiskSpace(DWORD dwRecordingSize, unsigned __int64 *piFree)
{
	DWORD dwSectPerClust,
		dwBytesPerSect,
		dwFreeClusters,
		dwTotalClusters;
	
	BOOL bResult = GetDiskFreeSpace (m_strPath.SpanExcluding("\\"), 
		&dwSectPerClust,
		&dwBytesPerSect, 
		&dwFreeClusters,
		&dwTotalClusters);
	if (!bResult)
	{
		char szMsg[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, szMsg, sizeof(szMsg), NULL);
		AfxMessageBox(CString("Could not check free disk space.\n\n") + szMsg);
		return TRUE;
	}

	*piFree = (__int64)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
	if ((unsigned __int64)dwRecordingSize > *piFree)
	{
		return FALSE;
	}
	return TRUE;
}

void COpD2dDlg::OnPreviewBtn() 
{
	UpdateData(TRUE);

	if (!IsWindow(g_hwndPreview))
	{
		CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_DEVICE_COMBO);
		g_nDeviceID = pCombo->GetCurSel();	

		AfxBeginThread(RUNTIME_CLASS(CPreviewThread));
	}
	else
		::BringWindowToTop(g_hwndPreview);
}

void COpD2dDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (IsWindow(g_hwndPreview))
		::SendMessage(g_hwndPreview, WM_CLOSE, 0, 0);	
}

void COpD2dDlg::SetCmdlineSizeDuration(TCHAR *szArg)
{
	m_strSizeDurationArg = szArg;
}

void COpD2dDlg::SetCmdlineFilename(TCHAR *szArg)
{
	m_strFilenameArg = szArg;
}
