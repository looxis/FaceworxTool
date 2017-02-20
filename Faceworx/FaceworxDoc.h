// FaceworxDoc.h : interface of the CFaceworxDoc class
//

#pragma once

#include "Types.h"
#include "LoopSubdivider.h"
#include "Settings.h"

struct VERTEX
{
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 vNorm;
	DWORD color;
	D3DXVECTOR2 vTex[3];
};

const DWORD VERTEX_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3 | D3DFVF_TEXCOORDSIZE2(0) |
	D3DFVF_TEXCOORDSIZE2(1) | D3DFVF_TEXCOORDSIZE2(2) | D3DFVF_DIFFUSE;

class CFaceworxDoc : public CDocument
{
protected: // create from serialization only
	CFaceworxDoc();
	DECLARE_DYNCREATE(CFaceworxDoc)

	// Attributes
public:

	// Operations
public:

	// Overrides
public:
	virtual void DeleteContents();
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);

	// Implementation
public:
	virtual ~CFaceworxDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	struct CVertexData
	{
		D3DXVECTOR3 vPos;
		D3DXVECTOR2 vTex[2];
	};

	typedef CAtlArray<CPointF> CNodePosArray;
	typedef CAtlArray<CVertexData> CVertexDataArray;

	struct CBlendZone
	{
		CBlendZone();
		CBlendZone(const CBlendZone& state);

		CBlendZone& operator=(const CBlendZone& blendZone);

		CPointFArray m_nodes;
		float m_fWidth;
	};

	struct CState
	{
		CState();
		CState(const CState& state);

		CState& operator=(const CState& state);

		CRectF m_bounds[2];
		CNodePosArray m_nodes[2];
		CVertexDataArray m_vertices;
		CBlendZone m_blendZone;
	};

	struct CQuad
	{
		WORD indices[4];
		WORD neighbours[4];
	};

	typedef CAtlList<CState> CStateList;

	CImage m_image[2];
	CNodeArray m_nodes[2];
	CContourArray m_contours[2];
	CPointFArray m_nodePos[2];
	CPointF m_offset[2];
	CRectF m_bounds[2];
	CBlendZone m_blendZone;
	CStateList m_states;
	CState m_state;
	POSITION m_posState;
	size_t m_nContour;
	bool m_bBox;
	bool m_bFeatures;
	int m_nLevel;
	bool m_bBlend;
	bool m_bWireframe;
	bool m_bLighting;
	bool m_bOrtho;
	int m_nDetail;
	bool m_bLeft;
	BOOL sharpens[3];

	CComPtr<IDirect3D9> m_spD3D;
	CComPtr<IDirect3DDevice9> m_spDevice;
	CComPtr<ID3DXMesh> m_spMesh, m_spEnhancedMesh;
	CAutoPtr<DWORD> m_spAdjacency;
	CComPtr<IDirect3DTexture9> m_spTexture[2];
	DWORD m_dwNumMaterials;
	D3DPRESENT_PARAMETERS m_pp;
	D3DXVECTOR3 m_vObjectCenter;
	float m_fObjectRadius;
	bool m_bUseHWNPatches;
	CAtlArray<CQuad> m_quads;
	CLoopSubdivider m_LoopSubdivider1, m_LoopSubdivider2;
	bool m_bLoopSubdivider1Inited, m_bLoopSubdivider2Inited;

	HRESULT TesselateQuads();
	HRESULT LoopSubdivide(ID3DXMesh**);

	void SerializeModel(CArchive& ar);
	void SerializeMesh(CArchive& ar);
	void CreateDevice();
	void LoadMesh();
	void SetNodeVertices(VERTEX* pVertices, DWORD dwNumVertices);
	void ComputeBox(int nSide);
	void SetTextureCoords(VERTEX* pVertices, DWORD dwNumVertices);
	void SplitMouth();

	float GetNumSegs() const;
	void CreateEnhancedMesh();
	void FillTextureAlpha();

	void SaveState(CState& state);
	void RestoreState(const CState& state);
	void BeginState();
	void EndState();
	void CancelState();

	bool GetCurve(int nSide, size_t nNode, size_t nContour, size_t nSegment, CIndexList& curve) const;
	void GetCurves(int nSide, size_t nNode, CAtlList<CIndexList>& curves) const;
	void GetNodeWeights(int nSide, size_t nNode, const CAtlList<CIndexList>& curves, CNodeWeightMap& weights) const;
	void GetNodeWeightsByNode(int nSide, size_t nNode, CNodeWeightMap& weights) const;
	void GetNodeWeightsByContour(int nSide, size_t nContour, CNodeWeightMap& weights) const;
	void GetVertexWeights(int nSide, const CNodeWeightMap& nodeWeights, CVertexWeightMap& weights) const;
	void SetBounds(int nSide, CRectF bounds);
	void MoveNodes(int nSide, CSizeF offset, const CNodeWeightMap& nodeWeights, const CVertexWeightMap& vertexWeights);
	void LoadImage(int nSide);

	CSize GetImageSize(int nSize) const;
	void AdjustBounds(int nSide);

	void MakePointsList();
	class C3DView* Get3DView();

	friend class CImageView;
	friend class CFrontView;
	friend class CSideView;
	friend class C3DView;
	friend class obj_exporter;
	friend CArchive& operator>>(CArchive& ar, CBlendZone& blend_zone);
	friend CArchive& operator<<(CArchive& ar, const CBlendZone& blend_zone);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnFileExport();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnViewBox();
	afx_msg void OnUpdateViewBox(CCmdUI *pCmdUI);
	afx_msg void OnViewFeatures();
	afx_msg void OnUpdateViewFeatures(CCmdUI *pCmdUI);
	afx_msg void OnViewPointsAll();
	afx_msg void OnUpdateViewPointsAll(CCmdUI *pCmdUI);
	afx_msg void OnViewPointsMore();
	afx_msg void OnUpdateViewPointsMore(CCmdUI *pCmdUI);
	afx_msg void OnViewPointsLess();
	afx_msg void OnUpdateViewPointsLess(CCmdUI *pCmdUI);
	afx_msg void OnViewPointsNo();
	afx_msg void OnUpdateViewPointsNo(CCmdUI *pCmdUI);
	afx_msg void OnViewBlendZone();
	afx_msg void OnUpdateViewBlendZone(CCmdUI *pCmdUI);
	afx_msg void OnViewWireframe();
	afx_msg void OnUpdateViewWireframe(CCmdUI *pCmdUI);
	afx_msg void OnViewSolid();
	afx_msg void OnUpdateViewSolid(CCmdUI *pCmdUI);
	afx_msg void OnViewLighting();
	afx_msg void OnUpdateViewLighting(CCmdUI *pCmdUI);
	afx_msg void OnViewOrthogonal();
	afx_msg void OnUpdateViewOrthogonal(CCmdUI *pCmdUI);
	afx_msg void OnViewPerspective();
	afx_msg void OnUpdateViewPerspective(CCmdUI *pCmdUI);
	afx_msg void OnViewModelDetailHigh();
	afx_msg void OnUpdateViewModelDetailHigh(CCmdUI *pCmdUI);
	afx_msg void OnViewModelDetailMedium();
	afx_msg void OnUpdateViewModelDetailMedium(CCmdUI *pCmdUI);
	afx_msg void OnViewModelDetailLow();
	afx_msg void OnUpdateViewModelDetailLow(CCmdUI *pCmdUI);
	afx_msg void OnModelLoadFront();
	afx_msg void OnModelLoadSide();
	afx_msg void OnModelLeftSide();
	afx_msg void OnUpdateModelLeftSide(CCmdUI *pCmdUI);
public:
	afx_msg void OnModeldetailSubdivisionsettings();
public:
	afx_msg void OnEditEditin3dwindow();
public:
	afx_msg void OnUpdateEditEditin3dwindow(CCmdUI *pCmdUI);
public:
	afx_msg void OnEditFixdirection();
public:
	afx_msg void OnEditResetmodifications();
public:
	afx_msg void OnEditSettings();
};

// CFaceworxDoc::CBlendZone

inline CFaceworxDoc::CBlendZone::CBlendZone()
{
}

inline CFaceworxDoc::CBlendZone::CBlendZone(const CBlendZone& blendZone)
{
	*this = blendZone;
}

inline CFaceworxDoc::CBlendZone& CFaceworxDoc::CBlendZone::operator=(const CBlendZone& blendZone)
{
	if (!m_nodes.SetCount(blendZone.m_nodes.GetCount()))
		AtlThrow(E_OUTOFMEMORY);

	for (size_t i = 0; i < blendZone.m_nodes.GetCount(); i++)
	{
		m_nodes[i] = blendZone.m_nodes[i];
	}

	m_fWidth = blendZone.m_fWidth;

	return *this;
}

// CFaceworxDoc::CState

inline CFaceworxDoc::CState::CState()
{
}

inline CFaceworxDoc::CState::CState(const CState& state)
{
	*this = state;
}

inline CFaceworxDoc::CState& CFaceworxDoc::CState::operator=(const CState& state)
{
	for (int i = 0; i < 2; i++)
	{
		m_bounds[i] = state.m_bounds[i];

		if (!m_nodes[i].SetCount(state.m_nodes[i].GetCount()))
			AtlThrow(E_OUTOFMEMORY);

		for (size_t j = 0; j < state.m_nodes[i].GetCount(); j++)
		{
			m_nodes[i][j] = state.m_nodes[i][j];
		}
	}

	if (!m_vertices.SetCount(state.m_vertices.GetCount()))
		AtlThrow(E_OUTOFMEMORY);

	for (size_t i = 0; i < state.m_vertices.GetCount(); i++)
	{
		m_vertices[i] = state.m_vertices[i];
	}

	m_blendZone = state.m_blendZone;

	return *this;
}
