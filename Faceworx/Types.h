#pragma once

struct CSizeF;
struct CPointF;
struct CRectF;

// CSizeF

struct CSizeF
{
	CSizeF() throw();
	CSizeF(float cx, float cy) throw();

	void SetSize(float cx, float cy) throw();

	void operator+=(CSizeF size) throw();
	void operator-=(CSizeF size) throw();

	CSizeF operator+(CSizeF size) const throw();
	CSizeF operator-(CSizeF size) const throw();

	float cx, cy;
};

// CPointF

struct CPointF
{
	CPointF() throw();
	CPointF(float x, float y) throw();

	void SetPoint(float x, float y) throw();

	void operator+=(CSizeF size) throw();
	void operator-=(CSizeF size) throw();
	void operator+=(CPointF point) throw();
	void operator-=(CPointF point) throw();

	CPointF operator+(CSizeF size) const throw();
	CPointF operator-(CSizeF size) const throw();
	CPointF operator+(CPointF point) const throw();
	CSizeF operator-(CPointF point) const throw();

	float x, y;
};

// CRectF

struct CRectF
{
	CRectF() throw();
	CRectF(float l, float t, float r, float b) throw();
	CRectF(CPointF point, CSizeF size) throw();
	CRectF(CPointF topLeft, CPointF bottomRight) throw();

	float Width() const throw();
	float Height() const throw();
	CSizeF Size() const throw();
	const CPointF& TopLeft() const throw();
	const CPointF& BottomRight() const throw();

	bool PtInRect(CPointF point) const throw();

	void SetRect(float l, float t, float r, float b) throw();
	void SetRect(CPointF topLeft, CPointF bottomRight) throw();
	void InflateRect(float x, float y) throw();
	void InflateRect(CSizeF size) throw();

	void NormalizeRect() throw();

	void operator+=(CSizeF size) throw();
	void operator-=(CSizeF size) throw();

	CRectF operator+(CSizeF size) const throw();
	CRectF operator-(CSizeF size) const throw();

	float left, top, right, bottom;
};

struct CNode;
struct CContour;
struct CSegment;

typedef CAtlArray<size_t> CIndexArray;
typedef CAtlArray<CPointF> CPointFArray;
typedef CAtlList<size_t> CIndexList;
typedef CAtlMap<size_t, float> CNodeWeightMap;
typedef CAtlMap<UINT, float> CVertexWeightMap;
typedef CAtlMap<size_t, CIndexArray> CIndexArrayMap;
typedef CAtlArray<CNode> CNodeArray;
typedef CAtlArray<CContour> CContourArray;
typedef CAtlArray<CSegment> CSegmentArray;

// CNode

struct CNode
{
	CNode();
	CNode(UINT nVertex, int nLevel);
	CNode(const CNode& node);

	CNode& operator=(const CNode& node);

	UINT m_nVertex;
	int m_nLevel;
	CIndexArrayMap m_contours;
	CVertexWeightMap m_vertices;
};

// CContour

struct CContour
{
	CContour();
	CContour(LPCTSTR strName, size_t nParent, int nLevel);
	CContour(const CContour& node);

	CContour& operator=(const CContour& contour);

	CString m_strName;
	size_t m_nParent;
	int m_nLevel;
	CSegmentArray m_segments;
	CIndexArray m_contours;
};

// CSegment

struct CSegment
{
	CSegment();
	CSegment(size_t nNode1, size_t nNode2);

	size_t m_nNode1, m_nNode2;
};

// CSizeF

inline CSizeF::CSizeF()	throw()
{
}

inline CSizeF::CSizeF(float cx, float cy) throw()
{
	this->cx = cx;
	this->cy = cy;
}

inline void CSizeF::SetSize(float cx, float cy) throw()
{
	this->cx = cx;
	this->cy = cy;
}

inline void CSizeF::operator+=(CSizeF size) throw()
{
	cx += size.cx;
	cy += size.cy;
}

inline void CSizeF::operator-=(CSizeF size) throw()
{
	cx -= size.cx;
	cy -= size.cy;
}

inline CSizeF CSizeF::operator+(CSizeF size) const throw()
{
	return CSizeF(cx + size.cx, cy + size.cy);
}

inline CSizeF CSizeF::operator-(CSizeF size) const throw()
{
	return CSizeF(cx - size.cx, cy - size.cy);
}

// CPointF

inline CPointF::CPointF() throw()
{
}

inline CPointF::CPointF(float x, float y) throw()
{
	this->x = x;
	this->y = y;
}

inline void CPointF::SetPoint(float x, float y) throw()
{
	this->x = x;
	this->y = y;
}

inline void CPointF::operator+=(CSizeF size) throw()
{
	x += size.cx;
	y += size.cy;
}

inline void CPointF::operator-=(CSizeF size) throw()
{
	x -= size.cx;
	y -= size.cy;
}

inline void CPointF::operator+=(CPointF point) throw()
{
	x += point.x;
	y += point.y;
}

inline void CPointF::operator-=(CPointF point) throw()
{
	x -= point.x;
	y -= point.y;
}

inline CPointF CPointF::operator+(CSizeF size) const throw()
{
	return CPointF(x + size.cx, y + size.cy);
}

inline CPointF CPointF::operator-(CSizeF size) const throw()
{
	return CPointF(x - size.cx, y - size.cy);
}

inline CPointF CPointF::operator+(CPointF point) const throw()
{
	return CPointF(x + point.x, y + point.y);
}

inline CSizeF CPointF::operator-(CPointF point) const throw()
{
	return CSizeF(x - point.x, y - point.y);
}

// CRectF

inline CRectF::CRectF() throw()
{
}

inline CRectF::CRectF(float l, float t, float r, float b) throw()
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline CRectF::CRectF(CPointF point, CSizeF size) throw()
{
	left = point.x;
	top = point.y;
	right = left + size.cx;
	bottom = top + size.cy;
}

inline CRectF::CRectF(CPointF topLeft, CPointF bottomRight) throw()
{
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline float CRectF::Width() const throw()
{
	return right - left;
}

inline float CRectF::Height() const throw()
{
	return bottom - top;
}

inline CSizeF CRectF::Size() const throw()
{
	return CSizeF(right - left, bottom - top);
}

inline const CPointF& CRectF::TopLeft() const throw()
{
	return *((CPointF*)this);
}

inline const CPointF& CRectF::BottomRight() const throw()
{
	return *((CPointF*)this + 1);
}

inline bool CRectF::PtInRect(CPointF point) const throw()
{
	return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
}

inline void CRectF::SetRect(float l, float t, float r, float b) throw()
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline void CRectF::SetRect(CPointF topLeft, CPointF bottomRight) throw()
{
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline void CRectF::InflateRect(float x, float y) throw()
{
	left -= x;
	top -= y;
	right += x;
	bottom += y;
}

inline void CRectF::InflateRect(CSizeF size) throw()
{
	left -= size.cx;
	top -= size.cy;
	right += size.cx;
	bottom += size.cy;
}

inline void CRectF::NormalizeRect() throw()
{
	float nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

inline void CRectF::operator+=(CSizeF size) throw()
{
	left += size.cx;
	top += size.cy;
	right += size.cx;
	bottom += size.cy;
}

inline void CRectF::operator-=(CSizeF size) throw()
{
	left -= size.cx;
	top -= size.cy;
	right -= size.cx;
	bottom -= size.cy;
}

inline CRectF CRectF::operator+(CSizeF size) const throw()
{
	return CRectF(left + size.cx, top + size.cy, right + size.cx, bottom + size.cy);
}

inline CRectF CRectF::operator-(CSizeF size) const throw()
{
	return CRectF(left - size.cx, top - size.cy, right - size.cx, bottom - size.cy);
}

// CNode

inline CNode::CNode()
{
}

inline CNode::CNode(UINT nVertex, int nLevel) : m_nVertex(nVertex), m_nLevel(nLevel)
{
}

inline CNode::CNode(const CNode& node)
{
	*this = node;
}

inline CNode& CNode::operator=(const CNode& node)
{
	m_nVertex = node.m_nVertex;
	m_nLevel = node.m_nLevel;

	POSITION pos = node.m_contours.GetStartPosition();
	while (pos != NULL)
	{
		CIndexArray& indices2 = m_contours[node.m_contours.GetKeyAt(pos)];
		const CIndexArray& indices1 = node.m_contours.GetNextValue(pos);
		if (!indices2.SetCount(indices1.GetCount()))
			AtlThrow(E_OUTOFMEMORY);

		for (size_t i = 0; i < indices1.GetCount(); i++)
		{
			indices2[i] = indices1[i];
		}
	}

	pos = node.m_vertices.GetStartPosition();
	while (pos != NULL)
	{
		UINT nVertex = node.m_vertices.GetKeyAt(pos);
		float fWeight = node.m_vertices.GetNextValue(pos);
		m_vertices.SetAt(nVertex, fWeight);
	}

	return *this;
}

// CContour

inline CContour::CContour()
{
}

inline CContour::CContour(LPCTSTR strName, size_t nParent, int nLevel) : m_strName(strName), m_nParent(nParent), m_nLevel(nLevel)
{
}

inline CContour::CContour(const CContour& node)
{
	*this = node;
}

inline CContour& CContour::operator=(const CContour& contour)
{
	m_strName = contour.m_strName;
	m_nParent = contour.m_nParent;
	m_nLevel = contour.m_nLevel;

	if (!m_segments.SetCount(contour.m_segments.GetCount()))
		AtlThrow(E_OUTOFMEMORY);

	for (size_t i = 0; i < contour.m_segments.GetCount(); i++)
	{
		m_segments[i] = contour.m_segments[i];
	}

	if (!m_contours.SetCount(contour.m_contours.GetCount()))
		AtlThrow(E_OUTOFMEMORY);

	for (size_t i = 0; i < contour.m_contours.GetCount(); i++)
	{
		m_contours[i] = contour.m_contours[i];
	}

	return *this;
}

// CSegment

inline CSegment::CSegment()
{
}

inline CSegment::CSegment(size_t nNode1, size_t nNode2) : m_nNode1(nNode1), m_nNode2(nNode2)
{
}
