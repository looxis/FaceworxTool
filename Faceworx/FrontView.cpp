// 0.cpp : implementation of the CFrontView class
//

#include "stdafx.h"
#include "Faceworx.h"
#include "FaceworxDoc.h"
#include "ImageView.h"
#include "FrontView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFrontView

IMPLEMENT_DYNCREATE(CFrontView, CImageView)

BEGIN_MESSAGE_MAP(CFrontView, CImageView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CFrontView construction/destruction

CFrontView::CFrontView()
{
	// TODO: add construction code here

}

CFrontView::~CFrontView()
{
}

// CFrontView drawing

void CFrontView::OnDraw(CDC* pDC)
{
	CImageView::OnDraw(pDC);
}

// CFrontView diagnostics

#ifdef _DEBUG
void CFrontView::AssertValid() const
{
	CImageView::AssertValid();
}

void CFrontView::Dump(CDumpContext& dc) const
{
	CImageView::Dump(dc);
}

CFaceworxDoc* CFrontView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceworxDoc)));
	return (CFaceworxDoc*)m_pDocument;
}
#endif //_DEBUG

int CFrontView::GetSide() const
{
	return 0;
}

// CFrontView message handlers

int CFrontView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CImageView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CString str;
	str.LoadString(IDS_FRONT_VIEW);
	SetWindowText(str);

	return 0;
}
