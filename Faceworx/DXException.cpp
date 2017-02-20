#include "StdAfx.h"
#include "DXException.h"

IMPLEMENT_DYNAMIC(CDXException, COleException)

void __declspec(noreturn) AFXAPI AfxThrowDXException(SCODE sc)
{
#ifdef _DEBUG
	TRACE(traceOle, 0, _T("Warning: constructing CDXException, scode = %s.\n"),
		AfxGetFullScodeString(sc));
#endif
	CDXException* pException = new CDXException;
	pException->m_sc = sc;
	THROW(pException);
}

//BOOL CDXException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext) const //2005
BOOL CDXException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext) //2003
{
	if (!COleException::GetErrorMessage(lpszError, nMaxError, pnHelpContext))
	{
		LPCTSTR lpBuffer = DXGetErrorDescription9(m_sc);
		//Checked::tcsncpy_s(lpszError, nMaxError, lpBuffer, _TRUNCATE); //2005
		_tcsncpy(lpszError, lpBuffer, nMaxError);
	}

	return TRUE;
}
