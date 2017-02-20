#include "StdAfx.h"
#include ".\obj_exporter.h"
#include "faceworxdoc.h"
#include "SubStream.h"
#include "DXException.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <set>
#include <vector>
#include <algorithm>
#include <gdiplus.h>

using namespace Gdiplus;

//#define USE_ATLAS

obj_exporter::obj_exporter(void)
{
	width=height=2048;
	export_normals=false;
	enlarge_triangles=true;
}

obj_exporter::~obj_exporter(void)
{
}

void obj_exporter::get_state_block(LPDIRECT3DDEVICE9 device,CComPtr<IDirect3DStateBlock9>& state_block,int w,int h)
{
	for (int n=0;n<2;n++)
	{
		if (0==n)
			device->BeginStateBlock();
		device->SetRenderState(D3DRS_ZENABLE,false);
		device->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
		device->SetRenderState(D3DRS_LIGHTING,FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)-1);
		device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		D3DXMATRIX proj,view;
		D3DXMatrixOrthoOffCenterLH(&proj,0,1,0,1,-1000,1000);
		D3DXMatrixIdentity(&view);
		device->SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&proj);
		device->SetTransform(D3DTS_VIEW,(D3DMATRIX*)&view);
		device->SetTransform(D3DTS_WORLD,(D3DMATRIX*)&view);

		D3DVIEWPORT9 vp;
		vp.X=vp.Y=0;
		vp.Width=w;
		vp.Height=h;
		vp.MinZ=-1000;
		vp.MaxZ=1000;
		device->SetViewport(&vp);

		if (0==n)
		{
			HRESULT hr = device->EndStateBlock(&state_block);
			if (FAILED(hr))
				AfxThrowDXException(hr);
			state_block->Capture();
		}
	}
}

void obj_exporter::save_surface(IDirect3DSurface9* surf,LPCTSTR tex_pathname)
{
	HRESULT hr;
	wchar_t szTempName[MAX_PATH];
	const int BUFSIZE=4096;
	DWORD dwBufSize=BUFSIZE;
	wchar_t lpPathBuffer[BUFSIZE];
	GetTempPath(dwBufSize,lpPathBuffer);
	GetTempFileName(lpPathBuffer,L"fw_",0,szTempName);

	const bool use_stream=false;
	CComPtr<ID3DXBuffer> bmp_buf;

	hr = use_stream ? D3DXSaveSurfaceToFileInMemory(&bmp_buf,D3DXIFF_BMP,surf,0,0):
		D3DXSaveSurfaceToFile(
			szTempName,
			D3DXIFF_BMP,surf,0,0);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	std::auto_ptr<Image> image;
	ULONG_PTR gdiplusToken;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (use_stream)
	{
		CMemFile file;
		file.Attach((BYTE*)bmp_buf->GetBufferPointer(),bmp_buf->GetBufferSize());
		{
			CArchive ar(&file,CArchive::load);
			CArchiveStream arr(&ar);
			ULARGE_INTEGER w;
			w.QuadPart=bmp_buf->GetBufferSize();
			CSubStream q(&arr,w);
			image.reset(new Image(&q));
			if (image.get()==0)
			{
				AfxMessageBox(L"Error loading texture from stream", MB_ICONERROR);
				AfxThrowUserException();
			}
		}
		file.Detach();
		bmp_buf.Release();
	} else
	{
		image.reset(new Image(szTempName));
		if (image.get()==0)
		{
			CString s;
			s.Format(L"Error loading texture from temporary file:\r\n%s", szTempName);
			AfxMessageBox(s, MB_ICONERROR);
			AfxThrowUserException();
		}
	}
	CLSID             encoderClsid;
	EncoderParameters encoderParameters;
	Status            stat;
	INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	if (-1==GetEncoderClsid(L"image/jpeg", &encoderClsid))
	{
		AfxMessageBox(L"Error accessing image/jpeg codec parameters", MB_ICONERROR);
		AfxThrowUserException();
	}
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &quality;
	stat = image->Save(tex_pathname, &encoderClsid, &encoderParameters);
	if (stat!=Gdiplus::Ok)
	{
		CString s;
		s.Format(L"Error saving texture:\r\n%s", tex_pathname);
		AfxMessageBox(s, MB_ICONERROR);
		AfxThrowUserException();
	}
	image.reset();
	DeleteFile(szTempName);
	GdiplusShutdown(gdiplusToken);
}

void obj_exporter::join_vertices(ID3DXMesh* mesh,CComPtr<ID3DXMesh>& out)
{
	HRESULT hr;
	CComPtr<IDirect3DDevice9> dev;
	mesh->GetDevice(&dev);
	make_good_mesh(dev,mesh,out);
	D3DXWELDEPSILONS Epsilons;
    Epsilons.Normal = 10000.0f;
    Epsilons.Position = 0.1f;
	DWORD *pFaceRemap[65536];
	std::auto_ptr<DWORD> pAdjacency(new DWORD[out->GetNumFaces() * sizeof(DWORD)*3]);
	const FLOAT g_cfEpsilon = 0.01f;
	hr = out->GenerateAdjacency( g_cfEpsilon, pAdjacency.get() );
	if (FAILED(hr))
		AfxThrowDXException(hr);
    for( int i=0; i < 65536; i++ )
        pFaceRemap[i] = 0;
    LPD3DXBUFFER     pAdjacencyBuffer = NULL;
    hr = D3DXWeldVertices ( out,
                            0,
                            0,
                            pAdjacency.get(),
                            pAdjacency.get(),
                            (DWORD*)pFaceRemap,
                            NULL );
	if( FAILED( hr ) )
		AfxThrowDXException(hr);
}

void obj_exporter::make_good_mesh(LPDIRECT3DDEVICE9 dev,
									 ID3DXMesh* src_mesh,
									 CComPtr<ID3DXMesh>& out)
{
	HRESULT hr;
	//first, get rid of 'null' faces:
	CComPtr<IDirect3DVertexBuffer9> vb1;
	hr = src_mesh->GetVertexBuffer(&vb1);
	DWORD vs_cnt1=src_mesh->GetNumVertices();

	CComPtr<IDirect3DIndexBuffer9> ib1;
	hr = src_mesh->GetIndexBuffer(&ib1);
	DWORD is_cnt1=src_mesh->GetNumFaces()*3;

	WORD* is1=0;
	hr = ib1->Lock(0, 0, (LPVOID*)&is1, 0);
	ASSERT(0==is_cnt1%3);
	WORD* tmp=new WORD[is_cnt1]; //it's faster
	memcpy(tmp,is1,sizeof(WORD)*is_cnt1);
	hr = ib1->Unlock();
	for (DWORD n=is_cnt1-3;n<0x80000000;n-=3)
	if (tmp[n]==tmp[n+1] || tmp[n]==tmp[n+2] || tmp[n+1]==tmp[n+2])
	{
		memmove(tmp,tmp+3,sizeof(WORD)*3*(is_cnt1-3-n));
		is_cnt1-=3;
	}
	D3DXCreateMeshFVF(is_cnt1/3,vs_cnt1,D3DXMESH_SYSTEMMEM,VERTEX_FVF,dev,&out);
	VERTEX* vs1=0;
	VERTEX* vs2=0;
	src_mesh->LockVertexBuffer(0,(LPVOID*)&vs1);
	out->LockVertexBuffer(0,(LPVOID*)&vs2);
	WORD* is2=0;
	out->LockIndexBuffer(0,(LPVOID*)&is2);
	memcpy(vs2,vs1,vs_cnt1*sizeof(VERTEX));
	memcpy(is2,tmp,is_cnt1*sizeof(WORD));
	delete tmp;
	src_mesh->UnlockVertexBuffer();
	out->UnlockVertexBuffer();
	out->UnlockIndexBuffer();
}

void obj_exporter::write_mesh(ID3DXMesh* mesh,LPCTSTR fn,LPCTSTR mtl_name,int ndetail)
{
	HRESULT hr;
	//get vertex/index buffers:
	CComPtr<IDirect3DVertexBuffer9> vb;
	hr = mesh->GetVertexBuffer(&vb);
	if (FAILED(hr))
		AfxThrowDXException(hr);
	CComPtr<IDirect3DIndexBuffer9> ib;
	hr = mesh->GetIndexBuffer(&ib);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	//lock them:
	VERTEX* vs;
	WORD* is;
	DWORD vs_cnt=mesh->GetNumVertices(),is_cnt=mesh->GetNumFaces()*3;

	hr = ib->Lock(0, 0, (LPVOID*)&is, 0);
	if (FAILED(hr))
		AfxThrowDXException(hr);
	try
	{
		hr = vb->Lock(0, 0, (LPVOID*)&vs, 0);
		if (FAILED(hr))
			AfxThrowDXException(hr);
		try
		{
			//perform writing:
			FILE* f=0;
			//_wfopen_s(&f,fn,L"wt");
			f = _wfopen(fn, L"wt");
			if (0==f)
			{
				CString s;
				s.Format(L"Error opening file for writing:\r\n%s", fn);
				AfxMessageBox(s, MB_ICONERROR);
				AfxThrowUserException();
			}
			fprintf(f,"mtllib %S\n",mtl_name);
			for (DWORD n=0;n<vs_cnt;n++)
			{
				fprintf(f,"v %g %g %g\n",vs[n].vPos.x,vs[n].vPos.y,vs[n].vPos.z);
			}
			for (DWORD n=0;n<vs_cnt;n++)
			{
				fprintf(f,"vt %g %g\n", vs[n].vTex[2].x, vs[n].vTex[2].y);
			}
			for (DWORD n=0;n<is_cnt;n+=3)
			{
				fprintf(f,"usemtl head\nf %d/%d %d/%d %d/%d\n",
					is[n]+1,is[n]+1,is[n+1]+1,is[n+1]+1,is[n+2]+1,is[n+2]+1);
			}
			fclose(f);

			float coeff=1.0f;
			if (enlarge_triangles)
			switch (ndetail)
			{
			case 0: coeff=1.2f; break;
			case 1: coeff=1.4f; break;
			case 2: coeff=1.8f; break;
			}
			for (DWORD n=0;n<is_cnt;n+=3)
			{
				VERTEX* v[3]={vs+is[n],vs+is[n+1],vs+is[n+2]};
				D3DXVECTOR2 center=(v[0]->vTex[2]+v[1]->vTex[2]+v[2]->vTex[2])/3.0f;
				for (int n=0;n<3;n++)
				{
					v[n]->vPos.x=center.x+(v[n]->vTex[2].x-center.x)*coeff;
					v[n]->vPos.y=center.y+(v[n]->vTex[2].y-center.y)*coeff;
					v[n]->vPos.z=0.5f;
					v[n]->vNorm.x=0.0f;
					v[n]->vNorm.y=0.0f;
					v[n]->vNorm.z=1.0f;
				}
			}
		}
		catch (...)
		{
			vb->Unlock();
			throw;
		}
		vb->Unlock();
	}
	catch (...)
	{
		ib->Unlock();
		throw;
	}
	ib->Unlock();
}

void obj_exporter::write_material(LPCTSTR mtl_pathname,LPCTSTR tex_name)
{
	FILE* f=0;
	//_wfopen_s(&f,mtl_pathname,L"wt");
	f = _wfopen(mtl_pathname,L"wt");
 	if (0==f)
	{
		CString s;
		s.Format(L"Error opening file for writing:\r\n%s", mtl_pathname);
		AfxMessageBox(s, MB_ICONERROR);
		AfxThrowUserException();
	}

	fprintf(f,"newmtl head\n");
	fprintf(f,"Ka 0 0 0\n");
	fprintf(f,"Kd 1 1 1\n");
	fprintf(f,"Ks 0 0 0\n");
	fprintf(f,"illum 2\n");
	fprintf(f,"Ns 8\n");
	fprintf(f,"map_Kd %S\n",tex_name);

	fclose(f);
}

void obj_exporter::draw_texture(LPDIRECT3DDEVICE9 device,ID3DXMesh* mesh,IDirect3DSurface9* surf,int w,int h)
{
	CComPtr<IDirect3DSurface9> ex_surf;
	HRESULT hr = device->GetRenderTarget(0,&ex_surf);
	if (FAILED(hr))
		AfxThrowDXException(hr);
	CComPtr<IDirect3DStateBlock9> state_block;

	hr = device->SetRenderTarget(0,surf);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	try
	{
		get_state_block(device,state_block,w,h);
		try
		{
			//draw:
			hr = device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				D3DCOLOR_XRGB(255, 255, 255), 1, 0);
			if (FAILED(hr))
				AfxThrowDXException(hr);

			hr = device->BeginScene();
			if (FAILED(hr))
				AfxThrowDXException(hr);
			try
			{
				hr = mesh->DrawSubset(0);
				if (FAILED(hr))
					AfxThrowDXException(hr);

				if (enlarge_triangles)
				{
					DWORD cnt=mesh->GetNumVertices();
					VERTEX* vs;
					hr = mesh->LockVertexBuffer(0,(LPVOID*)&vs);
					if (FAILED(hr))
						AfxThrowDXException(hr);
					try
					{
						for (DWORD n=0;n<cnt;n++)
						{
							vs[n].vPos.x=vs[n].vTex[2].x;
							vs[n].vPos.y=vs[n].vTex[2].y;
						}
					}
					catch (...)
					{
						mesh->UnlockVertexBuffer();
						throw;
					}
					mesh->UnlockVertexBuffer();
					hr = mesh->DrawSubset(0);
					if (FAILED(hr)) {ASSERT(0);} //todo handling
				}
			}
			catch (...)
			{
				hr = device->EndScene();
				if (FAILED(hr))
					AfxThrowDXException(hr);
				throw;
			}
			hr = device->EndScene();
				if (FAILED(hr))
					AfxThrowDXException(hr);
		}
		catch (...)
		{
			//revert device state:
			hr = state_block->Apply();
			if (FAILED(hr))
				AfxThrowDXException(hr);
			throw;
		}
		//revert device state:
		hr = state_block->Apply();
		if (FAILED(hr))
			AfxThrowDXException(hr);
	} catch (...)
	{
		hr = device->SetRenderTarget(0,ex_surf);
		if (FAILED(hr))
			AfxThrowDXException(hr);
		throw;
	}
	hr = device->SetRenderTarget(0,ex_surf);
		if (FAILED(hr))
			AfxThrowDXException(hr);
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

void obj_exporter::create_texture(LPDIRECT3DDEVICE9 device,CComPtr<IDirect3DSurface9>& surf,int w,int h)
{
	HRESULT hr = device->CreateRenderTarget(w,h,D3DFMT_X8R8G8B8,D3DMULTISAMPLE_NONE,0,0,&surf,0);
	if (FAILED(hr))
		AfxThrowDXException(hr);
}

void obj_exporter::init_pathes(const CString& fn,CString& tex_name,CString& tex_pathname,CString& mtl_name,CString& mtl_pathname)
{
	int i1=fn.ReverseFind('\\'),i2=fn.ReverseFind('/');
	int i=max(i1,i2);
	CString path = i>=0 ? fn.Left(i) : CString(),
		only_name=fn.Mid(i+1);
	i=only_name.ReverseFind('.');
	if (i!=-1) only_name=only_name.Left(i);
	tex_name=only_name+_T(".jpg"),
	tex_pathname=path+_T('\\')+tex_name,
	mtl_name=only_name+_T(".mtl"),
	mtl_pathname=path+_T('\\')+mtl_name;
}

bool obj_exporter::do_export(const CString& fn,CFaceworxDoc* doc)
{
	CComPtr<IDirect3DSurface9> surf;
	CComPtr<ID3DXMesh> mesh;
	CString tex_name,tex_pathname,mtl_name,mtl_pathname;

	//get all the neccessary filenames:
	init_pathes(fn,tex_name,tex_pathname,mtl_name,mtl_pathname);
	//ensure we have the mesh without null indices:
	join_vertices(doc->m_spEnhancedMesh,mesh);
	//write mesh contents (3d and texture coordinates,indices) to a file:
	write_mesh(mesh,fn,mtl_name,doc->m_nDetail);
	//write material (.mtl file):
	write_material(mtl_pathname,tex_name);
	//create texture as render target:
	create_texture(doc->m_spDevice,surf,width,height);
	//draw mesh on the texture (blend two source textures):
	draw_texture(doc->m_spDevice,mesh,surf,width,height);
	//save texture to the file:
	save_surface(surf,tex_pathname);

	return true;
}