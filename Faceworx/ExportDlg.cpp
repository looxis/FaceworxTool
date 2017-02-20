// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Faceworx.h"
#include "ExportDlg.h"
#include <dlgs.h>
#include <afxpriv.h>

// CExportDlg

IMPLEMENT_DYNAMIC(CExportDlg, CFileDialog)

BEGIN_MESSAGE_MAP(CExportDlg, CFileDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

CExportDlg::CExportDlg(LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
					   : CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
					   , m_nModelDetail(0)
					   , m_nTextureSize(0)
					   , m_nTextureQuality(0)
{
	GetOFN().Flags|=OFN_ENABLESIZING;
	SetTemplate(0, MAKEINTRESOURCE(IDD));
	m_bFirstTime = true;
}

CExportDlg::~CExportDlg()
{
}

void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_MODEL_DETAIL_COMBO, m_nModelDetail);
	DDX_CBIndex(pDX, IDC_TEXTURE_SIZE_COMBO, m_nTextureSize);
	DDX_Slider(pDX, IDC_TEXTURE_QUALITY_SLIDER, m_nTextureQuality);
}

INT_PTR CExportDlg::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
	ASSERT(m_ofn.lpfnHook != NULL); // can still be a user hook

	// zero out the file buffer for consistent parsing later
	ASSERT(AfxIsValidAddress(m_ofn.lpstrFile, m_ofn.nMaxFile));
	DWORD nOffset = lstrlen(m_ofn.lpstrFile)+1;
	ASSERT(nOffset <= m_ofn.nMaxFile);
	memset(m_ofn.lpstrFile+nOffset, 0, (m_ofn.nMaxFile-nOffset)*sizeof(TCHAR));

	// WINBUG: This is a special case for the file open/save dialog,
	// which sometimes pumps while it is coming up but before it has
	// disabled the main window.
	HWND hWndFocus = ::GetFocus();
	BOOL bEnableParent = FALSE;
	m_ofn.hwndOwner = PreModal();
	AfxUnhookWindowCreate();
	if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_ofn.hwndOwner, FALSE);
	}

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);

	if (m_ofn.Flags & OFN_EXPLORER)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);

	int nResult;
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName((OPENFILENAME*)&m_ofn);
	else
		nResult = ::GetSaveFileName((OPENFILENAME*)&m_ofn);

	if (nResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_ofn.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);

	PostModal();

	return nResult ? nResult : IDCANCEL;
	//return CFileDialog::DoModal();
}

// CExportDlg message handlers

void LayoutControl(CWnd* pWnd, int x, int y, int width = -1)
{
	ASSERT(pWnd != NULL);
	CRect rect;
	pWnd->GetWindowRect(&rect);
	CWnd* pParent = pWnd->GetParent();
	ASSERT(pParent != NULL);
	pParent->ScreenToClient(&rect);
	if (width == -1)
		width = rect.Width();
	int height = rect.Height();
	rect.left = x;
	rect.top = y;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	pWnd->MoveWindow(&rect);
}

BOOL CExportDlg::OnInitDialog()
{
	CFileDialog::OnInitDialog();

	CWnd* pWnd;

	pWnd = GetDlgItem(IDC_MODEL_DETAIL_COMBO);
	ASSERT(pWnd != NULL);
	CComboBox* pCombo = (CComboBox*)CComboBox::FromHandle(pWnd->m_hWnd);

	CString str;

	str.LoadString(IDS_MODEL_DETAIL_LOW);
	pCombo->AddString(str);

	str.LoadString(IDS_MODEL_DETAIL_MEDIUM);
	pCombo->AddString(str);

	str.LoadString(IDS_MODEL_DETAIL_HIGH);
	pCombo->AddString(str);

	pCombo->SetCurSel(1);

	pWnd = GetDlgItem(IDC_TEXTURE_SIZE_COMBO);
	ASSERT(pWnd != NULL);
	pCombo = (CComboBox*)CComboBox::FromHandle(pWnd->m_hWnd);

	pCombo->AddString(_T("64x64"));
	pCombo->AddString(_T("128x128"));
	pCombo->AddString(_T("256x256"));
	pCombo->AddString(_T("512x512"));
	pCombo->AddString(_T("1024x1024"));
	pCombo->AddString(_T("2048x2048"));
	pCombo->SetCurSel(4);

	pWnd = GetDlgItem(IDC_TEXTURE_QUALITY_SLIDER);
	ASSERT(pWnd != NULL);
	CSliderCtrl* pSlider = (CSliderCtrl*)CSliderCtrl::FromHandle(pWnd->m_hWnd);

	pSlider->SetRangeMax(100);
	pSlider->SetRangeMin(0);
	pSlider->SetPos(80);
	pSlider->SetTicFreq(10);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CExportDlg::OnSize(UINT nType, int cx, int cy)
{
	CFileDialog::OnSize(nType, cx, cy);

	if (m_bFirstTime)
	{
		// original dialog box
		CWnd* pParent = GetParent();
		ASSERT(pParent != NULL);

		CWnd* pWnd;
		// File name combobox
		pWnd = pParent->GetDlgItem(cmb13);
		ASSERT(pWnd != NULL);
		CRect rectCmb13;
		pWnd->GetWindowRect(&rectCmb13);
		pParent->ScreenToClient(&rectCmb13);

		// File name static
		pWnd = pParent->GetDlgItem(stc3);
		ASSERT(pWnd != NULL);
		CRect rectStc3;
		pWnd->GetWindowRect(&rectStc3);
		pParent->ScreenToClient(&rectStc3);

		// Files of type combobox
		pWnd = pParent->GetDlgItem(cmb1);
		ASSERT(pWnd != NULL);
		CRect rectCmb1;
		pWnd->GetWindowRect(&rectCmb1);
		pParent->ScreenToClient(&rectCmb1);

		// Files of type static
		pWnd = pParent->GetDlgItem(stc2);
		ASSERT(pWnd != NULL);
		CRect rectStc2;
		pWnd->GetWindowRect(&rectStc2);
		pParent->ScreenToClient(&rectStc2);

		int cy1 = rectCmb1.top - rectCmb13.bottom;
		int cy2 = rectStc2.top - rectCmb1.top;

		CRect rect;
		pWnd = GetDlgItem(IDC_MODEL_DETAIL_COMBO);
		LayoutControl(pWnd, rectCmb1.left, rectCmb1.bottom + cy1);
		pWnd->GetWindowRect(&rect);
		pParent->ScreenToClient(&rect);

		LayoutControl(GetDlgItem(IDC_MODEL_DETAIL_STATIC), rectStc2.left, rect.top + cy2);

		pWnd = GetDlgItem(IDC_TEXTURE_SIZE_COMBO);
		LayoutControl(pWnd, rectCmb1.left, rect.bottom + cy1);
		pWnd->GetWindowRect(&rect);
		pParent->ScreenToClient(&rect);

		LayoutControl(GetDlgItem(IDC_TEXTURE_SIZE_STATIC), rectStc2.left, rect.top + cy2);

		pWnd = GetDlgItem(IDC_TEXTURE_QUALITY_SLIDER);
		LayoutControl(pWnd, rectCmb1.left, rect.bottom + cy1);
		pWnd->GetWindowRect(&rect);
		pParent->ScreenToClient(&rect);

		LayoutControl(GetDlgItem(IDC_TEXTURE_QUALITY_STATIC), rectStc2.left, rect.top + cy2);

		// Read only checkbox
		if ((m_ofn.Flags & OFN_HIDEREADONLY) == 0)
			LayoutControl(pParent->GetDlgItem(chx1), rectCmb1.left, rect.bottom + cy1);

		pWnd = GetDlgItem(IDC_TEXTURE_QUALITY_SLIDER);
		pWnd->GetWindowRect(&rect);

		CRect rectParent;
		pParent->GetWindowRect(&rectParent);
		rectParent.bottom = rect.bottom + MulDiv(7, HIWORD(GetDialogBaseUnits()), 8);
		pParent->MoveWindow(rectParent);

		GetWindowRect(&rect);
		rect.right = rect.left + rectParent.Width();
		rect.bottom = rect.top + rectParent.Height();
		pParent->ScreenToClient(&rect);
		MoveWindow(rect);

		m_bFirstTime = false;
	}
}

BOOL CExportDlg::OnFileNameOK()
{
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return 1;
	}

	return CFileDialog::OnFileNameOK();
}
