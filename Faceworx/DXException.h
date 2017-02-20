#pragma once
#include "afxdisp.h"

class CDXException : public COleException
{
	DECLARE_DYNAMIC(CDXException)

public:
	//virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL) const; //2005
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL); //2003
};

void __declspec(noreturn) AFXAPI AfxThrowDXException(SCODE sc);
