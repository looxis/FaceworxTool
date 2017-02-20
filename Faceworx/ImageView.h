// ImageView.h : interface of the CImageView class
//

#pragma once

#include "Types.h"

class CImageView : public CScrollView
{
protected: // create from serialization only
	CImageView();
	DECLARE_DYNAMIC(CImageView)

	// Attributes
public:
	CFaceworxDoc* GetDocument() const;
	float GetScale();
	void SetScale(float fScale);

	// Operations
public:
	void AdjustScale();

	// Overrides
public:
	virtual void OnDraw(CDC* pDC); // overridden to draw this view

	// Implementation
public:
	virtual ~CImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nZoom;
	float m_fScale;
	int m_nBox;
	size_t m_nNode, m_nContour;
	int m_nDrag;
	CPoint m_ptSaved;
	bool m_bMouseEnter;
	bool m_bModified;
	bool m_bScroll;

	CNodeWeightMap m_nodeWeights;
	CVertexWeightMap m_vertexWeights;

	virtual int GetSide() const = 0;

	CSize GetScaledImageSize(float fScale) const;
	CSize GetOffset() const;

	void DrawImage(CDC& dc, CPoint pt, float fScale) const;
	void DrawBox(CDC& dc, CPoint pt, float fScale) const;
	void DrawContours(CDC& dc, CPoint pt, float fScale) const;
	virtual void Draw(CDC& dc, CPoint pt, float fScale) const;

	int HitTestBox(CPoint point) const;
	size_t HitTestNode(CPoint point) const;
	size_t HitTestContour(CPoint point) const;

	virtual bool SetCursor(UINT nHitTest);

	bool BeginDrag(CPoint point);
	void DoDrag(CPoint point);
	void EndDrag();
	void CancelDrag();

	bool BeginScroll(CPoint point);
	void DoScroll(CPoint point);
	void EndScroll();

	void CalcScrollSizes();

	void EnableScrollBarCtrl(int nBar, BOOL bEnable = TRUE);
	void SetScrollSizes(int nMapMode, SIZE sizeTotal,
		const SIZE& sizePage = sizeDefault,
		const SIZE& sizeLine = sizeDefault);
	void ScrollToPosition(POINT pt);    // set upper left position
	void ScrollToDevicePosition(POINT ptDev); // explicit scrolling no checking
	void UpdateBars();          // adjust scrollbars etc

	friend class CMagnifierDlg;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

public:
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewZoomIn();
	afx_msg void OnUpdateViewZoomIn(CCmdUI *pCmdUI);
	afx_msg void OnViewZoomOut();
	afx_msg void OnUpdateViewZoomOut(CCmdUI *pCmdUI);
	afx_msg void OnCaptureChanged(CWnd *pWnd);

protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
};

#define HANDLE_SIZE 5

inline CRect GetHandleRect(CPoint point)
{
	return CRect(point.x - HANDLE_SIZE / 2, point.y - HANDLE_SIZE / 2,
		point.x - HANDLE_SIZE / 2 + HANDLE_SIZE, point.y - HANDLE_SIZE / 2 + HANDLE_SIZE);
}

inline CPoint TranslateCoords(CPointF pt, CPoint ptOffset, float fScale)
{
	return CPoint((int)(pt.x * fScale), (int)(pt.y * fScale)) - ptOffset;
}

inline CPointF TranslateCoords(CPoint pt, CPoint ptOffset, float fScale)
{
	return CPointF((pt.x + ptOffset.x) / fScale, (pt.y + ptOffset.y) / fScale);
}

bool PtOnSegment(CPointF pt, CPointF pt1, CPointF pt2, float d);

#ifndef _DEBUG // debug version in ImageView.cpp
inline CFaceworxDoc* CImageView::GetDocument() const
{ return reinterpret_cast<CFaceworxDoc*>(m_pDocument); }
#endif
