/***************************************************************************

    file        : genetic.cpp
    created     : Sun Nov 06 09:15:00 CET 2011
    copyright   : (C) 2011-2013 by Wolf-Dieter Beelitz
    email       : wdb@wdbee.de
    version     : $Id: genetic.cpp 3657 2011-11-06 09:15:00Z wdbee $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		Helper for parameter handling while optimizations
    @author	<a href=mailto:wdb@wdbee.de>WDBee</a>
    @version	$Id: genetic.cpp 3657 2011-11-06 09:15:00Z wdbee $
*/

#include <float.h>
#include <tgf.h>

#include "genetic.h"
#include "portability.h"



//
// Class Genetic Parameter
//

// Default constructor
TGeneticParameter::TGeneticParameter():
	Handle(0),
	Active(false),
	Min(FLT_MIN),
	Max(FLT_MAX),
	Def(0.0),
	Val(0.0),
	Weight(1.0),
	Scale(1.0),
	Round(1.0),
	oLabel(NULL),
	oSection(NULL),
	oParameter(NULL),
	oUnit(NULL),
	Tries(0),
	Changed(0),
	Selected(false),
	LeftRight(false)
{
	Range = Max - Min;
};

// Constructor
TGeneticParameter::TGeneticParameter
(
	void* MetaDataFile,
	float MinDef,
	float ValDef,
	float MaxDef,
	const char *Label,
	const char *Section,
	const char *Parameter,
	const char *Unit,
	float ParamWeight,
	float ParamScale,
	float ParamRound,
	bool TwoSided
)
{
	Handle = MetaDataFile;
    Active = true;

	oLabel = strdup(Label);
	oSection = strdup(Section);
	oParameter = strdup(Parameter);
	if (Unit != 0)
	  oUnit = strdup(Unit);
	else
  	  oUnit = NULL;

	Min = GfParmGetNumMin(Handle, 
		Section, Parameter, Unit, MinDef);
	Max = GfParmGetNumMax(Handle, 
		Section, Parameter, Unit, MaxDef);
	Def = ValDef;
	Val = GfParmGetNum(Handle, 
		Section, Parameter, Unit, Def);
	Weight = ParamWeight;
	Range = Max - Min;
    Scale = ParamScale;
	Round = ParamRound;

	Tries = 0;
	Changed = 0;
	Selected = false;
	
	if (Range < 0.000001)
	{
		Min = MinDef;
		Max = MaxDef;
		Range = Max - Min;
	}

	LeftRight = TwoSided;

};

// Destructor
TGeneticParameter::~TGeneticParameter()
{
    DisplayStatistik();

	if (oLabel)
		free((void *) oLabel);
	if (oSection)
		free((void *) oSection);
	if (oParameter)
		free((void *) oParameter);
	if (oUnit)
		free((void *) oUnit);
};

// Display parameter definitions at console
void TGeneticParameter::DisplayParameter()
{
	GfLogOpt("%s: Min=%g Val=%g Max=%g Def=%g W=%g S=%g ,R=1/%g\n",oLabel,Min,Val,Max,Def,Weight,Scale,Round);
};

// Display parameter statistics at console
void TGeneticParameter::DisplayStatistik()
{
	GfLogOpt("%s: N=%d M=%d (%g %%)\n",oLabel,Tries,Changed,(100.0 * Changed)/Tries);
};

// Write parameter meta data to xml file
int TGeneticParameter::Set(const char* Part, int Index)
{
	char ParamSection[64];
	if (Part == NULL)
	  sprintf(ParamSection,"global/%d",Index);
	else
	  sprintf(ParamSection,"%s/%d",Part,Index);

	GfParmSetNum(Handle, ParamSection, "active", 0, (float) Active);
	GfParmSetNum(Handle, ParamSection, "twosided", 0, (float) LeftRight);

	GfParmSetStr(Handle, ParamSection, "label", oLabel);
	GfParmSetStr(Handle, ParamSection, "section", oSection);
	GfParmSetStr(Handle, ParamSection, "parameter", oParameter);
	GfParmSetStr(Handle, ParamSection, "unit", oUnit);

//	GfParmSetNumWithMinMax(Handle, ParamSection, "range", oUnit, Val, Min, Max);
	GfParmSetNumEx(Handle, ParamSection, "range", oUnit, Val, Min, Max);

	GfParmSetNum(Handle, ParamSection, "weight", 0, Weight);
	GfParmSetNum(Handle, ParamSection, "scale", 0, Scale);
	GfParmSetNum(Handle, ParamSection, "round", 0, Round);

	return 0;
};

// Read parameter meta data from xml file
int TGeneticParameter::Get(const char* Part, int Index)
{
	if (oLabel)
		free((void *) oLabel);
	if (oSection)
		free((void *) oSection);
	if (oParameter)
		free((void *) oParameter);
	if (oUnit)
		free((void *) oUnit);

	char ParamSection[64];
	if (Part == NULL)
	  sprintf(ParamSection,"global/%d",Index);
	else
	  sprintf(ParamSection,"%s/%d",Part,Index);

	Active = GfParmGetNum(Handle, ParamSection, "active", 0, 1) > 0;
	LeftRight = GfParmGetNum(Handle, ParamSection, "twosided", 0, 0) > 0;

	oLabel = strdup((char *) GfParmGetStr(Handle, ParamSection, "label", oLabel));
	oSection = strdup((char *) GfParmGetStr(Handle, ParamSection, "section", oSection));
	oParameter = strdup((char *) GfParmGetStr(Handle, ParamSection, "parameter", oParameter));
	oUnit = strdup((char *) GfParmGetStr(Handle, ParamSection, "unit", oUnit));

	Min = GfParmGetNumMin(Handle, ParamSection, "range", oUnit, Min);
	Max = GfParmGetNumMax(Handle, ParamSection, "range", oUnit, Max);
	Val = GfParmGetNum(Handle, ParamSection, "range", oUnit, Val);

	Weight = GfParmGetNum(Handle, ParamSection, "weight", 0, Weight);
	Scale = GfParmGetNum(Handle, ParamSection, "scale", 0, Scale);
	Round = GfParmGetNum(Handle, ParamSection, "round", 0, Round);

    Range = Max - Min;
	LastVal = OptVal = Val;

	return 0;
};

// Read initial value from setup file
int TGeneticParameter::GetVal(void* SetupHandle, int Index)
{
	char ParamSection[64];
	if (Index > 0)
	  sprintf(ParamSection,"%s/%d",oSection,Index);
	else
	  sprintf(ParamSection,"%s",oSection,Index);

	if (LeftRight)
	{
		char SideParam[64];
		sprintf(SideParam,ParamSection,"Left");
		Val = GfParmGetNum(SetupHandle, SideParam, oParameter, oUnit, Val);

		sprintf(SideParam,ParamSection,"Right");
		Val = (Val + GfParmGetNum(SetupHandle, SideParam, oParameter, oUnit, Val)) / 2;

	}
	else
		Val = GfParmGetNum(SetupHandle, oSection, oParameter, oUnit, Val);

	LastVal = OptVal = Val;

	return 0;
};

// Write parameter data to xml file
int TGeneticParameter::SetVal(void* SetupHandle, int Index)
{
	char ParamSection[64];
	if (Index > 0)
	  sprintf(ParamSection,"%s/%d",oSection,Index);
	else
	  sprintf(ParamSection,"%s",oSection,Index);

	if (LeftRight)
	{
		char SideParam[64];

		sprintf(SideParam,ParamSection,"Left");
		GfParmSetNumEx(SetupHandle, SideParam, oParameter, oUnit, Val, Min, Max);

		sprintf(SideParam,ParamSection,"Right");
		return GfParmSetNumEx(SetupHandle, SideParam, oParameter, oUnit, Val, Min, Max);
	}
	else
		return GfParmSetNumEx(SetupHandle, ParamSection, oParameter, oUnit, Val, Min, Max);
}


//
// Class Genetic Parameter Counter
//

// Default constructor
TGeneticParameterCounter::TGeneticParameterCounter():
	Handle(0),
	Count(0),
	oLabel(NULL),
	oSection(NULL),
	oParameter(NULL)
{
};

// Constructor
TGeneticParameterCounter::TGeneticParameterCounter
(
	void* MetaDataFile,
	const char *Label,
	const char *Section,
	const char *Parameter,
	int DefCount
)
{
	Handle = MetaDataFile;

	oLabel = strdup(Label);
	oSection = strdup(Section);
	oParameter = strdup(Parameter);

	Def = DefCount;
	Active = 1 > GfParmGetNum(Handle, 
		oSection, "active", 0, (float) Active);
	Count = (int) GfParmGetNum(Handle, 
		oSection, oParameter, 0, (float) DefCount);

	if (Count < 0)
		Count = -Count;
};

// Destructor
TGeneticParameterCounter::~TGeneticParameterCounter()
{
	free ((void *) oLabel);
	free ((void *) oSection);
	free ((void *) oParameter);
};

// Write counter meta data to xml file
int TGeneticParameterCounter::Set(int Index)
{
	char ParamSection[64];
	sprintf(ParamSection,"part/%d/counter",Index);

	GfParmSetNum(Handle, ParamSection, "active", 0, (float) Active);
	GfParmSetStr(Handle, ParamSection, "name", oLabel);
	GfParmSetStr(Handle, ParamSection, "section", oSection);
	GfParmSetStr(Handle, ParamSection, "parameter", oParameter);
	GfParmSetNum(Handle, ParamSection, "counter", 0, (float) Count);

	return 0;
};

// Read counter meta data from xml file
int TGeneticParameterCounter::Get(int Index)
{
	if (oLabel)
		free((void *) oLabel);
	if (oSection)
		free((void *) oSection);
	if (oParameter)
		free((void *) oParameter);

	char ParamSection[64];
	sprintf(ParamSection,"part/%d/counter",Index);

	Active = GfParmGetNum(Handle, ParamSection, "active", 0, (float) Active) > 0;
	oLabel = strdup((char *) GfParmGetStr(Handle, ParamSection, "name", oLabel));
	oSection = strdup((char *) GfParmGetStr(Handle, ParamSection, "section", oSection));
	oParameter = strdup((char *) GfParmGetStr(Handle, ParamSection, "parameter", oParameter));
	Count = (int) GfParmGetNum(Handle, ParamSection, "counter", 0, (float) Count);

	return 0;
};


//
// Class Genetic Parameter Table of Content
//

// Default constructor
TGeneticParameterTOC::TGeneticParameterTOC():
	Handle(0),
	GlobalParamCount(0),     
	ParamsGroupCount(0)
{
};        

// Constructor
TGeneticParameterTOC::TGeneticParameterTOC           
(
	void* MetaDataFile,			// Handle to read/write data
	int NbrOfGlobalParams,		// Number of global genetic parameters
	int NbrOfParamsGroups,		// Number of local parameters groups
	float WeightDamages,		// Weight of damages
	bool InitialVal				// get initial value from setup file
)
{
	Handle = MetaDataFile;
	GlobalParamCount = NbrOfGlobalParams;     
	ParamsGroupCount = NbrOfParamsGroups;
	WeightOfDamages = WeightDamages;
	GetInitialVal = InitialVal;
};

// Destructor
TGeneticParameterTOC::~TGeneticParameterTOC()
{
	// free allocated mem
}

// Write table of content to configuration file 
int TGeneticParameterTOC::Set()
{
	GfParmSetNum(Handle, 
		"table of content", "global param count", 0, (float) GlobalParamCount);
	GfParmSetNum(Handle, 
		"table of content", "params group count", 0, (float) ParamsGroupCount);
	GfParmSetNum(Handle, 
		"table of content", "weight of damages", 0, (float) WeightOfDamages);
	if (GetInitialVal)
	  GfParmSetNum(Handle, 
	  	"table of content", "get initial value", 0, 1);
	else
	  GfParmSetNum(Handle, 
	  	"table of content", "get initial value", 0, 0);

	return 0;
}; 

// Read table of content from configuration file 
int TGeneticParameterTOC::Get() 
{
	GlobalParamCount = (int) GfParmGetNum(Handle, 
		"table of content", "global param count", 0, (float) GlobalParamCount);
	ParamsGroupCount = (int) GfParmGetNum(Handle, 
		"table of content", "params group count", 0, (float) ParamsGroupCount);
	WeightOfDamages = GfParmGetNum(Handle, 
		"table of content", "weight of damages", 0, (float) WeightOfDamages);
	GetInitialVal = 0 < GfParmGetNum(Handle, 
		"table of content", "get initial value", 0, 1);

	return 0;
}; 

// end of file genetic.cpp