#pragma once


// C3DModificationSettingsDlg dialog

class C3DModificationSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(C3DModificationSettingsDlg)

public:
	C3DModificationSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~C3DModificationSettingsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_3DMODIFICATION_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_R1;
public:
	double m_R2;
public:
	double m_Shift;
};
