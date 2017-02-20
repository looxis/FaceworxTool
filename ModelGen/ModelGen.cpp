// ModelGen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ModelGen.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

CWinApp theApp;

using namespace std;

class CNodesAccessor
{
public:
	LONG m_nSide;
	LONG m_nNum;
	LONG m_nVertex;
	LONG m_nLevel;

	DEFINE_COMMAND_EX(CNodesAccessor, L"SELECT Side, Num, Vertex, [Level] FROM Nodes ORDER BY Side, Num");

	BEGIN_COLUMN_MAP(CNodesAccessor)
		COLUMN_ENTRY(1, m_nSide)
		COLUMN_ENTRY(2, m_nNum)
		COLUMN_ENTRY(3, m_nVertex)
		COLUMN_ENTRY(4, m_nLevel)
	END_COLUMN_MAP()
};

class CContoursAccessor
{
public:
	LONG m_nSide;
	LONG m_nNum;
	TCHAR m_szName[51];
	LONG m_nParent;
	LONG m_nLevel;

	DBSTATUS m_dwParentStatus;

	DEFINE_COMMAND_EX(CContoursAccessor, L"SELECT Side, Num, Name, Parent, [Level] FROM Contours ORDER BY Side, Num");

	BEGIN_COLUMN_MAP(CContoursAccessor)
		COLUMN_ENTRY(1, m_nSide)
		COLUMN_ENTRY(2, m_nNum)
		COLUMN_ENTRY(3, m_szName)
		COLUMN_ENTRY_STATUS(4, m_nParent, m_dwParentStatus)
		COLUMN_ENTRY(5, m_nLevel)
	END_COLUMN_MAP()
};

class CSegmentsAccessor
{
public:
	LONG m_nSide;
	LONG m_nContour;
	LONG m_nNode1;
	LONG m_nNode2;

	DEFINE_COMMAND_EX(CSegmentsAccessor, L"SELECT Side, Contour, Node1, Node2 FROM Segments ORDER BY Side, Contour, Node1, Node2");

	BEGIN_COLUMN_MAP(CSegmentsAccessor)
		COLUMN_ENTRY(1, m_nSide)
		COLUMN_ENTRY(2, m_nContour)
		COLUMN_ENTRY(3, m_nNode1)
		COLUMN_ENTRY(4, m_nNode2)
	END_COLUMN_MAP()
};

class CVerticesAccessor
{
public:
	LONG m_nNum;
	D3DXVECTOR3 m_vPos;
	D3DXVECTOR2 m_vTex;

	DEFINE_COMMAND_EX(CVerticesAccessor, L"SELECT Num, X, Y, Z, U, V FROM Vertices ORDER BY Num");

	BEGIN_COLUMN_MAP(CVerticesAccessor)
		COLUMN_ENTRY(1, m_nNum)
		COLUMN_ENTRY(2, m_vPos.x)
		COLUMN_ENTRY(3, m_vPos.y)
		COLUMN_ENTRY(4, m_vPos.z)
		COLUMN_ENTRY(5, m_vTex.x)
		COLUMN_ENTRY(6, m_vTex.y)
	END_COLUMN_MAP()
};

class CIndicesAccessor
{
public:
	LONG m_nFace;
	LONG m_nVertex;

	DEFINE_COMMAND_EX(CIndicesAccessor, L"SELECT Face, Vertex FROM Indices ORDER BY Face, Num");

	BEGIN_COLUMN_MAP(CIndicesAccessor)
		COLUMN_ENTRY(1, m_nFace)
		COLUMN_ENTRY(2, m_nVertex)
	END_COLUMN_MAP()
};

typedef CAtlMap<LONG, size_t> CIndexMap;
typedef CAtlArray<size_t> CIndexArray;

struct CNode
{
	UINT m_nVertex;
	int m_nLevel;
};

typedef CAtlArray<CNode> CNodeArray;

struct CSegment
{
	size_t m_nNode1, m_nNode2;
};

typedef CAtlArray<CSegment> CSegmentArray;

struct CContour
{
	CString m_strName;
	size_t m_nParent;
	int m_nLevel;
	CSegmentArray m_segments;
};

typedef CAtlArray<CContour> CContourArray;

struct CVertex
{
	D3DXVECTOR3 m_vPos;
	D3DXVECTOR2 m_vTex;
};

typedef CAtlArray<CVertex> CVertexArray;

struct CFace
{
	CIndexArray m_indices;
};

typedef CAtlMap<LONG, CFace> CFaceMap;

CDataSource connection;
CSession session;

void SaveModel(LPCTSTR szPath)
{
	HRESULT hr;
	CCommand<CAccessor<CVerticesAccessor> > command1;
	CCommand<CAccessor<CNodesAccessor> > command2;
	CCommand<CAccessor<CContoursAccessor> > command3;
	CCommand<CAccessor<CSegmentsAccessor> > command4;
	CNodeArray nodes[2];
	CContourArray contours[2];
	CIndexMap vertexIndices;
	CIndexMap nodeIndices[2];
	CIndexMap contourIndices[2];

	hr = command1.Open(session);
	if (FAILED(hr))
		AtlThrow(hr);

	size_t i = 0;
	hr = command1.MoveFirst();
	while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
	{
		vertexIndices.SetAt(command1.m_nNum, i++);
		hr = command1.MoveNext();
	}

	if (FAILED(hr))
		AtlThrow(hr);

	command1.Close();
	command1.ReleaseCommand();

	hr = command2.Open(session);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = command2.MoveFirst();
	while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
	{
		size_t i = nodes[command2.m_nSide].Add();
		CNode& node = nodes[command2.m_nSide][i];

		if (!vertexIndices.Lookup(command2.m_nVertex, node.m_nVertex))
			AtlThrow(E_FAIL);

		node.m_nLevel = command2.m_nLevel;
		nodeIndices[command2.m_nSide].SetAt(command2.m_nNum, i);
		hr = command2.MoveNext();
	}

	if (FAILED(hr))
		AtlThrow(hr);

	command2.Close();
	command2.ReleaseCommand();

	hr = command3.Open(session);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = command3.MoveFirst();
	while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
	{
		size_t i = contours[command3.m_nSide].Add();
		CContour& contour = contours[command3.m_nSide][i];
		contour.m_strName = command3.m_szName;
		contour.m_nParent = command3.m_dwParentStatus == DBSTATUS_S_OK ? command3.m_nParent : -1;
		contour.m_nLevel = command3.m_nLevel;
		contourIndices[command3.m_nSide].SetAt(command3.m_nNum, i);
		hr = command3.MoveNext();
	}

	if (FAILED(hr))
		AtlThrow(hr);

	command3.Close();
	command3.ReleaseCommand();

	hr = command4.Open(session);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = command4.MoveFirst();
	while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
	{
		size_t nContour;
		if (!contourIndices[command4.m_nSide].Lookup(command4.m_nContour, nContour))
			AtlThrow(E_FAIL);
		CContour& contour = contours[command4.m_nSide][nContour];

		size_t i = contour.m_segments.Add();
		CSegment& segment = contour.m_segments[i];

		if (!nodeIndices[command4.m_nSide].Lookup(command4.m_nNode1, segment.m_nNode1))
			AtlThrow(E_FAIL);

		if (!nodeIndices[command4.m_nSide].Lookup(command4.m_nNode2, segment.m_nNode2))
			AtlThrow(E_FAIL);

		hr = command4.MoveNext();
	}

	if (FAILED(hr))
		AtlThrow(hr);

	command4.Close();
	command4.ReleaseCommand();

	TCHAR szFileName[MAX_PATH];
	PathCombine(szFileName, szPath, _T("model.bin"));

	CFile file;
	if (!file.Open(szFileName, CFile::modeCreate | CFile::modeWrite))
		AtlThrow(E_FAIL);

	CArchive ar(&file, CArchive::store);

	for (int i = 0; i < 2; i++)
	{
		ar.WriteCount((UINT)nodes[i].GetCount());

		for (size_t j = 0; j < nodes[i].GetCount(); j++)
		{
			const CNode& node = nodes[i][j];
			ar.WriteCount(node.m_nVertex);
			ar.WriteCount(node.m_nLevel);
		}

		ar.WriteCount((UINT)contours[i].GetCount());

		for (size_t j = 0; j < contours[i].GetCount(); j++)
		{
			const CContour& contour = contours[i][j];
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

	ar.Close();
	file.Close();
}

void SaveMesh(LPCTSTR szPath)
{
	HRESULT hr;
	CCommand<CAccessor<CVerticesAccessor> > command1;
	CCommand<CAccessor<CIndicesAccessor> > command2;
	CVertexArray vertices;
	CFaceMap faces;
	CIndexMap vertexIndices;

	hr = command1.Open(session);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = command1.MoveFirst();
	while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
	{
		size_t i = vertices.Add();
		CVertex& vertex = vertices[i];
		vertex.m_vPos = command1.m_vPos;
		vertex.m_vTex = command1.m_vTex;
		vertexIndices.SetAt(command1.m_nNum, i);
		hr = command1.MoveNext();
	}

	if (FAILED(hr))
		AtlThrow(hr);

	command1.Close();
	command1.ReleaseCommand();

	hr = command2.Open(session);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = command2.MoveFirst();
	while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
	{
		CFace& face = faces[command2.m_nFace];

		size_t j = face.m_indices.Add();
		if (!vertexIndices.Lookup(command2.m_nVertex, face.m_indices[j]))
			AtlThrow(E_FAIL);

		hr = command2.MoveNext();
	}

	if (FAILED(hr))
		AtlThrow(hr);

	command2.Close();
	command2.ReleaseCommand();

	ostringstream oss;

	oss.precision(6);
	oss << fixed;
	oss << "xof 0303txt 0032" << endl;
	oss << "Mesh {" << endl;
	oss << vertices.GetCount() << ';' << endl;

	for (size_t i = 0; i < vertices.GetCount(); i++)
	{
		const CVertex& vertex = vertices[i];
		oss << vertex.m_vPos.x << ';' << vertex.m_vPos.y << ';' << vertex.m_vPos.z << ';' <<
			(i < vertices.GetCount() - 1 ? ',' : ';') << endl;
	}

	oss << faces.GetCount() << ';' << endl;

	size_t i = 0;
	POSITION pos = faces.GetStartPosition();
	while (pos != NULL)
	{
		const CFace& face = faces.GetNextValue(pos);

		oss << face.m_indices.GetCount() << ';';
		for (size_t j = 0; j < face.m_indices.GetCount(); j++)
		{
			oss << face.m_indices[j] << (j < face.m_indices.GetCount() - 1 ? ',' : ';');
		}
		oss << (i++ < faces.GetCount() - 1 ? ',' : ';') << endl;
	}

	oss << "MeshTextureCoords {" << endl;
	oss << vertices.GetCount() << ';' << endl;

	for (size_t i = 0; i < vertices.GetCount(); i++)
	{
		const CVertex& vertex = vertices[i];
		oss << vertex.m_vTex.x << ';' << vertex.m_vTex.y << ';' <<
			(i < vertices.GetCount() - 1 ? ',' : ';') << endl;
	}

	oss << "}" << endl << "}";

	CComPtr<IDirect3D9> spD3D;
	spD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if (!spD3D)
		AtlThrow(E_FAIL);

	CWnd wnd;
	if (!wnd.CreateEx(0, AfxRegisterWndClass(NULL), NULL, 0, CRect(0, 0, 100, 100), NULL, 0))
		AtlThrow(E_FAIL);

	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(D3DPRESENT_PARAMETERS));
	pp.AutoDepthStencilFormat = D3DFMT_D16;
	pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	pp.BackBufferCount = 1;
	pp.EnableAutoDepthStencil = TRUE;
	pp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	pp.MultiSampleQuality = 0;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.Windowed = TRUE;
	pp.hDeviceWindow = wnd.m_hWnd;

	CComPtr<IDirect3DDevice9> spDevice;
	hr = spD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd.m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp,
		&spDevice);
	if (FAILED(hr))
		AtlThrow(hr);

	TCHAR szFileName[MAX_PATH];
	PathCombine(szFileName, szPath, _T("mesh.bin"));

	DWORD dwNumMaterials;
	CComPtr<ID3DXMesh> spMesh;

	hr = D3DXLoadMeshFromXInMemory(oss.str().c_str(), oss.str().size(), D3DXMESH_SYSTEMMEM, spDevice, NULL, NULL, NULL,
		&dwNumMaterials, &spMesh);
	if (FAILED(hr))
		AtlThrow(hr);

	CAutoPtr<DWORD> spAdjacency;
	ATLTRY(spAdjacency.Attach(new DWORD[spMesh->GetNumFaces() * 3]));
	if (!spAdjacency)
		AtlThrow(E_OUTOFMEMORY);

	hr = spMesh->GenerateAdjacency(0.01f, spAdjacency);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = spMesh->OptimizeInplace(D3DXMESHOPT_STRIPREORDER | D3DXMESHOPT_IGNOREVERTS, spAdjacency, NULL, NULL, NULL);
	if (FAILED(hr))
		AtlThrow(hr);

	hr = D3DXSaveMeshToX(szFileName, spMesh, NULL, NULL, NULL, 0, D3DXF_FILEFORMAT_BINARY | D3DXF_FILEFORMAT_COMPRESSED);
	if (FAILED(hr))
		AtlThrow(hr);
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		return 1;
	}

	CString strPath;
	if (argc < 2)
		strPath = _T(".");
	else
		strPath = argv[1];

	TRY
	{
		HRESULT hr;

		hr = CoInitialize(NULL);
		if (FAILED(hr))
			AtlThrow(hr);

		TCHAR szFileName[MAX_PATH];
		PathCombine(szFileName, strPath, _T("model.mdb"));

		CStringW strInit;
		strInit.Format(L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s", szFileName);

		hr = connection.OpenFromInitializationString(strInit);
		if (FAILED(hr))
			AtlThrow(hr);

		hr = session.Open(connection);
		if (FAILED(hr))
			AtlThrow(hr);

		cout << "Generating model..." << endl;
		SaveModel(strPath);
		cout << "Generating mesh..." << endl;
		SaveMesh(strPath);

		session.Close();
		connection.Close();

		CoUninitialize();
	}
	CATCH_ALL(e)
	{
		cout << "Fatal Error";

		CComPtr<IErrorInfo> spErrorInfo;
		HRESULT hr = GetErrorInfo(0, &spErrorInfo);
		if (hr == S_OK)
		{
			CComBSTR bstrDesc;
			hr = spErrorInfo->GetDescription(&bstrDesc);
			if (SUCCEEDED(hr))
				cout << ": " << CT2A(COLE2T((BSTR)bstrDesc));
		}

		cout << endl;

		return 1;
	}
	END_CATCH_ALL;

	return 0;
}
