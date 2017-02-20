// 3DView.cpp : implementation of the C3DView class
//

#include "stdafx.h"
#include "Faceworx.h"
#include "FaceworxDoc.h"
#include "3DView.h"
#include "DXException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// C3DView

IMPLEMENT_DYNCREATE(C3DView, CView)

BEGIN_MESSAGE_MAP(C3DView, CView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// C3DView construction/destruction

C3DView::C3DView()
{
	vDirection.x = FLT_MAX;
	bMarkPlaced = false;
	m_b3DEditing = false;
	m_bDeviceLost = false;
	m_fZoom = 1.0f;
	m_bDrag = false;
	m_fRadius = 0.85f;
	m_fRadiusTranslation = 500;
	D3DXQuaternionIdentity(&m_qDown);
	D3DXQuaternionIdentity(&m_qNow);
	D3DXMatrixIdentity(&m_matRotation);
	D3DXMatrixIdentity(&m_matRotationDelta);
	D3DXMatrixIdentity(&m_matTranslation);
	D3DXMatrixIdentity(&m_matTranslationDelta);
	m_R1 = 60, m_R2 = 30;
	m_Shift = 2.0;
}

C3DView::~C3DView()
{
}

// C3DView drawing

void C3DView::OnDraw(CDC* /*pDC*/)
{
	Render();
}

// C3DView diagnostics

#ifdef _DEBUG
void C3DView::AssertValid() const
{
	CView::AssertValid();
}

void C3DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFaceworxDoc* C3DView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceworxDoc)));
	return (CFaceworxDoc*)m_pDocument;
}
#endif //_DEBUG

HRESULT C3DView::RestoreDevice()
{
	HRESULT hr;
	CFaceworxDoc* pDoc = GetDocument();
	LPDIRECT3DDEVICE9 pDevice = pDoc->m_spDevice;
	if( FAILED( hr = pDevice->TestCooperativeLevel() ) )
    {
		if( D3DERR_DEVICELOST == hr )
			return D3DERR_DEVICELOST;

		hr = pDevice->Reset( &pDoc->m_pp );
		if( FAILED(hr) )  
		{
			if( hr == D3DERR_DEVICELOST )
				return D3DERR_DEVICELOST; // Reset could legitimately fail if the device is lost
			else
				AfxThrowDXException(hr);
		}
    }
    m_bDeviceLost = false;
	return D3D_OK;
}

void C3DView::Render()
{
	CFaceworxDoc* pDoc = GetDocument();
	LPDIRECT3DDEVICE9 pDevice = pDoc->m_spDevice;

	if (pDevice == NULL)
		return;

	if (m_bDeviceLost)
	{
		RestoreDevice();
	}

	pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pDevice->SetRenderState( D3DRS_AMBIENT, 0x00202020 );

	D3DMATERIAL9 mtrl;
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    pDevice->SetMaterial( &mtrl );

	if (!pDoc->m_bWireframe)
	{
		D3DLIGHT9 light;
		memset(&light, 0, sizeof light);
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r = 1;
		light.Diffuse.g = 1;
		light.Diffuse.b = 1;
		light.Direction.x = 0;
		light.Direction.y = 0;
		light.Direction.z = 1;
		pDevice->SetLight(0, &light);
		pDevice->LightEnable(0, TRUE);
		pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);//D3DCULL_CCW);
		pDevice->SetRenderState(D3DRS_LIGHTING, pDoc->m_bLighting);

		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)-1);
		pDevice->SetTexture(0, pDoc->m_spTexture[0]);
		pDevice->SetTexture(1, pDoc->m_spTexture[1]);
		pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
		pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

		pDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE );
		pDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	} else
	{
		D3DCAPS9 caps;
		pDevice->GetDeviceCaps(&caps);
		pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		if (caps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP)
		{
			pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEMP);
			pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		} else
		if (caps.PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT)
		{
			pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
			pDevice->SetTextureStageState(0, D3DTSS_CONSTANT, 0x00000000);
			pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		} else
		{
			pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}

	D3DXMATRIXA16 matWorld;
	D3DXMatrixTranslation(&matWorld, -pDoc->m_vObjectCenter.x,
		-pDoc->m_vObjectCenter.y, -pDoc->m_vObjectCenter.z);
	D3DXMatrixMultiply(&matWorld, &matWorld, &m_matRotation);
	D3DXMatrixMultiply(&matWorld, &matWorld, &m_matTranslation);
	pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	D3DXVECTOR3 vFrom(0, 0, -2 * pDoc->m_fObjectRadius);
	D3DXVECTOR3 vAt(0, 0, 0);
	D3DXVECTOR3 vUp(0, 1, 0);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vFrom, &vAt, &vUp);
	pDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIXA16 matProj;
	CRect rect;
	GetClientRect(&rect);
	float fAspect = (float)rect.Width() / (float)rect.Height();
	if (pDoc->m_bOrtho)
	{
		float f1 = pDoc->m_fObjectRadius * m_fZoom, f2 = f1 * fAspect;
		D3DXMatrixOrthoOffCenterLH(&matProj, -f2, f2, -f1, f1, -100000, 100000);
	} else
	{
		D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4 * m_fZoom, fAspect,
			pDoc->m_fObjectRadius / 64, pDoc->m_fObjectRadius * 200);
	}
	pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(255, 255, 255), 1, 0);

	HRESULT hr;
	if (SUCCEEDED(hr = pDevice->BeginScene()))
	{
        if (pDoc->m_bUseHWNPatches)
			pDoc->m_spDevice->SetNPatchMode(pDoc->GetNumSegs());

/*		if (pDoc->m_bWireframe)
		{
			VERTEX* vs=0;
			WORD* is;
			pDoc->m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);
			pDoc->m_spMesh->LockIndexBuffer(0,(LPVOID*)&is);
			pDoc->m_spDevice->SetFVF(VERTEX_FVF);
			for (size_t n=0;n<pDoc->m_quads.GetCount();n++)
			//for (size_t n=0;n<pDoc->m_spMesh->GetNumFaces()*3;n+=6)
			{
				WORD idx[5]={pDoc->m_quads[n].indices[0],
					pDoc->m_quads[n].indices[1],
					pDoc->m_quads[n].indices[2],
					pDoc->m_quads[n].indices[3],
					pDoc->m_quads[n].indices[0]};
				//WORD idx[5]={is[n],is[n+1],is[n+2],is[n+5],is[n]};
				pDoc->m_spDevice->DrawIndexedPrimitiveUP(
					D3DPT_LINESTRIP,
					//D3DPT_TRIANGLELIST,
					0,
					pDoc->m_spMesh->GetNumVertices(),
					4,
					idx,
					D3DFMT_INDEX16,
					vs,
					sizeof *vs);
			}
			pDoc->m_spMesh->UnlockVertexBuffer();
			pDoc->m_spMesh->UnlockIndexBuffer();
		} else*/
		for (UINT i = 0; i < pDoc->m_dwNumMaterials; i++)
			pDoc->m_spEnhancedMesh->DrawSubset(i);

        if (pDoc->m_bUseHWNPatches)
            pDoc->m_spDevice->SetNPatchMode(0);

		pDevice->EndScene();
	} else
	{
		CString q=DXGetErrorString9(hr);
		TRACE(L"BeginScene fails: %d %s\n", hr, (LPCTSTR)q);
	}

	hr = pDevice->Present(NULL, NULL, m_hWnd, NULL);
	if (FAILED(hr))
	if (D3DERR_DEVICELOST == hr)
	{
		TRACE(L"D3DERR_DEVICELOST\n");
		m_bDeviceLost = true;
	} else
	{
		TRACE(L"pDevice->Present unknown error: %d\n", hr);
	}

}

D3DXVECTOR3 C3DView::ScreenToVector(CPoint point)
{
	CRect rect;
	GetClientRect(&rect);

	float x = -(point.x - rect.Width() / 2) / (m_fRadius * rect.Width() / 2);
	float y = (point.y - rect.Height() / 2) / (m_fRadius * rect.Height() / 2);

	float z = 0;
	float mag = x * x + y * y;

	if (mag > 1)
	{
		float scale = 1 / sqrtf(mag);
		x *= scale;
		y *= scale;
	}
	else
		z = sqrtf(1 - mag);

	return D3DXVECTOR3(x, y, z);
}

// C3DView message handlers

void C3DView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

BOOL C3DView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void C3DView::ModifyAt(const POINT& point, bool outside)
{
	if (vDirection.x == FLT_MAX)
		FixDirection();
	INTERSECTION intersection;
	GetIntersection(point, intersection);
	ptStart = intersection;
	D3DXVec3Normalize(&ptStart.vLook, &ptStart.vLook);

	CFaceworxDoc* pDoc = GetDocument();
	VERTEX* vs;
	pDoc->m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);
	int nc = pDoc->m_spMesh->GetNumVertices();
	//weights.SetCount(nc);
	if (0 == modifications.GetCount())
	{
		modifications.SetCount(nc);
		memset(modifications.GetData(), 0,
			modifications.GetCount() * sizeof modifications[0]);
	}

	for (int n = 0; n < nc; n++)
	{
		VERTEX& v = vs[n];
		D3DXVECTOR3 d = v.vPos - ptStart.vIntersection.vPos;
		float r = D3DXVec3Length(&d);
		float weight;
		if (r > m_R1)
			weight = 0; else
		if (r < m_R2)
			weight = 1; else
			weight = (m_R1 - r) / (m_R1 - m_R2);
		if (weight)
		{
			D3DXVECTOR3 mod = ((outside ? m_Shift : -m_Shift) * weight) * vDirection;
			v.vPos += mod;
			modifications[n] += mod;
		}
	}

	pDoc->m_spMesh->UnlockVertexBuffer();

	pDoc->CreateEnhancedMesh();
	Invalidate();
}

void C3DView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_b3DEditing && (nFlags & MK_CONTROL))
	{
		ModifyAt(point, true);
		return;
	}
#ifdef _DEBUG
	if (MK_CONTROL & nFlags)
	{
		INTERSECTION intersection;
		GetIntersection(point, intersection);

		CString s;
		s.Format(L"%d %d, [%d %d %d] - %d",
			intersection.uFace,
			intersection.uVert,
			intersection.uVertices[0],
			intersection.uVertices[1],
			intersection.uVertices[2],
			intersection.uVertices[intersection.uVert]);
		AfxGetMainWnd()->SetWindowText(s);
	}
#endif
	if (GetCapture() == NULL)
	{
		m_bDrag = true;
		m_vDown = ScreenToVector(point);
		m_qDown = m_qNow;
		SetCapture();
		return;
	}

	CView::OnLButtonDown(nFlags, point);
}

void C3DView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDrag)
	{
		ReleaseCapture();
		m_bDrag = false;
		return;
	}

	CView::OnLButtonUp(nFlags, point);
}

void C3DView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (GetCapture() == NULL)
	{
		m_bDrag = true;
		m_ptSaved = point;
		SetCapture();
		return;
	}

	CView::OnMButtonDown(nFlags, point);
}

void C3DView::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDrag)
	{
		ReleaseCapture();
		m_bDrag = false;
		return;
	}

	CView::OnMButtonUp(nFlags, point);
}

void C3DView::OnMouseMove(UINT nFlags, CPoint point)
{
/*	if (m_b3DEditing)
	{
		INTERSECTION intersection;
		GetIntersection(point, intersection);
		D3DXVec3Normalize(&ptStart.vLook, &ptStart.vLook);

		CFaceworxDoc* pDoc = GetDocument();
		VERTEX* vs;
		pDoc->m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);
		const float r1 = 100, r2 = 50;
		int nc = pDoc->m_spMesh->GetNumVertices();

		for (int n = 0; n < nc; n++)
		{
			VERTEX& v = vs[n];
			D3DXVECTOR3 d = v.vPos - intersection.vIntersection.vPos;
			float r = D3DXVec3Length(&d);
			float weight;
			if (r > r1)
				weight = 0; else
			if (r < r2)
				weight = 1; else
				weight = (r1 - r) / (r1 - r2);
			int w = int(255 * weight);
			v.color = 0xFFFF00 | w;
		}

		pDoc->m_spMesh->UnlockVertexBuffer();

		pDoc->CreateEnhancedMesh();
		Invalidate();

		return;
	}
	if (m_b3DEditing && (nFlags & MK_RBUTTON) && (nFlags & MK_CONTROL))
	{
		CSize diff = point - m_ptSaved;
		m_ptSaved = point;
		CFaceworxDoc* pDoc = GetDocument();
		VERTEX* vs;
		pDoc->m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);
		for (int n = 0, nc = pDoc->m_spMesh->GetNumVertices(); n < nc; n++)
		{
			VERTEX& v = vs[n];
			float weight = weights[n];
			if (weight)
			{
				D3DXVECTOR3 mod = (diff.cy * weight) * ptStart.vLook;
				v.vPos += mod;
				modifications[n] += mod;
			}
		}
		pDoc->m_spMesh->UnlockVertexBuffer();

		pDoc->CreateEnhancedMesh();
		Invalidate();
		return;
	}*/
	if (m_bDrag)
	{
		if (nFlags & MK_LBUTTON)
		{
			D3DXVECTOR3 vCur = ScreenToVector(point);
			D3DXVECTOR3 vAxis;
			D3DXVec3Cross(&vAxis, &m_vDown, &vCur);
			D3DXQUATERNION qAxisToAxis(vAxis.x, vAxis.y, vAxis.z, D3DXVec3Dot(&m_vDown, &vCur));
			m_qNow = m_qDown * qAxisToAxis;
			D3DXMatrixRotationQuaternion(&m_matRotationDelta, &qAxisToAxis);
			D3DXMatrixRotationQuaternion(&m_matRotation, &m_qNow);
		}
		else
		{
			CRect rect;
			GetClientRect(&rect);
			float fDeltaX = m_fZoom * (m_ptSaved.x - point.x) * m_fRadiusTranslation / rect.Width();
			float fDeltaY = m_fZoom * (m_ptSaved.y - point.y) * m_fRadiusTranslation / rect.Height();

			bool bMoving = 0 != (nFlags & MK_MBUTTON);
			bMoving ^= CSettings::Get().bRightButtonMoving;

			if (bMoving)
			{
				D3DXMatrixTranslation(&m_matTranslationDelta, -2 * fDeltaX, 2 * fDeltaY, 0.0f);
				D3DXMatrixMultiply(&m_matTranslation, &m_matTranslation, &m_matTranslationDelta);
			}
			else
			{
				float fPrev = m_fZoom;
				m_fZoom *= exp(fDeltaY/500.0f);
				if (m_fZoom > 3.9)
					m_fZoom = fPrev;
				Invalidate(FALSE);
			}

			m_ptSaved = point;
		}

		Invalidate(FALSE);
		return;
	}

	CView::OnMouseMove(nFlags, point);
}

void C3DView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_b3DEditing && (nFlags & MK_CONTROL))
	{
		ModifyAt(point, false);
		return;
	}
	if (GetCapture() == NULL)
	{
		m_bDrag = true;
		m_ptSaved = point;
		SetCapture();
		return;
	}

	CView::OnRButtonDown(nFlags, point);
}

void C3DView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDrag)
	{
		ReleaseCapture();
		m_bDrag = false;
		return;
	}

	CView::OnRButtonUp(nFlags, point);
}

BOOL C3DView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	float fPrev = m_fZoom;
	if (zDelta < 0)
		m_fZoom *= 1.1f;
	else
		m_fZoom /= 1.1f;
	if (m_fZoom > 3.9)
		m_fZoom = fPrev;
	Invalidate(FALSE);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void C3DView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (cx == 0 || cy == 0)
		return;

	CFaceworxDoc* pDoc = GetDocument();

	LPDIRECT3DDEVICE9 pDevice = pDoc->m_spDevice;
	if (pDoc->m_spDevice == NULL)
		return;
	if (0 == cx || 0 == cy)
		return;
	pDoc->m_pp.BackBufferWidth = 0;
	pDoc->m_pp.BackBufferHeight = 0;
	pDoc->m_pp.hDeviceWindow = m_hWnd;
	HRESULT hr = pDevice->Reset(&pDoc->m_pp);
	if (FAILED(hr))
		AfxThrowDXException(hr);
	pDoc->CreateEnhancedMesh();
}

void C3DView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	static_cast<CFaceworxApp*>(AfxGetApp())->Update3DView();
}

int C3DView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CString str;
	str.LoadString(IDS_3D_VIEW);
	SetWindowText(str);

	return 0;
}

void C3DView::GetDirection(const POINT& point, D3DXVECTOR3& eye, D3DXVECTOR3& dir)
{
	CFaceworxDoc* pDoc = GetDocument();
	LPDIRECT3DDEVICE9 pDevice = pDoc->m_spDevice;

	D3DVIEWPORT9 m_ViewPort;
	D3DMATRIX view,proj,world;
	pDevice->GetViewport(&m_ViewPort);
	pDevice->GetTransform(D3DTS_VIEW,&view);
	pDevice->GetTransform(D3DTS_PROJECTION,&proj);
	pDevice->GetTransform(D3DTS_WORLD,&world);
	D3DXMATRIX* pmProj=(D3DXMATRIX*)&proj;
	D3DXMATRIX* pmView=(D3DXMATRIX*)&view;
	D3DXMATRIX* pmWorld=(D3DXMATRIX*)&world;

	D3DXVECTOR3 vRayDirNDC((float)point.x,(float)point.y,0.0f);
	D3DXVECTOR3 vRayDirB((float)point.x,(float)point.y,1.0f);
	D3DXVECTOR3 vNPt;
	D3DXVECTOR3 vNPtB;

	D3DXVec3Unproject(&eye,&vRayDirNDC,&m_ViewPort,pmProj,pmView,pmWorld);
	D3DXVec3Unproject(&dir,&vRayDirB,&m_ViewPort,pmProj,pmView,pmWorld);

	dir -= eye;
}

bool C3DView::GetIntersection(const POINT& point, INTERSECTION& intersection)
{
	CFaceworxDoc* pDoc = GetDocument();
	LPDIRECT3DDEVICE9 pDevice = pDoc->m_spDevice;

	D3DXVECTOR3 vNPt, vNPtB;
	GetDirection(point, vNPt, vNPtB);

	const D3DXVECTOR3 *pEye = &vNPt;
	const D3DXVECTOR3 *pRayDir = &vNPtB;
    
	BOOL bHit;
	DWORD uFaceIndex=0;
	float fU,fV,fG;
	float fDist;
	unsigned int uVert=0;
	unsigned short *psVerts=NULL;
	DWORD *pwVerts=NULL;
	bool bUseShorts=false;
	HRESULT hr = D3DXIntersect(pDoc->m_spEnhancedMesh,
		pEye,pRayDir,&bHit,&uFaceIndex,&fV,&fG,&fDist,NULL,NULL);
	if (FAILED(hr))
		AfxThrowDXException(hr);
	if (!bHit) 
		return false;

	fU = 1.0f-fG-fV;
	uVert = (fU>fV)?(fU>fG)?0:2:(fV>fG)?1:2;

	WORD* is;
	pDoc->m_spEnhancedMesh->LockIndexBuffer(D3DLOCK_READONLY,(LPVOID*)&is);

	intersection.uFace = uFaceIndex;
	intersection.uVert = uVert;
	intersection.uVertices[0] = is[uFaceIndex*3];
	intersection.uVertices[1] = is[uFaceIndex*3 + 1];
	intersection.uVertices[2] = is[uFaceIndex*3 + 2];
	intersection.fDistance = fDist;

	pDoc->m_spEnhancedMesh->UnlockIndexBuffer();

	VERTEX* vs;
	pDoc->m_spEnhancedMesh->LockVertexBuffer(D3DLOCK_READONLY,(LPVOID*)&vs);

	int i1 = intersection.uVertices[0];
	int i2 = intersection.uVertices[1];
	int i3 = intersection.uVertices[2];

	D3DXVECTOR3 dv1 = vs[i2].vPos - vs[i1].vPos;
    D3DXVECTOR3 dv2 = vs[i3].vPos - vs[i1].vPos;
	D3DXVECTOR3 dn1 = vs[i2].vNorm - vs[i1].vNorm;
	D3DXVECTOR3 dn2 = vs[i3].vNorm - vs[i1].vNorm;
    intersection.vIntersection.vPos = vs[i1].vPos + fV * dv1 + fG * dv2;
	intersection.vIntersection.vNorm = vs[i1].vNorm + fV * dn1 + fG * dn2;
	intersection.vLook = vNPtB;

	pDoc->m_spEnhancedMesh->UnlockVertexBuffer();

	return true;
}

void C3DView::SetEditMode(bool b)
{
	m_b3DEditing = b;
/*	if (!b)
		bMarkPlaced = false;

	CFaceworxDoc* pDoc = GetDocument();
	VERTEX* vs;
	pDoc->m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);

	for (int n = 0, nc = pDoc->m_spMesh->GetNumVertices(); n < nc; n++)
	{
		VERTEX& v = vs[n];
		v.color = (b && !bMarkPlaced) ? 0xFFFF00 : 0xFFFFFF;
	}

	pDoc->m_spMesh->UnlockVertexBuffer();
	pDoc->CreateEnhancedMesh();
	Invalidate();*/
}

void C3DView::FixDirection()
{
	CRect rct;
	GetClientRect(rct);
	INTERSECTION intersection;
	D3DXVECTOR3 eye;
	GetDirection(rct.CenterPoint(), eye, vDirection);
	D3DXVec3Normalize(&vDirection, &vDirection);
}

void C3DView::ResetModifications()
{
	if (0 == modifications.GetCount())
		return;
	CFaceworxDoc* pDoc = GetDocument();
	VERTEX* vs;
	pDoc->m_spMesh->LockVertexBuffer(0,(LPVOID*)&vs);

	int nc = pDoc->m_spMesh->GetNumVertices();
	for (int n = 0; n < nc; n++)
	{
		VERTEX& v = vs[n];
		v.vPos -= modifications[n];
	}
	memset(modifications.GetData(), 0,
		modifications.GetCount() * sizeof modifications[0]);

	pDoc->m_spMesh->UnlockVertexBuffer();

	pDoc->CreateEnhancedMesh();
	Invalidate();
}