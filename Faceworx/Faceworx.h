// Faceworx.h : main header file for the Faceworx application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

// CFaceworxApp:
// See Faceworx.cpp for the implementation of this class
//

class CFaceworxApp : public CWinApp
{
public:
	CFaceworxApp();

	void Update3DView();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
private:
	bool m_bUpdate3DView;

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnIdle(LONG lCount);
};

extern CFaceworxApp theApp;
