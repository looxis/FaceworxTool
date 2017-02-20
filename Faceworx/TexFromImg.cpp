#include "stdafx.h"
#include "DXException.h"
#include "pixeler.h"

struct smart_color
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char unused;
};

struct color_16
{
	unsigned char b:5;
	unsigned char g:5;
	unsigned char r:5;
	unsigned char unused:1;
};

void RGBCase(char* p, const CImage& image, DWORD width, DWORD height, DWORD format, DWORD pitch);
void MaskedCase(char* p, const CImage& image, DWORD width, DWORD height, DWORD format, DWORD pitch);
void IndexedCase(char* p, const CImage& image, DWORD width, DWORD height, DWORD format, DWORD pitch, DWORD bpp, BITMAPINFO& bminfo);

void CreateDefaultTexture(LPDIRECT3DDEVICE9 pDevice, DWORD usage, D3DPOOL pool, LPDIRECT3DTEXTURE9* ppTexture, int w, int h)
{
	HRESULT hr;

	hr = D3DXCreateTexture(pDevice, w, h, 1, usage, D3DFMT_A8R8G8B8, pool, ppTexture);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	D3DLOCKED_RECT rect;
	hr = (*ppTexture)->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	D3DSURFACE_DESC desc;
	hr = (*ppTexture)->GetLevelDesc(0, &desc);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	PBYTE p = (PBYTE)rect.pBits;
	const unsigned char r = 0x40, g = 0x40, b = 0x40;
	for (UINT y = 0; y < desc.Height; y++)
	{
		char* pp = (char*)p;
		switch (desc.Format)
		{
		case D3DFMT_A8R8G8B8:
			{
				typedef pixeler<24, 8, 16, 8, 8, 8, 0, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < desc.Width; x++)
					pix::iterate_line(pp, r, g, b, 0);
			}
			break;
		case D3DFMT_A8B8G8R8:
			{
				typedef pixeler<24, 8, 0, 8, 8, 8, 16, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < desc.Width; x++)
					pix::iterate_line(pp, r, g, b, 0);
			}
			break;
		case D3DFMT_A2B10G10R10:
			{
				typedef pixeler<30, 2, 0, 10, 10, 10, 20, 10, 4, unsigned int> pix;
				for (DWORD x = 0; x < desc.Width; x++)
					pix::iterate_line(pp, r, g, b, 0);
			}
			break;
		case D3DFMT_A4R4G4B4:
			{
				typedef pixeler<12, 4, 8, 4, 4, 4, 0, 4, 2, unsigned short> pix;
				for (DWORD x = 0; x < desc.Width; x++)
					pix::iterate_line(pp, r, g, b, 0);
			}
			break;
		}
		p += rect.Pitch;
	}

	hr = (*ppTexture)->UnlockRect(0);
	if (FAILED(hr))
		AfxThrowDXException(hr);
}

void CreateTextureFromImage(LPDIRECT3DDEVICE9 pDevice, const CImage& image, DWORD usage, D3DPOOL pool, LPDIRECT3DTEXTURE9* ppTexture)
{
	HRESULT hr;

	DWORD width = image.GetWidth();
	DWORD height = image.GetHeight();

	UINT width2 = width, height2 = height;
	D3DXCheckTextureRequirements(pDevice, &width2, &height2, 0, 0, 0, pool);

	if (width2 < width || height2 < height)
	{
		AfxMessageBox(L"Image resolution is too high (hardware restriction)", MB_ICONERROR);
		AfxThrowUserException();
	}

	hr = D3DXCreateTexture(pDevice, width, height, 1, usage, D3DFMT_A8R8G8B8, pool, ppTexture);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	D3DSURFACE_DESC desc;
	hr = (*ppTexture)->GetLevelDesc(0, &desc);
	if (FAILED(hr))
		AfxThrowDXException(hr);

	D3DLOCKED_RECT rect;
	hr = (*ppTexture)->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
	if (FAILED(hr))
		AfxThrowDXException(hr);
	try
	{
		char* p = (char*)rect.pBits;
		ASSERT(image.IsDIBSection());
		char bminfo_buf[sizeof(BITMAPINFO) + sizeof(RGBQUAD)*512];
		BITMAPINFO& bminfo = (BITMAPINFO&)bminfo_buf;
		memset(&bminfo, 0, sizeof bminfo_buf);
		bminfo.bmiHeader.biSize = sizeof bminfo_buf;
		if (0 == GetDIBits(image.GetDC(), image, 0, 0, 0, &bminfo, DIB_RGB_COLORS))
		{
			AfxMessageBox(L"Unsupported image format", MB_ICONERROR);
			AfxThrowUserException();
		}
		image.ReleaseDC();
		if (0 == GetDIBits(image.GetDC(), image, 0, 0, 0, &bminfo, DIB_RGB_COLORS))
		{
			AfxMessageBox(L"Unsupported image format", MB_ICONERROR);
			AfxThrowUserException();
		}
		image.ReleaseDC();
		DWORD src_bpp = image.GetBPP();
		switch (src_bpp)
		{
		case 32:
		case 24:
		case 16:
			switch (bminfo.bmiHeader.biCompression)
			{
			case BI_RGB:
				RGBCase(p, image, width, height, desc.Format, rect.Pitch);
				break;
			case BI_BITFIELDS:
				MaskedCase(p, image, width, height, desc.Format, rect.Pitch);
				break;
			default:
				AfxMessageBox(L"Unsupported image format", MB_ICONERROR);
				AfxThrowUserException();
			}
			break;
		case 8:
		case 4:
		case 1:
			IndexedCase(p, image, width, height, desc.Format, rect.Pitch, src_bpp, bminfo);
			break;
		default:
			AfxMessageBox(L"Unsupported image format", MB_ICONERROR);
			AfxThrowUserException();
		}
	}
	catch(...)
	{  
		hr = (*ppTexture)->UnlockRect(0);
		if (FAILED(hr))
			AfxThrowDXException(hr);
		throw;
	}
	hr = (*ppTexture)->UnlockRect(0);
	if (FAILED(hr))
			AfxThrowDXException(hr);
}

void RGBCase(char* p, const CImage& image, DWORD width, DWORD height, DWORD format, DWORD pitch)
{
	char* src = (char*)image.GetBits();
	DWORD src_pitch = image.GetPitch();
	DWORD src_bpp = image.GetBPP();
	if (0 == src)
	{
		AfxMessageBox(L"Unsupported image format", MB_ICONERROR);
		AfxThrowUserException();
	}
	DWORD src_bytespp = src_bpp/8;
	if (2==src_bytespp)
	for (DWORD y = 0; y < height; y++)
	{
		char* pp = (char*)p;
		char* src_pp = src;
		switch (format)
		{
		case D3DFMT_A8R8G8B8:
			{
				typedef pixeler<24, 8, 16, 8, 8, 8, 0, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					color_16* color = (color_16*)src_pp;
					src_pp += 2;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A8B8G8R8:
			{
				typedef pixeler<24, 8, 0, 8, 8, 8, 16, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					color_16* color = (color_16*)src_pp;
					src_pp += 2;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A2B10G10R10:
			{
				typedef pixeler<30, 2, 0, 10, 10, 10, 20, 10, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					color_16* color = (color_16*)src_pp;
					src_pp += 2;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A4R4G4B4:
			{
				typedef pixeler<12, 4, 8, 4, 4, 4, 0, 4, 2, unsigned short> pix;
				for (DWORD x = 0; x < width; x++)
				{
					color_16* color = (color_16*)src_pp;
					src_pp += 2;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		}
		p += pitch;
		src += src_pitch;
	} else
	for (DWORD y = 0; y < height; y++)
	{
		char* pp = (char*)p;
		char* src_pp = src;
		switch (format)
		{
		case D3DFMT_A8R8G8B8:
			{
				typedef pixeler<24, 8, 16, 8, 8, 8, 0, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					smart_color* color = (smart_color*)src_pp;
					src_pp += src_bytespp;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A8B8G8R8:
			{
				typedef pixeler<24, 8, 0, 8, 8, 8, 16, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					smart_color* color = (smart_color*)src_pp;
					src_pp += src_bytespp;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A2B10G10R10:
			{
				typedef pixeler<30, 2, 0, 10, 10, 10, 20, 10, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					smart_color* color = (smart_color*)src_pp;
					src_pp += src_bytespp;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A4R4G4B4:
			{
				typedef pixeler<12, 4, 8, 4, 4, 4, 0, 4, 2, unsigned short> pix;
				for (DWORD x = 0; x < width; x++)
				{
					smart_color* color = (smart_color*)src_pp;
					src_pp += src_bytespp;
					pix::iterate_line(pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		}
		p += pitch;
		src += src_pitch;
	}
}

void MaskedCase(char* p, const CImage& image, DWORD width, DWORD height, DWORD format, DWORD pitch)
{
	for (DWORD y = 0; y < height; y++)
	{
		char* pp = (char*)p;
		switch (format)
		{
		case D3DFMT_A8R8G8B8:
			{
				typedef pixeler<24, 8, 16, 8, 8, 8, 0, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					COLORREF color=image.GetPixel(x, y);
					pix::iterate_line(pp, GetRValue(color), GetGValue(color), GetBValue(color), 0);
				}
			}
			break;
		case D3DFMT_A8B8G8R8:
			{
				typedef pixeler<24, 8, 0, 8, 8, 8, 16, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					COLORREF color=image.GetPixel(x, y);
					pix::iterate_line(pp, GetRValue(color), GetGValue(color), GetBValue(color), 0);
				}
			}
			break;
		case D3DFMT_A2B10G10R10:
			{
				typedef pixeler<30, 2, 0, 10, 10, 10, 20, 10, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					COLORREF color=image.GetPixel(x, y);
					pix::iterate_line(pp, GetRValue(color), GetGValue(color), GetBValue(color), 0);
				}
			}
			break;
		case D3DFMT_A4R4G4B4:
			{
				typedef pixeler<12, 4, 8, 4, 4, 4, 0, 4, 2, unsigned short> pix;
				for (DWORD x = 0; x < width; x++)
				{
					COLORREF color=image.GetPixel(x, y);
					pix::iterate_line(pp, GetRValue(color), GetGValue(color), GetBValue(color), 0);
				}
			}
			break;
		}
		p += pitch;
	}
}

void IndexedCase(char* p, const CImage& image, DWORD width, DWORD height, DWORD format, DWORD pitch, DWORD bpp, BITMAPINFO& bminfo)
{
	char* src = (char*)image.GetBits();
	DWORD src_pitch = image.GetPitch();
	if (0 == src)
	{
		AfxMessageBox(L"Unsupported image format", MB_ICONERROR);
		AfxThrowUserException();
	}
	unsigned char mask = unsigned char((1 << bpp) - 1);
	unsigned char times_mask, shift_shift;
	switch (bpp)
	{
	case 1: times_mask = 7; shift_shift = 0; break;
	case 4: times_mask = 1; shift_shift = 2; break;
	case 8: times_mask = 0; shift_shift = 0; break;
	}
	unsigned char times = unsigned char(8 / bpp); 
	for (DWORD y = 0; y < height; y++)
	{
		unsigned char* pp = (unsigned char*)p;
		char* src_pp = src;
		unsigned char ntimes = 0xFF;
		switch (format)
		{
		case D3DFMT_A8R8G8B8:
			{
				typedef pixeler<24, 8, 16, 8, 8, 8, 0, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					unsigned char ntimes_masked = ntimes-- & times_mask;
					unsigned char shift = (ntimes_masked) << shift_shift;
					unsigned char ncol = mask & (*src_pp >> shift);
					smart_color* color = (smart_color*)&bminfo.bmiColors[ncol];
					if (0 == ntimes_masked)
						src_pp ++;
					pix::iterate_line((char*&)pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A8B8G8R8:
			{
				typedef pixeler<24, 8, 0, 8, 8, 8, 16, 8, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					unsigned char ntimes_masked = ntimes-- & times_mask;
					unsigned char shift = (ntimes_masked) << shift_shift;
					unsigned char ncol = mask & (*src_pp >> shift);
					smart_color* color = (smart_color*)&bminfo.bmiColors[ncol];
					if (0 == ntimes_masked)
						src_pp ++;
					pix::iterate_line((char*&)pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A2B10G10R10:
			{
				typedef pixeler<30, 2, 0, 10, 10, 10, 20, 10, 4, unsigned int> pix;
				for (DWORD x = 0; x < width; x++)
				{
					unsigned char ntimes_masked = ntimes-- & times_mask;
					unsigned char shift = (ntimes_masked) << shift_shift;
					unsigned char ncol = mask & (*src_pp >> shift);
					smart_color* color = (smart_color*)&bminfo.bmiColors[ncol];
					if (0 == ntimes_masked)
						src_pp ++;
					pix::iterate_line((char*&)pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		case D3DFMT_A4R4G4B4:
			{
				typedef pixeler<12, 4, 8, 4, 4, 4, 0, 4, 2, unsigned short> pix;
				for (DWORD x = 0; x < width; x++)
				{
					unsigned char ntimes_masked = ntimes-- & times_mask;
					unsigned char shift = (ntimes_masked) << shift_shift;
					unsigned char ncol = mask & (*src_pp >> shift);
					smart_color* color = (smart_color*)&bminfo.bmiColors[ncol];
					if (0 == ntimes_masked)
						src_pp ++;
					pix::iterate_line((char*&)pp, color->r, color->g, color->b, 0);
				}
			}
			break;
		}
		p += pitch;
		src += src_pitch;
	}
}
