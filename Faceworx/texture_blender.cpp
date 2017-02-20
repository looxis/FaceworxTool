#include "StdAfx.h"
#include "texture_blender.h"
#include "pixeler.h"

texture_blender::texture_blender(void)
{
}

texture_blender::~texture_blender(void)
{
}

void get_intersection(float y,CPointFArray& line,float& x)
{
	if (y<=line[0].y)
	{
		x=line[0].x;
		return;
	}
	if (y>=line[line.GetCount()-1].y)
	{
		x=line[line.GetCount()-1].x;
		return;
	}
	for (int n=1,nc=line.GetCount();n<nc;n++)
	{
		CPointF& p1=line[n-1];
		CPointF& p2=line[n];
		if (y>=p1.y && y<=p2.y)
		{
			x=p1.x+(p2.x-p1.x)*(y-p1.y)/(p2.y-p1.y);
			return;
		}
	}
	ASSERT(0);
}

void get_intersections(float y,CPointFArray& line1,CPointFArray& line2,CSize sz,float& x1,float& x2,float& rx1,float& rx2)
{
	get_intersection(y,line1,x1);
	get_intersection(y,line2,x2);
	rx1=x1;
	rx2=x2;
	if (x1<0)
		x1=0;
	if (x1>(float)sz.cx)
		x1=(float)sz.cx;
	if (x2<0)
		x2=0;
	if (x2>(float)sz.cx)
		x2=(float)sz.cx;
}

template <
	char off,char len,char bytes,typename type
>
struct blender
{
	enum {off_bits=off%8};
	enum {off_bytes=off/8};
	enum {mask=calc_mask<off,len>::val};
	enum {mask_type=calc_mask<off_bits,len>::val};
	enum {maxval=(1<<len)-1};
	template <bool qwe>
	static void set(type& q1,type val);
	template <>
	static void set<true>(type& q1,type val)
	{
		q1=val;
	}
	template <>
	static void set<false>(type& q1,type val)
	{
		q1&=~(type)mask_type;
		q1|=val<<off_bits;
	}
	static void blend(CPointFArray& line1,CPointFArray& line2,D3DLOCKED_RECT& r,CSize sz)
	{
		char* p=(char*)r.pBits;
		for (int y=0;y<sz.cy;y++)
		{
			char* pp=p;
			pp+=off_bytes;
			float x1,x2,rx1,rx2;
			get_intersections((float)y,line1,line2,sz,x1,x2,rx1,rx2);
			float d=rx2-rx1;
			int ix1=(int)x1;
			int ix2=(int)x2;
			if (ix2>sz.cx)
				ix2=sz.cx;
			int x=0;
			for (;x<ix1;x++)
			{
				set<off_bits==0 && sizeof(type)*8==len>(*(type*)pp,maxval);
				pp+=bytes;
			}
			if (ix1<ix2)
			{
				set<off_bits==0 && sizeof(type)*8==len>(*(type*)pp,maxval);
				x++;
				pp+=bytes;
				float q1=(x-rx1)/d;
				float q2=(ix2-1-rx1)/d;
				unsigned int v1=maxval-(unsigned char)(q1*maxval);
				unsigned int v2=maxval-(unsigned char)(q2*maxval);
				int dx=ix2-1-x;
				if (0==dx)
				{
					set<off_bits==0 && sizeof(type)*8==len>(*(type*)pp,v1);
					pp+=bytes;
				} else
				{
					v1<<=24;
					v2<<=24;
					unsigned int dv=(v1-v2)/dx;
					for (;x<ix2;x++)
					{
						set<off_bits==0 && sizeof(type)*8==len>(*(type*)pp,v1>>24);
						v1-=dv;
						pp+=bytes;
					}
				}
				set<off_bits==0 && sizeof(type)*8==len>(*(type*)pp,0);
				x++;
				pp+=bytes;
			}
			for (;x<sz.cx;x++)
			{
				set<off_bits==0 && sizeof(type)*8==len>(*(type*)pp,0);
				pp+=bytes;
			}
			p+=r.Pitch;
		}
	}
};

HRESULT texture_blender::blend(IDirect3DTexture9* tex,CPointFArray& line1,CPointFArray& line2)
{
	HRESULT hr;
	D3DSURFACE_DESC desc;
	tex->GetLevelDesc(0,&desc);
	D3DLOCKED_RECT r;
	VERIFY(D3D_OK==(hr=tex->LockRect(0,&r,0,0)));
	if (FAILED(hr)) return hr;
	switch (desc.Format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_A8B8G8R8:
			blender<24,8,4,unsigned char>::blend(line1,line2,r,CSize(desc.Width,desc.Height));
		break;
	case D3DFMT_A2B10G10R10:
			blender<30,2,4,unsigned char>::blend(line1,line2,r,CSize(desc.Width,desc.Height));
		break;
	case D3DFMT_A4R4G4B4:
			blender<12,4,2,unsigned char>::blend(line1,line2,r,CSize(desc.Width,desc.Height));
		break;
	default:
		VERIFY(D3D_OK==(hr=tex->UnlockRect(0)));
		ASSERT(0);
		return E_NOTIMPL;
	}
	VERIFY(D3D_OK==(hr=tex->UnlockRect(0)));
	if (FAILED(hr)) return hr;
	return D3D_OK;
}