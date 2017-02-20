// ImageView.cpp : implementation of the CImageView class
//

#include "stdafx.h"
#include "Faceworx.h"
#include "FaceworxDoc.h"
#include "ImageView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BOX_WIDTH_MIN 10.0f
#define BOX_HEIGHT_MIN 10.0f

// CImageView

IMPLEMENT_DYNAMIC(CImageView, CScrollView)

BEGIN_MESSAGE_MAP(CImageView, CScrollView)
	ON_WM_CAPTURECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_MESSAGE(WM_MOUSELEAVE, &CImageView::OnMouseLeave)
	ON_COMMAND(ID_VIEW_ZOOM_IN, &CImageView::OnViewZoomIn)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_IN, &CImageView::OnUpdateViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, &CImageView::OnViewZoomOut)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_OUT, &CImageView::OnUpdateViewZoomOut)
END_MESSAGE_MAP()

// CImageView construction/destruction

CImageView::CImageView()
{
	m_nBox = -1;
	m_nNode = m_nContour = (size_t)-1;
	m_nDrag = -1;
	m_bMouseEnter = false;
	m_bScroll = false;
}

CImageView::~CImageView()
{
}

inline CSize CImageView::GetScaledImageSize(float fScale) const
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CSize size = pDoc->GetImageSize(GetSide());
	return CSize((int)(size.cx * fScale), (int)(size.cy * fScale));
}

CSize CImageView::GetOffset() const
{
	CRect rect;
	GetClientRect(&rect);

	CSize size = GetScaledImageSize(m_fScale);
	CSize offset(0, 0);
	if (rect.Width() > size.cx)
		offset.cx = (rect.Width() - size.cx) / 2;
	if (rect.Height() > size.cy)
		offset.cy = (rect.Height() - size.cy) / 2;

	return offset;
}

// CImageView drawing

void CImageView::DrawImage(CDC& dc, CPoint pt, float fScale) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	dc.SetStretchBltMode(COLORONCOLOR);

	const CImage& image = pDoc->m_image[GetSide()];
	if (!image.IsNull())
		VERIFY(image.Draw(dc.m_hDC, CRect(TranslateCoords(CPointF(0.0f, 0.0f), pt, fScale), GetScaledImageSize(fScale))));
	else
		dc.FillSolidRect(CRect(TranslateCoords(CPointF(0.0f, 0.0f), pt, fScale), GetScaledImageSize(fScale)), RGB(255, 255, 255));
}

void CImageView::DrawBox(CDC& dc, CPoint pt, float fScale) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int nSide = GetSide();
	const CRectF& bounds = pDoc->m_bounds[nSide];

	CPen pen1(PS_DOT, 1, RGB(0, 0, 0));
	CPen* pOldPen = dc.SelectObject(&pen1);
	ASSERT(pOldPen != NULL);

	const CPointFArray& nodePos = pDoc->m_nodePos[nSide];
	const CContourArray& contours = pDoc->m_contours[nSide];

	if (contours.GetCount() > 0)
	{
		const CContour& contour = contours[0];
		for (size_t i = 0; i < contour.m_segments.GetCount(); i++)
		{
			const CSegment& segment = contour.m_segments[i];

			CPointF pt1 = nodePos[segment.m_nNode1];
			dc.MoveTo(TranslateCoords(pt1, pt, fScale));

			CPointF pt2 = nodePos[segment.m_nNode2];
			VERIFY(dc.LineTo(TranslateCoords(pt2, pt, fScale)));
		}
	}

	CPen pen2(PS_DOT, 1, RGB(m_nBox == 8 ? 255 : 0, 0, 0));
	VERIFY(dc.SelectObject(&pen2));

	dc.MoveTo(TranslateCoords(bounds.TopLeft(), pt, fScale));
	VERIFY(dc.LineTo(TranslateCoords(CPointF(bounds.right, bounds.top), pt, fScale)));
	VERIFY(dc.LineTo(TranslateCoords(bounds.BottomRight(), pt, fScale)));
	VERIFY(dc.LineTo(TranslateCoords(CPointF(bounds.left, bounds.bottom), pt, fScale)));
	VERIFY(dc.LineTo(TranslateCoords(bounds.TopLeft(), pt, fScale)));

	CPen pen3(PS_SOLID, 1, RGB(255, 255, 255));
	VERIFY(dc.SelectObject(&pen3));

	CBrush brush1(RGB(0, 0, 0));
	CBrush brush2(RGB(255, 0, 0));
	CBrush* pOldBrush = dc.SelectObject(&brush1);
	ASSERT(pOldBrush != NULL);

	for (int i = 0; i < 8; i++)
	{
		if (m_nBox != i && m_nBox != 8)
			VERIFY(dc.SelectObject(&brush1));
		else
			VERIFY(dc.SelectObject(&brush2));

		switch (i)
		{
		case 0:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(bounds.TopLeft(), pt, fScale))));
			break;
		case 1:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(CPointF(bounds.right, bounds.top), pt, fScale))));
			break;
		case 2:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(bounds.BottomRight(), pt, fScale))));
			break;
		case 3:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(CPointF(bounds.left, bounds.bottom), pt, fScale))));
			break;
		case 4:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(CPointF((bounds.left + (bounds.right - bounds.left) / 2),
				bounds.top), pt, fScale))));
			break;
		case 5:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(CPointF(bounds.right,
				(bounds.top + (bounds.bottom - bounds.top) / 2)), pt, fScale))));
			break;
		case 6:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(CPointF((bounds.left + (bounds.right - bounds.left) / 2),
				bounds.bottom), pt, fScale))));
			break;
		case 7:
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(CPointF(bounds.left,
				(bounds.top + (bounds.bottom - bounds.top) / 2)), pt, fScale))));
			break;
		}
	}

	VERIFY(dc.SelectObject(pOldPen));
	VERIFY(dc.SelectObject(pOldBrush));
}

void CImageView::DrawContours(CDC& dc, CPoint pt, float fScale) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CPen pen1(PS_DOT, 1, RGB(0, 0, 0));
	CPen pen2(PS_DOT, 1, RGB(255, 0, 0));
	CPen* pOldPen = dc.SelectObject(&pen1);
	ASSERT(pOldPen != NULL);

	int nSide = GetSide();
	const CNodeArray& nodes = pDoc->m_nodes[nSide];
	const CPointFArray& nodePos = pDoc->m_nodePos[nSide];
	const CContourArray& contours = pDoc->m_contours[nSide];

	for (size_t i = 0; i < contours.GetCount(); i++)
	{
		const CContour& contour = contours[i];

		if (i != m_nContour)
			VERIFY(dc.SelectObject(&pen1));
		else
			VERIFY(dc.SelectObject(&pen2));

		if (contour.m_nLevel <= pDoc->m_nLevel)
		{
			for (size_t i = 0; i < contour.m_segments.GetCount(); i++)
			{
				const CSegment& segment = contour.m_segments[i];

				CPointF pt1 = nodePos[segment.m_nNode1];
				dc.MoveTo(TranslateCoords(pt1, pt, fScale));

				CPointF pt2 = nodePos[segment.m_nNode2];
				VERIFY(dc.LineTo(TranslateCoords(pt2, pt, fScale)));
			}
		}
	}

	CPen pen3(PS_SOLID, 1, RGB(255, 255, 255));
	VERIFY(dc.SelectObject(&pen3));

	CBrush brush1(RGB(0, 0, 0));
	CBrush brush2(RGB(255, 0, 0));
	CBrush* pOldBrush = dc.SelectObject(&brush1);
	ASSERT(pOldBrush != NULL);

	for (size_t i = 0; i < nodes.GetCount(); i++)
	{
		const CNode& node = nodes[i];

		if (i != m_nNode && (m_nContour == (size_t)-1 || node.m_contours.Lookup(m_nContour) == NULL))
			VERIFY(dc.SelectObject(&brush1));
		else
			VERIFY(dc.SelectObject(&brush2));

		if (node.m_nLevel <= pDoc->m_nLevel)
			VERIFY(dc.Rectangle(GetHandleRect(TranslateCoords(nodePos[i], pt, fScale))));
	}

	VERIFY(dc.SelectObject(pOldPen));
	VERIFY(dc.SelectObject(pOldBrush));
}

void CImageView::Draw(CDC& dc, CPoint pt, float fScale) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	DrawImage(dc, pt, fScale);

	if (pDoc->m_bBox)
		DrawBox(dc, pt, fScale);

	if (pDoc->m_bFeatures)
		DrawContours(dc, pt, fScale);
}

void CImageView::OnDraw(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	CPoint pt = GetScrollPosition();

	CDC dc;
	CBitmap bitmap;
	CBitmap* pOldBitmap;

	VERIFY(dc.CreateCompatibleDC(pDC));
	VERIFY(bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()));

	pOldBitmap = dc.SelectObject(&bitmap);
	ASSERT(pOldBitmap != NULL);

	CBrush brush;
	VERIFY(brush.CreateSysColorBrush(COLOR_BTNFACE));

	dc.FillRect(rect, &brush);

	Draw(dc, pt - GetOffset(), m_fScale);

	VERIFY(pDC->BitBlt(pt.x, pt.y, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY));

	VERIFY(dc.SelectObject(pOldBitmap));
}

// CImageView diagnostics

#ifdef _DEBUG
void CImageView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CImageView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CFaceworxDoc* CImageView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceworxDoc)));
	return (CFaceworxDoc*)m_pDocument;
}
#endif //_DEBUG

float CImageView::GetScale()
{
	return m_fScale;
}

void CImageView::SetScale(float fScale)
{
	m_fScale = fScale;
}

int CImageView::HitTestBox(CPoint point) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CPointF pt = TranslateCoords(point, GetScrollPosition() - GetOffset(), m_fScale);
	float d = HANDLE_SIZE / 2.0f / m_fScale;

	int nSide = GetSide();
	const CRectF& bounds = pDoc->m_bounds[nSide];

	CRectF rect;
	CSizeF size(0.0f, 0.0f);

	rect = CRectF(bounds.TopLeft(), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 0;

	rect = CRectF(CPointF(bounds.right, bounds.top), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 1;

	rect = CRectF(bounds.BottomRight(), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 2;

	rect = CRectF(CPointF(bounds.left, bounds.bottom), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 3;

	rect = CRectF(CPointF(bounds.left + (bounds.right - bounds.left) / 2, bounds.top), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 4;

	rect = CRectF(CPointF(bounds.right, bounds.top + (bounds.bottom - bounds.top) / 2), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 5;

	rect = CRectF(CPointF(bounds.left + (bounds.right - bounds.left) / 2, bounds.bottom), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 6;

	rect = CRectF(CPointF(bounds.left, bounds.top + (bounds.bottom - bounds.top) / 2), size);
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 7;

	rect.SetRect(bounds.TopLeft(), CPointF(bounds.right, bounds.top));
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 8;

	rect.SetRect(bounds.TopLeft(), CPointF(bounds.right, bounds.top));
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 8;

	rect.SetRect(CPointF(bounds.right, bounds.top), bounds.BottomRight());
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 8;

	rect.SetRect(CPointF(bounds.left, bounds.bottom), bounds.BottomRight());
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 8;

	rect.SetRect(bounds.TopLeft(), CPointF(bounds.left, bounds.bottom));
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
		return 8;

	return -1;
}

size_t CImageView::HitTestNode(CPoint point) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int nSide = GetSide();
	const CNodeArray& nodes = pDoc->m_nodes[nSide];
	const CPointFArray& nodePos = pDoc->m_nodePos[nSide];

	CPointF pt = TranslateCoords(point, GetScrollPosition() - GetOffset(), m_fScale);
	float d = HANDLE_SIZE / 2.0f / m_fScale;

	for (size_t i = 0; i < nodes.GetCount(); i++)
	{
		const CNode& node = nodes[i];
		if (node.m_nLevel <= pDoc->m_nLevel)
		{
			CPointF pt2 = nodePos[i];
			CRectF rect(pt2.x, pt2.y, pt2.x, pt2.y);
			rect.InflateRect(d, d);
			if (rect.PtInRect(pt))
				return i;
		}
	}

	return (size_t)-1;
}

bool PtOnSegment(CPointF pt, CPointF pt1, CPointF pt2, float d)
{
	CRectF rect(pt1, pt2);
	rect.NormalizeRect();
	rect.InflateRect(d, d);
	if (rect.PtInRect(pt))
	{
		int s;
		CPointF pt11, pt12, pt21, pt22;
		if ((pt1.x < pt2.x && pt1.y < pt2.y) || (pt1.x >= pt2.x && pt1.y >= pt2.y))
		{
			s = pt1.x >= pt2.x && pt1.y >= pt2.y ? 1 : -1;
			pt11.SetPoint(pt1.x + d, pt1.y - d);
			pt12.SetPoint(pt2.x + d, pt2.y - d);
			pt21.SetPoint(pt1.x - d, pt1.y + d);
			pt22.SetPoint(pt2.x - d, pt2.y + d);
		}
		else
		{
			s = pt1.x >= pt2.x && pt1.y < pt2.y ? 1 : -1;
			pt11.SetPoint(pt1.x - d, pt1.y - d);
			pt12.SetPoint(pt2.x - d, pt2.y - d);
			pt21.SetPoint(pt1.x + d, pt1.y + d);
			pt22.SetPoint(pt2.x + d, pt2.y + d);
		}

		float a1 = pt12.y - pt11.y;
		float b1 = -(pt12.x - pt11.x);
		float c1 = -pt11.x * (pt12.y - pt11.y) + pt11.y * (pt12.x - pt11.x);

		float a2 = pt22.y - pt21.y;
		float b2 = -(pt22.x - pt21.x);
		float c2 = -pt21.x * (pt22.y - pt21.y) + pt21.y * (pt22.x - pt21.x);

		if ((a1 * pt.x + b1 * pt.y + c1) * s > 0 && (a2 * pt.x + b2 * pt.y + c2) * s < 0)
			return true;
	}

	return false;
}

size_t CImageView::HitTestContour(CPoint point) const
{
	const CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int nSide = GetSide();
	const CPointFArray& nodePos = pDoc->m_nodePos[nSide];
	const CContourArray& contours = pDoc->m_contours[nSide];

	CPointF pt = TranslateCoords(point, GetScrollPosition() - GetOffset(), m_fScale);
	float d = HANDLE_SIZE / 2.0f / m_fScale;

	for (size_t i = 0; i < contours.GetCount(); i++)
	{
		const CContour& contour = contours[i];
		if (contour.m_nLevel <= pDoc->m_nLevel)
		{
			for (size_t j = 0; j < contour.m_segments.GetCount(); j++)
			{
				const CSegment& segment = contour.m_segments[j];

				CPointF pt1 = nodePos[segment.m_nNode1];
				CPointF pt2 = nodePos[segment.m_nNode2];

				if (PtOnSegment(pt, pt1, pt2, d))
					return i;
			}
		}
	}

	return (size_t)-1;
}

bool CImageView::SetCursor(UINT nHitTest)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (nHitTest == HTCLIENT)
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		if (pDoc->m_bBox)
		{
			int nBox = m_nBox;
			m_nBox = HitTestBox(point);
			if (m_nBox != -1)
			{
				switch (m_nBox)
				{
				case 0:
				case 2:
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_TRACKNWSE));
					break;
				case 1:
				case 3:
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_TRACKNESW));
					break;
				case 4:
				case 6:
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_TRACKNS));
					break;
				case 5:
				case 7:
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_TRACKWE));
					break;
				case 8:
					::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_MOVE4WAY));
					break;
				}
			}

			if (nBox != m_nBox)
			{
				Invalidate(FALSE);
				((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
			}

			if (m_nBox != -1)
			{
				m_nNode =  m_nContour = -1;
				return true;
			}
		}

		if (pDoc->m_bFeatures)
		{
			size_t nNode = m_nNode;
			size_t nContour = m_nContour;
			m_nNode = HitTestNode(point);
			if (m_nNode != (size_t)-1)
				m_nContour = (size_t)-1;
			else
				m_nContour = HitTestContour(point);

			if (nNode != m_nNode || nContour != m_nContour)
			{
				Invalidate(FALSE);
				((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
			}

			if (m_nNode != (size_t)-1 || m_nContour != (size_t)-1)
			{
				::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_MOVE4WAY));
				return true;
			}
		}
	}

	return false;
}

bool CImageView::BeginDrag(CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (GetCapture() != NULL)
		return false;

	int nSide = GetSide();

	if (pDoc->m_bBox)
	{
		m_nBox = HitTestBox(point);
		if (m_nBox != -1)
			m_nDrag = m_nBox == 8 ? 0 : 1;
	}

	if (m_nDrag == -1 && pDoc->m_bFeatures)
	{
		if ((m_nNode = HitTestNode(point)) != (size_t)-1)
		{
			m_nodeWeights.RemoveAll();
			m_vertexWeights.RemoveAll();

			pDoc->GetNodeWeightsByNode(nSide, m_nNode, m_nodeWeights);
			pDoc->GetVertexWeights(nSide, m_nodeWeights, m_vertexWeights);

			m_nDrag = 2;
			m_nContour = (size_t)-1;
		}
		else if ((m_nContour = HitTestContour(point)) != (size_t)-1)
		{
			if (m_nContour != pDoc->m_nContour)
			{
				m_nodeWeights.RemoveAll();
				m_vertexWeights.RemoveAll();

				pDoc->GetNodeWeightsByContour(nSide, m_nContour, m_nodeWeights);
				pDoc->GetVertexWeights(nSide, m_nodeWeights, m_vertexWeights);

				m_nDrag = 2;
			}
			else
				m_nDrag = 0;
		}
	}

	if (m_nDrag != -1)
	{
		pDoc->BeginState();

		m_bModified = false;
		m_ptSaved = point;

		SetCapture();

		return true;
	}

	return false;
}

void CImageView::DoDrag(CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CSizeF offset((point.x - m_ptSaved.x) / m_fScale, (point.y - m_ptSaved.y) / m_fScale);
	if (offset.cx != 0.0f || offset.cy != 0.0f)
	{
		bool bSave = true;
		int nSide = GetSide();

		switch (m_nDrag)
		{
		case 0:
			{
				pDoc->SetBounds(nSide, pDoc->m_bounds[nSide] + offset);
				m_ptSaved = point;
				break;
			}
		case 1:
			{
				CPoint ptSaved = m_ptSaved;
				m_ptSaved = point;
				CRectF bounds(pDoc->m_bounds[nSide]);
				CSizeF size = bounds.Size();
				if (m_nBox == 0 || m_nBox == 3 || m_nBox == 7)
				{
					bounds.left += offset.cx;
					if (bounds.Width() < BOX_WIDTH_MIN)
					{
						bounds.left = bounds.right - BOX_WIDTH_MIN;
						m_ptSaved.x = ptSaved.x + (int)((size.cx - BOX_WIDTH_MIN) * m_fScale);
					}
				}
				if (m_nBox == 0 || m_nBox == 1 || m_nBox == 4)
				{
					bounds.top += offset.cy;
					if (bounds.Height() < BOX_HEIGHT_MIN)
					{
						bounds.top = bounds.bottom - BOX_HEIGHT_MIN;
						m_ptSaved.y = ptSaved.y + (int)((size.cy - BOX_HEIGHT_MIN) * m_fScale);
					}
				}
				if (m_nBox == 1 || m_nBox == 2 || m_nBox == 5)
				{
					bounds.right += offset.cx;
					if (bounds.Width() < BOX_WIDTH_MIN)
					{
						bounds.right = bounds.left + BOX_WIDTH_MIN;
						m_ptSaved.x = ptSaved.x - (int)((size.cx - BOX_WIDTH_MIN) * m_fScale);
					}
				}
				if (m_nBox == 2 || m_nBox == 3 || m_nBox == 6)
				{
					bounds.bottom += offset.cy;
					if (bounds.Height() < BOX_HEIGHT_MIN)
					{
						bounds.bottom = bounds.top + BOX_HEIGHT_MIN;
						m_ptSaved.y = ptSaved.y - (int)((size.cy - BOX_HEIGHT_MIN) * m_fScale);
					}
				}

				pDoc->SetBounds(nSide, bounds);
				break;
			}
		default:
			{
				pDoc->MoveNodes(nSide, offset, m_nodeWeights, m_vertexWeights);
				m_ptSaved = point;
				break;
			}
		}

		m_bModified = true;

		pDoc->CreateEnhancedMesh();

		Invalidate(FALSE);
		((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
		((CFaceworxApp*)AfxGetApp())->Update3DView();
	}
}

void CImageView::EndDrag()
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

	m_nDrag = -1;

	ReleaseCapture();

	CalcScrollSizes();
}

void CImageView::CancelDrag()
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->CancelState();

	ReleaseCapture();

	m_nDrag = -1;

	CalcScrollSizes();

	pDoc->CreateEnhancedMesh();
	Invalidate(FALSE);
	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
	((CFaceworxApp*)AfxGetApp())->Update3DView();
}

bool CImageView::BeginScroll(CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (GetCapture() != NULL)
		return false;

	m_bScroll = true;
	m_ptSaved = point;

	SetCapture();

	return true;
}

void CImageView::DoScroll(CPoint point)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CPoint pt = GetScrollPosition();

	CRect rect;
	GetClientRect(&rect);
	CSize size = GetScaledImageSize(m_fScale);

	CSize offset(point - m_ptSaved);
	if (size.cx < rect.Width())
		offset.cx = 0;
	if (size.cy < rect.Height())
		offset.cy = 0;

	ScrollToPosition(pt - offset);

	m_ptSaved = point;

	Invalidate(FALSE);
	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
}

void CImageView::EndScroll()
{
	ReleaseCapture();
	m_bScroll = false;
}

void CImageView::CalcScrollSizes()
{
	SetScrollSizes(MM_TEXT, GetScaledImageSize(m_fScale));
}

void CImageView::AdjustScale()
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect rect;
	GetClientRect(&rect);

	CSize size = pDoc->GetImageSize(GetSide());
	m_fScale = min((float)rect.Width() / (size.cx + 100), (float)rect.Height() / (size.cy + 100));
	m_nZoom = (int)(log(m_fScale * 100.0f / 10.0f) / 0.2f);

	CalcScrollSizes();
}

void CImageView::EnableScrollBarCtrl(int nBar, BOOL bEnable)
{
	CScrollBar* pScrollBar;
	if (nBar == SB_BOTH)
	{
		EnableScrollBarCtrl(SB_HORZ, bEnable);
		EnableScrollBarCtrl(SB_VERT, bEnable);
	}
	else if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		// control scrollbar - enable or disable
		pScrollBar->EnableWindow(bEnable);
	}
	else
	{
		// WS_?SCROLL scrollbar - show or hide
		//ShowScrollBar(nBar, bEnable);
		EnableScrollBar(nBar, bEnable ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	}
}

void CImageView::SetScrollSizes(int nMapMode, SIZE sizeTotal,
	const SIZE& sizePage, const SIZE& sizeLine)
{
	ASSERT(sizeTotal.cx >= 0 && sizeTotal.cy >= 0);
	ASSERT(nMapMode > 0);
	ASSERT(nMapMode != MM_ISOTROPIC && nMapMode != MM_ANISOTROPIC);

	int nOldMapMode = m_nMapMode;
	m_nMapMode = nMapMode;
	m_totalLog = sizeTotal;

	//BLOCK: convert logical coordinate space to device coordinates
	{
		CWindowDC dc(NULL);
		ASSERT(m_nMapMode > 0);
		dc.SetMapMode(m_nMapMode);

		// total size
		m_totalDev = m_totalLog;
		dc.LPtoDP((LPPOINT)&m_totalDev);
		m_pageDev = sizePage;
		dc.LPtoDP((LPPOINT)&m_pageDev);
		m_lineDev = sizeLine;
		dc.LPtoDP((LPPOINT)&m_lineDev);
		if (m_totalDev.cy < 0)
			m_totalDev.cy = -m_totalDev.cy;
		if (m_pageDev.cy < 0)
			m_pageDev.cy = -m_pageDev.cy;
		if (m_lineDev.cy < 0)
			m_lineDev.cy = -m_lineDev.cy;
	} // release DC here

	// now adjust device specific sizes
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
	if (m_pageDev.cx == 0)
		m_pageDev.cx = m_totalDev.cx / 10;
	if (m_pageDev.cy == 0)
		m_pageDev.cy = m_totalDev.cy / 10;
	if (m_lineDev.cx == 0)
		m_lineDev.cx = m_pageDev.cx / 10;
	if (m_lineDev.cy == 0)
		m_lineDev.cy = m_pageDev.cy / 10;

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBars();
		if (nOldMapMode != m_nMapMode)
			Invalidate(TRUE);
	}
}

void CImageView::ScrollToPosition(POINT pt)    // logical coordinates
{
	ASSERT(m_nMapMode > 0);     // not allowed for shrink to fit
	if (m_nMapMode != MM_TEXT)
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.LPtoDP((LPPOINT)&pt);
	}

	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);
	if (pt.x < 0)
		pt.x = 0;
	else if (pt.x > xMax)
		pt.x = xMax;
	if (pt.y < 0)
		pt.y = 0;
	else if (pt.y > yMax)
		pt.y = yMax;

	ScrollToDevicePosition(pt);
}

void CImageView::ScrollToDevicePosition(POINT ptDev)
{
	ASSERT(ptDev.x >= 0);
	ASSERT(ptDev.y >= 0);

	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CScrollView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.

	int xOrig = GetScrollPos(SB_HORZ);
	SetScrollPos(SB_HORZ, ptDev.x);
	int yOrig = GetScrollPos(SB_VERT);
	SetScrollPos(SB_VERT, ptDev.y);
	//ScrollWindow(xOrig - ptDev.x, yOrig - ptDev.y);
	Invalidate(FALSE);
}

void CImageView::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = TRUE;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	CRect rectClient;
	BOOL bCalcClient = TRUE;

	// allow parent to do inside-out layout first
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
		// if parent window responds to this message, use just
		//  client area for scroll bar calc -- not "true" client area
		if ((BOOL)pParentWnd->SendMessage(WM_RECALCPARENT, 0,
			(LPARAM)(LPCRECT)&rectClient) != 0)
		{
			// use rectClient instead of GetTrueClientSize for
			//  client size calculation.
			bCalcClient = FALSE;
		}
	}

	CSize sizeClient;
	CSize sizeSb;

	if (bCalcClient)
	{
		// get client rect
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			// no room for scroll bars (common for zero sized elements)
			CRect rect;
			GetClientRect(&rect);
			if (rect.right > 0 && rect.bottom > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, FALSE);
			}
			m_bInsideUpdate = FALSE;
			return;
		}
	}
	else
	{
		// let parent window determine the "client" rect
		GetScrollBarSizes(sizeSb);
		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, bCalcClient);
	if (needSb.cx)
		sizeClient.cy -= sizeSb.cy;
	if (needSb.cy)
		sizeClient.cx -= sizeSb.cx;

	// first scroll the window as needed
	ScrollToDevicePosition(ptMove); // will set the scroll bar positions too

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, needSb.cx);
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx-1;
		if (!SetScrollInfo(SB_HORZ, &info, TRUE))
			SetScrollRange(SB_HORZ, 0, sizeRange.cx, TRUE);
	}
	EnableScrollBarCtrl(SB_VERT, needSb.cy);
	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy-1;
		if (!SetScrollInfo(SB_VERT, &info, TRUE))
			SetScrollRange(SB_VERT, 0, sizeRange.cy, TRUE);
	}

	// remove recursion lockout
	m_bInsideUpdate = FALSE;
}

// CImageView message handlers

void CImageView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	ShowScrollBar(SB_BOTH, TRUE);
	EnableScrollBar(SB_BOTH, ESB_DISABLE_BOTH);
}

void CImageView::OnCaptureChanged(CWnd *pWnd)
{
	if (m_nDrag != -1)
		CancelDrag();

	CScrollView::OnCaptureChanged(pWnd);
}

BOOL CImageView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		if (m_nDrag != -1)
		{
			CancelDrag();
			return;
		}

		if (m_bScroll)
		{
			EndScroll();
			return;
		}
	}

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (BeginDrag(point) || BeginScroll(point))
		return;

	CScrollView::OnLButtonDown(nFlags, point);
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nDrag != -1)
	{
		DoDrag(point);
		EndDrag();
		return;
	}

	if (m_bScroll)
	{
		DoScroll(point);
		EndScroll();
		return;
	}

	CScrollView::OnLButtonUp(nFlags, point);
}

LRESULT CImageView::OnMouseLeave(WPARAM, LPARAM)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (m_nBox != -1 || m_nNode != (size_t)-1 || m_nContour != (size_t)-1)
	{
		m_nBox = -1;
		m_nNode = m_nContour = (size_t)-1;
	}

	Invalidate(FALSE);
	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();

	m_bMouseEnter = false;

	return 0;
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point)
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

	if (m_nDrag != -1)
	{
		DoDrag(point);
		return;
	}

	if (m_bScroll)
	{
		DoScroll(point);
		return;
	}

	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();

	CScrollView::OnMouseMove(nFlags, point);
}

BOOL CImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	m_nZoom += zDelta / WHEEL_DELTA;
	if (m_nZoom < 0)
		m_nZoom = 0;
	else if (m_nZoom > 29)
		m_nZoom = 29;

	CRect rect;
	GetClientRect(&rect);

	CPoint pt1 = GetScrollPosition();

	CSize size1 = GetScaledImageSize(m_fScale);

	float fScale = 10.0f * exp(m_nZoom * 0.2f) / 100.0f;
	CSize size2 = GetScaledImageSize(fScale);

	if (rect.Width() < size2.cx)
	{
		if (rect.Width() > size1.cx)
			pt1.x = (size2.cx - rect.Width()) / 2;
		else
			pt1.x = (int)((pt1.x + rect.Width() / 2) / m_fScale * fScale - rect.Width() / 2);
	}
	else
		pt1.x = 0;

	if (rect.Height() < size2.cy)
	{
		if (rect.Height() > size1.cy)
			pt1.y = (size2.cy - rect.Height()) / 2;
		else
			pt1.y = (int)((pt1.y + rect.Height() / 2) / m_fScale * fScale - rect.Height() / 2);
	}
	else
		pt1.y = 0;

	m_fScale = fScale;

	CalcScrollSizes();
	ScrollToPosition(pt1);
	SetCursor(HTCLIENT);

	Invalidate(FALSE);

	return TRUE;
}

void CImageView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nDrag != -1)
	{
		CancelDrag();
		return;
	}

	if (m_bScroll)
	{
		EndScroll();
		return;
	}

	CScrollView::OnRButtonDown(nFlags, point);
}

BOOL CImageView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	int xOrig, x;
	int yOrig, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		sizeScroll.cy = 0;
	}
	pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0;
	}

	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	int xMax = GetScrollLimit(SB_HORZ);
	x += sizeScroll.cx;
	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	yOrig = y = GetScrollPos(SB_VERT);
	int yMax = GetScrollLimit(SB_VERT);
	y += sizeScroll.cy;
	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig)
		return FALSE;

	if (bDoScroll)
	{
		// do scroll and update scroll positions
		//ScrollWindow(-(x-xOrig), -(y-yOrig));
		Invalidate(FALSE);
		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);
		if (y != yOrig)
			SetScrollPos(SB_VERT, y);
	}
	return TRUE;
}

BOOL CImageView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (pWnd == this && SetCursor(nHitTest))
		return TRUE;

	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CImageView::OnSize(UINT nType, int cx, int cy)
{
	//CScrollView::OnSize(nType, cx, cy);
	CView::OnSize(nType, cx, cy);
	UpdateBars();
	AdjustScale();
}

void CImageView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	Invalidate(FALSE);
	((CMainFrame*)AfxGetMainWnd())->UpdateMagnifier();
}

void CImageView::OnViewZoomIn()
{
	// TODO: Add your command handler code here
}

void CImageView::OnUpdateViewZoomIn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CImageView::OnViewZoomOut()
{
	// TODO: Add your command handler code here
}

void CImageView::OnUpdateViewZoomOut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CImageView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	AfxGetMainWnd()->Invalidate(FALSE);

	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
