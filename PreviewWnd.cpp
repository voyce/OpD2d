// PreviewWnd.cpp : implementation file
//

#include "stdafx.h"
#include "OpD2d.h"
#include "PreviewWnd.h"
#include "PreviewThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HWND g_hwndPreview;

/////////////////////////////////////////////////////////////////////////////
// CPreviewWnd

CPreviewWnd::CPreviewWnd():m_pBuffer(NULL),m_pThread(NULL)
{
}

CPreviewWnd::~CPreviewWnd()
{
}


BEGIN_MESSAGE_MAP(CPreviewWnd, CWnd)
	//{{AFX_MSG_MAP(CPreviewWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_RECORDBUFFER, OnRecordBuffer)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPreviewWnd message handlers

void CPreviewWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (!m_pBuffer)
		return;

	CRect rect;
	GetClientRect(rect);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap *pBmp = dcMem.SelectObject(&bmp);

	dcMem.FillSolidRect( rect, RGB(0,0,0));	
	
	dcMem.SelectStockObject(DC_PEN);
	dcMem.SetDCPenColor( RGB(0, 255,0));

	DWORD dwLen = m_dwBufferLen;
	short *p = reinterpret_cast<short *>(m_pBuffer);
	CRect rectChannel(rect);
	rectChannel.bottom = rect.Height()/2;
	dcMem.MoveTo(0,rectChannel.Height()/2);
	long dStep = (double)(dwLen/2)/(double)rect.Width();
	int xpos=0;
	for (int x=0; x < dwLen; x+=dStep)
	{
		double d = ((double)p[x]/(double)0xffff) * (double)rect.Height();
		dcMem.LineTo(xpos++, (rectChannel.Height()/2)+d);
	}
	
	rectChannel.OffsetRect(0, rect.Height()/2);
	dcMem.MoveTo(rectChannel.left, rectChannel.top+(rectChannel.Height()/2));
	xpos = 0;
	for ( int x=1; x < dwLen; x+=dStep)
	{
		double d = ((double)p[x]/(double)0xffff) * (double)rect.Height();
		dcMem.LineTo(xpos++, rectChannel.top+(rectChannel.Height()/2)+d);
	}
	
	dc.BitBlt(0,0,rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	if (pBmp)
		dcMem.SelectObject(pBmp);
}

LRESULT CPreviewWnd::OnRecordBuffer(WPARAM wParam, LPARAM lParam)
{
	static long l;
	
	m_dwBufferLen = lParam;
	WORD *p = reinterpret_cast<WORD *>(wParam);
	if (!m_pBuffer)
		m_pBuffer = new WORD[m_dwBufferLen];
	memcpy(m_pBuffer, p, m_dwBufferLen);
	
	InvalidateRect(NULL, FALSE);
	return 0L;
}

void CPreviewWnd::OnClose() 
{
	if (m_pThread)
		static_cast<CPreviewThread*>(m_pThread)->Stop();
	CWnd::OnClose();
}

int CPreviewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	g_hwndPreview = m_hWnd;
	
	return 0;
}
