// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Faceworx.h"
#include "MainFrm.h"
#include "FaceworxDoc.h"
#include "ImageView.h"
#include "FrontView.h"
#include "SideView.h"
#include "3DView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_FRONT, &CMainFrame::OnWindowFront)
	ON_COMMAND(ID_WINDOW_FRONT_AND_SIDE, &CMainFrame::OnWindowFrontAndSide)
	ON_COMMAND(ID_WINDOW_SIDE, &CMainFrame::OnWindowSide)
	ON_COMMAND(ID_WINDOW_FRONT_AND_3D, &CMainFrame::OnWindowFrontAnd3d)
	ON_COMMAND(ID_WINDOW_SIDE_AND_3D, &CMainFrame::OnWindowSideAnd3d)
	ON_COMMAND(ID_WINDOW_3D, &CMainFrame::OnWindow3d)
	ON_COMMAND(ID_WINDOW_ALL, &CMainFrame::OnWindowAll)
	ON_COMMAND(ID_VIEW_MAGNIFIER, &CMainFrame::OnViewMagnifier)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAGNIFIER, &CMainFrame::OnUpdateViewMagnifier)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR, // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
	delete m_pSplitter1;
	delete m_pSplitter2;
}

void CMainFrame::Update3DView()
{
	CDocument* pDoc = GetActiveDocument();
	if (pDoc != NULL)
	{
		POSITION pos = pDoc->GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = pDoc->GetNextView(pos);
			if (pView->IsKindOf(RUNTIME_CLASS(C3DView)))
			{
				pView->Invalidate(FALSE);
				break;
			}
		}
	}
}

void CMainFrame::UpdateMagnifier()
{
	if (m_magnifierDlg.m_hWnd != NULL)
		m_magnifierDlg.Invalidate(FALSE);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1; // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1; // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	m_magnifierDlg.Create(IDD_MAGNIFIERDLG, this);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	// the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

// CMainFrame message handlers

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_pSplitter1 = new CSplitterWndEx();
	m_pSplitter1->Create(this, NULL, NULL, pContext, true);
	m_pSplitter1->AddView(LEFT_SIDE, RUNTIME_CLASS(CFrontView), pContext);
	m_pSplitter1->AddView(LEFT_SIDE, RUNTIME_CLASS(CSideView), pContext);
	m_pSplitter2 = m_pSplitter1->AddSubDivision(RIGHT_SIDE, RUNTIME_CLASS(CSideView),
		RUNTIME_CLASS(C3DView), pContext, false);
	m_pSplitter1->SwitchToView(LEFT_SIDE, 0);

	CRect rect;
	GetClientRect(&rect);

	m_pSplitter1->SetColumnInfo(0, rect.Width() / 2, 0);
	m_pSplitter1->SetColumnInfo(1, rect.Width() / 2, 0);

	m_pSplitter2->SetRowInfo(0, rect.Height() / 2, 0);
	m_pSplitter2->SetRowInfo(1, rect.Height() / 2, 0);

	return TRUE;
}

void CMainFrame::OnViewMagnifier()
{
	m_magnifierDlg.ShowWindow(m_magnifierDlg.IsWindowVisible() ? SW_HIDE : SW_SHOWNA);
}

void CMainFrame::OnUpdateViewMagnifier(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_magnifierDlg.IsWindowVisible());
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnWindowFront()
{
	m_pSplitter1->ShowSide(LEFT_SIDE);
	m_pSplitter1->HideSide(RIGHT_SIDE);
	m_pSplitter1->SwitchToView(LEFT_SIDE, 0);
}

void CMainFrame::OnWindowFrontAndSide()
{
	m_pSplitter1->ShowSide(LEFT_SIDE);
	m_pSplitter1->ShowSide(RIGHT_SIDE);
	m_pSplitter2->ShowSide(TOP_SIDE);
	m_pSplitter2->HideSide(BOTTOM_SIDE);
	m_pSplitter1->SwitchToView(LEFT_SIDE, 0);
}

void CMainFrame::OnWindowSide()
{
	m_pSplitter1->ShowSide(RIGHT_SIDE);
	m_pSplitter2->ShowSide(TOP_SIDE);
	m_pSplitter1->HideSide(LEFT_SIDE);
	m_pSplitter2->HideSide(BOTTOM_SIDE);
}

void CMainFrame::OnWindowFrontAnd3d()
{
	m_pSplitter1->ShowSide(LEFT_SIDE);
	m_pSplitter1->ShowSide(RIGHT_SIDE);
	m_pSplitter2->ShowSide(BOTTOM_SIDE);
	m_pSplitter2->HideSide(TOP_SIDE);
	m_pSplitter1->SwitchToView(LEFT_SIDE, 0);
}

void CMainFrame::OnWindowSideAnd3d()
{
	m_pSplitter1->ShowSide(LEFT_SIDE);
	m_pSplitter1->ShowSide(RIGHT_SIDE);
	m_pSplitter2->ShowSide(BOTTOM_SIDE);
	m_pSplitter2->HideSide(TOP_SIDE);
	m_pSplitter1->SwitchToView(LEFT_SIDE, 1);
}

void CMainFrame::OnWindow3d()
{
	m_pSplitter1->ShowSide(RIGHT_SIDE);
	m_pSplitter2->ShowSide(BOTTOM_SIDE);
	m_pSplitter1->HideSide(LEFT_SIDE);
	m_pSplitter2->HideSide(TOP_SIDE);
}

void CMainFrame::OnWindowAll()
{
	m_pSplitter1->ShowSide(LEFT_SIDE);
	m_pSplitter1->ShowSide(RIGHT_SIDE);
	m_pSplitter2->ShowSide(BOTTOM_SIDE);
	m_pSplitter2->ShowSide(TOP_SIDE);
	m_pSplitter1->SwitchToView(LEFT_SIDE, 0);
}
