// FrontView.h : interface of the CFrontView class
//

#pragma once

class CFrontView : public CImageView
{
protected: // create from serialization only
	CFrontView();
	DECLARE_DYNCREATE(CFrontView)

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
	virtual ~CFrontView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG // debug version in 0.cpp
inline CFaceworxDoc* CFrontView::GetDocument() const
{ return reinterpret_cast<CFaceworxDoc*>(m_pDocument); }
#endif
