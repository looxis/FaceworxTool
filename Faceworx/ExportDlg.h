#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CExportDlg

class CExportDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CExportDlg)

public:
	CExportDlg(LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);
	virtual ~CExportDlg();

	enum { IDD = IDD_EXPORTDLG };

// Operations
	virtual INT_PTR DoModal();

// Overridables
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

private:
	bool m_bFirstTime;

public:
	int m_nModelDetail;
	int m_nTextureSize;
	int m_nTextureQuality;

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
	virtual BOOL OnFileNameOK();
};
