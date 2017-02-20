#pragma once

// CMagnifierDlg dialog

class CMagnifierDlg : public CDialog
{
	DECLARE_DYNAMIC(CMagnifierDlg)

public:
	CMagnifierDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMagnifierDlg();

// Dialog Data
	enum { IDD = IDD_MAGNIFIERDLG };

protected:
	int m_nZoom;
	float m_fScale;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnPaint();
};
