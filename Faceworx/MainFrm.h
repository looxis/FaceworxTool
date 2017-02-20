// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "SplitterWndEx.h"
#include "MagnifierDlg.h"

class CMainFrame : public CFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

	// Attributes
public:

	// Operations
public:
	void Update3DView();
	void UpdateMagnifier();

	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected: // control bar embedded members
	CStatusBar m_wndStatusBar;
	CToolBar m_wndToolBar;
	CSplitterWndEx* m_pSplitter1;
	CSplitterWndEx* m_pSplitter2;
	CMagnifierDlg m_magnifierDlg;

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

public:
	afx_msg void OnViewMagnifier();
	afx_msg void OnUpdateViewMagnifier(CCmdUI *pCmdUI);
	afx_msg void OnWindowFront();
	afx_msg void OnWindowFrontAndSide();
	afx_msg void OnWindowSide();
	afx_msg void OnWindowFrontAnd3d();
	afx_msg void OnWindowSideAnd3d();
	afx_msg void OnWindow3d();
	afx_msg void OnWindowAll();
};
