// SplitterWndEx.h : interface of the CSplitterWndEx class
//

#pragma once

#define LEFT_SIDE 0
#define RIGHT_SIDE 1
#define TOP_SIDE LEFT_SIDE
#define BOTTOM_SIDE RIGHT_SIDE

class CSplitterWndEx : public CSplitterWnd
{
	DECLARE_DYNAMIC(CSplitterWndEx)

public:
	CSplitterWndEx(int nLevel = 0);
	virtual ~CSplitterWndEx();

	bool Create(CWnd* pParentWnd, CRuntimeClass* pView1, CRuntimeClass* pView2, CCreateContext* pContext,
		bool bVertical = true, int nID = AFX_IDW_PANE_FIRST);

	bool IsSideHidden(int nSide);
	void ShowSide(int nSide);
	void HideSide(int nSide);

	inline bool IsVertical() const;

	CSplitterWndEx* AddSubDivision(int nSide, CRuntimeClass* pView1, CRuntimeClass* pView2, CCreateContext* pContext,
		bool bVertical);

	int AddView(int nSide, CRuntimeClass * pViewClass, CCreateContext* pContext);
	void SwitchToView(int nSide, int nView = -1);

	virtual void RecalcLayout(); // call after changing sizes

protected:
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	virtual void DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside);
	virtual void TrackRowSize(int y, int row);

	inline void SideToRowCol(int nSide, int* nRow, int* nCol) const;

	void ShowRow();
	void HideRow(int rowHide);
	void ShowColumn();
	void HideColumn(int colHide);

	void RememberSize();

	bool HideView(int nRow, int nCol);
	bool ShowView(int nRow, int nCol, CWnd* pWnd);

	CSplitterWndEx* m_pSubSplitterWnd[2];
	int m_nHiddenCol;
	int m_nHiddenRow;
	int m_nPaneSize[2];
	int m_nPaneMinSize[2];
	bool m_bVertical;
	int m_nLevel;

	CArray<CWnd*, CWnd*> m_views[2];
	int m_nCurrentView[2];

	DWORD m_clrWindowFrame, m_clrBtnFace, m_clrBtnShadow, m_clrBtnHilite,
		m_clrActiveCaption, m_clrInactiveCaption, m_clrCaptionText, m_clrInactiveCaptionText;
	int m_cxVScroll, m_cyHScroll;

public:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
