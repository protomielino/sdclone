/***************************************************************************

    file                 : carselect.h
    created              : July 2010
    copyright            : (C) 2010 Jean-Philippe Meuret
    web                  : speed-dreams.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CARSELECT_H_
#define _CARSELECT_H_

#include <map>

#include <carinfo.h>
#include <tgfclient.h>

#include "racescreens.h"

struct rmdDrvElt;


class RACESCREENS_API RmCarSelectMenu : public GfuiMenuScreen
{
public:

	RmCarSelectMenu();
	void RunMenu(trmdDrvElt* pDriver);
	
	void resetCarCategoryComboBox(const std::string& strSelectedCategoryName = "");
	void resetCarModelComboBox(const std::string& strCategoryName,
							   const std::string& strSelectedRealCarName = "");
	void resetCarDataSheet(const std::string& strSelectedCarName);
	void resetCarSkinComboBox(const std::string& strRealCarName,
							  const std::string& strSelectedSkinName = "");
	void resetCarPreviewImage(const std::string& strSelectedSkinName = "");
	
protected:
	
	bool Initialize();

	const struct rmdDrvElt* getDriver() const;
	struct rmdDrvElt* getDriver();
	void setDriver(struct rmdDrvElt* pDriver);

	const CarData* getSelectedCarModel() const;
	const char* getSelectedCarSkin() const;
	int getSelectedCarSkinTargets() const;

	// Control callback functions (have to be static, as used as tgfclient controls callbacks).
	static void onActivateCB(void *pCarSelectMenu);
	static void onChangeCategory(tComboBoxInfo *pInfo);
	static void onChangeModel(tComboBoxInfo *pInfo);
	static void onChangeSkin(tComboBoxInfo *pInfo);
	static void onGarageCB(void *pCarSelectMenu);
	static void onAcceptCB(void *pCarSelectMenu);
	static void onCancelCB(void *pCarSelectMenu);

private:

	// The target driver.
	struct rmdDrvElt* _pDriver;

	// Currently selected car params handle.
	//void* _hCarParams;
	
	// Skin names and targets + associated skinned livery preview files
	std::vector<std::string> _vecSkinNames;
	std::map<std::string, int> _mapSkinTargets;
	std::map<std::string, std::string> _mapPreviewFiles;
	size_t _nCurSkinIndex;

};

#endif //_CARSELECT_H_
