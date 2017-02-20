#include "StdAfx.h"
#include "LoopSubdivider.h"
#include "faceworxdoc.h"

#include <float.h>
#include <map>

CLoopSubdivider::CLoopSubdivider(void)
{
	sharpen_pts_cnt = 0;
}

CLoopSubdivider::~CLoopSubdivider(void)
{
}

bool SameVertex(const VERTEX* vs, int i1, int i2)
{
	D3DXVECTOR3 r = vs[i1].vPos - vs[i2].vPos;
	float dist = D3DXVec3LengthSq(&r);
	if (dist <= 0.01f * 0.01f)
		return true;
	return false;
}

//#define DUMP_ADJS

void CLoopSubdivider::Init(ID3DXMesh* mesh, DWORD* adjacency, LPVOID pAdjs, DWORD AdjsSize)
{
#ifdef DUMP_ADJS
	WORD faces = (WORD)mesh->GetNumFaces();
	WORD vertices = (WORD)mesh->GetNumVertices();
	WORD edges = 0;

	for (DWORD n = 0, nc = faces * 3; n < nc; n++)
	{
		DWORD adj = adjacency[n];
		if (0xFFFFFFFF == adj || adj >= n / 3)
			edges++;
	}

	verts.SetCount(vertices + edges);

	VERTEX* vs;
	WORD* is;
	mesh->LockIndexBuffer(0, (LPVOID*)&is);
	mesh->LockVertexBuffer(0, (LPVOID*)&vs);

	memset(verts.GetData(), 0, vertices * sizeof VERT);
	for (WORD n = 0; n < vertices; n++)
	{
		VERT& v = verts[n];
		v.index = n;
	}

	for (WORD n = 0, nc = faces * 3; n < nc; n++)
	{	
		WORD i0 = n - n % 3;
		WORD i1 = is[n];
		WORD n2 = (n + 2) % 3 + i0;
		DWORD adj1 = adjacency[n];
		DWORD adj2 = adjacency[n2];
		VERT& v = verts[i1];
		if (0xFFFFFFFF == adj1 || 0xFFFFFFFF == adj2)
			v.boundary |= true;
	}

	for (WORD n = 0, nc = faces * 3; n < nc; n++)
	{
		WORD i1 = is[n];
		VERT& v = verts[i1];

		for (WORD m = 0; m < nc; m++)
		if (m != n)
		{
			WORD m0 = m - m % 3;
			WORD m1 = (m + 1) % 3 + m0;
			WORD m2 = (m + 2) % 3 + m0;
			WORD i2 = is[m];

			if (is[m1] == i1 ||
				is[m2] == i1 ||
				SameVertex(vs, is[m1], i1) ||
				SameVertex(vs, is[m2], i1))
			{
			}
				else continue;
			if (0 == v.adjs_cnt)
			{
				v.adjs = (unsigned int)adjs.GetCount();
				v.adjs_cnt = 1;
				adjs.SetCount(v.adjs + 1);
				adjs[v.adjs].index = i2;
				adjs[v.adjs].next = 0;
			} else
			{
				unsigned int cur = v.adjs;
				for (;;)
				{
					ADJ& adj = adjs[cur];
					if (adj.index == i2 || SameVertex(vs, adj.index, i2))
						i2 = i1;
					if (0 == adj.next)
						break;
					cur = adj.next;
				}
				if (i2 != i1)
				{
					v.adjs_cnt++;
					unsigned int ex_cnt = (unsigned int)adjs.GetCount();
					adjs.SetCount(ex_cnt + 1);
					adjs[cur].next = ex_cnt;
					adjs[ex_cnt].next = 0;
					adjs[ex_cnt].index = i2;
					cur = ex_cnt;
				}
			}
		}
	}

/*	for (WORD n = 0, nc = faces * 3; n < nc; n++)
	{
		WORD i0 = n - n % 3;
		WORD n1 = (n + 1) % 3 + i0;
		WORD n2 = (n + 2) % 3 + i0;
		WORD i1 = is[n];
		WORD i2 = is[n1];
		WORD i3 = is[n2];
		VERT& v = verts[i1];
		DWORD adj = adjacency[n];
		DWORD adj2 = adjacency[n2];

lb_add_points:
		if (0 == v.adjs_cnt)
		{
			v.adjs = (unsigned int)adjs.GetCount();
			v.adjs_cnt = 2;
			adjs.SetCount(v.adjs + 2);
			adjs[v.adjs].index = i2;
			adjs[v.adjs].next = v.adjs + 1;
			adjs[v.adjs + 1].index = i3;
			adjs[v.adjs + 1].next = 0;
		} else
		{
			unsigned int cur = v.adjs;
			for (;;)
			{
				ADJ& adj = adjs[cur];
				if (adj.index == i2)
					i2 = i1; else
				if (adj.index == i3)
					i3 = i1;
				if (0 == adj.next)
					break;
				cur = adj.next;
			}
			if (i2 != i1)
			{
				v.adjs_cnt++;
				unsigned int ex_cnt = (unsigned int)adjs.GetCount();
				adjs.SetCount(ex_cnt + 1);
				adjs[cur].next = ex_cnt;
				adjs[ex_cnt].next = 0;
				adjs[ex_cnt].index = i2;
				cur = ex_cnt;
			}
			if (i3 != i1)
			{
				v.adjs_cnt++;
				unsigned int ex_cnt = (unsigned int)adjs.GetCount();
				adjs.SetCount(ex_cnt + 1);
				adjs[cur].next = ex_cnt;
				adjs[ex_cnt].next = 0;
				adjs[ex_cnt].index = i3;
			}
		}

		i1 = is[n];
		i2 = is[n1];
		i3 = is[n2];
		if (adj != 0xFFFFFFFF)
		{
			WORD t[3] = {is[adj * 3],
						is[adj * 3 + 1],
						is[adj * 3 + 2]};
			WORD adj_tr_nodes[3];
			FindNodes(vs, i1, i2, t, adj_tr_nodes);
			if (adj_tr_nodes[0] != i1)
			{
				i2 = adj_tr_nodes[1];
				i3 = adj_tr_nodes[2];
				adj = 0xFFFFFFFF;
				goto lb_add_points;
			}
		}
		if (adj2 != 0xFFFFFFFF)
		{
			WORD t[3] = {is[adj2 * 3],
						is[adj2 * 3 + 1],
						is[adj2 * 3 + 2]};
			WORD adj_tr_nodes[3];
			FindNodes(vs, i1, i3, t, adj_tr_nodes);
			if (adj_tr_nodes[0] != i1)
			{
				i2 = adj_tr_nodes[1];
				i3 = adj_tr_nodes[2];
				adj2 = 0xFFFFFFFF;
				goto lb_add_points;
			}
		}
	}*/

	WORD index = vertices;
	m_Indices.SetCount(faces * 12);

	typedef std::map<std::pair<unsigned int,unsigned int>, unsigned int> map_vs_t;
	map_vs_t map_vs;
	int nindices = 0;
	for (WORD n = 0, nc = faces * 3; n < nc;)
	{
		WORD pts[9];
		for (WORD nn = 0; nn < 3; nn++, n++)
		{
			DWORD adj = adjacency[n];

			WORD i0 = n - n % 3;
			WORD i1 = is[n];
			WORD i2 = is[(n + 1) % 3 + i0];
			WORD i3 = is[(n + 2) % 3 + i0];
			WORD* p = pts + nn * 3;

			if (0xFFFFFFFF != adj && adj < WORD(n / 3))
			{
				WORD t[3] = {is[adj * 3],
					is[adj * 3 + 1],
					is[adj * 3 + 2]};
				WORD adj_tr_nodes[3];
				FindNodes(vs, i2, i1, t, adj_tr_nodes);
				map_vs_t::iterator it = map_vs.find(std::make_pair(
					min(adj_tr_nodes[0],adj_tr_nodes[1]),
					max(adj_tr_nodes[0],adj_tr_nodes[1])));
				if (it != map_vs.end())
				{
					p[1] = it->second;
				} else
				{
					p[1] = i1;
					ASSERT(0);
				}
				p[0] = i1;
				p[2] = i2;
				continue;
			}

			VERT& v = verts[index];

			v.index = index;
			v.parents[0] = i1;
			v.parents[1] = i2;

			
			p[0] = i1;
			p[1] = index;
			p[2] = i2;
			map_vs[std::make_pair(min(i1,i2),max(i1,i2))] = index;

			index++;

			if (0xFFFFFFFF == adj)
			{
				v.across[0] = v.across[1] = 0xFFFF;
				continue;
			}
			WORD t[3] = {is[adj * 3],
				is[adj * 3 + 1],
				is[adj * 3 + 2]};
			WORD adj_tr_nodes[3];
			FindNodes(vs, i1, i2, t, adj_tr_nodes);
			v.across[0] = i3;
			v.across[1] = adj_tr_nodes[2];
		}
		m_Indices[nindices++] = pts[0];
		m_Indices[nindices++] = pts[1];
		m_Indices[nindices++] = pts[7];

		m_Indices[nindices++] = pts[1];
		m_Indices[nindices++] = pts[2];
		m_Indices[nindices++] = pts[4];

		m_Indices[nindices++] = pts[4];
		m_Indices[nindices++] = pts[5];
		m_Indices[nindices++] = pts[7];

		m_Indices[nindices++] = pts[4];
		m_Indices[nindices++] = pts[7];
		m_Indices[nindices++] = pts[1];
	}

	mesh->UnlockIndexBuffer();
	mesh->UnlockVertexBuffer();

	{
		CFile file(L"c:\\adjs", CFile::modeWrite | CFile::modeCreate | CFile::typeBinary);
		CArchive ar(&file, CArchive::store);

		ar << (DWORD)adjs.GetCount();
		ar.Write(adjs.GetData(), adjs.GetCount() * sizeof ADJ);

		ar << (DWORD)verts.GetCount();
		ar.Write(verts.GetData(), verts.GetCount() * sizeof VERT);

		ar << (DWORD)m_Indices.GetCount();
		ar.Write(m_Indices.GetData(), m_Indices.GetCount() * sizeof WORD);
	}
#else
	{
		CMemFile file((BYTE*)pAdjs, AdjsSize);
		CArchive ar(&file, CArchive::load);
		DWORD cnt;

		ar >> cnt;
		adjs.SetCount(cnt);
		ar.Read(adjs.GetData(), adjs.GetCount() * sizeof ADJ);

		ar >> cnt;
		verts.SetCount(cnt);
		ar.Read(verts.GetData(), verts.GetCount() * sizeof VERT);

		ar >> cnt;
		m_Indices.SetCount(cnt);
		ar.Read(m_Indices.GetData(), m_Indices.GetCount() * sizeof WORD);

		ar.Close();
		file.Detach();
	}
	for (int m = 0; m < sharpen_pts_cnt; m++)
	{
		if (0 == sharpen_pts[m])
			continue;
		for (size_t n = 0; sharpen_pts[m][n] != -1; n++)
		{
			verts[sharpen_pts[m][n]].boundary = true;
		}
		for (size_t n = mesh->GetNumVertices(), nc = verts.GetCount(); n < nc; n++)
		{
			VERT& v = verts[n];
			for (size_t n = 0; sharpen_pts[m][n] != -1; n++)
			{
				if (v.parents[0] == sharpen_pts[m][n] ||
					v.parents[1] == sharpen_pts[m][n])
				{
					v.across[0] = v.across[1];
					break;
				}
			}
		}
	}
#endif
}

void Blend(VERTEX& out, const VERTEX& vs, float a)
{
	out.vPos.x += vs.vPos.x * a;
	out.vPos.y += vs.vPos.y * a;
	out.vPos.z += vs.vPos.z * a;
	out.vNorm.x += vs.vNorm.x * a;
	out.vNorm.y += vs.vNorm.y * a;
	out.vNorm.z += vs.vNorm.z * a;
	unsigned char c1 = (unsigned char)vs.color;
	unsigned char c2 = (unsigned char)(vs.color >> 8);
	unsigned char c3 = (unsigned char)(vs.color >> 16);
	c1 = unsigned char(c1 * a) + (unsigned char)out.color;
	c2 = unsigned char(c2 * a) + (unsigned char)(out.color >> 8);
	c3 = unsigned char(c3 * a) + (unsigned char)(out.color >> 16);
	out.color = c1 | (c2 << 8) | (c3 << 16);
	for (int m = 0; m < 3; m++)
	{
		out.vTex[m].x += vs.vTex[m].x * a;
		out.vTex[m].y += vs.vTex[m].y * a;
	}
}

void CLoopSubdivider::Subdivide(ID3DXMesh* mesh, ID3DXMesh** mesh_out)
{
	VERTEX* vs, *vs2;
	WORD* is, *is2;
	WORD faces = (WORD)mesh->GetNumFaces();
	WORD vertices = (WORD)mesh->GetNumVertices();
	mesh->LockVertexBuffer(0, (LPVOID*)&vs);
	mesh->LockIndexBuffer(0, (LPVOID*)&is);

	CComPtr<IDirect3DDevice9> device;
	CComPtr<ID3DXMesh> temp_mesh;
	mesh->GetDevice(&device);
	D3DXCreateMeshFVF(faces * 4, (DWORD)verts.GetCount(), D3DXMESH_SYSTEMMEM, VERTEX_FVF,
		device, &temp_mesh);

	temp_mesh->LockVertexBuffer(0, (LPVOID*)&vs2);
	temp_mesh->LockIndexBuffer(0, (LPVOID*)&is2);
	memset(vs2, 0, verts.GetCount() * sizeof VERTEX);

	for (WORD n = 0, nc = (WORD)verts.GetCount(); n < nc; n++)
	{
		VERT& v = verts[n];
		VERTEX& a = vs[n];
		VERTEX& b = vs2[n];
		if (n < vertices) //old
		{
			if (0)
			{
				b = a;
			} else
			if (v.boundary)
			{
				ASSERT(v.adjs_cnt >= 2);
				Blend(b, a, 6.0f / 8.0f);
				ADJ* cur = &adjs[v.adjs];
				DWORD cnt = 0;
				for (;;)
				{
					// Exactly 2 should be on boundary.
					VERT& q = verts[cur->index];
					if (q.boundary)
					{
						ASSERT(cnt < 2);
						Blend(b, vs[cur->index], 1.0f / 8.0f);
						cnt++;
						if (2 == cnt)
							break;
					}
					if (0 == cur->next)
						break;
					cur = &adjs[cur->next];
				}
				if (2 != cnt)
				{
					memset(&b, 0, sizeof VERTEX);
					goto lb_non_boundary;
				}
				ASSERT(2 == cnt);
/*				if (2 != cnt)
				{
					b.vPos = D3DXVECTOR3(300,0,0);
				}*/
			} else
			{
lb_non_boundary:
				int n = v.adjs_cnt;
				float temp = 3.0f + 2.0f * cos(2.0f * float(M_PI) / float(n));
				float alpha = (40.0f - temp * temp) / 64.0f;
				Blend(b, a, 1 - alpha);
				alpha /= (float)n;
				ADJ* cur = &adjs[v.adjs];
				for (;;)
				{
					Blend(b, vs[cur->index], alpha);
					if (0 == cur->next)
						break;
					cur = &adjs[cur->next];
				}
			}
		} else
		{
			VERTEX& par0 = vs[v.parents[0]];
			VERTEX& par1 = vs[v.parents[1]];

			if (v.across[0] == v.across[1])
			{
				Blend(b, par0, 0.5f);
				Blend(b, par1, 0.5f);
			} else
			{
				VERTEX& acc0 = vs[v.across[0]];
				VERTEX& acc1 = vs[v.across[1]];
				Blend(b, par0, 3.0f / 8.0f);
				Blend(b, par1, 3.0f / 8.0f);
				Blend(b, acc0, 1.0f / 8.0f);
				Blend(b, acc1, 1.0f / 8.0f);
			}
		}
	}
	memcpy(is2, m_Indices.GetData(), m_Indices.GetCount() * sizeof WORD);

	mesh->UnlockIndexBuffer();
	mesh->UnlockVertexBuffer();
	*mesh_out = temp_mesh.Detach();
}

void CLoopSubdivider::FindNodes(VERTEX* vs, WORD i1, WORD i2, const WORD* t, WORD* nodes)
{
	for (char n = 0; n < 3; n++)
	if (t[n] == i1)
	{
		for (char m = 0; m < 3; m++)
		if (m != n && t[m] == i2)
		{
			nodes[0] = t[n];
			nodes[1] = t[m];
			nodes[2] = t[3 - n - m];
			return;
		}
	}
	char n1, n2;
	float r1 = FLT_MAX, r2 = FLT_MAX;
	for (char n = 0; n < 3; n++)
	{
		WORD i = t[n];
		D3DXVECTOR3 r = vs[i].vPos - vs[i1].vPos;
		float dist = D3DXVec3LengthSq(&r);
		if (dist < r1)
		{
			r1 = dist;
			n1 = n;
		}
		r = vs[i].vPos - vs[i2].vPos;
		dist = D3DXVec3LengthSq(&r);
		if (dist < r2)
		{
			r2 = dist;
			n2 = n;
		}
	}
	nodes[0] = t[n1];
	nodes[1] = t[n2];
	ASSERT(r1 <= 0.01f * 0.01f && r2 <= 0.01f * 0.01f);
	nodes[2] = t[3 - n1 - n2];
}