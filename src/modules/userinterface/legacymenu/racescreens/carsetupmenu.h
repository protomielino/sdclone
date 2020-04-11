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

#include <array>

#include <tgfclient.h>

#include "confscreens.h"

class GfRace;
class GfDriver;
class GfCar;
class GfTrack;

#define ITEMS_PER_PAGE  10

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
        std::string label;
        int         precision;

        attnum() :
            labelId(0), editId(0), defaultLabelId(0), exists(false),
            value(0), minValue(0), defaultValue(0), maxValue(0), precision(0)
        {
        }
    };

protected:

	//callback functions must be static
	static void onActivate(void *pMenu);
	static void onAccept(void *pMenu);
	static void onCancel(void *pMenu);
	static void onReset(void *pMenu);
	static void onPrevious(void *pMenu);
	static void onNext(void *pMenu);

    // The target race.
    const GfRace *_pRace;

    // The target driver.
    const GfDriver *_pDriver;

    void readCurrentPage();

    std::vector<std::array<attnum, ITEMS_PER_PAGE> > items;
    size_t  currentPage;
};

#endif /* _CARSETUPMENU_H_ */
