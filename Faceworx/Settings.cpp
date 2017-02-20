#include "StdAfx.h"
#include "Settings.h"

CSettings::CSettings(void)
{
	wchar_t buf[_MAX_PATH * 2];
	GetCurrentDirectory(sizeof buf, buf);
	ini_fn = buf;
	ini_fn += L"\\voxelworx.ini";

	iInitialModelDetail = 1;
	bRightButtonMoving = false;
}

CSettings::~CSettings(void)
{
}

#define SWITCH(x,y) \
	if (strncmp(x, y, sizeof(y) - 1) == 0 && (x += (sizeof(y) - 1), true))

void CSettings::Load()
{
	FILE* f = 0;
	//_wfopen_s(&f, ini_fn, L"rt");
	f = _wfopen(ini_fn, L"rt");
	if (0 == f)
		return;
	char buf[512];
	while (!feof(f))
	{
		fgets(buf, sizeof buf, f);
		char* p = buf;
		SWITCH(p, "Right button moving")
		{
			if (char* q = strchr(p, '='))
				p = q + 1;
			bRightButtonMoving = 0 != atoi(p);
		} else
		SWITCH(p, "Initial model detail")
		{
			if (char* q = strchr(p, '='))
				p = q + 1;
			iInitialModelDetail = atoi(p);
			if (iInitialModelDetail < 1 || iInitialModelDetail > 3)
				iInitialModelDetail = 1;
		}
	}
	fclose(f);
}

void CSettings::Save()
{
	FILE* f = 0;
	//_wfopen_s(&f, ini_fn, L"rt");
	f = _wfopen(ini_fn, L"rt");
	if (0 == f)
		return;
	fprintf(f,"Right button moving = ", (int)bRightButtonMoving);
	fprintf(f,"Initial model detail = ", iInitialModelDetail);
	fclose(f);
}