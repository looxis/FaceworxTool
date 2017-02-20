#pragma once


// CSubdivisionSettingsDlg dialog

class CSubdivisionSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CSubdivisionSettingsDlg)

public:
	CSubdivisionSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSubdivisionSettingsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SUBDIVISION_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bNose;
public:
	BOOL m_bLips;
public:
	BOOL m_bEyebrow;
};
