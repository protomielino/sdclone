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

#include <carinfo.h>
#include <tgfclient.h>

#include "driver.h"
#include "racescreens.h"


class RACESCREENS_API RmCarSelectMenu : public GfuiMenuScreen
{
public:

	RmCarSelectMenu();
	bool Initialize();
	void RunMenu(const trmdDrvElt* pDriver);
	
	void resetCarCategoryComboBox(const std::string& strSelectedCategoryName = "");
	void resetCarModelComboBox(const std::string& strCategoryName,
							   const std::string& strSelectedRealCarName = "");
	void resetCarSkinComboBox(const std::string& strRealCarName,
							  const std::string& strSelectedSkinName = "");
	
protected:
	
	const trmdDrvElt* getDriver() const;
	void setDriver(const trmdDrvElt* pDriver);

	const CarData* getSelectedCarModel() const;

	// Control callback functions (must be static).
	static void onActivateCB(void *pCarSelectMenu);
	static void onChangeCategory(tComboBoxInfo *pInfo);
	static void onChangeModel(tComboBoxInfo *pInfo);
	static void onChangeSkin(tComboBoxInfo *pInfo);
	static void onGarageCB(void *pCarSelectMenu);
	static void onAcceptCB(void *pCarSelectMenu);
	static void onCancelCB(void *pCarSelectMenu);

private:

	// The target driver.
	const trmdDrvElt* m_pDriver;
};

#endif //_CARSELECT_H_
