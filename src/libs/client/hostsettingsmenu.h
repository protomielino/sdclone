#pragma once
#include "menuscreen.h"

class HostMenuSettings: public MenuScreen
{
public:

	HostMenuSettings(){ m_bCollisions = true;};
	bool Init(void *pPrevMenu);
	void Activate(void* p);
protected:
	//callback functions must be static
	static void onActCB(void *p);
	static void onAcceptCB(void *p);
	static void onCancelCB(void *p);
	static void CarControlCB(tChoiceInfo * pInfo);
	static void CarCollideCB(tChoiceInfo * pInfo);
	static void humanhostCB(tChoiceInfo *pChoices);
	static void onPlayerReady(void *p);

	static  std::string m_strCarCat;
	static bool m_bCollisions;
	static bool m_bHumanHost;

};

