/***************************************************************************

    file                 : credits.cpp
    created              : Tue Mar 3 12:00:00 CEST 2009
    copyright            : (C) 2009 by Jean-Philippe Meuret
    web                  : torcs-ng.sourceforge.net

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

// Max number of screen lines in a credit page.
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

// Create a credit page screen for given chapter and starting line.
static void* creditsPageCreate(int startChapterIndex, int startLineIndex)
{
    static const unsigned maxBufSize = 256;
    static char	buf[maxBufSize];
    //static char	bufColInd[4];

    static float colNameColor[4] = {1.0, 0.0, 1.0, 1.0};

    // Open and parse credits file
    sprintf(buf, "%s%s", GetDataDir(), "credits.xml");
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
    
    // Create screen, set title from chapter name, and set background image
    void* pageScrHdle = GfuiScreenCreate();

    sprintf(buf, "Credits - %s", chapName);
    GfuiTitleCreate(pageScrHdle, buf, strlen(buf));
    GfuiScreenAddBgImg(pageScrHdle, "data/img/splash-dtm.png"); // TODO : special credits background image.
    
    // Get columns info (names, width and line index) and display column titles
    // (each chapter line may need more than 1 screen line, given the column width sum ...)
    sprintf(buf, "chapters/%d/columns", startChapterIndex);
    const int nColsInChapter = (int)GfParmGetEltNb(crHdle, buf);
    if (nColsInChapter <= 0)
	return 0;

    std::map<const char*, tColumnDesc> columns;
    std::vector<const char*> orderedColumnIds;

    int x0 = 20;
    int x = x0;
    int y = 400;
    int nScreenLinesPerLine = 1;
    sprintf(buf, "chapters/%d/columns", startChapterIndex);
    GfParmListSeekFirst(crHdle, buf);
    do
    {
	tColumnDesc column;
	column.name = GfParmGetCurStr(crHdle, buf, "name", "<no name>");
	column.width = GfParmGetCurNum(crHdle, buf, "width", 0, 20);
	if (x >= 600+20) // Do we need 1 more screen line for the current credits line ?
	{
	    x0 += 10;
	    x = x0;
	    y -= 17;
	    nScreenLinesPerLine++;
	} 
	
	const char* colId = GfParmListGetCurEltName(crHdle, buf);
	GfuiLabelCreateEx(pageScrHdle, column.name, colNameColor, GFUI_FONT_MEDIUM_C, 
			  x, y, GFUI_ALIGN_HL_VB, 0);
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
	x0 = x = 20;
	y -= 20;
	sprintf(buf, "chapters/%d/lines/%d", startChapterIndex, nLineInd);
	std::vector<const char*>::const_iterator colIdIter;
	for (colIdIter = orderedColumnIds.begin(); colIdIter != orderedColumnIds.end(); colIdIter++)
	{
	    const char* colValue = GfParmGetStr(crHdle, buf, *colIdIter, "");
	    if (x >= 600+20) // Do we need 1 more screen line for the current credits line ?
	    {
		x0 += 10;
		x = x0;
		y -= 17;
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
    
    // Add "Previous page" button if not the first page.
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
	GfuiGrButtonCreate(pageScrHdle, "data/img/arrow-up.png", "data/img/arrow-up.png",
			   "data/img/arrow-up.png", "data/img/arrow-up-pushed.png",
			   80, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&PrevPageRequest, creditsPageChange,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(pageScrHdle, GLUT_KEY_PAGE_UP, "Previous page", 
		    (void*)&PrevPageRequest, creditsPageChange, NULL);
    }
    
    // Add "Continue" button (credits screen exit).
    GfuiButtonCreate(pageScrHdle, "Continue", GFUI_FONT_LARGE, 320, 40, 150, GFUI_ALIGN_HC_VB, 0,
		     RetScrHdle, GfuiScreenReplace, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    
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
	GfuiGrButtonCreate(pageScrHdle, "data/img/arrow-down.png", "data/img/arrow-down.png",
			   "data/img/arrow-down.png", "data/img/arrow-down-pushed.png",
			   540, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&NextPageRequest, creditsPageChange,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(pageScrHdle, GLUT_KEY_PAGE_DOWN, "Next Page", 
		    (void*)&NextPageRequest, creditsPageChange, NULL);
    }

    // Add standard keyboard shortcuts.
    GfuiAddKey(pageScrHdle, (unsigned char)27, "Exit Credits Screen", 
	       RetScrHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(pageScrHdle, (unsigned char)13, "Exit Credits Screen", 
	       RetScrHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(pageScrHdle, GLUT_KEY_F12, "Take a Screen Shot", 
		NULL, GfuiScreenShot, NULL);
    
    return pageScrHdle;
}

// Menu entry.
void TorcsCreditsScreenActivate(void *retScrHdle)
{
    // Store return screen handle.
    RetScrHdle = retScrHdle;
    
    // Create first page screen and return its handle.
    GfuiScreenActivate(creditsPageCreate(0, 0));
}
