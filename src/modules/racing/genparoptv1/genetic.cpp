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

// Class variables
tgenData TGeneticParameter::Data;	// Structure with all data

// Default constructor
TGeneticParameter::TGeneticParameter():
	Handle(0),
	Active(false),
	Min(FLT_MIN),
	Max(FLT_MAX),
	Val(0.0),
	Def(0.0),
	Weight(1.0),
	Scale(1.0),
	Round(1.0),
	Tries(0),
	Changed(0),
	Selected(false),
	LeftRight(false),
	oLabel(NULL),
	oSection(NULL),
	oParameter(NULL),
	oUnit(NULL)
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

	if (Label)
		oLabel = strdup(Label);
	else
		oLabel = NULL;

	if (Section)
		oSection = strdup(Section);
	else
		oSection = NULL;

	if (Parameter)
		oParameter = strdup(Parameter);
	else
		oParameter = NULL;

	if (Unit)
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
	  sprintf(ParamSection,"%s/%d",SECT_GLOBAL,Index);
	else
	  sprintf(ParamSection,"%s/%d",Part,Index);

	GfParmSetNum(Handle, ParamSection, PRM_ACTIVE, 0, (float) Active);
	GfParmSetNum(Handle, ParamSection, PRM_TWOSIDE, 0, (float) LeftRight);

	GfParmSetStr(Handle, ParamSection, PRM_LABEL, oLabel);
	GfParmSetStr(Handle, ParamSection, PRM_SECT, oSection);
	GfParmSetStr(Handle, ParamSection, PRM_PRM, oParameter);
	GfParmSetStr(Handle, ParamSection, PRM_UNIT, oUnit);
	GfParmSetNum(Handle, ParamSection, PRM_RANGE, oUnit, Val, Min, Max);
	GfParmSetNum(Handle, ParamSection, PRM_WEIGHT, 0, Weight);
	GfParmSetNum(Handle, ParamSection, PRM_SCALE, 0, Scale);
	GfParmSetNum(Handle, ParamSection, PRM_ROUND, 0, Round);

	return 0;
};

// Read parameter meta data from xml file
int TGeneticParameter::Get(bool First, const char* Part)
{
	char ParamSection[64];

	if (Part == NULL)
	  sprintf(ParamSection,SECT_GLOBAL);
	else
	  sprintf(ParamSection,"%s",Part);

	if (First)
	{
		GfParmListSeekFirst(Handle, ParamSection);
		First = false;
	}
	else
		GfParmListSeekNext(Handle, ParamSection);

	Active = 0 < GfParmGetCurNum(Handle, ParamSection, PRM_ACTIVE, 0, 1);
	LeftRight = 0 < GfParmGetCurNum(Handle, ParamSection, PRM_TWOSIDE, 0, 0);

	char* Value = (char *) GfParmGetCurStr(Handle, ParamSection, PRM_LABEL, oLabel);
	if (oLabel)
		free((void *) oLabel);
	if (Value)
		oLabel = strdup(Value);
	else
		oLabel = NULL;

	Value = (char *) GfParmGetCurStr(Handle, ParamSection, PRM_SECT, oSection);
	if (oSection)
		free((void *) oSection);
	if (Value)
		oSection = strdup(Value);
	else
		oSection = NULL;

	Value = (char *) GfParmGetCurStr(Handle, ParamSection, PRM_PRM, oParameter);
	if (oParameter)
		free((void *) oParameter);
	if (Value)
		oParameter = strdup(Value);
	else
		oParameter = NULL;

	Value = (char *) GfParmGetCurStr(Handle, ParamSection, PRM_UNIT, oUnit);
	if (oUnit)
		free((void *) oUnit);
	if (Value)
		oUnit = strdup(Value);
	else
		oUnit = NULL;

	Min = GfParmGetCurNumMin(Handle, ParamSection, PRM_RANGE, oUnit, Min);
	Max = GfParmGetCurNumMax(Handle, ParamSection, PRM_RANGE, oUnit, Max);
	Val = GfParmGetCurNum(Handle, ParamSection, PRM_RANGE, oUnit, Val);

	Weight = GfParmGetCurNum(Handle, ParamSection, PRM_WEIGHT, 0, Weight);
	Scale = GfParmGetCurNum(Handle, ParamSection, PRM_SCALE, 0, Scale);
	Round = GfParmGetCurNum(Handle, ParamSection, PRM_ROUND, 0, Round);

    Range = Max - Min;
	Def = LastVal = OptVal = Val;

	return 0;
};

// Read initial value from setup file
int TGeneticParameter::GetVal(void* SetupHandle, bool First, bool Local)
{
	char ParamSection[64];
	sprintf(ParamSection,"%s",oSection);

	if (Local)
	{
		if (First)
		{
			GfParmListSeekFirst(Handle, ParamSection);
		}
		else
			GfParmListSeekNext(Handle, ParamSection);

		if (LeftRight)
		{
			char SideParam[64];
			sprintf(SideParam,ParamSection,SECT_PH_LEFT);
			Val = GfParmGetCurNum(SetupHandle, SideParam, oParameter, oUnit, Val);

			sprintf(SideParam,ParamSection,SECT_PH_RGHT);
			Val = (Val + GfParmGetCurNum(SetupHandle, SideParam, oParameter, oUnit, Val)) / 2;

		}
		else
			Val = GfParmGetCurNum(SetupHandle, oSection, oParameter, oUnit, Val);

	}
	else
	{
		if (LeftRight)
		{
			char SideParam[64];
			sprintf(SideParam,ParamSection,SECT_PH_LEFT);
			Val = GfParmGetNum(SetupHandle, SideParam, oParameter, oUnit, Val);

			sprintf(SideParam,ParamSection,SECT_PH_RGHT);
			Val = (Val + GfParmGetNum(SetupHandle, SideParam, oParameter, oUnit, Val)) / 2;

		}
		else
			Val = GfParmGetNum(SetupHandle, oSection, oParameter, oUnit, Val);
	}

	Def = LastVal = OptVal = Val;

	return 0;
};

// Write parameter data to xml file
int TGeneticParameter::SetVal(void* SetupHandle, int Index)
{
	char ParamSection[64];
	if (Index > 0)
	  sprintf(ParamSection,"%s/%d",oSection,Index);
	else
	  sprintf(ParamSection,"%s",oSection);

	if (LeftRight)
	{
		char SideParam[64];

		sprintf(SideParam,ParamSection,SECT_PH_LEFT);
		GfParmSetNum(SetupHandle, SideParam, oParameter, oUnit, Val, Min, Max);

		sprintf(SideParam,ParamSection,SECT_PH_RGHT);
		return GfParmSetNum(SetupHandle, SideParam, oParameter, oUnit, Val, Min, Max);
	}
	else
		return GfParmSetNum(SetupHandle, ParamSection, oParameter, oUnit, Val, Min, Max);
}


//
// Class Genetic Parameter Part
//

// Default constructor
TGeneticParameterPart::TGeneticParameterPart():
	Handle(0),
	Label(NULL),
	Section(NULL),
	Subsection(NULL),
	Parameter(NULL)
{
};

// Constructor
TGeneticParameterPart::TGeneticParameterPart
(
	void* MetaDataFile,
	const char *ShortLabel,
	const char *SectionName,
	const char *ParameterName,
	const char *SubsectionName
)
{
	Handle = MetaDataFile;

	if (ShortLabel)
		Label = strdup(ShortLabel);
	else
		Label = NULL;

	if (SectionName)
		Section = strdup(SectionName);
	else
		Section = NULL;

	if (SubsectionName)
		Subsection = strdup(SubsectionName);
	else
		Subsection = NULL;

	if (ParameterName)
		Parameter = strdup(ParameterName);
	else
		Parameter = strdup("track param count");

	Active = 0 < GfParmGetNum(Handle, 
		Section, PRM_ACTIVE, 0, (float) Active);
};

// Destructor
TGeneticParameterPart::~TGeneticParameterPart()
{
	free ((void *) Label);
	free ((void *) Section);
	free ((void *) Subsection);
	free ((void *) Parameter);
};

// Write meta data of part to xml file
int TGeneticParameterPart::Set(int Index)
{
	char ParamSection[64];
	sprintf(ParamSection,"%s/%d/%s",SECT_LOCAL,Index,SECT_DEFINE);

	GfParmSetNum(Handle, ParamSection, PRM_ACTIVE, 0, (float) Active);
	GfParmSetStr(Handle, ParamSection, PRM_NAME, Label);
	GfParmSetStr(Handle, ParamSection, PRM_SECT, Section);
	GfParmSetStr(Handle, ParamSection, PRM_SUBSECT, Subsection);
	GfParmSetStr(Handle, ParamSection, PRM_PRM, Parameter);

	return 0;
};

// Read meta data of from xml file
int TGeneticParameterPart::Get(int Index)
{
	char ParamSection[64];
	sprintf(ParamSection,"%s/%d/%s",SECT_LOCAL,Index,SECT_DEFINE);

	Active = 0 < GfParmGetNum(Handle, ParamSection, PRM_ACTIVE, 0, (float) Active);

	char* Value = (char *) GfParmGetStr(Handle, ParamSection, PRM_NAME, Label);
	if (Label)
		free((void *) Label);
	if (Value)
		Label = strdup(Value);
	else
		Label = NULL;

	Value = (char *) GfParmGetStr(Handle, ParamSection, PRM_SECT, NULL);
	if (Section)
		free((void *) Section);
	if (Value)
		Section = strdup(Value);
	else
		Section = NULL;

	Value = (char *) GfParmGetStr(Handle, ParamSection, PRM_SUBSECT, NULL);
	if (Subsection)
		free((void *) Subsection);
	if (Value)
		Subsection = strdup(Value);
	else
		Subsection = NULL;

	Value = (char *) GfParmGetStr(Handle, ParamSection, PRM_PRM, NULL);
	if (Parameter)
		free((void *) Parameter);
	if (Value)
		Parameter = strdup(Value);
	else
		Parameter = NULL;

	return 0;
};


//
// Class Genetic Parameter Table of Content
//

// Default constructor
TGeneticParameterTOC::TGeneticParameterTOC():
	Handle(0),
	Author(NULL),
	Private(NULL),
	OptimisationLoops(1000),
	WeightOfDamages(1.0f),
	GetInitialVal(true)
{
};        

// Constructor
TGeneticParameterTOC::TGeneticParameterTOC           
(
	void* MetaDataFile,			// Handle to read/write data
	char* AuthorName,			// Name of author of setup
	char* PrivateSection,		// Name of private data section
	int Loops,					// Number of optimisation loops
	float WeightDamages,		// Weight of damages
	bool InitialVal				// get initial value from setup file
)
{
	Handle = MetaDataFile;
	if (AuthorName)
		Author = strdup(AuthorName);
	else
		Author = NULL;
	if (PrivateSection)
		Private = strdup(PrivateSection);
	else
		Private = NULL;
	OptimisationLoops = Loops; 
	WeightOfDamages = WeightDamages;
	GetInitialVal = InitialVal;
};

// Destructor
TGeneticParameterTOC::~TGeneticParameterTOC()
{
	// free allocated mem
	free(Private);
	free(Author);
}

// Write table of content to configuration file 
int TGeneticParameterTOC::Set()
{
	GfParmSetStr(Handle, 
		SECT_TOC, PRM_AUTHOR, Author);
	GfParmSetStr(Handle, 
		SECT_TOC, PRM_PRIVATE, Private);
	GfParmSetNum(Handle, 
		SECT_TOC, PRM_LOOPS, 0, (float) OptimisationLoops);
	GfParmSetNum(Handle, 
		SECT_TOC, PRM_DAMAGES, 0, (float) WeightOfDamages);
	if (GetInitialVal)
	  GfParmSetNum(Handle, 
	  	SECT_TOC, PRM_INITIAL, 0, 1);
	else
	  GfParmSetNum(Handle, 
	  	SECT_TOC, PRM_INITIAL, 0, 0);

	return 0;
}; 

// Read table of content from configuration file 
int TGeneticParameterTOC::Get() 
{
	char* Value = (char*) GfParmGetStr(Handle, 
		SECT_TOC, PRM_AUTHOR, "Wolf-Dieter Beelitz");
	if (Author)
		free(Author);
	if (Value)
		Author = strdup(Value);
	else
		Author = NULL;

	Value = (char*) GfParmGetStr(Handle, 
		SECT_TOC, PRM_PRIVATE, "simplix private");
	if (Private)
		free(Private);
	if (Value)
		Private = strdup(Value);
	else
		Private = NULL;

	OptimisationLoops = (int) GfParmGetNum(Handle, 
		SECT_TOC, PRM_LOOPS, 0, (float) OptimisationLoops);
	WeightOfDamages = GfParmGetNum(Handle, 
		SECT_TOC, PRM_DAMAGES, 0, (float) WeightOfDamages);
	GetInitialVal = 0 < GfParmGetNum(Handle, 
		SECT_TOC, PRM_INITIAL, 0, 1);

	return 0;
}; 

// end of file genetic.cpp
