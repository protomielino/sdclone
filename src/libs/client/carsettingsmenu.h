#pragma once
#include "menuscreen.h"

class CarMenuSettings: public MenuScreen
{
public:

	CarMenuSettings(){};
	bool Init(void *pPrevMenu,const char *pzaCar);
	void Activate(void* p);
protected:
	//callback functions must be static
	static void onActCB(void *p);
	static void onAcceptCB(void *p);
	static void onCancelCB(void *p);
	static void CarPickCB(tChoiceInfo * pInfo);

	static std::string m_strCar;
};

