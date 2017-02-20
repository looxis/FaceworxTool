// 3DModificationSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Faceworx.h"
#include "3DModificationSettingsDlg.h"


// C3DModificationSettingsDlg dialog

IMPLEMENT_DYNAMIC(C3DModificationSettingsDlg, CDialog)

C3DModificationSettingsDlg::C3DModificationSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(C3DModificationSettingsDlg::IDD, pParent)
	, m_R1(0)
	, m_R2(0)
	, m_Shift(0)
{

}

C3DModificationSettingsDlg::~C3DModificationSettingsDlg()
{
}

void C3DModificationSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_R1);
	DDX_Text(pDX, IDC_EDIT2, m_R2);
	DDX_Text(pDX, IDC_EDIT3, m_Shift);
}


BEGIN_MESSAGE_MAP(C3DModificationSettingsDlg, CDialog)
END_MESSAGE_MAP()


// C3DModificationSettingsDlg message handlers
