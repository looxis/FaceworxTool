#pragma once

#include "types.h"

class texture_blender
{
public:
	texture_blender(void);
	~texture_blender(void);
	HRESULT blend(IDirect3DTexture9* tex,CPointFArray& line1,CPointFArray& line2);
};
