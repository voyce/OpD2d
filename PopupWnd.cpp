// PopupWnd.cpp : implementation file
//

#include "stdafx.h"
#include "OpD2d.h"
#include "OpD2dDlg.h"
#include "PopupWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupWnd

CPopupWnd::CPopupWnd()
{
}

CPopupWnd::~CPopupWnd()
{
}


BEGIN_MESSAGE_MAP(CPopupWnd, CWnd)
	//{{AFX_MSG_MAP(CPopupWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPopupWnd message handlers

int CPopupWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_hBmp = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP2));
	HRGN hRgn = COpD2dDlg::BitmapToRegion((HBITMAP)m_hBmp);	
	SetWindowRgn(hRgn, TRUE);
	
	m_font.CreatePointFont(85, "Tahoma");

	return 0;
}

void CPopupWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	BITMAP bm;
	GetObject(m_hBmp, sizeof(bm), &bm);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap *pBmp = dcMem.SelectObject(CBitmap::FromHandle(m_hBmp));
	dc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pBmp);

	CString str;
	GetWindowText(str);
	
	CRect rect;
	GetClientRect(rect);
	InflateRect(rect, -10, -10);

	//dc.FrameRect(rect, CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));

	CFont *pFont = dc.SelectObject(&m_font);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(str, &rect, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_CALCRECT);
	dc.DrawText(str, &rect, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
	dc.SelectObject(pFont);
}

UINT CPopupWnd::OnNcHitTest(CPoint point) 
{
	return CWnd::OnNcHitTest(point);
}
