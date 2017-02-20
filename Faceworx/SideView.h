// SideView.h : interface of the CSideView class
//

#pragma once

class CSideView : public CImageView
{
protected: // create from serialization only
	CSideView();
	DECLARE_DYNCREATE(CSideView)

	// Attributes
public:
	CFaceworxDoc* GetDocument() const;

	// Operations
public:

protected:
	virtual int GetSide() const;

	// Overrides
public:
	virtual void OnDraw(CDC* pDC); // overridden to draw this view

	// Implementation
public:
	virtual ~CSideView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nBlendNode;
	int m_nBlendLine;
	bool m_bDrag;
	CPoint m_ptSaved;
	bool m_bModified;

	virtual void Draw(CDC& dc, CPoint ptOffset, float fScale) const;

	int HitTestBlendNode(CPoint point) const;
	int HitTestBlendLine(CPoint point) const;

	virtual bool SetCursor(UINT nHitTest);

	bool BeginDrag(CPoint point);
	void DoDrag(CPoint point);
	void EndDrag();
	void CancelDrag();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG // debug version in SideView.cpp
inline CFaceworxDoc* CSideView::GetDocument() const
{ return reinterpret_cast<CFaceworxDoc*>(m_pDocument); }
#endif
