// FaceworxView.h : interface of the CFaceworxView class
//

#pragma once

class CFaceworxView : public CView
{
protected: // create from serialization only
	CFaceworxView();
	DECLARE_DYNCREATE(CFaceworxView)

// Attributes
public:
	CFaceworxDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CFaceworxView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FaceworxView.cpp
inline CFaceworxDoc* CFaceworxView::GetDocument() const
   { return reinterpret_cast<CFaceworxDoc*>(m_pDocument); }
#endif

