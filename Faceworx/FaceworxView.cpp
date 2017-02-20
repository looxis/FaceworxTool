// FaceworxView.cpp : implementation of the CFaceworxView class
//

#include "stdafx.h"
#include "Faceworx.h"

#include "FaceworxDoc.h"
#include "FaceworxView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFaceworxView

IMPLEMENT_DYNCREATE(CFaceworxView, CView)

BEGIN_MESSAGE_MAP(CFaceworxView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CFaceworxView construction/destruction

CFaceworxView::CFaceworxView()
{
	// TODO: add construction code here

}

CFaceworxView::~CFaceworxView()
{
}

BOOL CFaceworxView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CFaceworxView drawing

void CFaceworxView::OnDraw(CDC* /*pDC*/)
{
	CFaceworxDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

// CFaceworxView printing

BOOL CFaceworxView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFaceworxView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFaceworxView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

// CFaceworxView diagnostics

#ifdef _DEBUG
void CFaceworxView::AssertValid() const
{
	CView::AssertValid();
}

void CFaceworxView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFaceworxDoc* CFaceworxView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceworxDoc)));
	return (CFaceworxDoc*)m_pDocument;
}
#endif //_DEBUG

// CFaceworxView message handlers
