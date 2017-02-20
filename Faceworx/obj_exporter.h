#pragma once

class CFaceworxDoc;

class obj_exporter
{
public:
	void make_good_mesh(LPDIRECT3DDEVICE9 dev,ID3DXMesh* src_mesh,CComPtr<ID3DXMesh>& out);
	void join_vertices(ID3DXMesh* mesh,CComPtr<ID3DXMesh>& out);
	void get_state_block(LPDIRECT3DDEVICE9 device,CComPtr<IDirect3DStateBlock9>& state_block,int w,int h);
	void save_surface(IDirect3DSurface9* surf,LPCTSTR tex_pathname);
	void write_mesh(ID3DXMesh* mesh,LPCTSTR fn,LPCTSTR mtl_name,int ndetail);
	void write_material(LPCTSTR mtl_pathname,LPCTSTR tex_name);
	void draw_texture(LPDIRECT3DDEVICE9 device,ID3DXMesh* mesh,IDirect3DSurface9* surf,int w,int h);
	void create_texture(LPDIRECT3DDEVICE9 device,CComPtr<IDirect3DSurface9>&,int w,int h);
	void init_pathes(const CString& fn,CString& tex_name,CString& tex_pathname,CString& mtl_name,CString& mtl_pathname);
public:
	obj_exporter(void);
	~obj_exporter(void);
	int width,height;
	int quality;
	bool export_normals;
	bool enlarge_triangles;
	bool do_export(const CString& fn,CFaceworxDoc* doc);
};
