// MagnifierDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Faceworx.h"
#include "MagnifierDlg.h"
#include "FaceworxDoc.h"
#include "ImageView.h"

// CMagnifierDlg dialog

IMPLEMENT_DYNAMIC(CMagnifierDlg, CDialog)

CMagnifierDlg::CMagnifierDlg(CWnd* pParent /*=NULL*/) : CDialog(CMagnifierDlg::IDD, pParent)
{
	m_fScale = 1.0f;
	m_nZoom = (int)(log(m_fScale * 100.0f / 10.0f) / 0.2f);
}

CMagnifierDlg::~CMagnifierDlg()
{
}

void CMagnifierDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMagnifierDlg, CDialog)
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CMagnifierDlg message handlers

BOOL CMagnifierDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_nZoom += zDelta / WHEEL_DELTA;
	if (m_nZoom < 0)
		m_nZoom = 0;
	else if (m_nZoom > 29)
		m_nZoom = 29;

	m_fScale = 10.0f * exp(m_nZoom * 0.2f) / 100.0f;

	Invalidate(FALSE);

	return TRUE;
}

void CMagnifierDlg::OnPaint()
{
	CPaintDC pdc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	CDC dc;
	CBitmap bitmap;
	CBitmap* pOldBitmap;

	VERIFY(dc.CreateCompatibleDC(&pdc));
	VERIFY(bitmap.CreateCompatibleBitmap(&pdc, rect.Width(), rect.Height()));

	pOldBitmap = dc.SelectObject(&bitmap);
	ASSERT(pOldBitmap != NULL);

	CBrush brush;
	VERIFY(brush.CreateSysColorBrush(COLOR_BTNFACE));

	dc.FillRect(rect, &brush);

	CPoint pt;
	GetCursorPos(&pt);

	CWnd* pWnd = GetCapture();
	if (pWnd == NULL)
	{
		pWnd = WindowFromPoint(pt);
		if (pWnd != NULL)
		{
			CPoint pt2(pt);
			pWnd->ScreenToClient(&pt2);

			CRect rect2;
			pWnd->GetClientRect(&rect2);

			if (!rect2.PtInRect(pt2))
				pWnd = NULL;
		}
	}

	if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CImageView)))
	{
		CImageView* pView = (CImageView*)pWnd;

		CPoint pt2(pt);
		pView->ScreenToClient(&pt2);

		CPoint ptOffset = TranslateCoords(TranslateCoords(pt2, pView->GetScrollPosition() - pView->GetOffset(),
			pView->GetScale()), CPoint(0, 0), m_fScale) - CSize(rect.Width() / 2, rect.Height() / 2);

		pView->Draw(dc, ptOffset, m_fScale);

		HCURSOR hCursor = GetCursor();
		ICONINFO iconinfo;
		GetIconInfo(hCursor, &iconinfo);
		dc.DrawIcon(rect.Width() / 2 - iconinfo.xHotspot, rect.Height() / 2 - iconinfo.yHotspot, hCursor);
	}

	VERIFY(pdc.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY));
	VERIFY(dc.SelectObject(pOldBitmap));
}
