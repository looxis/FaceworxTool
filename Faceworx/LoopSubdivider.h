#pragma once

typedef struct
{
	union
	{
		struct
		{
			unsigned short parents[2]; // Verts on birth edge.
			unsigned short across[2]; // Verts opposite birth edge.
		};
		struct
		{
			unsigned int adjs; // Vertices sharing an edge. (starting index of array)
			unsigned int adjs_cnt:31;
			bool boundary:1;
		};
	};
	unsigned short index;
} VERT;

struct VERTEX;

struct ADJ
{
	unsigned short index;
	unsigned int next;
};

class CLoopSubdivider
{
	CAtlArray<WORD> m_Indices;
	CAtlArray<VERT> verts;
	CAtlArray<ADJ> adjs;
	static void FindNodes(VERTEX* vs, WORD i1, WORD i2, const WORD* t, WORD* nodes);
public:
	CLoopSubdivider(void);
	~CLoopSubdivider(void);
	void Init(ID3DXMesh* mesh, DWORD* adjacency, LPVOID pAdjs, DWORD AdjsSize);
	void Subdivide(ID3DXMesh* mesh, ID3DXMesh** mesh_out);
	int* sharpen_pts[10];
	int sharpen_pts_cnt;
};
