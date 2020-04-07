/***************************************************************************

    file                 : carsettupmenu.h
    created              : April 2020
    copyright            : (C) 2020 Robert Reif
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

#ifndef _CARSETUPMENU_H_
#define _CARSETUPMENU_H_

#include <tgfclient.h>

#include "confscreens.h"

class GfRace;
class GfDriver;
class GfCar;
class GfTrack;

class CarSetupMenu : public GfuiMenuScreen
{
public:

	CarSetupMenu();
	bool initialize(void *pPrevMenu,
                    const GfRace *pRace,
                    const GfDriver *pDriver);

    void storeSettings();
    void loadSettings();

    void updateControls();

    const GfCar *getCar() const { return _pDriver->getCar(); }
    const GfTrack *getTrack() const { return _pRace->getTrack(); }

protected:

	//callback functions must be static
	static void onActivate(void *pMenu);
	static void onAccept(void *pMenu);
	static void onCancel(void *pMenu);
	static void onReset(void *pMenu);

    // The target race.
    const GfRace *_pRace;

    // The target driver.
    const GfDriver *_pDriver;

    struct attnum
    {
        int         labelId;
        int         editId;
        int         defaultLabelId;
        bool        exists;
        tdble       value;
        tdble       minValue;
        tdble       defaultValue;
        tdble       maxValue;
        std::string section;
        std::string param;
        std::string units;

        attnum() :
            labelId(0), editId(0), defaultLabelId(0), exists(false),
            value(0), minValue(0), defaultValue(0), maxValue(0)
        {
        }
    };

    void updateControl(const attnum &att);
    void loadSetting(const char *label, const char *edit, const char *defaultLabel,
                     void *hparmCar, void *hparmCarSetup, attnum &att,
                     const char *section, const char *param, const char *labelStr,
                     const char *units);
    void storeSetting(void *hparmCarSetup, attnum &att);

    attnum  brakeRepartition;
    attnum  frontARB;
    attnum  rearARB;
    attnum  frontWing;
    attnum  rearWing;
    attnum  rearDiffSlip;
    attnum  rearDiffCoastSlip;
};

#endif /* _CARSETUPMENU_H_ */
