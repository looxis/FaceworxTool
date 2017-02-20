// SplitterWndEx.cpp : implementation of the CSplitterWndEx class
//

#include "stdafx.h"
#include "SplitterWndEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CX_BORDER 1
#define CY_BORDER 1
#define HEADER_HEIGHT 18

// CMainFrame

IMPLEMENT_DYNAMIC(CSplitterWndEx, CSplitterWnd)

BEGIN_MESSAGE_MAP(CSplitterWndEx, CSplitterWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

CSplitterWndEx::CSplitterWndEx(int nLevel/* = 0*/) : m_nHiddenCol(-1), m_nHiddenRow(-1), m_nLevel(nLevel)
{
	m_nPaneSize[0] = 100;
	m_nPaneSize[1] = 100;
	m_nPaneMinSize[0] = 10;
	m_nPaneMinSize[1] = 10;
	m_pSubSplitterWnd[0] = NULL;
	m_pSubSplitterWnd[1] = NULL;
	m_nCurrentView[0] = 0;
	m_nCurrentView[1] = 0;

	m_clrWindowFrame = GetSysColor(COLOR_WINDOWFRAME);
	m_clrBtnFace = GetSysColor(COLOR_BTNFACE);
	m_clrBtnShadow = GetSysColor(COLOR_BTNSHADOW);
	m_clrBtnHilite = GetSysColor(COLOR_BTNHIGHLIGHT);
	m_clrActiveCaption = GetSysColor(COLOR_ACTIVECAPTION);
	m_clrInactiveCaption = GetSysColor(COLOR_INACTIVECAPTION);
	m_clrCaptionText = GetSysColor(COLOR_CAPTIONTEXT);
	m_clrInactiveCaptionText = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
	m_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	m_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
}

CSplitterWndEx::~CSplitterWndEx()
{
}

bool CSplitterWndEx::Create(CWnd* pParentWnd, CRuntimeClass* pView1, CRuntimeClass* pView2, CCreateContext* pContext,
							bool bVertical, int nID)
{
	int nRow, nCol;
	m_bVertical = bVertical;
	if (bVertical)
	{
		nRow = 1;
		nCol = 2;
	}
	else
	{
		nRow = 2;
		nCol = 1;
	}

	VERIFY(CreateStatic(pParentWnd, nRow, nCol, WS_CHILD | WS_VISIBLE | WS_BORDER, nID));

	if (pView1 != NULL)
		VERIFY(CreateView(0, 0, pView1, CSize(100, 100), pContext));

	if (pView2 != NULL)
	{
		if (bVertical)
			VERIFY(CreateView(0, 1, pView2, CSize(100, 100), pContext));
		else
			VERIFY(CreateView(1, 0, pView2, CSize(100, 100), pContext));
	}

	return true;
}

inline bool CSplitterWndEx::IsVertical() const
{
	return m_bVertical;
};

bool CSplitterWndEx::IsSideHidden(int nSide)
{
	if (IsVertical())
	{
		if (m_nHiddenCol == nSide)
			return true;
	}
	else
	{
		if (m_nHiddenRow == nSide)
			return true;
	}

	return false;
}

void CSplitterWndEx::ShowColumn()
{
	ASSERT_VALID(this);
	ASSERT(m_nCols < m_nMaxCols);
	ASSERT(m_nHiddenCol != -1);

	int colNew = m_nHiddenCol;
	m_nHiddenCol = -1;
	m_nCols++; // add a column
	ASSERT(m_nCols == m_nMaxCols);

	// fill the hidden column
	int col;
	for (int row = 0; row < m_nRows; row++)
	{
		CWnd* pPaneShow = GetDlgItem(AFX_IDW_PANE_FIRST + row * 16 + m_nCols);
		ASSERT(pPaneShow != NULL);
		pPaneShow->ShowWindow(SW_SHOWNA);

		for (col = m_nCols - 2; col >= colNew; col--)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row, col + 1));
		}

		pPaneShow->SetDlgCtrlID(IdFromRowCol(row, colNew));
	}
	// new panes have been created -- recalculate layout
	RecalcLayout();
}

void CSplitterWndEx::HideColumn(int colHide)
{
	ASSERT_VALID(this);
	ASSERT(m_nCols > 1);
	ASSERT(colHide < m_nCols);
	ASSERT(m_nHiddenCol == -1);
	if (m_nHiddenCol != -1)
		return;

	RememberSize();

	m_nHiddenCol = colHide;

	// if the column has an active window -- change it
	int rowActive, colActive;
	if (GetActivePane(&rowActive, &colActive) != NULL && colActive == colHide)
	{
		if (++colActive >= m_nCols)
			colActive = 0;
		SetActivePane(rowActive, colActive);
	}

	// hide all column panes
	for (int row = 0; row < m_nRows; row++)
	{
		CWnd* pPaneHide = GetPane(row, colHide);
		ASSERT(pPaneHide != NULL);
		pPaneHide->ShowWindow(SW_HIDE);
		pPaneHide->SetDlgCtrlID(AFX_IDW_PANE_FIRST + row * 16 + m_nCols);

		for (int col = colHide + 1; col < m_nCols; col++)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row, col - 1));
		}
	}
	m_nCols--;
	m_pColInfo[m_nCols].nCurSize = m_pColInfo[colHide].nCurSize;
	RecalcLayout();
}

void CSplitterWndEx::ShowRow()
{
	ASSERT_VALID(this);
	ASSERT(m_nRows < m_nMaxRows);
	ASSERT(m_nHiddenRow != -1);

	int rowNew = m_nHiddenRow;
	m_nHiddenRow = -1;
	m_nRows++; // add a row
	ASSERT(m_nRows == m_nMaxRows);

	// fill the hidden row
	int row;
	for (int col = 0; col < m_nCols; col++)
	{
		CWnd* pPaneShow = GetDlgItem(AFX_IDW_PANE_FIRST + m_nRows * 16 + col);
		ASSERT(pPaneShow != NULL);
		pPaneShow->ShowWindow(SW_SHOWNA);

		for (row = m_nRows - 2; row >= rowNew; row--)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row + 1, col));
		}

		pPaneShow->SetDlgCtrlID(IdFromRowCol(rowNew, col));
	}

	// new panes have been created -- recalculate layout
	RecalcLayout();
}

void CSplitterWndEx::HideRow(int rowHide)
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 1);
	ASSERT(rowHide < m_nRows);
	ASSERT(m_nHiddenRow == -1);
	if (m_nHiddenRow != -1)
		return;

	RememberSize();

	m_nHiddenRow = rowHide;

	// if the column has an active window -- change it
	int rowActive, colActive;
	if (GetActivePane(&rowActive, &colActive) != NULL && rowActive == rowHide)
	{
		if (++rowActive >= m_nRows)
			rowActive = 0;
		SetActivePane(rowActive, colActive);
	}

	// hide all row panes
	for (int col = 0; col < m_nCols; col++)
	{
		CWnd* pPaneHide = GetPane(rowHide, col);
		ASSERT(pPaneHide != NULL);
		pPaneHide->ShowWindow(SW_HIDE);
		pPaneHide->SetDlgCtrlID(AFX_IDW_PANE_FIRST + m_nRows * 16);

		for (int row = rowHide + 1; row < m_nRows; row++)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row - 1, col));
		}
	}
	m_nRows--;
	m_pRowInfo[m_nRows].nCurSize = m_pRowInfo[rowHide].nCurSize;

	RecalcLayout();
}

void CSplitterWndEx::ShowSide(int nSide)
{
	if (IsVertical())
	{
		if (m_nHiddenCol == nSide) // show this row, only if this row is hidden
			ShowColumn();
	}
	else
	{
		if (m_nHiddenRow == nSide) // show this column, only if this column is hidden
			ShowRow();
	}
}

void CSplitterWndEx::HideSide(int nSide)
{
	if (IsVertical())
	{
		if (m_nHiddenCol == -1) // can only hide this row, if the other row in not hidden
			HideColumn(nSide);
	}
	else
	{
		if (m_nHiddenRow == -1) // can only hide this column, if the other colum in not hidden
			HideRow(nSide);
	}
}

void CSplitterWndEx::RememberSize()
{
	if (m_pSubSplitterWnd[0] != NULL)
		m_pSubSplitterWnd[0]->RememberSize();
	if (m_pSubSplitterWnd[1] != NULL)
		m_pSubSplitterWnd[1]->RememberSize();

	if (IsVertical())
	{
		if (m_nHiddenCol == -1)
		{
			// if not hidden
			GetColumnInfo(0, m_nPaneSize[0], m_nPaneMinSize[0]);
			GetColumnInfo(1, m_nPaneSize[1], m_nPaneMinSize[1]);
		}
	}
	else
	{
		if (m_nHiddenRow == -1)
		{
			// if not hidden
			GetRowInfo(0, m_nPaneSize[0], m_nPaneMinSize[0]);
			GetRowInfo(1, m_nPaneSize[1], m_nPaneMinSize[1]);
		}
	}
}

inline void CSplitterWndEx::SideToRowCol(int nSide, int* nRow, int* nCol) const
{
	if (m_bVertical)
	{
		*nRow = 0;
		*nCol = nSide;
	}
	else
	{
		*nRow = nSide;
		*nCol = 0;
	}
};

CSplitterWndEx* CSplitterWndEx::AddSubDivision(int nSide, CRuntimeClass* pView1, CRuntimeClass* pView2,
											   CCreateContext* pContext, bool bVertical)
{
	ASSERT(nSide == 0 || nSide == 1);
	ASSERT(m_pSubSplitterWnd[nSide] == NULL);

	int nRow, nCol;
	SideToRowCol(nSide, &nRow, &nCol);

	int nID = IdFromRowCol(nRow, nCol);
	m_pSubSplitterWnd[nSide] = new CSplitterWndEx(m_nLevel + 1);
	m_pSubSplitterWnd[nSide]->Create(this, pView1, pView2, pContext, bVertical, nID);
	return m_pSubSplitterWnd[nSide];
}

bool CSplitterWndEx::HideView(int nRow, int nCol)
{
	CWnd* pWnd = GetPane(nRow, nCol);
	if (!pWnd)
		return false;
	pWnd->SetDlgCtrlID(0);
	pWnd->ShowWindow(SW_HIDE);
	return true;
}

bool CSplitterWndEx::ShowView(int nRow, int nCol, CWnd* pWnd)
{
	pWnd->SetDlgCtrlID(IdFromRowCol(nRow, nCol));
	pWnd->ShowWindow(SW_SHOW);
	return true;
}

int CSplitterWndEx::AddView(int nSide, CRuntimeClass * pViewClass, CCreateContext* pContext)
{
	int nRow, nCol;
	SideToRowCol(nSide, &nRow, &nCol);

	// hide the current view of the pane if there is a view attached already
	if (GetDlgItem(IdFromRowCol(nRow, nCol)))
		HideView(nRow, nCol);

	// create the new view, if fail, set the previous view current
	if (CreateView(nRow, nCol, pViewClass, CSize(10, 10), pContext) == 0)
		return -1;

	// get and store the new view
	CWnd* pWnd = GetPane(nRow, nCol);
	m_views[nSide].Add(pWnd);
	m_nCurrentView[nSide] = m_views[nSide].GetCount() - 1;

	ShowView(nRow, nCol, pWnd);

	RedrawWindow();
	return m_nCurrentView[nSide];
}

void CSplitterWndEx::SwitchToView(int nSide, int nView /* = -1 */)
{
	// if the View is -1 then just use the next view...
	if (nView == -1)
	{
		nView = m_nCurrentView[nSide] + 1;
		if (nView >= m_views[nSide].GetCount())
			nView = 0; // rollover to first view
	}

	CWnd* pWnd = m_views[nSide][nView];

	int nRow, nCol;

	if (IsSideHidden(LEFT_SIDE))
	{
		nRow = 0;
		nCol = 0;
	}
	else
		SideToRowCol(nSide, &nRow, &nCol);

	HideView(nRow, nCol);
	ShowView(nRow, nCol, pWnd);

	m_nCurrentView[nSide] = nView;
	RecalcLayout();
	RedrawWindow();
}

AFX_STATIC void AFXAPI _AfxLayoutRowCol(CSplitterWnd::CRowColInfo* pInfoArray,
	int nMax, int nSize, int nSizeSplitter)
{
	ASSERT(pInfoArray != NULL);
	ASSERT(nMax > 0);
	ASSERT(nSizeSplitter > 0);

	CSplitterWnd::CRowColInfo* pInfo;
	int i;

	if (nSize < 0)
		nSize = 0;  // if really too small, layout as zero size

	// start with ideal sizes
	for (i = 0, pInfo = pInfoArray; i < nMax-1; i++, pInfo++)
	{
		if (pInfo->nIdealSize < pInfo->nMinSize)
			pInfo->nIdealSize = 0;      // too small to see
		pInfo->nCurSize = pInfo->nIdealSize;
	}
	pInfo->nCurSize = INT_MAX;  // last row/column takes the rest

	for (i = 0, pInfo = pInfoArray; i < nMax; i++, pInfo++)
	{
		ASSERT(nSize >= 0);
		if (nSize == 0)
		{
			// no more room (set pane to be invisible)
			pInfo->nCurSize = 0;
			continue;       // don't worry about splitters
		}
		else if (nSize < pInfo->nMinSize && i != 0)
		{
			// additional panes below the recommended minimum size
			//   aren't shown and the size goes to the previous pane
			pInfo->nCurSize = 0;

			// previous pane already has room for splitter + border
			//   add remaining size and remove the extra border
			ASSERT(CX_BORDER * 2 == CY_BORDER * 2);
			(pInfo-1)->nCurSize += nSize + CX_BORDER * 2;
			nSize = 0;
		}
		else
		{
			// otherwise we can add the second pane
			ASSERT(nSize > 0);
			if (pInfo->nCurSize == 0)
			{
				// too small to see
				if (i != 0)
					pInfo->nCurSize = 0;
			}
			else if (nSize < pInfo->nCurSize)
			{
				// this row/col won't fit completely - make as small as possible
				pInfo->nCurSize = nSize;
				nSize = 0;
			}
			else
			{
				// can fit everything
				nSize -= pInfo->nCurSize;
			}
		}

		// see if we should add a splitter
		ASSERT(nSize >= 0);
		if (i != nMax - 1)
		{
			// should have a splitter
			if (nSize > nSizeSplitter)
			{
				nSize -= nSizeSplitter; // leave room for splitter + border
				ASSERT(nSize > 0);
			}
			else
			{
				// not enough room - add left over less splitter size
				ASSERT(CX_BORDER * 2 == CY_BORDER * 2);
				pInfo->nCurSize += nSize;
				if (pInfo->nCurSize > (nSizeSplitter - CX_BORDER * 2))
					pInfo->nCurSize -= (nSizeSplitter - CY_BORDER * 2);
				nSize = 0;
			}
		}
	}
	ASSERT(nSize == 0); // all space should be allocated
}

// repositions client area of specified window
// assumes everything has WS_BORDER or is inset like it does
//  (includes scroll bars)
AFX_STATIC void AFXAPI _AfxDeferClientPos(AFX_SIZEPARENTPARAMS* lpLayout,
	CWnd* pWnd, int x, int y, int cx, int cy, BOOL bScrollBar)
{
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd != NULL);

	if (bScrollBar)
	{
		// if there is enough room, draw scroll bar without border
		// if there is not enough room, set the WS_BORDER bit so that
		//   we will at least get a proper border drawn
		BOOL bNeedBorder = (cx <= CX_BORDER || cy <= CY_BORDER);
		pWnd->ModifyStyle(bNeedBorder ? 0 : WS_BORDER,
			bNeedBorder ? WS_BORDER : 0);
	}
	CRect rect(x, y, x+cx, y+cy);

	// adjust for 3d border (splitter windows have implied border)
	if ((pWnd->GetExStyle() & WS_EX_CLIENTEDGE) ||
		  pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		rect.InflateRect(CX_BORDER * 2, CY_BORDER * 2);

	// first check if the new rectangle is the same as the current
	CRect rectOld;
	pWnd->GetWindowRect(rectOld);
	pWnd->GetParent()->ScreenToClient(&rectOld);
	if (rect != rectOld)
		AfxRepositionWindow(lpLayout, pWnd->m_hWnd, rect);
}

void CSplitterWndEx::RecalcLayout()
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 0 && m_nCols > 0); // must have at least one pane

	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.InflateRect(-m_cxBorder, -m_cyBorder);

	CRect rectInside;
	GetInsideRect(rectInside);

	// layout columns (restrict to possible sizes)
	_AfxLayoutRowCol(m_pColInfo, m_nCols, rectInside.Width(), m_cxSplitterGap);
	_AfxLayoutRowCol(m_pRowInfo, m_nRows, rectInside.Height(), m_cySplitterGap);

	// adjust the panes (and optionally scroll bars)

	// give the hint for the maximum number of HWNDs
	AFX_SIZEPARENTPARAMS layout;
	layout.hDWP = ::BeginDeferWindowPos((m_nCols + 1) * (m_nRows + 1) + 1);

	// size of scrollbars
	int cxScrollbar = rectClient.right - rectInside.right;
	int cyScrollbar = rectClient.bottom - rectInside.bottom;

	// reposition size box
	if (m_bHasHScroll && m_bHasVScroll)
	{
		CWnd* pScrollBar = GetDlgItem(AFX_IDW_SIZE_BOX);
		ASSERT(pScrollBar != NULL);

		// fix style if necessary
		BOOL bSizingParent = (GetSizingParent() != NULL);
		// modifyStyle returns TRUE if style changes
		if (pScrollBar->ModifyStyle(SBS_SIZEGRIP|SBS_SIZEBOX,
				bSizingParent ? SBS_SIZEGRIP : SBS_SIZEBOX))
			pScrollBar->Invalidate();
		pScrollBar->EnableWindow(bSizingParent);

		// reposition the size box
		_AfxDeferClientPos(&layout, pScrollBar,
			rectInside.right, rectInside.bottom, cxScrollbar, cyScrollbar, TRUE);
	}

	// reposition scroll bars
	if (m_bHasHScroll)
	{
		int cxSplitterBox = m_cxSplitter;// split box bigger
		int x = rectClient.left;
		int y = rectInside.bottom;
		for (int col = 0; col < m_nCols; col++)
		{
			CWnd* pScrollBar = GetDlgItem(AFX_IDW_HSCROLL_FIRST + col);
			ASSERT(pScrollBar != NULL);
			int cxCol = m_pColInfo[col].nCurSize;
			if (col == 0 && m_nCols < m_nMaxCols)
				x += cxSplitterBox, cxCol -= cxSplitterBox;
			_AfxDeferClientPos(&layout, pScrollBar, x, y, cxCol, cyScrollbar, TRUE);
			x += cxCol + m_cxSplitterGap;
		}
	}

	if (m_bHasVScroll)
	{
		int cySplitterBox = m_cySplitter;// split box bigger
		int x = rectInside.right;
		int y = rectClient.top;
		for (int row = 0; row < m_nRows; row++)
		{
			CWnd* pScrollBar = GetDlgItem(AFX_IDW_VSCROLL_FIRST + row);
			ASSERT(pScrollBar != NULL);
			int cyRow = m_pRowInfo[row].nCurSize;
			if (row == 0 && m_nRows < m_nMaxRows)
				y += cySplitterBox, cyRow -= cySplitterBox;
			_AfxDeferClientPos(&layout, pScrollBar, x, y, cxScrollbar, cyRow, TRUE);
			y += cyRow + m_cySplitterGap;
		}
	}

	//BLOCK: Reposition all the panes
	{
		int x = rectClient.left;
		for (int col = 0; col < m_nCols; col++)
		{
			int cxCol = m_pColInfo[col].nCurSize;
			int y = rectClient.top;
			for (int row = 0; row < m_nRows; row++)
			{
				int cyRow = m_pRowInfo[row].nCurSize;
				CWnd* pWnd = GetPane(row, col);
				if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
				{
					y += HEADER_HEIGHT;
					cyRow -= HEADER_HEIGHT;
				}
				_AfxDeferClientPos(&layout, pWnd, x, y, cxCol, cyRow, FALSE);
				y += cyRow + m_cySplitterGap;
			}
			x += cxCol + m_cxSplitterGap;
		}
	}

	// move and resize all the windows at once!
	if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
		TRACE(traceAppMsg, 0, "Warning: DeferWindowPos failed - low system resources.\n");

	// invalidate all the splitter bars (with NULL pDC)
	DrawAllSplitBars(NULL, rectInside.right, rectInside.bottom);
}

void CSplitterWndEx::OnDrawSplitter(CDC* pDC, ESplitType nType,
	const CRect& rectArg)
{
	// if pDC == NULL, then just invalidate
	if (pDC == NULL)
	{
		RedrawWindow(rectArg, NULL, RDW_INVALIDATE|RDW_NOCHILDREN);
		return;
	}
	ASSERT_VALID(pDC);

	// otherwise, actually draw
	CRect rect = rectArg;
	rect.top += HEADER_HEIGHT;
	switch (nType)
	{
	case splitBorder:
		pDC->Draw3dRect(rect, m_clrBtnShadow, m_clrBtnHilite);
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
		pDC->Draw3dRect(rect, m_clrWindowFrame, m_clrBtnFace);
		return;

	case splitIntersection:
		break;

	case splitBox:
		pDC->Draw3dRect(rect, m_clrBtnFace, m_clrWindowFrame);
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
		pDC->Draw3dRect(rect, m_clrBtnHilite, m_clrBtnShadow);
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
		break;

	case splitBar:
		break;

	default:
		ASSERT(FALSE);  // unknown splitter type
	}

	// fill the middle
	COLORREF clr = m_clrBtnFace;
	rect.top -= HEADER_HEIGHT;
	pDC->FillSolidRect(rect, clr);
}

void CSplitterWndEx::DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside)
{
	ASSERT_VALID(this);

	int col;
	int row;

	// draw column split bars
	CRect rect;
	GetClientRect(rect);
	rect.left += m_cxBorder;
	for (col = 0; col < m_nCols - 1; col++)
	{
		rect.left += m_pColInfo[col].nCurSize + m_cxBorderShare;
		rect.right = rect.left + m_cxSplitter;
		if (rect.left > cxInside)
			break;      // stop if not fully visible
		rect.right -= CX_BORDER * 2;
		OnDrawSplitter(pDC, splitBar, rect);
		rect.left = rect.right + m_cxBorderShare;
	}

	// draw row split bars
	GetClientRect(rect);
	rect.top += m_cyBorder;
	for (row = 0; row < m_nRows - 1; row++)
	{
		rect.top += m_pRowInfo[row].nCurSize + m_cyBorderShare;
		rect.bottom = rect.top + m_cySplitter;
		if (rect.top > cyInside)
			break;      // stop if not fully visible
		rect.bottom -= CY_BORDER * 2;
		OnDrawSplitter(pDC, splitBar, rect);
		rect.top = rect.bottom + m_cyBorderShare;
	}

	// draw pane borders
	GetClientRect(rect);
	int x = rect.left;
	for (col = 0; col < m_nCols; col++)
	{
		int cx = m_pColInfo[col].nCurSize + 2*m_cxBorder;
		if (col == m_nCols-1 && m_bHasVScroll)
			cx += m_cxVScroll;
		int y = rect.top;
		for (row = 0; row < m_nRows; row++)
		{
			int cy = m_pRowInfo[row].nCurSize + 2*m_cyBorder;
			if (row == m_nRows-1 && m_bHasHScroll)
				cy += m_cyHScroll;
			CWnd* pWnd = GetPane(row, col);
			if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
				OnDrawSplitter(pDC, splitBorder, CRect(x, y, x+cx, y+cy));
			else
				pWnd->Invalidate(FALSE);
			y += cy + m_cySplitterGap - 2*m_cyBorder;
		}
		x += cx + m_cxSplitterGap - 2*m_cxBorder;
	}
}

void CSplitterWndEx::TrackRowSize(int y, int row)
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 1);

	CPoint pt(0, y);
	ClientToScreen(&pt);
	GetPane(row, 0)->ScreenToClient(&pt);
	m_pRowInfo[row].nIdealSize = pt.y + HEADER_HEIGHT;      // new size
	if (pt.y < m_pRowInfo[row].nMinSize)
	{
		// resized too small
		m_pRowInfo[row].nIdealSize = 0; // make it go away
		if (GetStyle() & SPLS_DYNAMIC_SPLIT)
			DeleteRow(row);
	}
	else if (m_pRowInfo[row].nCurSize + m_pRowInfo[row+1].nCurSize
			< pt.y + m_pRowInfo[row+1].nMinSize)
	{
		// not enough room for other pane
		if (GetStyle() & SPLS_DYNAMIC_SPLIT)
			DeleteRow(row + 1);
	}
}

void CSplitterWndEx::OnPaint()
{
	ASSERT_VALID(this);
	CPaintDC dc(this);

	CRect rectClient;
	GetClientRect(&rectClient);
	rectClient.InflateRect(-m_cxBorder, -m_cyBorder);

	CRect rectInside;
	GetInsideRect(rectInside);

	// draw the splitter boxes
	if (m_bHasVScroll && m_nRows < m_nMaxRows)
	{
		OnDrawSplitter(&dc, splitBox,
			CRect(rectInside.right, rectClient.top,
				rectClient.right, rectClient.top + m_cySplitter));
	}

	if (m_bHasHScroll && m_nCols < m_nMaxCols)
	{
		OnDrawSplitter(&dc, splitBox,
			CRect(rectClient.left, rectInside.bottom,
				rectClient.left + m_cxSplitter, rectClient.bottom));
	}

	// extend split bars to window border (past margins)
	DrawAllSplitBars(&dc, rectInside.right, rectInside.bottom);

	int x = rectClient.left;
	for (int col = 0; col < m_nCols; col++)
	{
		int cxCol = m_pColInfo[col].nCurSize;
		int y = rectClient.top;
		for (int row = 0; row < m_nRows; row++)
		{
			int cyRow = m_pRowInfo[row].nCurSize;
			CWnd* pWnd = GetPane(row, col);
			if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
			{
				ASSERT(pWnd != NULL);
				ASSERT(pWnd->m_hWnd != NULL);
				CRect rect(x, y, x + cxCol, y + HEADER_HEIGHT);

				if ((pWnd->GetExStyle() & WS_EX_CLIENTEDGE) || pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
					rect.InflateRect(CX_BORDER * 2, CY_BORDER * 2);

				rect.left -= CX_BORDER * 2;
				rect.top -= CY_BORDER * 2;
				rect.right += CX_BORDER * 2;
				rect.bottom -= CY_BORDER * 2;

				dc.Draw3dRect(rect, m_clrBtnFace, m_clrBtnFace);
				rect.InflateRect(-CX_BORDER, -CY_BORDER);

				if (pWnd == GetActivePane())
				{
					dc.Draw3dRect(rect, m_clrActiveCaption, m_clrBtnFace);
					rect.InflateRect(-CX_BORDER, -CY_BORDER);
					dc.FillSolidRect(rect, m_clrActiveCaption);
					dc.SetTextColor(m_clrCaptionText);
				}
				else
				{
					dc.Draw3dRect(rect, m_clrInactiveCaption, m_clrBtnFace);
					rect.InflateRect(-CX_BORDER, -CY_BORDER);
					dc.FillSolidRect(rect, m_clrInactiveCaption);
					dc.SetTextColor(m_clrInactiveCaptionText);
				}

				CFont* pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
				CFont* pOldFont = dc.SelectObject(pFont);
				ASSERT(pOldFont != NULL);

				CString str;
				pWnd->GetWindowText(str);
				VERIFY(dc.TextOut(rect.left, rect.top, str));

				VERIFY(dc.SelectObject(pOldFont));
			}
			y += cyRow + m_cySplitterGap;
		}
		x += cxCol + m_cxSplitterGap;
	}
}

void CSplitterWndEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rectClient;
	GetClientRect(&rectClient);
	rectClient.InflateRect(-m_cxBorder, -m_cyBorder);

	bool b = false;
	int x = rectClient.left;
	for (int col = 0; col < m_nCols; col++)
	{
		int cxCol = m_pColInfo[col].nCurSize;
		int y = rectClient.top;
		for (int row = 0; row < m_nRows; row++)
		{
			int cyRow = m_pRowInfo[row].nCurSize;
			CWnd* pWnd = GetPane(row, col);
			if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
			{
				ASSERT(pWnd != NULL);
				ASSERT(pWnd->m_hWnd != NULL);
				CRect rect(x, y, x + cxCol, y + cyRow);

				if ((pWnd->GetExStyle() & WS_EX_CLIENTEDGE) || pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
					rect.InflateRect(CX_BORDER * 2, CY_BORDER * 2);

				if (rect.PtInRect(point))
				{
					b = true;
					SetActivePane(row, col);
					break;
				}
			}
			y += cyRow + m_cySplitterGap;
		}

		if (b)
			break;

		x += cxCol + m_cxSplitterGap;
	}

	CSplitterWnd::OnLButtonDown(nFlags, point);
}
