// SideView.cpp : implementation of the CSideView class
//

#include "stdafx.h"
#include "Faceworx.h"
#include "FaceworxDoc.h"
#include "ImageView.h"
#include "SideView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BLEND_WIDTH_MIN 10.0f

// CSideView

IMPLEMENT_DYNCREATE(CSideView, CImageView)

BEGIN_MESSAGE_MAP(CSideView, CImageView)
	ON_WM_CAPTURECHANGED()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_MESSAGE(WM_MOUSELEAVE, &CSideView::OnMouseLeave)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CSideView construction/destruction

CSideView::CSideView()
{
	m_nBlendNode = m_nBlendLine = -1;
	m_bDrag = false;
}

CSideView::~CSideView()
{
}

// CSideView drawing

void CSideView::OnDraw(CDC* pDC)
{
	CImageView::OnDraw(pDC);
}

// CSideView diagnostics

#ifdef _DEBUG
void CSideView::AssertValid() const
{
	CImageView::AssertValid();
}

void CSideView::Dump(CDumpContext& dc) const
{
	CImageView::Dump(dc);
}

CFaceworxDoc* CSideView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceworxDoc)));
	return (CFaceworxDoc*)m_pDocument;
}
#endif //_DEBUG

int CSideView::GetSide() const
{
	return 1;
}

void CSideView::Draw(CDC& dc, CPoint ptOffset, float fScale) const
{
	CImageView::Draw(dc, ptOffset, fScale);

	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bBlend)
	{
		const CFaceworxDoc::CBlendZone& blendZone = pDoc->m_blendZone;

		CPen pen1(PS_DOT, 1, RGB(0, 0, 0));
		CPen pen2(PS_DOT, 1, RGB(255, 0, 0));
		CPen pen3(PS_SOLID, 1, RGB(255, 255, 255));

		CPen* pOldPen = dc.SelectObject(&pen1);
		ASSERT(pOldPen != NULL);

		CBrush brush1(RGB(0, 0, 0));
		CBrush brush2(RGB(255, 0, 0));

		CBrush* pOldBrush = dc.SelectObject(&brush1);
		ASSERT(pOldBrush != NULL);

		CPoint pts1[3];
		for (size_t i = 0; i <= blendZone.m_nodes.GetCount(); i++)
		{
			CPoint pts2[3];

			if (i < blendZone.m_nodes.GetCount())
			{
				CPointF pt = blendZone.m_nodes[i];

				pts2[0] = TranslateCoords(pt, ptOffset, fScale);
				pts2[1] = TranslateCoords(CPointF(pt.x - blendZone.m_fWidth, pt.y), ptOffset, fScale);
				pts2[2] = TranslateCoords(CPointF(pt.x + blendZone.m_fWidth, pt.y), ptOffset, fScale);

				if (i == 0)
				{
					pts1[0].SetPoint(pts2[0].x, SHRT_MIN);
					pts1[1].SetPoint(pts2[1].x, SHRT_MIN);
					pts1[2].SetPoint(pts2[2].x, SHRT_MIN);
				}
			}
			else
			{
				pts2[0].SetPoint(pts1[0].x, SHRT_MAX);
				pts2[1].SetPoint(pts1[1].x, SHRT_MAX);
				pts2[2].SetPoint(pts1[2].x, SHRT_MAX);
			}

			if (m_nBlendLine != 0)
				VERIFY(dc.SelectObject(&pen1));
			else
				VERIFY(dc.SelectObject(&pen2));

			dc.MoveTo(pts1[0]);
			dc.LineTo(pts2[0]);

			if (m_nBlendLine == -1)
				VERIFY(dc.SelectObject(&pen1));
			else
				VERIFY(dc.SelectObject(&pen2));

			dc.MoveTo(pts1[1]);
			dc.LineTo(pts2[1]);

			dc.MoveTo(pts1[2]);
			dc.LineTo(pts2[2]);

			pts1[0] = pts2[0];
			pts1[1] = pts2[1];
			pts1[2] = pts2[2];
		}

		for (size_t i = 0; i < blendZone.m_nodes.GetCount(); i++)
		{
			CPoint pt = TranslateCoords(blendZone.m_nodes[i], ptOffset, fScale);

			VERIFY(dc.SelectObject(&pen3));

			if (m_nBlendNode != i && m_nBlendLine != 0)
				VERIFY(dc.SelectObject(&brush1));
			else
				VERIFY(dc.SelectObject(&brush2));

			VERIFY(dc.Rectangle(GetHandleRect(pt)));
		}

		VERIFY(dc.SelectObject(pOldPen));
		VERIFY(dc.SelectObject(pOldBrush));
	}
}

int CSideView::HitTestBlendNode(CPoint point) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	const CFaceworxDoc::CBlendZone& blendZone = pDoc->m_blendZone;

	CPointF pt = TranslateCoords(point, GetScrollPosition() - GetOffset(), m_fScale);
	float d = HANDLE_SIZE / 2.0f / m_fScale;

	for (size_t i = 0; i < blendZone.m_nodes.GetCount(); i++)
	{
		CPointF pt2 = blendZone.m_nodes[i];
		CRectF rect(pt2.x, pt2.y, pt2.x, pt2.y);
		rect.InflateRect(d, d);
		if (rect.PtInRect(pt))
			return i;
	}

	return -1;
}

int CSideView::HitTestBlendLine(CPoint point) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	const CFaceworxDoc::CBlendZone& blendZone = pDoc->m_blendZone;

	CPointF pt = TranslateCoords(point, GetScrollPosition() - GetOffset(), m_fScale);
	float d = HANDLE_SIZE / 2.0f / m_fScale;

	CPointF pt1;
	for (size_t i = 0; i <= blendZone.m_nodes.GetCount(); i++)
	{
		CPointF pt2;

		if (i < blendZone.m_nodes.GetCount())
		{
			pt2 = blendZone.m_nodes[i];
			if (i == 0)
				pt1.SetPoint(pt2.x, SHRT_MIN);
		}
		else
			pt2.SetPoint(pt1.x, SHRT_MAX);

		if (PtOnSegment(pt, pt1, pt2, d))
			return 0;
		else if (PtOnSegment(pt, CPointF(pt1.x - blendZone.m_fWidth, pt1.y),
			CPointF(pt2.x - blendZone.m_fWidth, pt2.y), d))
			return 1;
		else if (PtOnSegment(pt, CPointF(pt1.x + blendZone.m_fWidth, pt1.y),
			CPointF(pt2.x + blendZone.m_fWidth, pt2.y), d))
			return 2;

		pt1 = pt2;
	}

	return -1;
}

bool CSideView::SetCursor(UINT nHitTest)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (nHitTest == HTCLIENT)
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		if (pDoc->m_bBlend)
		{
			int nBlendNode = m_nBlendNode;
			int nBlendLine = m_nBlendLine;
			if ((m_nBlendNode = HitTestBlendNode(point)) != -1)
			{
				::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_MOVE4WAY));
				m_nBlendLine = -1;
			}
			else if ((m_nBlendLine = HitTestBlendLine(point)) != -1)
			{
				if (m_nBlendLine == 0)
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_MOVE4WAY));
				else
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_TRACKWE));
			}

			if (nBlendNode != m_nBlendNode || nBlendLine != m_nBlendLine)
			{
				Invalidate(FALSE);
				((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
			}

			if (m_nBlendNode != -1 || m_nBlendLine != -1)
			{
				m_nBox = m_nNode =  m_nContour = -1;
				return true;
			}
		}
	}

	return CImageView::SetCursor(nHitTest);
}

bool CSideView::BeginDrag(CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (GetCapture() != NULL)
		return false;

	if (pDoc->m_bBlend)
	{
		if ((m_nBlendNode = HitTestBlendNode(point)) != -1)
		{
			m_bDrag = true;
			m_nBlendLine = -1;
		}
		else if ((m_nBlendLine = HitTestBlendLine(point)) != -1)
			m_bDrag = true;

		if (m_bDrag)
		{
			pDoc->BeginState();

			m_bModified = false;
			m_ptSaved = point;

			SetCapture();

			return true;
		}
	}

	return false;
}

void CSideView::DoDrag(CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CFaceworxDoc::CBlendZone& blendZone = pDoc->m_blendZone;

	CSizeF offset((point.x - m_ptSaved.x) / m_fScale, (point.y - m_ptSaved.y) / m_fScale);
	if (offset.cx != 0.0f || offset.cy != 0.0f)
	{
		bool bSave = true;
		if (m_nBlendNode != -1)
		{
			CPointF& pt = blendZone.m_nodes[m_nBlendNode];
			CPointF pt1 = pt;
			pt += offset;

			if (m_nBlendNode > 0)
			{
				CPointF pt2 = blendZone.m_nodes[m_nBlendNode - 1];
				if (pt.y < pt2.y + BLEND_WIDTH_MIN)
				{
					pt.y = pt2.y + BLEND_WIDTH_MIN;
					m_ptSaved.SetPoint(point.x, m_ptSaved.y - (int)((pt1.y - pt2.y - BLEND_WIDTH_MIN) * m_fScale));
					bSave = false;
				}
			}

			if (m_nBlendNode < (int)blendZone.m_nodes.GetCount() - 1)
			{
				CPointF pt2 = blendZone.m_nodes[m_nBlendNode + 1];
				if (pt.y > pt2.y - BLEND_WIDTH_MIN)
				{
					pt.y = pt2.y - BLEND_WIDTH_MIN;
					m_ptSaved.SetPoint(point.x, m_ptSaved.y - (int)((pt1.y - pt2.y + BLEND_WIDTH_MIN) * m_fScale));
					bSave = false;
				}
			}
		}
		else
		{
			if (m_nBlendLine == 0)
			{
				for (size_t i = 0; i < blendZone.m_nodes.GetCount(); i++)
					blendZone.m_nodes[i] += offset;
			}
			else
			{
				float fWidth = blendZone.m_fWidth;
				if (m_nBlendLine == 1)
				{
					blendZone.m_fWidth -= offset.cx;
					if (blendZone.m_fWidth < BLEND_WIDTH_MIN)
					{
						blendZone.m_fWidth = BLEND_WIDTH_MIN;
						m_ptSaved.SetPoint(m_ptSaved.x + (int)((fWidth - BLEND_WIDTH_MIN) * m_fScale), point.y);
						bSave = false;
					}
				}
				else
				{
					blendZone.m_fWidth += offset.cx;
					if (blendZone.m_fWidth < BLEND_WIDTH_MIN)
					{
						blendZone.m_fWidth = BLEND_WIDTH_MIN;
						m_ptSaved.SetPoint(m_ptSaved.x - (int)((fWidth - BLEND_WIDTH_MIN) * m_fScale), point.y);
						bSave = false;
					}
				}
			}
		}

		m_bModified = true;
		if (bSave)
			m_ptSaved = point;

		pDoc->CreateEnhancedMesh();
		pDoc->FillTextureAlpha();

		Invalidate(FALSE);
		((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
		((CFaceworxApp*)AfxGetApp())->Update3DView();
	}
}

void CSideView::EndDrag()
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (m_bModified)
	{
		pDoc->EndState();
		pDoc->SetModifiedFlag();
	}
	else
		pDoc->CancelState();

	m_bDrag = false;

	ReleaseCapture();

	CalcScrollSizes();
}

void CSideView::CancelDrag()
{
	CFaceworxDoc* pDoc = GetDocument();
	CPoint pt = GetScrollPosition();
	ASSERT_VALID(pDoc);

	pDoc->CancelState();

	m_bDrag = false;

	ReleaseCapture();

	CalcScrollSizes();

	pDoc->CreateEnhancedMesh();

	Invalidate(FALSE);
	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
	((CFaceworxApp*)AfxGetApp())->Update3DView();
}

// CSideView message handlers

void CSideView::OnCaptureChanged(CWnd *pWnd)
{
	if (m_bDrag)
		CancelDrag();

	CImageView::OnCaptureChanged(pWnd);
}

void CSideView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE && m_bDrag)
	{
		CancelDrag();
		return;
	}

	CImageView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSideView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (BeginDrag(point))
		return;

	CImageView::OnLButtonDown(nFlags, point);
}

void CSideView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDrag)
	{
		DoDrag(point);
		EndDrag();
		return;
	}

	CImageView::OnLButtonUp(nFlags, point);
}

LRESULT CSideView::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	if (m_nBlendNode != -1 || m_nBlendLine != -1)
	{
		m_nBlendNode = m_nBlendLine = -1;
		Invalidate(FALSE);
	}

	return CImageView::OnMouseLeave(wParam, lParam);
}

void CSideView::OnMouseMove(UINT nFlags, CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (!m_bMouseEnter)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
		m_bMouseEnter = true;
	}

	if (m_bDrag)
	{
		DoDrag(point);
		return;
	}

	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();

	CImageView::OnMouseMove(nFlags, point);
}

void CSideView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_bDrag)
	{
		CancelDrag();
		return;
	}

	CImageView::OnRButtonDown(nFlags, point);
}

int CSideView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CImageView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CString str;
	str.LoadString(IDS_SIDE_VIEW);
	SetWindowText(str);

	return 0;
}
