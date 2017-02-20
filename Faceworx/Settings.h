#pragma once

class CSettings
{
	CString ini_fn;
public:
	CSettings(void);
	~CSettings(void);
	void Save();
	void Load();
	//
	bool bRightButtonMoving;
	int iInitialModelDetail;
	//
	static CSettings& Get()
	{
		static CSettings settings;
		return settings;
	}
};
