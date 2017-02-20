// SubdivisionSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Faceworx.h"
#include "SubdivisionSettingsDlg.h"


// CSubdivisionSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSubdivisionSettingsDlg, CDialog)

CSubdivisionSettingsDlg::CSubdivisionSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubdivisionSettingsDlg::IDD, pParent)
	, m_bNose(FALSE)
	, m_bLips(FALSE)
	, m_bEyebrow(FALSE)
{

}

CSubdivisionSettingsDlg::~CSubdivisionSettingsDlg()
{
}

void CSubdivisionSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_NOSE, m_bNose);
	DDX_Check(pDX, IDC_CHECK_LIPS, m_bLips);
	DDX_Check(pDX, IDC_CHECK_EYEBROW, m_bEyebrow);
}


BEGIN_MESSAGE_MAP(CSubdivisionSettingsDlg, CDialog)
END_MESSAGE_MAP()


// CSubdivisionSettingsDlg message handlers
