#include "StdAfx.h"
#include "SubStream.h"

CSubStream::CSubStream(IStream* pstm) : m_pstm(pstm)
{
	LARGE_INTEGER liMove;
	liMove.QuadPart = 0;
	HRESULT hr = m_pstm->Seek(liMove, STREAM_SEEK_CUR, &m_uliPos);
	if (FAILED(hr))
		AtlThrow(hr);

	m_uliSize.QuadPart = (ULONGLONG)-1;
}

CSubStream::CSubStream(IStream* pstm, ULARGE_INTEGER uliSize) : m_pstm(pstm), m_uliSize(uliSize)
{
	LARGE_INTEGER liMove;
	liMove.QuadPart = 0;
	HRESULT hr = m_pstm->Seek(liMove, STREAM_SEEK_CUR, &m_uliPos);
	if (FAILED(hr))
		AtlThrow(hr);
}

STDMETHODIMP_(ULONG)CSubStream::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG)CSubStream::Release()
{
	return 0;
}

STDMETHODIMP CSubStream::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	if (iid == IID_IUnknown || iid == IID_IStream)
	{
		if (ppvObj == NULL)
		{
			return E_POINTER;
		}
		*ppvObj = this;
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CSubStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	ASSERT(m_pstm != NULL);

	if (m_uliSize.QuadPart != (ULONGLONG)-1)
	{
		LARGE_INTEGER liMove;
		ULARGE_INTEGER uliPos;
		liMove.QuadPart = 0;
		HRESULT hr = m_pstm->Seek(liMove, STREAM_SEEK_CUR, &uliPos);
		if (FAILED(hr))
			AtlThrow(hr);

		if (uliPos.QuadPart + cb > m_uliPos.QuadPart + m_uliSize.QuadPart)
			cb = (ULONG)(m_uliPos.QuadPart + m_uliSize.QuadPart - uliPos.QuadPart);
	}

	return m_pstm->Read(pv, cb, pcbRead);
}

STDMETHODIMP CSubStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	ASSERT(m_pstm != NULL);

	if (m_uliSize.QuadPart != (ULONGLONG)-1)
	{
		LARGE_INTEGER liMove;
		ULARGE_INTEGER uliPos;
		liMove.QuadPart = 0;
		HRESULT hr = m_pstm->Seek(liMove, STREAM_SEEK_CUR, &uliPos);
		if (FAILED(hr))
			AtlThrow(hr);

		if (uliPos.QuadPart + cb > m_uliPos.QuadPart + m_uliSize.QuadPart)
			m_uliSize.QuadPart = uliPos.QuadPart + cb - m_uliPos.QuadPart;
	}

	return m_pstm->Write(pv, cb, pcbWritten);
}

STDMETHODIMP CSubStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
	ASSERT(m_pstm != NULL);

	HRESULT hr;
	ULARGE_INTEGER uliNewPos;

	if (dwOrigin == STREAM_SEEK_SET)
	{
		if (dlibMove.QuadPart > 0)
		{
			dlibMove.QuadPart += m_uliPos.QuadPart;
			if (m_uliSize.QuadPart != (ULONGLONG)-1 && (ULONGLONG)dlibMove.QuadPart > m_uliPos.QuadPart + m_uliSize.QuadPart)
				dlibMove.QuadPart = m_uliPos.QuadPart + m_uliSize.QuadPart;
		}
		else
			dlibMove.QuadPart = m_uliPos.QuadPart;

		hr = m_pstm->Seek(dlibMove, dwOrigin, &uliNewPos);
		if (FAILED(hr))
			return hr;
	}
	else if (dwOrigin == STREAM_SEEK_CUR)
	{
		hr = m_pstm->Seek(dlibMove, dwOrigin, &uliNewPos);
		if (FAILED(hr))
			return hr;

		if (uliNewPos.QuadPart < m_uliPos.QuadPart)
		{
			hr = m_pstm->Seek(*(LARGE_INTEGER*)&m_uliPos, STREAM_SEEK_SET, &uliNewPos);
			if (FAILED(hr))
				return hr;
		}
		else if (m_uliSize.QuadPart != (ULONGLONG)-1 && uliNewPos.QuadPart > m_uliPos.QuadPart + m_uliSize.QuadPart)
		{
			dlibMove.QuadPart = m_uliPos.QuadPart + m_uliSize.QuadPart;
			hr = m_pstm->Seek(dlibMove, STREAM_SEEK_SET, &uliNewPos);
			if (FAILED(hr))
				return hr;
		}
	}
	else // dwOrigin == STREAM_SEEK_END
	{
		if (dlibMove.QuadPart >= 0)
		{
			if (m_uliSize.QuadPart != (ULONGLONG)-1)
			{
				dlibMove.QuadPart = m_uliPos.QuadPart + m_uliSize.QuadPart - dlibMove.QuadPart;
				if (dlibMove.QuadPart < 0 || (ULONGLONG)dlibMove.QuadPart < m_uliPos.QuadPart)
					dlibMove.QuadPart = m_uliPos.QuadPart;

				hr = m_pstm->Seek(dlibMove, STREAM_SEEK_SET, &uliNewPos);
				if (FAILED(hr))
					return hr;
			}
			else
			{
				hr = m_pstm->Seek(dlibMove, dwOrigin, &uliNewPos);
				if (FAILED(hr))
					return hr;

				if (uliNewPos.QuadPart < m_uliPos.QuadPart)
				{
					hr = m_pstm->Seek(*(LARGE_INTEGER*)&m_uliPos, STREAM_SEEK_SET, &uliNewPos);
					if (FAILED(hr))
						return hr;
				}
			}
		}
		else
		{
			if (m_uliSize.QuadPart != (ULONGLONG)-1)
				dlibMove.QuadPart = m_uliPos.QuadPart + m_uliSize.QuadPart;

			hr = m_pstm->Seek(dlibMove, STREAM_SEEK_SET, &uliNewPos);
			if (FAILED(hr))
				return hr;
		}
	}

	if (plibNewPosition != NULL)
		plibNewPosition->QuadPart = uliNewPos.QuadPart - m_uliPos.QuadPart;

	return hr;
}

STDMETHODIMP CSubStream::SetSize(ULARGE_INTEGER)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::Commit(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::Revert()
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::Stat(STATSTG*, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSubStream::Clone(LPSTREAM*)
{
	return E_NOTIMPL;
}
