// 3DView.h : interface of the C3DView class
//

#pragma once

class C3DView : public CView
{
protected: // create from serialization only
	C3DView();
	DECLARE_DYNCREATE(C3DView)

	// Attributes
public:
	CFaceworxDoc* GetDocument() const;

	// Operations
public:

	// Overrides
public:
	virtual void OnDraw(CDC* pDC); // overridden to draw this view

	// Implementation
public:
	virtual ~C3DView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void SetEditMode(bool);
	void FixDirection();
	void ResetModifications();
	bool m_b3DEditing;
	float m_R1, m_R2, m_Shift;
protected:
	bool m_bDrag;
	float m_fRadius;
	float m_fRadiusTranslation;
	float m_fZoom;
	D3DXQUATERNION m_qDown;
	D3DXQUATERNION m_qNow;
	D3DXMATRIXA16 m_matRotation;
	D3DXMATRIXA16 m_matRotationDelta;
	D3DXMATRIXA16 m_matTranslation;
	D3DXMATRIXA16 m_matTranslationDelta;
	CPoint m_ptSaved;
	D3DXVECTOR3 m_vDown;

	void Render();
	D3DXVECTOR3 ScreenToVector(CPoint point);
	HRESULT RestoreDevice();
	bool m_bDeviceLost;
	struct INTERSECTION
	{
		unsigned int uFace;
		unsigned int uVert;
		unsigned int uVertices[3];
		float fDistance;
		VERTEX vIntersection;
		D3DXVECTOR3 vLook;
	};
	bool GetIntersection(const POINT& pt, INTERSECTION&);
	void GetDirection(const POINT& point, D3DXVECTOR3& eye, D3DXVECTOR3& dir);
	void ModifyAt(const POINT& point, bool outside);

	INTERSECTION ptStart;
	bool bMarkPlaced;
	CAtlArray<float> weights;
	D3DXVECTOR3 vDirection;
	CAtlArray<D3DXVECTOR3> modifications;

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

public:
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG // debug version in FaceworxView.cpp
inline CFaceworxDoc* C3DView::GetDocument() const
{ return reinterpret_cast<CFaceworxDoc*>(m_pDocument); }
#endif
