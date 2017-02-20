// FaceworxDoc.cpp : implementation of the CFaceworxDoc class
//

#include "stdafx.h"
#include "Faceworx.h"
#include "FaceworxDoc.h"
#include "DXException.h"
#include "3DView.h"
#include "SubStream.h"
#include "ImageView.h"
#include "FrontView.h"
#include "SideView.h"
#include "3DView.h"
#include "ExportDlg.h"
#include "obj_exporter.h"
#include "texture_blender.h"
#include "SubdivisionSettingsDlg.h"
#include "3DModificationSettingsDlg.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CURRENT_VERSION 1
#define MAX_LEVEL 5
#define ADJACENCY_EPSILON 0.01f
#define IMAGE_WIDTH 600
#define IMAGE_HEIGHT 800

void CreateTextureFromImage(LPDIRECT3DDEVICE9 pDevice, const CImage& image, DWORD usage, D3DPOOL pool, LPDIRECT3DTEXTURE9* ppTexture);

// CFaceworxDoc

IMPLEMENT_DYNCREATE(CFaceworxDoc, CDocument)

BEGIN_MESSAGE_MAP(CFaceworxDoc, CDocument)
	ON_COMMAND(ID_FILE_EXPORT, &CFaceworxDoc::OnFileExport)
	ON_COMMAND(ID_EDIT_UNDO, &CFaceworxDoc::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CFaceworxDoc::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CFaceworxDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CFaceworxDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_VIEW_BOX, &CFaceworxDoc::OnViewBox)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BOX, &CFaceworxDoc::OnUpdateViewBox)
	ON_COMMAND(ID_VIEW_FEATURES, &CFaceworxDoc::OnViewFeatures)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FEATURES, &CFaceworxDoc::OnUpdateViewFeatures)
	ON_COMMAND(ID_VIEW_POINTS_ALL, &CFaceworxDoc::OnViewPointsAll)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POINTS_ALL, &CFaceworxDoc::OnUpdateViewPointsAll)
	ON_COMMAND(ID_VIEW_POINTS_MORE, &CFaceworxDoc::OnViewPointsMore)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POINTS_MORE, &CFaceworxDoc::OnUpdateViewPointsMore)
	ON_COMMAND(ID_VIEW_POINTS_LESS, &CFaceworxDoc::OnViewPointsLess)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POINTS_LESS, &CFaceworxDoc::OnUpdateViewPointsLess)
	ON_COMMAND(ID_VIEW_POINTS_NO, &CFaceworxDoc::OnViewPointsNo)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POINTS_NO, &CFaceworxDoc::OnUpdateViewPointsNo)
	ON_COMMAND(ID_VIEW_BLEND_ZONE, &CFaceworxDoc::OnViewBlendZone)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BLEND_ZONE, &CFaceworxDoc::OnUpdateViewBlendZone)
	ON_COMMAND(ID_VIEW_WIREFRAME, &CFaceworxDoc::OnViewWireframe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAME, &CFaceworxDoc::OnUpdateViewWireframe)
	ON_COMMAND(ID_VIEW_SOLID, &CFaceworxDoc::OnViewSolid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOLID, &CFaceworxDoc::OnUpdateViewSolid)
	ON_COMMAND(ID_VIEW_LIGHTING, &CFaceworxDoc::OnViewLighting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIGHTING, &CFaceworxDoc::OnUpdateViewLighting)
	ON_COMMAND(ID_VIEW_ORTHOGONAL, &CFaceworxDoc::OnViewOrthogonal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ORTHOGONAL, &CFaceworxDoc::OnUpdateViewOrthogonal)
	ON_COMMAND(ID_VIEW_PERSPECTIVE, &CFaceworxDoc::OnViewPerspective)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PERSPECTIVE, &CFaceworxDoc::OnUpdateViewPerspective)
	ON_COMMAND(ID_VIEW_MODEL_DETAIL_HIGH, &CFaceworxDoc::OnViewModelDetailHigh)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL_DETAIL_HIGH, &CFaceworxDoc::OnUpdateViewModelDetailHigh)
	ON_COMMAND(ID_VIEW_MODEL_DETAIL_MEDIUM, &CFaceworxDoc::OnViewModelDetailMedium)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL_DETAIL_MEDIUM, &CFaceworxDoc::OnUpdateViewModelDetailMedium)
	ON_COMMAND(ID_VIEW_MODEL_DETAIL_LOW, &CFaceworxDoc::OnViewModelDetailLow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL_DETAIL_LOW, &CFaceworxDoc::OnUpdateViewModelDetailLow)
	ON_COMMAND(ID_MODEL_LOAD_FRONT, &CFaceworxDoc::OnModelLoadFront)
	ON_COMMAND(ID_MODEL_LOAD_SIDE, &CFaceworxDoc::OnModelLoadSide)
	ON_COMMAND(ID_MODEL_LEFT_SIDE, &CFaceworxDoc::OnModelLeftSide)
	ON_UPDATE_COMMAND_UI(ID_MODEL_LEFT_SIDE, &CFaceworxDoc::OnUpdateModelLeftSide)
	ON_COMMAND(ID_DETAIL_1, &CFaceworxDoc::OnViewModelDetailLow)
	ON_COMMAND(ID_DETAIL_2, &CFaceworxDoc::OnViewModelDetailMedium)
	ON_COMMAND(ID_DETAIL_3, &CFaceworxDoc::OnViewModelDetailHigh)
	ON_COMMAND(ID_MODELDETAIL_SUBDIVISIONSETTINGS, &CFaceworxDoc::OnModeldetailSubdivisionsettings)
	ON_COMMAND(ID_EDIT_EDITIN3DWINDOW, &CFaceworxDoc::OnEditEditin3dwindow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_EDITIN3DWINDOW, &CFaceworxDoc::OnUpdateEditEditin3dwindow)
	ON_COMMAND(ID_EDIT_FIXDIRECTION, &CFaceworxDoc::OnEditFixdirection)
	ON_COMMAND(ID_EDIT_RESETMODIFICATIONS, &CFaceworxDoc::OnEditResetmodifications)
	ON_COMMAND(ID_EDIT_SETTINGS, &CFaceworxDoc::OnEditSettings)
END_MESSAGE_MAP()

// CFaceworxDoc construction/destruction

CFaceworxDoc::CFaceworxDoc()
{
	m_posState = NULL;
	m_nContour = 0;
	m_bBox = true;
	m_bFeatures = false;
	m_nLevel = 0;
	m_bBlend = true;
	m_bWireframe = false;
	m_bLighting = false;
	m_bOrtho = true;
	m_nDetail = CSettings().Get().iInitialModelDetail - 1;
	m_nContour = 0;
	m_bLeft = false;
	m_bLoopSubdivider1Inited = m_bLoopSubdivider2Inited = false;
}

CFaceworxDoc::~CFaceworxDoc()
{
}

void CFaceworxDoc::DeleteContents()
{
	for (int i = 0; i < 2; i++)
	{
		m_image[i].Destroy();
		m_nodes[i].RemoveAll();
		m_contours[i].RemoveAll();
		m_spTexture[i].Release();
	}

	m_states.RemoveAll();

	m_spMesh.Release();
	m_spEnhancedMesh.Release();
	m_spDevice.Release();

	m_spAdjacency.Free();

	CDocument::DeleteContents();
}

void CreateDefaultTexture(LPDIRECT3DDEVICE9 pDevice, DWORD usage, D3DPOOL pool, LPDIRECT3DTEXTURE9* ppTexture, int w, int h);

void CFaceworxDoc::SplitMouth()
{
//	return;
/*	WORD* is = 0;
	m_spMesh->LockIndexBuffer(0, (LPVOID*)&is);
	struct TR_PT
	{
		int tr,pt;
	} tr_pts[] = {
		//top lip:
		{216, 2},
		{221, 1},
		{229, 1},
		{357, 1},
		{670, 1},
		{668, 0},
		//bottom lip:
		{215, 2},
		{222, 2},
		{228, 2},
		{356, 1},
		{671, 1},
		{676, 2},
	};
	for (size_t n = 0, nc = sizeof tr_pts / sizeof tr_pts[0]; n < nc; n++)
	{
		m_spAdjacency.m_p[tr_pts[n].tr * 3 + tr_pts[n].pt] = 0xFFFFFFFF;
	}
	m_spMesh->UnlockIndexBuffer();*/
}

void CFaceworxDoc::LoadMesh()
{
	HRESULT hr;
	// Mesh loading
	CComPtr<ID3DXBuffer> spMaterials;
	hr = D3DXLoadMeshFromXResource(AfxGetResourceHandle(), (LPCSTR)IDR_MESH, "DATA", D3DXMESH_SYSTEMMEM, m_spDevice,
		NULL, &spMaterials, NULL, &m_dwNumMaterials, &m_spMesh);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	if (m_dwNumMaterials == 0)
		AtlThrow(E_FAIL);

	ATLTRY(m_spAdjacency.Attach(new DWORD[m_spMesh->GetNumFaces() * 3]));
	if (!m_spAdjacency)
		AtlThrow(E_OUTOFMEMORY);

	hr = m_spMesh->GenerateAdjacency(ADJACENCY_EPSILON, m_spAdjacency);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	SplitMouth();

	if (m_spMesh->GetFVF() != VERTEX_FVF)
	{
		CComPtr<ID3DXMesh> spMesh;
		hr = m_spMesh->CloneMeshFVF(m_spMesh->GetOptions(), VERTEX_FVF, m_spDevice, &spMesh);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		m_spMesh.Attach(spMesh.Detach());
	}

	VERTEX* pVertices;
	DWORD dwNumVertices = m_spMesh->GetNumVertices();

	hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	for (UINT i = 0; i < dwNumVertices; i++)
	{
		pVertices[i].vTex[2] = pVertices[i].vTex[0];
		pVertices[i].color = 0xFFFFFFFF;
	}

	m_spMesh->UnlockVertexBuffer();

	//MakePointsList();
	//m_LoopSubdivider.Init(m_spMesh, m_spAdjacency);
}

void CFaceworxDoc::AdjustBounds(int nSide)
{
	CSize size1 = GetImageSize(nSide);
	float k = min(size1.cx / m_bounds[nSide].Width(), size1.cy / m_bounds[nSide].Height());
	CSizeF size2(m_bounds[nSide].Width() * k - 100.0f, m_bounds[nSide].Height() * k - 100.0f);
	CRectF bounds(CPointF((size1.cx - size2.cx) / 2, (size1.cy - size2.cy) / 2), size2);
	SetBounds(nSide, bounds);
}

BOOL CFaceworxDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	try
	{
		// Model loading
		HINSTANCE hInst = AfxGetResourceHandle();
		HRSRC hRes = FindResource(hInst, (LPCTSTR)IDR_MODEL, _T("DATA"));
		if (hRes == NULL)
			AtlThrowLastWin32();

		DWORD nResSize = SizeofResource(hInst, hRes);
		if (nResSize == 0)
			AtlThrowLastWin32();

		HGLOBAL hResData = LoadResource(hInst, hRes);
		if (hResData == NULL)
			AtlThrowLastWin32();

		LPVOID pResData = LockResource(hResData);
		if (pResData == NULL)
			AtlThrowLastWin32();

		CMemFile file((BYTE*)pResData, nResSize);
		CArchive ar(&file, CArchive::load);

		SerializeModel(ar);

		FreeResource(hResData);

		CreateDevice();
		LoadMesh();

		DWORD dwNumVertices = m_spMesh->GetNumVertices();
		DWORD dwStride = D3DXGetFVFVertexSize(m_spMesh->GetFVF());

		HRESULT hr;
		VERTEX* pVertices;
		hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		SetNodeVertices(pVertices, dwNumVertices);

		D3DXVECTOR3 vMin, vMax;
		hr = D3DXComputeBoundingBox((D3DXVECTOR3*)pVertices, dwNumVertices, dwStride, &vMin, &vMax);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		for (int i = 0; i < 2; i++)
		{
			m_offset[i] = CPointF(i == 0 ? -vMin.x : -vMin.z, -vMin.y);

			if (!m_nodePos[i].SetCount(m_nodes[i].GetCount()))
				AtlThrow(E_OUTOFMEMORY);

			for (size_t j = 0; j < m_nodePos[i].GetCount(); j++)
			{
				const D3DXVECTOR3& v = pVertices[m_nodes[i][j].m_nVertex].vPos;
				m_nodePos[i][j] = CPointF(i == 0 ? v.x : -v.z, -v.y) + m_offset[i];
			}

			ComputeBox(i);
		}

		if (m_bLeft)
		{
			for (size_t i = 0; i < m_nodePos[1].GetCount(); i++)
				m_nodePos[1][i].x = 2 * m_bounds[1].left + m_bounds[1].Width() - m_nodePos[1][i].x;
		}

		SetTextureCoords(pVertices, dwNumVertices);

		hr = D3DXComputeBoundingSphere((D3DXVECTOR3*)pVertices, dwNumVertices,
			dwStride, &m_vObjectCenter, &m_fObjectRadius);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		m_spMesh->UnlockVertexBuffer();

		for (int i = 0; i < 2; i++)
		{
			CreateDefaultTexture(m_spDevice, 0, D3DPOOL_MANAGED, &m_spTexture[i], IMAGE_WIDTH, IMAGE_HEIGHT);
			AdjustBounds(i);
		}

		CreateEnhancedMesh();

		m_blendZone.m_fWidth = 100;
		m_blendZone.m_nodes.SetCount(5);
		m_blendZone.m_nodes[0] = CPointF(IMAGE_WIDTH * 7 / 12, 0);
		m_blendZone.m_nodes[1] = CPointF(IMAGE_WIDTH * 7 / 12, IMAGE_HEIGHT / 4);
		m_blendZone.m_nodes[2] = CPointF(IMAGE_WIDTH * 7 / 12, IMAGE_HEIGHT * 2 / 4);
		m_blendZone.m_nodes[3] = CPointF(IMAGE_WIDTH * 7 / 12, IMAGE_HEIGHT * 3 / 4);
		m_blendZone.m_nodes[4] = CPointF(IMAGE_WIDTH * 7 / 12, IMAGE_HEIGHT);

		POSITION pos = GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = GetNextView(pos);
			if (pView->IsKindOf(RUNTIME_CLASS(CImageView)))
				((CImageView*)pView)->AdjustScale();
		}
	}
	catch (CException* e)
	{
		TCHAR szError[255];
		e->GetErrorMessage(szError, sizeof(szError) / sizeof(TCHAR));
		e->Delete();
		AfxMessageBox(szError, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

BOOL CFaceworxDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	try
	{
		HRESULT hr;
		VERTEX* pVertices;
		hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		DWORD dwNumVertices = m_spMesh->GetNumVertices();
		DWORD dwStride = D3DXGetFVFVertexSize(m_spMesh->GetFVF());

		hr = D3DXComputeBoundingSphere((D3DXVECTOR3*)pVertices, dwNumVertices,
			dwStride, &m_vObjectCenter, &m_fObjectRadius);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		m_spMesh->UnlockVertexBuffer();

		CreateEnhancedMesh();

		for (int i = 0; i < 2; i++)
		{
			if (m_image[i].IsNull())
				CreateDefaultTexture(m_spDevice, 0, D3DPOOL_MANAGED, &m_spTexture[i], IMAGE_WIDTH, IMAGE_HEIGHT);
			else
				CreateTextureFromImage(m_spDevice, m_image[i], 0, D3DPOOL_MANAGED, &m_spTexture[i]);
		}

		FillTextureAlpha();

		POSITION pos = GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = GetNextView(pos);
			if (pView->IsKindOf(RUNTIME_CLASS(CImageView)))
				((CImageView*)pView)->AdjustScale();
		}
	}
	catch (CException* e)
	{
		TCHAR szError[255];
		e->GetErrorMessage(szError, sizeof(szError) / sizeof(TCHAR));
		e->Delete();
		AfxMessageBox(szError, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

// CFaceworxDoc serialization

inline CArchive& operator<<(CArchive& ar, CPointF pt)
{
	return ar << pt.x << pt.y;
}

inline CArchive& operator>>(CArchive& ar, CPointF& pt)
{
	return ar >> pt.x >> pt.y;
}

inline CArchive& operator<<(CArchive& ar, const CRectF& rect)
{
	return ar << rect.left << rect.top << rect.right << rect.bottom;
}

inline CArchive& operator>>(CArchive& ar, CRectF& rect)
{
	return ar >> rect.left >> rect.top >> rect.right >> rect.bottom;
}

CArchive& operator<<(CArchive& ar, const CFaceworxDoc::CBlendZone& blendZone)
{
	ar << blendZone.m_fWidth;

	ar.WriteCount(blendZone.m_nodes.GetCount());

	for (size_t i = 0; i < blendZone.m_nodes.GetCount(); i++)
		ar << blendZone.m_nodes[i];

	return ar;
}

CArchive& operator>>(CArchive& ar, CFaceworxDoc::CBlendZone& blendZone)
{
	ar >> blendZone.m_fWidth;

	if (!blendZone.m_nodes.SetCount(ar.ReadCount()))
		AtlThrow(E_OUTOFMEMORY);

	for (size_t i = 0; i < blendZone.m_nodes.GetCount(); i++)
		ar >> blendZone.m_nodes[i];

	return ar;
}

CArchive& operator<<(CArchive& ar, const CImage& image)
{
	CFile* pFile = ar.GetFile();
	ar.Flush();
	ULONGLONG ulPos = pFile->GetPosition();
	pFile->Seek(sizeof(ULONGLONG), CFile::current);

	HRESULT hr;
	CArchiveStream strm(&ar);
	CSubStream sstrm(&strm);

	hr = image.Save(&sstrm, Gdiplus::ImageFormatJPEG);
	if (FAILED(hr))
		AtlThrow(hr);

	ar.Flush();
	ULONGLONG ulPos2 = pFile->SeekToEnd();
	pFile->Seek(ulPos, CFile::begin);
	ar << ulPos2 - ulPos - sizeof(ULONGLONG);
	ar.Flush();
	pFile->SeekToEnd();

	return ar;
}

CArchive& operator>>(CArchive& ar, CImage& image)
{
	ULARGE_INTEGER uliSize;
	ar >> uliSize.QuadPart;

	CFile* pFile = ar.GetFile();
	ar.Flush();
	ULONGLONG ulPos = pFile->GetPosition();

	HRESULT hr;
	CArchiveStream strm(&ar);
	CSubStream sstrm(&strm, uliSize);

	hr = image.Load(&sstrm);
	if (FAILED(hr))
		AtlThrow(hr);

	ar.Flush();
	pFile->Seek(ulPos + uliSize.QuadPart, CFile::begin);

	return ar;
}

void CFaceworxDoc::SerializeModel(CArchive& ar)
{
	if (ar.IsStoring())
	{
		for (int i = 0; i < 2; i++)
		{
			ar.WriteCount((UINT)m_nodes[i].GetCount());

			for (size_t j = 0; j < m_nodes[i].GetCount(); j++)
			{
				const CNode& node = m_nodes[i][j];
				ar.WriteCount(node.m_nVertex);
				ar.WriteCount(node.m_nLevel);
			}

			ar.WriteCount((UINT)m_contours[i].GetCount());

			for (size_t j = 0; j < m_contours[i].GetCount(); j++)
			{
				const CContour& contour = m_contours[i][j];
				ar << contour.m_strName;
				ar.WriteCount(contour.m_nParent);
				ar.WriteCount(contour.m_nLevel);

				ar.WriteCount((UINT)contour.m_segments.GetCount());

				for (size_t k = 0; k < contour.m_segments.GetCount(); k++)
				{
					const CSegment& segment = contour.m_segments[k];
					ar.WriteCount(segment.m_nNode1);
					ar.WriteCount(segment.m_nNode2);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			UINT nCount;
			nCount = ar.ReadCount();
			if (!m_nodes[i].SetCount(nCount))
				AtlThrow(E_OUTOFMEMORY);

			for (size_t j = 0; j < m_nodes[i].GetCount(); j++)
			{
				CNode& node = m_nodes[i][j];
				node.m_nVertex = ar.ReadCount();
				node.m_nLevel = ar.ReadCount();
			}

			nCount = ar.ReadCount();
			if (!m_contours[i].SetCount(nCount))
				AtlThrow(E_OUTOFMEMORY);

			for (size_t j = 0; j < m_contours[i].GetCount(); j++)
			{
				CContour& contour = m_contours[i][j];
				ar >> contour.m_strName;
				contour.m_nParent = ar.ReadCount();
				contour.m_nLevel = ar.ReadCount();

				if (contour.m_nParent != (size_t)-1)
					m_contours[i][contour.m_nParent].m_contours.Add(j);

				nCount = ar.ReadCount();
				if (!contour.m_segments.SetCount(nCount))
					AtlThrow(E_OUTOFMEMORY);

				for (size_t k = 0; k < contour.m_segments.GetCount(); k++)
				{
					CSegment& segment = contour.m_segments[k];
					segment.m_nNode1 = ar.ReadCount();
					segment.m_nNode2 = ar.ReadCount();

					m_nodes[i][segment.m_nNode1].m_contours[j].Add(k);
					m_nodes[i][segment.m_nNode2].m_contours[j].Add(k);
				}
			}
		}
	}
}

void CFaceworxDoc::SerializeMesh(CArchive& ar)
{
	HRESULT hr;
	if (ar.IsStoring())
	{
		TCHAR szTempName[MAX_PATH];
		TCHAR szTempPath[MAX_PATH];
		BYTE pBuffer[4096];

		if (GetTempPath(MAX_PATH, szTempPath) == 0)
			AtlThrowLastWin32();

		if (GetTempFileName(szTempPath, _T("fwx"), 0, szTempName) == 0)
			AtlThrowLastWin32();

		hr = D3DXSaveMeshToX(szTempName, m_spMesh, NULL, NULL, NULL, 0, D3DXF_FILEFORMAT_BINARY | D3DXF_FILEFORMAT_COMPRESSED);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		CFile file(szTempName, CFile::modeRead);
		ar.WriteCount((UINT)file.GetLength());

		UINT nRead;
		while (nRead = file.Read(pBuffer, sizeof(pBuffer)))
			ar.Write(pBuffer, nRead);

		file.Close();

		if (DeleteFile(szTempName) == 0)
			AtlThrowLastWin32();
	}
	else
	{
		UINT nSize = ar.ReadCount();

		CHeapPtr<BYTE> pBuffer;
		if (!pBuffer.Allocate(nSize))
			AtlThrow(E_OUTOFMEMORY);

		if (ar.Read(pBuffer, nSize) != nSize)
			AtlThrow(E_FAIL);

		CreateDevice();

		hr = D3DXLoadMeshFromXInMemory(pBuffer, nSize, D3DXMESH_SYSTEMMEM, m_spDevice, NULL, NULL, NULL, &m_dwNumMaterials, &m_spMesh);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		if (m_dwNumMaterials == 0)
			AtlThrow(E_FAIL);

		ATLTRY(m_spAdjacency.Attach(new DWORD[m_spMesh->GetNumFaces() * 3]));
		if (!m_spAdjacency)
			AtlThrow(E_OUTOFMEMORY);

		hr = m_spMesh->GenerateAdjacency(ADJACENCY_EPSILON, m_spAdjacency);
		if (FAILED(hr))
			AfxThrowDXException(hr);
	}
}

void CFaceworxDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar.WriteCount(CURRENT_VERSION);

		SerializeModel(ar);
		SerializeMesh(ar);

		for (int i = 0; i < 2; i++)
		{
			for (size_t j = 0; j < m_nodes[i].GetCount(); j++)
			{
				const CNode& node = m_nodes[i][j];

				ar.WriteCount((UINT)node.m_vertices.GetCount());

				POSITION pos = node.m_vertices.GetStartPosition();
				while (pos != NULL)
				{
					ar.WriteCount(node.m_vertices.GetKeyAt(pos));
					ar << node.m_vertices.GetNextValue(pos);
				}
			}

			ar << m_offset[i] << m_bounds[i];

			ar.WriteCount(m_nodePos[i].GetCount());
			for (size_t j = 0; j < m_nodePos[i].GetCount(); j++)
				ar << m_nodePos[i][j];

			if (!m_image[i].IsNull())
			{
				ar << true;
				ar << m_image[i];
			}
			else
				ar << false;
		}

		ar << m_blendZone;
		ar << m_bLeft;
	}
	else
	{
		if (ar.ReadCount() != CURRENT_VERSION)
			//AfxThrowArchiveException(CArchiveException::genericException); //2005
			AfxThrowArchiveException(CArchiveException::generic); //2003

		SerializeModel(ar);
		SerializeMesh(ar);

		for (int i = 0; i < 2; i++)
		{
			for (size_t j = 0; j < m_nodes[i].GetCount(); j++)
			{
				CNode& node = m_nodes[i][j];

				UINT nCount = ar.ReadCount();

				for (size_t k = 0; k < nCount; k++)
				{
					UINT nVertex = ar.ReadCount();
					float fWeight;
					ar >> fWeight;

					node.m_vertices.SetAt(nVertex, fWeight);
				}
			}

			ar >> m_offset[i] >> m_bounds[i];

			if (!m_nodePos[i].SetCount(ar.ReadCount()))
				AtlThrow(E_OUTOFMEMORY);

			for (size_t j = 0; j < m_nodePos[i].GetCount(); j++)
				ar >> m_nodePos[i][j];

			bool b;
			ar >> b;
			if (b)
				ar >> m_image[i];
		}

		ar >> m_blendZone;
		ar >> m_bLeft;
	}
}

// CFaceworxDoc diagnostics

#ifdef _DEBUG
void CFaceworxDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFaceworxDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

C3DView* CFaceworxDoc::Get3DView()
{
	C3DView* p3DView = NULL;
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(C3DView)))
		{
			p3DView = (C3DView*)pView;
			break;
		}
	}
	ASSERT(p3DView != NULL);
	return p3DView;
}

void CFaceworxDoc::CreateDevice()
{
	HRESULT hr;

	m_spD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if (!m_spD3D)
		AtlThrow(E_FAIL);

	C3DView* p3DView = Get3DView();

	ZeroMemory(&m_pp, sizeof(D3DPRESENT_PARAMETERS));
	m_pp.AutoDepthStencilFormat = D3DFMT_D16;
	m_pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_pp.BackBufferCount = 1;
	m_pp.EnableAutoDepthStencil = TRUE;
	m_pp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	m_pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_pp.MultiSampleQuality = 0;
	m_pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_pp.Windowed = TRUE;
	m_pp.hDeviceWindow = p3DView->m_hWnd;

	hr = m_spD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, p3DView->m_hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_pp, &m_spDevice);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	D3DCAPS9 caps;
	hr = m_spDevice->GetDeviceCaps(&caps);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	m_bUseHWNPatches = (caps.DevCaps & D3DDEVCAPS_NPATCHES) != 0;
}

void CFaceworxDoc::SetNodeVertices(VERTEX* pVertices, DWORD dwNumVertices)
{
	CAtlArray<float> distances, powers;

	for (int i = 0; i < 2; i++)
	{
		if (!distances.SetCount(m_nodes[i].GetCount()))
			AtlThrow(E_OUTOFMEMORY);

		if (!powers.SetCount(m_nodes[i].GetCount()))
			AtlThrow(E_OUTOFMEMORY);

		for (UINT j = 0; j < dwNumVertices; j++)
		{
			const D3DXVECTOR3& v = pVertices[j].vPos;

			bool b = false;
			for (size_t k = 0; k < m_nodes[i].GetCount(); k++)
			{
				CNode& node = m_nodes[i][k];
				const D3DXVECTOR3& v1 = pVertices[node.m_nVertex].vPos;
				distances[k] = i == 0 ? sqrt((v.x - v1.x) * (v.x - v1.x) + (v.y - v1.y) * (v.y - v1.y)) :
					sqrt((v.z - v1.z) * (v.z - v1.z) + (v.y - v1.y) * (v.y - v1.y));
				if (distances[k] == 0)
					b = true;
			}

			if (b)
			{
				for (size_t k = 0; k < m_nodes[i].GetCount(); k++)
				{
					CNode& node = m_nodes[i][k];

					if (distances[k] == 0)
						node.m_vertices.SetAt(j, 1.0f);
				}
			}
			else
			{
				float sum = 0;
				for (size_t k = 0; k < m_nodes[i].GetCount(); k++)
				{
					powers[k] = pow(distances[k], -4.0f);
					sum += powers[k];
				}

				for (size_t k = 0; k < m_nodes[i].GetCount(); k++)
				{
					CNode& node = m_nodes[i][k];
					float weight = powers[k] / sum;
					if (weight > 0.0f)
						node.m_vertices.SetAt(j, weight);
				}
			}
		}
	}
}

void CFaceworxDoc::ComputeBox(int nSide)
{
	m_bounds[nSide] = CRectF(m_nodePos[nSide][0], m_nodePos[nSide][0]);

	for (size_t i = 1; i < m_nodePos[nSide].GetCount(); i++)
	{
		CPointF& pt = m_nodePos[nSide][i];
		m_bounds[nSide] = CRectF(min(m_bounds[nSide].left, pt.x), min(m_bounds[nSide].top, pt.y),
			max(m_bounds[nSide].right, pt.x), max(m_bounds[nSide].bottom, pt.y));
	}
}

void CFaceworxDoc::SetTextureCoords(VERTEX* pVertices, DWORD dwNumVertices)
{
	for (int i = 0; i < 2; i++)
	{
		CSize size = GetImageSize(i);

		for (UINT j = 0; j < dwNumVertices; j++)
		{
			VERTEX& v = pVertices[j];

			if (i == 0)
				v.vTex[i].x = (v.vPos.x + m_offset[i].x) / size.cx;
			else
			{
				if (!m_bLeft)
					v.vTex[i].x = (-v.vPos.z + m_offset[i].x) / size.cx;
				else
					v.vTex[i].x = (v.vPos.z + m_offset[i].x) / size.cx;
			}
			v.vTex[i].y = (-v.vPos.y + m_offset[i].y) / size.cy;
		}
	}
}

inline float CFaceworxDoc::GetNumSegs() const
{
	if (m_nDetail == 0)
		return 1.0f;
	else if (m_nDetail == 1)
		return 2.0f;
	else
		return 4.0f;
}

void CFaceworxDoc::CreateEnhancedMesh()
{
	HRESULT hr;

	if (m_nDetail > 0)
	{
		hr = D3DXComputeNormals(m_spMesh, m_spAdjacency);
		if (FAILED(hr))
			AfxThrowDXException(hr);

		CComPtr<ID3DXMesh> spTempMesh;

		if (m_bUseHWNPatches)
		{
			hr = m_spMesh->CloneMeshFVF(D3DXMESH_WRITEONLY | D3DXMESH_NPATCHES |
				(m_spMesh->GetOptions() & D3DXMESH_32BIT),
				m_spMesh->GetFVF(), m_spDevice, &spTempMesh);
			if (FAILED(hr))
				AfxThrowDXException(hr);
		}
		else
		{
			CComPtr<ID3DXMesh> spTempMesh2;

			CComPtr<IDirect3DVertexBuffer9> vb;
			hr = m_spMesh->GetVertexBuffer(&vb);
/*
			float numsegs[4];
			numsegs[0]=numsegs[1]=numsegs[2]=numsegs[3]=GetNumSegs();
			D3DRECTPATCH_INFO RectInfo;
			RectInfo.Width = 4;
			RectInfo.Height = 4;
			RectInfo.Stride = 4;
			RectInfo.Basis = D3DBASIS_BEZIER;
			RectInfo.StartVertexOffsetWidth = 0;
			RectInfo.StartVertexOffsetHeight = 0;
			D3DVERTEXELEMENT9 el;
			el.Stream=0;
			el.Offset=0;
			el.Type=D3DDECLTYPE_FLOAT1;
			el.Method=D3DDECLMETHOD_DEFAULT;
			el.Usage=D3DDECLUSAGE_POSITION;
			el.UsageIndex=0;
			hr = D3DXTessellateRectPatch(vb,numsegs,&el,&RectInfo,m_spMesh);
*/
			//hr = D3DXTessellateNPatches(m_spMesh, m_spAdjacency, GetNumSegs(), FALSE, &spTempMesh2, NULL);
			//hr = TesselateQuads();
			hr = LoopSubdivide(&spTempMesh2);
			if (FAILED(hr))
			{
				hr = m_spMesh->CloneMeshFVF(D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT,
					m_spMesh->GetFVF(), m_spDevice, &spTempMesh);
				if (FAILED(hr))
					AfxThrowDXException(hr);

				hr = D3DXTessellateNPatches(spTempMesh, NULL, GetNumSegs(), FALSE, &spTempMesh2, NULL);
				if (FAILED(hr))
					AfxThrowDXException(hr);

				spTempMesh.Release();
			}

			hr = spTempMesh2->CloneMeshFVF(D3DXMESH_SYSTEMMEM, m_spMesh->GetFVF(), m_spDevice, &spTempMesh);
			if (FAILED(hr))
				AfxThrowDXException(hr);
		}

		m_spEnhancedMesh.Attach(spTempMesh.Detach());
	}
	else
	{
		hr = D3DXComputeNormals(m_spMesh, m_spAdjacency);
		m_spEnhancedMesh = m_spMesh;
	}
}

void CFaceworxDoc::FillTextureAlpha()
{
	if (!m_spTexture[1])
		return;
	CPointFArray line1, line2;
	line1.Copy(m_blendZone.m_nodes);
	line2.Copy(m_blendZone.m_nodes);
	float fMin = FLT_MAX, fMax = -FLT_MAX;
	for (int iline = 0; iline < 2; iline++)
	{
		CPointFArray& line = iline ? line1 : line2;
		for (size_t n = 0, nc = line.GetCount(); n < nc; n++)
		{
			CPointF& v = line[n];
			float x = v.x;
			if (iline)
				x += m_blendZone.m_fWidth * 0.5f;
			else
				x -= m_blendZone.m_fWidth * 0.5f;
			v.x = x;
		}
	}
	if (m_bLeft)
	{
		int w = m_image[1].IsNull() ? IMAGE_WIDTH : m_image[1].GetWidth();
		for (size_t n = 0, nc = line1.GetCount(); n < nc; n++)
		{
			line1[n].x = w - line1[n].x;
		}
		for (size_t n = 0, nc = line1.GetCount(); n < nc; n++)
		{
			line2[n].x = w - line2[n].x;
		}
		struct qwe {
			char q[sizeof(CPointFArray)];
		} a = (qwe&)line1;
		(qwe&)line1 = (qwe&)line2;
		(qwe&)line2 = a;
	}

	texture_blender bl;
	bl.blend(m_spTexture[1], line2, line1);
}

bool CFaceworxDoc::GetCurve(int nSide, size_t nNode, size_t nContour, size_t nSegment, CIndexList& curve) const
{
	const CNode& node = m_nodes[nSide][nNode];
	const CContour& contour = m_contours[nSide][nContour];

	const CIndexArrayMap::CPair* pPair = node.m_contours.Lookup(nContour);
	if (pPair != NULL)
	{
		const CIndexArray& segments = pPair->m_value;
		for (size_t i = 0; i < segments.GetCount(); i++)
		{
			size_t nSegment2 = segments[i];
			if (nSegment2 != nSegment)
			{
				const CSegment& segment = contour.m_segments[nSegment2];
				size_t nNode2 = segment.m_nNode1 == nNode ? segment.m_nNode2 : segment.m_nNode1;
				curve.AddTail(nNode2);
				const CNode& node = m_nodes[nSide][nNode2];
				if (node.m_nLevel <= m_nLevel)
					return true;
				return GetCurve(nSide, nNode2, nContour, nSegment2, curve);
			}
		}
	}

	return false;
}

void CFaceworxDoc::GetCurves(int nSide, size_t nNode, CAtlList<CIndexList>& curves) const
{
	const CNode& node = m_nodes[nSide][nNode];
	POSITION pos = node.m_contours.GetStartPosition();
	while (pos != NULL)
	{
		size_t nContour = node.m_contours.GetKeyAt(pos);
		const CIndexArray& segments = node.m_contours.GetNextValue(pos);
		for (size_t i = 0; i < segments.GetCount(); i++)
		{
			const CContour& contour = m_contours[nSide][nContour];
			size_t nSegment = segments[i];
			const CSegment& segment = contour.m_segments[nSegment];
			size_t nNode2 = segment.m_nNode1 == nNode ? segment.m_nNode2 : segment.m_nNode1;
			const CNode& node = m_nodes[nSide][nNode2];
			if (node.m_nLevel > m_nLevel)
			{
				POSITION pos = curves.AddTail();
				CIndexList& curve = curves.GetAt(pos);
				curve.AddTail(nNode2);
				GetCurve(nSide, nNode2, nContour, nSegment, curve);
			}
		}
	}
}

void CFaceworxDoc::GetNodeWeights(int nSide, size_t nNode, const CAtlList<CIndexList>& curves, CNodeWeightMap& weights) const
{
	weights.SetAt(nNode, 1.0f);

	POSITION pos = curves.GetHeadPosition();
	while (pos != NULL)
	{
		const CIndexList& nodeList = curves.GetNext(pos);

		CAtlArray<float> lengths;
		if (!lengths.SetCount(nodeList.GetCount() - 1))
			AtlThrow(E_OUTOFMEMORY);

		size_t nNode1 = nNode;
		float nLength = 0;
		int i = 0;
		POSITION pos2 = nodeList.GetHeadPosition();
		while (pos2 != NULL)
		{
			CPointF pt1 = m_nodePos[nSide][nNode1];
			size_t nNode2 = nodeList.GetNext(pos2);
			CPointF pt2 = m_nodePos[nSide][nNode2];

			nLength += sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) + (pt2.y - pt1.y) * (pt2.y - pt1.y));

			if (pos2 != NULL)
			{
				lengths[i++] = nLength;
				nNode1 = nNode2;
			}
		}

		i = 0;
		pos2 = nodeList.GetHeadPosition();
		while (pos2 != NULL)
		{
			size_t nNode = nodeList.GetNext(pos2);
			if (pos2 != NULL)
			{
				CNodeWeightMap::CPair* pPair = weights.Lookup(nNode);
				float fWeight = 1.0f - lengths[i++] / nLength;
				if (pPair == NULL)
				{
					weights.SetAt(nNode, fWeight);
				}
				else
				{
					pPair->m_value += fWeight;
					if (pPair->m_value > 1.0f)
						pPair->m_value = 1.0f;
				}
			}
		}
	}
}

void CFaceworxDoc::GetNodeWeightsByNode(int nSide, size_t nNode, CNodeWeightMap& weights) const
{
	CAtlList<CIndexList> curves;
	GetCurves(nSide, nNode, curves);
	GetNodeWeights(nSide, nNode, curves, weights);
}

void CFaceworxDoc::GetNodeWeightsByContour(int nSide, size_t nContour, CNodeWeightMap& weights) const
{
	const CContour& contour = m_contours[nSide][nContour];

	CAtlMap<size_t, bool> nodes;
	for (size_t i = 0; i < contour.m_segments.GetCount(); i++)
	{
		const CSegment& segment = contour.m_segments[i];

		if (nodes.Lookup(segment.m_nNode1) == NULL)
		{
			nodes.SetAt(segment.m_nNode1, true);
			GetNodeWeightsByNode(nSide, segment.m_nNode1, weights);
		}

		if (nodes.Lookup(segment.m_nNode2) == NULL)
		{
			nodes.SetAt(segment.m_nNode2, true);
			GetNodeWeightsByNode(nSide, segment.m_nNode2, weights);
		}
	}

	for (size_t i = 0; i < contour.m_contours.GetCount(); i++)
		GetNodeWeightsByContour(nSide, contour.m_contours[i], weights);
}

void CFaceworxDoc::GetVertexWeights(int nSide, const CNodeWeightMap& nodeWeights, CVertexWeightMap& weights) const
{
	VERTEX* pVertices;
	HRESULT hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	POSITION pos = nodeWeights.GetStartPosition();
	while (pos != NULL)
	{
		size_t nNode = nodeWeights.GetKeyAt(pos);
		float fWeight = nodeWeights.GetNextValue(pos);
		const CNode& node = m_nodes[nSide][nNode];

		POSITION pos2 = node.m_vertices.GetStartPosition();
		while (pos2 != NULL)
		{
			UINT nVertex = node.m_vertices.GetKeyAt(pos2);
			float fWeight2 = node.m_vertices.GetNextValue(pos2);

			CVertexWeightMap::CPair* pPair = weights.Lookup(nVertex);
			if (pPair == NULL)
				weights.SetAt(nVertex, fWeight * fWeight2);
			else
				pPair->m_value += fWeight * fWeight2;
		}
	}

	m_spMesh->UnlockVertexBuffer();
}

void CFaceworxDoc::SaveState(CState& state)
{
	for (int i = 0; i < 2; i++)
	{
		state.m_bounds[i] = m_bounds[i];

		if (!state.m_nodes[i].SetCount(m_nodePos[i].GetCount()))
			AtlThrow(E_OUTOFMEMORY);

		for (size_t j = 0; j < m_nodePos[i].GetCount(); j++)
			state.m_nodes[i][j] = m_nodePos[i][j];
	}

	VERTEX* pVertices;
	HRESULT hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	DWORD dwNumVertices = m_spMesh->GetNumVertices();

	if (!state.m_vertices.SetCount(dwNumVertices))
		AtlThrow(E_OUTOFMEMORY);

	for (UINT i = 0; i < dwNumVertices; i++)
	{
		CVertexData& vertexData = state.m_vertices[i];
		const VERTEX& v = pVertices[i];

		vertexData.vPos = v.vPos;
		vertexData.vTex[0] = v.vTex[0];
		vertexData.vTex[1] = v.vTex[1];
	}

	m_spMesh->UnlockVertexBuffer();

	state.m_blendZone = m_blendZone;
}

void CFaceworxDoc::RestoreState(const CState& state)
{
	for (int i = 0; i < 2; i++)
	{
		m_bounds[i] = state.m_bounds[i];

		for (size_t j = 0; j < m_nodePos[i].GetCount(); j++)
			m_nodePos[i][j] = state.m_nodes[i][j];
	}

	VERTEX* pVertices;
	HRESULT hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	DWORD dwNumVertices = m_spMesh->GetNumVertices();

	for (UINT i = 0; i < dwNumVertices; i++)
	{
		const CVertexData& vertexData = state.m_vertices[i];
		VERTEX& v = pVertices[i];

		v.vPos = vertexData.vPos;
		v.vTex[0] = vertexData.vTex[0];
		v.vTex[1] = vertexData.vTex[1];
	}

	m_spMesh->UnlockVertexBuffer();

	m_blendZone = state.m_blendZone;
}

void CFaceworxDoc::BeginState()
{
	if (m_posState == NULL)
		SaveState(m_state);
}

void CFaceworxDoc::EndState()
{
	if (m_posState != NULL)
	{
		while (m_posState != m_states.GetTailPosition())
			m_states.RemoveTailNoReturn();
		m_posState = NULL;
	}
	else
		m_states.AddTail(m_state);
}

void CFaceworxDoc::CancelState()
{
	if (m_posState == NULL)
		RestoreState(m_state);
	else
		RestoreState(m_states.GetAt(m_posState));
}

void CFaceworxDoc::SetBounds(int nSide, CRectF bounds)
{
	CSizeF offset1 = bounds.TopLeft() - m_bounds[nSide].TopLeft();
	CSizeF offset2 = bounds.BottomRight() - m_bounds[nSide].BottomRight() - offset1;

	CSizeF k(offset2.cx / (m_bounds[nSide].right - m_bounds[nSide].left),
		offset2.cy / (m_bounds[nSide].bottom - m_bounds[nSide].top));

	for (size_t i = 0; i < m_nodePos[nSide].GetCount(); i++)
	{
		CPointF& pt = m_nodePos[nSide][i];
		pt += CSizeF((pt.x - m_bounds[nSide].left) * k.cx,
			(pt.y - m_bounds[nSide].top) * k.cy) + offset1;
	}

	VERTEX* pVertices;
	HRESULT hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	DWORD dwNumVertices = m_spMesh->GetNumVertices();
	DWORD dwStride = D3DXGetFVFVertexSize(m_spMesh->GetFVF());

	D3DXVECTOR3 vMin, vMax;
	hr = D3DXComputeBoundingBox((D3DXVECTOR3*)pVertices, dwNumVertices, dwStride, &vMin, &vMax);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	D3DXVECTOR2 vTexMin = pVertices[0].vTex[nSide];

	for (UINT i = 1; i < dwNumVertices; i++)
	{
		const D3DXVECTOR2& v = pVertices[i].vTex[nSide];
		vTexMin = D3DXVECTOR2(min(vTexMin.x, v.x), min(vTexMin.y, v.y));
	}

	D3DXVECTOR3 vOffest((vMin + vMax) / 2);
	CSize size = GetImageSize(nSide);

	for (UINT i = 0; i < dwNumVertices; i++)
	{
		VERTEX& vertex = pVertices[i];

		if (nSide == 0)
		{
			vertex.vPos.x += (vertex.vPos.x - vOffest.x) * k.cx;
			vertex.vPos.y -= (vOffest.y - vertex.vPos.y) * k.cy;
			vertex.vPos.z -= (vOffest.z - vertex.vPos.z) * k.cy;
		}
		else
		{
			vertex.vPos.z = vOffest.z - (vOffest.z - vertex.vPos.z) / (1 + k.cy);
			vertex.vPos.z -= (vOffest.z - vertex.vPos.z) * k.cx;
		}

		vertex.vTex[nSide] += D3DXVECTOR2((vertex.vTex[nSide].x - vTexMin.x) * k.cx + offset1.cx / size.cx,
			(vertex.vTex[nSide].y - vTexMin.y) * k.cy + offset1.cy / size.cy);
	}

	m_spMesh->UnlockVertexBuffer();

	m_bounds[nSide] = bounds;
}

void CFaceworxDoc::MoveNodes(int nSide, CSizeF offset, const CNodeWeightMap& nodeWeights, const CVertexWeightMap& vertexWeights)
{
	POSITION pos = nodeWeights.GetStartPosition();
	while (pos != NULL)
	{
		size_t nNode = nodeWeights.GetKeyAt(pos);
		float fWeight = nodeWeights.GetNextValue(pos);
		m_nodePos[nSide][nNode] += CPointF(offset.cx * fWeight, offset.cy * fWeight);
	}

	ComputeBox(nSide);

	VERTEX* pVertices;
	HRESULT hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	DWORD dwNumVertices = m_spMesh->GetNumVertices();
	DWORD dwStride = D3DXGetFVFVertexSize(m_spMesh->GetFVF());

	CSize size = GetImageSize(nSide);

	pos = vertexWeights.GetStartPosition();
	while (pos != NULL)
	{
		UINT nVertex = vertexWeights.GetKeyAt(pos);
		float fWeight = vertexWeights.GetNextValue(pos);
		VERTEX& vertex = pVertices[nVertex];
		CSizeF offset2(offset.cx * fWeight, offset.cy * fWeight);

		if (nSide == 0)
			vertex.vPos.x += offset2.cx;
		else
		{
			if (!m_bLeft)
				vertex.vPos.z -= offset2.cx;
			else
				vertex.vPos.z += offset2.cx;
		}

		vertex.vPos.y -= offset2.cy / 2.0f;

		vertex.vTex[nSide] += D3DXVECTOR2(offset2.cx / size.cx, offset2.cy / size.cy);
	}

	m_spMesh->UnlockVertexBuffer();
}

CSize CFaceworxDoc::GetImageSize(int nSize) const
{
	if (m_image[nSize].IsNull())
		return CSize(IMAGE_WIDTH, IMAGE_HEIGHT);

	return CSize(m_image[nSize].GetWidth(), m_image[nSize].GetHeight());
}

void CFaceworxDoc::LoadImage(int nSide)
{
	CSize size = GetImageSize(nSide);
	CString strImporters;
	CSimpleArray<GUID> aguidFileTypes;
	CImage::GetImporterFilterString(strImporters, aguidFileTypes, _T("All Image Files"));

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strImporters);
	if (dlg.DoModal() != IDOK)
		return;

	m_image[nSide].Destroy();
	HRESULT hr = m_image[nSide].Load(dlg.GetPathName());
	if (FAILED(hr))
		AtlThrow(hr);

	m_spTexture[nSide].Release();
	CreateTextureFromImage(m_spDevice, m_image[nSide], 0, D3DPOOL_MANAGED, &m_spTexture[nSide]);

	VERTEX* pVertices;
	hr = m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	DWORD dwNumVertices = m_spMesh->GetNumVertices();

	for (UINT i = 0; i < dwNumVertices; i++)
	{
		VERTEX& v = pVertices[i];
		v.vTex[nSide].x = v.vTex[nSide].x * size.cx / m_image[nSide].GetWidth();
		v.vTex[nSide].y = v.vTex[nSide].y * size.cy / m_image[nSide].GetHeight();
	}

	if (nSide == 1)
	{
		for (UINT i = 0; i < m_blendZone.m_nodes.GetCount(); i++)
		{
			CPointF& pt = m_blendZone.m_nodes[i];
			pt.x = pt.x / size.cx * m_image[nSide].GetWidth();
			pt.y = pt.y / size.cy * m_image[nSide].GetHeight();
		}
	}

	m_spMesh->UnlockVertexBuffer();

	AdjustBounds(nSide);

	CreateEnhancedMesh();
	FillTextureAlpha();

	SetModifiedFlag();
}

// CFaceworxDoc commands

void CFaceworxDoc::OnFileExport()
{
	try
	{
		obj_exporter exp;

		CString strFilter;
		strFilter.LoadString(IDS_EXPORT_FILTER);

		CString strTitle = GetTitle();
		PathRemoveExtension(strTitle.GetBuffer());
		strTitle.ReleaseBuffer();
		CExportDlg dlg(_T("obj"), strTitle, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, AfxGetMainWnd());
		if (dlg.DoModal() != IDOK)
			return;

		int ex_detail = m_nDetail;
		if (ex_detail != dlg.m_nModelDetail)
		{
			m_nDetail = dlg.m_nModelDetail;
			CreateEnhancedMesh();
		}

		switch (dlg.m_nTextureSize)
		{
		case 0:
			exp.width = 64;
			exp.height = 64;
			break;
		case 1:
			exp.width = 128;
			exp.height = 128;
			break;
		case 2:
			exp.width = 256;
			exp.height = 256;
			break;
		case 3:
			exp.width = 512;
			exp.height = 512;
			break;
		case 4:
			exp.width = 1024;
			exp.height = 1024;
			break;
		default:
			exp.width = 2048;
			exp.height = 2048;
		}

		exp.quality = dlg.m_nTextureQuality;

		exp.do_export(dlg.GetPathName(), this);

		if (ex_detail != m_nDetail)
		{
			m_nDetail = ex_detail;
			CreateEnhancedMesh();
		}
	}
	catch (CUserException*)
	{
		throw;
	}
	catch (CException* e)
	{
		TCHAR szError[255];
		e->GetErrorMessage(szError, sizeof(szError) / sizeof(TCHAR));
		e->Delete();
		AfxMessageBox(szError, MB_OK | MB_ICONSTOP);
	}
}

void CFaceworxDoc::OnEditUndo()
{
	if (m_posState == NULL)
	{
		m_posState = m_states.GetTailPosition();
		SaveState(m_states.GetAt(m_states.AddTail()));
	}
	else
		m_states.GetPrev(m_posState);
	RestoreState(m_states.GetAt(m_posState));
	CreateEnhancedMesh();
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_posState != m_states.GetHeadPosition());
}

void CFaceworxDoc::OnEditRedo()
{
	m_states.GetNext(m_posState);
	RestoreState(m_states.GetAt(m_posState));

	CreateEnhancedMesh();
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_posState != NULL && m_posState != m_states.GetTailPosition());
}

void CFaceworxDoc::OnViewBox()
{
	m_bBox = !m_bBox;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewBox(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bBox);
}

void CFaceworxDoc::OnViewFeatures()
{
	m_bFeatures = !m_bFeatures;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewFeatures(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bFeatures);
}

void CFaceworxDoc::OnViewPointsAll()
{
	m_nLevel = MAX_LEVEL;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewPointsAll(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nLevel < MAX_LEVEL);
}

void CFaceworxDoc::OnViewPointsMore()
{
	m_nLevel++;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewPointsMore(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nLevel < MAX_LEVEL);
}

void CFaceworxDoc::OnViewPointsLess()
{
	m_nLevel--;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewPointsLess(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nLevel > -1);
}

void CFaceworxDoc::OnViewPointsNo()
{
	m_nLevel = -1;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewPointsNo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nLevel > -1);
}

void CFaceworxDoc::OnViewBlendZone()
{
	m_bBlend = !m_bBlend;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewBlendZone(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bBlend);
}

void CFaceworxDoc::OnViewWireframe()
{
	m_bWireframe = !m_bWireframe;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewWireframe(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_bWireframe);
}

void CFaceworxDoc::OnViewSolid()
{
	m_bWireframe = !m_bWireframe;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewSolid(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(!m_bWireframe);
}

void CFaceworxDoc::OnViewLighting()
{
	m_bLighting = !m_bLighting;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewLighting(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bLighting);
}

void CFaceworxDoc::OnViewOrthogonal()
{
	m_bOrtho = true;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewOrthogonal(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_bOrtho);
}

void CFaceworxDoc::OnViewPerspective()
{
	m_bOrtho = false;
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewPerspective(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(!m_bOrtho);
}

void CFaceworxDoc::OnViewModelDetailHigh()
{
	m_nDetail = 2;
	CreateEnhancedMesh();
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewModelDetailHigh(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_nDetail == 2);
}

void CFaceworxDoc::OnViewModelDetailMedium()
{
	m_nDetail = 1;
	CreateEnhancedMesh();
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewModelDetailMedium(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_nDetail == 1);
}

void CFaceworxDoc::OnViewModelDetailLow()
{
	m_nDetail = 0;
	CreateEnhancedMesh();
	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateViewModelDetailLow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_nDetail == 0);
}

void CFaceworxDoc::OnModelLoadFront()
{
	LoadImage(0);
	UpdateAllViews(NULL);

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CFrontView)))
			((CFrontView*)pView)->AdjustScale();
	}
}

void CFaceworxDoc::OnModelLoadSide()
{
	LoadImage(1);
	UpdateAllViews(NULL);

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CSideView)))
			((CSideView*)pView)->AdjustScale();
	}
}

void CFaceworxDoc::OnModelLeftSide()
{
	m_bLeft = !m_bLeft;

	for (size_t i = 0; i < m_nodePos[1].GetCount(); i++)
		m_nodePos[1][i].x = 2 * m_bounds[1].left + m_bounds[1].Width() - m_nodePos[1][i].x;

	FillTextureAlpha();

	UpdateAllViews(NULL);
}

void CFaceworxDoc::OnUpdateModelLeftSide(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bLeft);
	pCmdUI->Enable(FALSE);
}


void CFaceworxDoc::MakePointsList()
{
	//m_quads

	VERTEX* pVertices;
	WORD* pIndices;
/*
	obj_exporter oe;
	CComPtr<ID3DXMesh> tempMesh;
	oe.join_vertices(m_spMesh, tempMesh);
	m_spMesh = tempMesh;*/

	WORD dwNumVertices = (WORD)m_spMesh->GetNumVertices();
	WORD dwNumIndices = (WORD)m_spMesh->GetNumFaces() * 3;

	m_spMesh->LockVertexBuffer(0, (LPVOID*)&pVertices);
	m_spMesh->LockIndexBuffer(0, (LPVOID*)&pIndices);

	int bad = 0;
	for (WORD n = 0; n < dwNumIndices; n += 6, pIndices += 6)
	{
		WORD n1 = pIndices[0];
		WORD n2 = pIndices[1];
		WORD n3 = pIndices[2];
		WORD n4 = std::find(pIndices + 3, pIndices + 6, n1) - pIndices;
		WORD n5 = std::find(pIndices + 3, pIndices + 6, n2) - pIndices;
		WORD n6 = std::find(pIndices + 3, pIndices + 6, n3) - pIndices;
		WORD nn = 3 + 4 + 5 + 6 - n4 - n5 - n6;
		VERTEX vs[6]={pVertices[pIndices[0]],
			pVertices[pIndices[1]],
			pVertices[pIndices[2]],
			pVertices[pIndices[3]],
			pVertices[pIndices[4]],
			pVertices[pIndices[5]],
		};
		CQuad q;
		if (nn >= 3 && nn < 6)
		{
			D3DXVECTOR3 v[4]={
				D3DXVECTOR3(pVertices[n1].vPos.x,pVertices[n1].vPos.y,pVertices[n1].vPos.z),
				D3DXVECTOR3(pVertices[n2].vPos.x,pVertices[n2].vPos.y,pVertices[n2].vPos.z),
				D3DXVECTOR3(pVertices[n3].vPos.x,pVertices[n3].vPos.y,pVertices[n3].vPos.z),
				D3DXVECTOR3(pVertices[pIndices[nn]].vPos.x,pVertices[pIndices[nn]].vPos.y,pVertices[pIndices[nn]].vPos.z),
			};
			D3DXVECTOR3 center=v[0]+v[1]+v[2]+v[3];
			center*=0.25f;
			D3DXVECTOR3 normal;
			D3DXVec3Cross(&normal,&(v[1]-v[0]),&(v[2]-v[0]));
			D3DXVECTOR3 perp;
			D3DXVec3Cross(&perp,&(v[0]-center),&normal);
			std::pair<float,int> an[4];
			an[0].second=pIndices[0];
			an[1].second=pIndices[1];
			an[2].second=pIndices[2];
			an[3].second=pIndices[nn];
			for (int n=0;n<4;n++)
			{
				float x=D3DXVec3Dot(&(v[n]-center),&(v[0]-center));
				float y=D3DXVec3Dot(&(v[n]-center),&perp);
				an[n].first=n ? atan2(y,x) : 0;
				if (an[n].first<0)
				{
					an[n].first+=float(M_PI)*2;
				}
			}
			std::sort(an,an+4);
			q.indices[0]=an[0].second;
			q.indices[1]=an[1].second;
			q.indices[2]=an[2].second;
			q.indices[3]=an[3].second;
			ASSERT(q.indices[0]==pIndices[0]);
/*
			q.indices[0] = pIndices[0];
			q.indices[1] = pIndices[1];
			q.indices[2] = pIndices[2];
			q.indices[3] = pIndices[nn];*/
			m_quads.Add(q);
		} else
		{
			bad++;
			q.indices[0] = pIndices[0];
			q.indices[1] = pIndices[1];
			q.indices[2] =
			q.indices[3] = pIndices[2];
			m_quads.Add(q);
/*			q.indices[0] = pIndices[3];
			q.indices[1] = pIndices[4];
			q.indices[2] =
			q.indices[3] = pIndices[5];
			m_quads.Add(q);*/
//			pIndices[0]=
//				pIndices[1]=
//				pIndices[2]=0;
			n -= 3;
			pIndices -= 3;
		}
		//m_quads.Add(q);
	}

	for (size_t iquad = 0, quads = m_quads.GetCount(); iquad < quads; iquad++)
	{
		CQuad& q = m_quads[iquad];
		for (int neighbour = 0; neighbour < 4; neighbour++)
		{
			q.neighbours[neighbour] = 0xFFFF;
			WORD i1 = q.indices[neighbour];
			WORD i2 = q.indices[(neighbour + 1) % 4];
			for (size_t n = 0, nc = m_quads.GetCount(); n < nc; n++)
			if (n != iquad)
			{
				CQuad& q2 = m_quads[n];
				int joints = 0;
				for (int index = 0; index < 4; index++)
				if (i1 == q2.indices[index] ||
					i2 == q2.indices[index])
					joints++;
				//ASSERT(joints <= 2);
				if (joints >= 2)
				{
					q.neighbours[neighbour] = (WORD)n;
					break;
				}
			}
		}
	}

	m_spMesh->UnlockVertexBuffer();
	m_spMesh->UnlockIndexBuffer();
}

HRESULT CFaceworxDoc::TesselateQuads()
{
/*	CAtlArray<VERTEX> pts;
	VERTEX* vs=0;
	m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);
	for (size_t n=0;n<m_quads.GetCount();n++)
	{
		CQuad& q=m_quads[n];
		for (int side=0;side<4;side++)
		{
			int i1=q.indices[side];
			int i2=q.indices[(side+1)%4];
			int neighbour3=q.neighbours[(side+1)%4];
			int neighbour0=q.neighbours[(side+3)%4];
			if (neighbour3!=0xFFFF && neighbour0!=0xFFFF)
			{
				CQuad& q1=m_quads[neighbour0];
				CQuad& q2=m_quads[neighbour3];
			} else
			{
//				pts.Add(vs[);
			}
		}
	}
	m_spMesh->UnlockVertexBuffer();*/
	return S_OK;
}

HRESULT CFaceworxDoc::LoopSubdivide(ID3DXMesh** mesh_out)
{
	//DWORD faces=m_spMesh->GetNumFaces();
	//DWORD max=*std::max_element(m_spAdjacency.m_p,m_spAdjacency.m_p+faces*3);
/*	if (0)
	{
		CComPtr<ID3DXMesh> mesh;
		if (0)
		{
			LPCTSTR path=L"D:\\Program Files\\Microsoft DirectX 9.0 SDK (October 2005)\\Samples\\Media\\misc\\cell.x";
			CComPtr<ID3DXBuffer> spMaterials;
			DWORD aaa;
			D3DXLoadMeshFromX(path, D3DXMESH_SYSTEMMEM,m_spDevice,NULL, &spMaterials, NULL, &aaa, &mesh);
		} else
		{
			D3DXCreateMeshFVF(2,4,D3DXMESH_SYSTEMMEM,VERTEX_FVF,m_spDevice,&mesh);
			WORD* is=0;
			VERTEX* vs=0;
			mesh->LockVertexBuffer(0,(LPVOID*)&vs);
			mesh->LockIndexBuffer(0,(LPVOID*)&is);
			memset(vs,0,sizeof(VERTEX)*4);
			vs[0].vPos=D3DXVECTOR3(0,0,0);
			vs[1].vPos=D3DXVECTOR3(100,0,0);
			vs[2].vPos=D3DXVECTOR3(100,100,0);
			vs[3].vPos=D3DXVECTOR3(0,100,0);
			vs[0].vNorm=D3DXVECTOR3(0,0,1);
			vs[1].vNorm=D3DXVECTOR3(0,0,1);
			vs[2].vNorm=D3DXVECTOR3(0,0,1);
			vs[3].vNorm=D3DXVECTOR3(0,0,1);
			is[0]=0;
			is[1]=1;
			is[2]=2;
			is[3]=0;
			is[4]=2;
			is[5]=3;
			mesh->UnlockVertexBuffer();
			mesh->UnlockIndexBuffer();
		}
		DWORD faces=mesh->GetNumFaces();
		DWORD* adj=new DWORD[faces*3];
		HRESULT hr = mesh->GenerateAdjacency(ADJACENCY_EPSILON, adj);
		if (1)
		{
			CLoopSubdivider q;
			q.Init(mesh,adj);
			q.Subdivide(mesh_out);
		} else
			*mesh_out=mesh.Detach();
		delete adj;
	} else*/
	
	if (!m_bLoopSubdivider1Inited)
	{
		HRSRC hadjs_med = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ADJS_MED), L"DATA");
		HGLOBAL hres = LoadResource(AfxGetResourceHandle(), hadjs_med);
		DWORD size = (DWORD)SizeofResource(AfxGetResourceHandle(), hadjs_med);
		LPVOID pres = LockResource(hres);
		m_LoopSubdivider1.Init(m_spMesh, m_spAdjacency, pres, size);
		m_bLoopSubdivider1Inited = true;
	}
	m_LoopSubdivider1.Subdivide(m_spMesh, mesh_out);

	if (1 == m_nDetail)
		return S_OK;

	CComPtr<ID3DXMesh> temp_mesh;
	temp_mesh.Attach(*mesh_out);
	*mesh_out = 0;

	if (!m_bLoopSubdivider2Inited)
	{
		DWORD* temp_adjacency = new DWORD[temp_mesh->GetNumFaces() * 3];
		temp_mesh->GenerateAdjacency(ADJACENCY_EPSILON, temp_adjacency);

		HRSRC hadjs_high = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ADJS_HIGH), L"DATA");
		HGLOBAL hres = LoadResource(AfxGetResourceHandle(), hadjs_high);
		DWORD size = (DWORD)SizeofResource(AfxGetResourceHandle(), hadjs_high);
		LPVOID pres = LockResource(hres);
		m_LoopSubdivider2.Init(temp_mesh, temp_adjacency, pres, size);
		m_bLoopSubdivider1Inited = true;

		delete temp_adjacency;
		m_bLoopSubdivider2Inited = true;
	}
	m_LoopSubdivider2.Subdivide(temp_mesh, mesh_out);

	return S_OK;
}

void CFaceworxDoc::OnModeldetailSubdivisionsettings()
{
	static int nose_split[] = {
		89,
		110,
		139,
		192,
		223,
		238,
		259,
		280,
		293,
		320,
		-1,
	};
	static int mouth_split[] = {
		411,
		413,
		416,
		415,
		417,
		414,
		412,
		-1,
	};
	static int eyebrow_split[] = {
		83,
		81,
		85,
		87,
		102,
		84,
		82,
		86,
		88,
		103,
		-1,
	};
	CSubdivisionSettingsDlg dlg;
	dlg.m_bNose = sharpens[0];
	dlg.m_bLips = sharpens[1];
	dlg.m_bEyebrow = sharpens[2];
	if (IDOK != dlg.DoModal())
		return;
	sharpens[0] = dlg.m_bNose;
	sharpens[1] = dlg.m_bLips;
	sharpens[2] = dlg.m_bEyebrow;
	m_LoopSubdivider1.sharpen_pts[0] = sharpens[0] ? nose_split : 0;
	m_LoopSubdivider1.sharpen_pts[1] = sharpens[1] ? mouth_split : 0;
	m_LoopSubdivider1.sharpen_pts[2] = sharpens[2] ? eyebrow_split : 0;
	m_LoopSubdivider1.sharpen_pts_cnt = 3;
	m_bLoopSubdivider1Inited = false;
	CreateEnhancedMesh();
	((CFaceworxApp*)AfxGetApp())->Update3DView();
}

void CFaceworxDoc::OnEditEditin3dwindow()
{
	Get3DView()->SetEditMode(!Get3DView()->m_b3DEditing);
}

void CFaceworxDoc::OnUpdateEditEditin3dwindow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(Get3DView()->m_b3DEditing);
}

void CFaceworxDoc::OnEditFixdirection()
{
	Get3DView()->FixDirection();
}

void CFaceworxDoc::OnEditResetmodifications()
{
	Get3DView()->ResetModifications();
}

void CFaceworxDoc::OnEditSettings()
{
	C3DView* view = Get3DView();
	C3DModificationSettingsDlg dlg;
	dlg.m_R1 = view->m_R1;
	dlg.m_R2 = view->m_R2;
	dlg.m_Shift = view->m_Shift;
	if (dlg.DoModal() != IDOK)
		return;
	view->m_R1 = (float)dlg.m_R1;
	view->m_R2 = (float)dlg.m_R2;
	view->m_Shift = (float)dlg.m_Shift;
}
