/***************************************************************************

    file                 : creditsmenu.cpp
    created              : Tue Mar 3 12:00:00 CEST 2009
    copyright            : (C) 2009 by Jean-Philippe Meuret
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

#include <cstring>
#include <map>
#include <vector>

#include <tgfclient.h>


// Max number of screen lines in a credits page.
static const unsigned MaxScreenLinesPerPage = 16;

// Description of a columns in the credits XML file
typedef struct
{
    // Displayed name.
    const char *name;
    
    // Display width (in screen pixels 640x400).
    int	width;

} tColumnDesc;


// Parameter structure for the page change callback.
typedef struct
{
    // Handle of the screen of the page that is about to be closed/released.
    void	*prevPageScrHdle;
    
    // Index of the starting chapter for the requested page.
    int		startChapterIndex;

    // Index of the starting line for the requested page.
    int		startLineIndex;

} tPageChangeRequest;

// Previous and next parameters for the page change callback.
static tPageChangeRequest	NextPageRequest;
static tPageChangeRequest	PrevPageRequest;


// Handle of the screen to return when exiting from this credits pages.
static void* RetScrHdle = 0;

// Internal function declarations.
static void* creditsPageCreate(int startChapterIndex, int startLineIndex);

// Page change callback.
static void creditsPageChange(void *vpcr)
{
    tPageChangeRequest *pcr = (tPageChangeRequest*)vpcr;
    
    // Get the screen handle of the currently activated page.
    void* prevPageScrHdle = pcr->prevPageScrHdle;
    
    // Create and activate requested page screen : it is now the currently activated page.
    GfuiScreenActivate(creditsPageCreate(pcr->startChapterIndex, pcr->startLineIndex));
    
    // Release the previously activated page screen.
    GfuiScreenRelease(prevPageScrHdle);
}

// Create a credits page screen for given chapter and starting line.
static void* creditsPageCreate(int startChapterIndex, int startLineIndex)
{
    static const unsigned maxBufSize = 256;
    static char	buf[maxBufSize];

    // TODO: Get 'colNameColor' property value from XML when available.
    static float colNameColor[4] = {1.0, 0.0, 1.0, 1.0};

    // Open and parse credits file
    sprintf(buf, "%s%s", GfDataDir(), "credits.xml");
    void* crHdle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
    if (!crHdle)
        return 0;

    // Get the number of chapters
    const int nChapters = (int)GfParmGetEltNb(crHdle, "chapters");
    if (startChapterIndex < 0 || startChapterIndex >= nChapters)
	return 0;
    
    // Get requested chapter info
    sprintf(buf, "chapters/%d", startChapterIndex);
    const char* chapName = GfParmGetStr(crHdle, buf, "name", "<no name>");

    sprintf(buf, "chapters/%d/lines", startChapterIndex);
    const int nLinesInChapter = (int)GfParmGetEltNb(crHdle, buf);
    if (startLineIndex >= nLinesInChapter)
	return 0;
    
    // Create screen, load menu XML descriptor and create static controls.
    void* pageScrHdle = GfuiScreenCreate();

    void *menuXMLDescHdle = LoadMenuXML("creditsmenu.xml");

    CreateStaticControls(menuXMLDescHdle, pageScrHdle);

    // Create title label from chapter name
    sprintf(buf, "Credits - %s", chapName);
    const int titleId = CreateLabelControl(pageScrHdle, menuXMLDescHdle, "title");
    GfuiLabelSetText(pageScrHdle, titleId, buf);
    
    // Get columns info (names, width and line index) and display column titles
    // (each chapter line may need more than 1 screen line, given the column width sum ...)
    sprintf(buf, "chapters/%d/columns", startChapterIndex);
    const int nColsInChapter = (int)GfParmGetEltNb(crHdle, buf);
    if (nColsInChapter <= 0)
	return 0;

    std::map<const char*, tColumnDesc> columns;
    std::vector<const char*> orderedColumnIds;

    int x0 = 20; // TODO: Get 'xLeft1stCol' property value from XML when available.
    int x = x0;
    int y = 400; // TODO: Get 'yBottom1stLine' property value from XML when available.
    int nScreenLinesPerLine = 1;
    sprintf(buf, "chapters/%d/columns", startChapterIndex);
    GfParmListSeekFirst(crHdle, buf);
    do
    {
	tColumnDesc column;
	column.name = GfParmGetCurStr(crHdle, buf, "name", "<no name>");
	column.width = GfParmGetCurNum(crHdle, buf, "width", 0, 20); 
	// TODO: Get 'xRightLastCol' property value from XML when available.
	if (x >= 600+20) // Do we need 1 more screen line for the current credits line ?
	{
	    x0 += 10; // TODO: Get 'xSubLineShift' property value from XML when available.
	    x = x0;
	    y -= 17; // TODO: Get 'ySubLineShift' property value from XML when available.
	    nScreenLinesPerLine++;
	} 
	
	const char* colId = GfParmListGetCurEltName(crHdle, buf);
	GfuiLabelCreate(pageScrHdle, column.name, GFUI_FONT_MEDIUM_C, 
					x, y, GFUI_ALIGN_HL_VB, 0, colNameColor);
	x += column.width;
	orderedColumnIds.push_back(colId);
	columns.insert(std::pair<const char*, tColumnDesc>(colId, column));
    }
    while (GfParmListSeekNext(crHdle, buf) == 0);

    // Display each column of each line
    const int maxLinesPerPage = MaxScreenLinesPerPage / nScreenLinesPerLine;
    if (startLineIndex < 0)
	startLineIndex = maxLinesPerPage * ((nLinesInChapter - 1) / maxLinesPerPage);
    int nLineInd = startLineIndex;
    int nScreenLines = 0; 
    for (; nLineInd < nLinesInChapter && nLineInd - startLineIndex < maxLinesPerPage; nLineInd++)
    {
	x0 = x = 20; // TODO: Get 'xLeft1stCol' property value from XML when available.
	y -= 20;// TODO: Get 'yLineShift' property value from XML when available.
	sprintf(buf, "chapters/%d/lines/%d", startChapterIndex, nLineInd);
	std::vector<const char*>::const_iterator colIdIter;
	for (colIdIter = orderedColumnIds.begin(); colIdIter != orderedColumnIds.end(); colIdIter++)
	{
	    const char* colValue = GfParmGetStr(crHdle, buf, *colIdIter, "");
	    if (x >= 600+20) // Do we need 1 more screen line for the current credits line ?
	    {
		x0 += 10; // TODO: Get 'xSubLineShift' property value from XML when available.
		x = x0;
		y -= 17; // TODO: Get 'ySubLineShift' property value from XML when available.
		nScreenLines++;
	    }
	    GfuiLabelCreate(pageScrHdle, colValue, GFUI_FONT_MEDIUM_C,
						x, y, GFUI_ALIGN_HL_VB, 0);
	    x += columns[*colIdIter].width;
	}
	while (GfParmListSeekNext(crHdle, buf) == 0);
    }

    // Close credits file
    GfParmReleaseHandle(crHdle);
    
    // Create "Previous page" button if not the first page.
    if (startLineIndex > 0 || startChapterIndex > 0)
    {
	PrevPageRequest.prevPageScrHdle = pageScrHdle;
	if (startLineIndex > 0)
	{
	    PrevPageRequest.startChapterIndex = startChapterIndex;
	    PrevPageRequest.startLineIndex    = startLineIndex - maxLinesPerPage;
	}
	else
	{
	    PrevPageRequest.startChapterIndex = startChapterIndex - 1;
	    PrevPageRequest.startLineIndex    = -1;
	}
	CreateButtonControl(pageScrHdle, menuXMLDescHdle, "previouspagearrow",
			    (void*)&PrevPageRequest, creditsPageChange);
	GfuiAddKey(pageScrHdle, GFUIK_PAGEUP, "Previous page", 
		    (void*)&PrevPageRequest, creditsPageChange, NULL);
    }
    
    // Add "Continue" button (credits screen exit).
    CreateButtonControl(pageScrHdle, menuXMLDescHdle, "backbutton", RetScrHdle, GfuiScreenReplace);
    
    // Add "Next page" button if not the last page.
    if (nLineInd < nLinesInChapter || startChapterIndex + 1 < nChapters) 
    {
	NextPageRequest.prevPageScrHdle = pageScrHdle;
	if (nLineInd < nLinesInChapter) 
	{
	    NextPageRequest.startChapterIndex = startChapterIndex;
	    NextPageRequest.startLineIndex    = startLineIndex + maxLinesPerPage;
	}
	else
	{
	    NextPageRequest.startChapterIndex = startChapterIndex + 1;
	    NextPageRequest.startLineIndex    = 0;
	}
	CreateButtonControl(pageScrHdle, menuXMLDescHdle, "nextpagearrow",
			    (void*)&NextPageRequest, creditsPageChange);
	GfuiAddKey(pageScrHdle, GFUIK_PAGEDOWN, "Next Page", 
		    (void*)&NextPageRequest, creditsPageChange, NULL);
    }

    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Add standard keyboard shortcuts.
    GfuiAddKey(pageScrHdle, GFUIK_ESCAPE, "Return to previous menu", 
	       RetScrHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(pageScrHdle, GFUIK_RETURN, "Return to previous menu", 
	       RetScrHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(pageScrHdle, GFUIK_F1, "Help", 
		pageScrHdle, GfuiHelpScreen, NULL);
    GfuiAddKey(pageScrHdle, GFUIK_F12, "Take a Screen Shot", 
		NULL, GfuiScreenShot, NULL);
    
    return pageScrHdle;
}

// Menu entry.
void CreditsMenuActivate(void *retScrHdle)
{
    // Store return screen handle.
    RetScrHdle = retScrHdle;
    
    // Create first page screen and return its handle.
    GfuiScreenActivate(creditsPageCreate(0, 0));
}
