/***************************************************************************

    file                 : genetic.h
    created              : Tue Nov 04 17:45:00 CET 2010
    copyright            : (C) 2010-2013 by Wolf-Dieter Beelitz
    email                : wdb@wdbee.de

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
/**
    @defgroup	genetictools	Tools for genetic parameter optimization.
    Collection of functions for genetic parameter optimization.
*/

#ifndef __GENETIC_H__
#define __GENETIC_H__

#include "car.h"

//  
// Genetic parameters are handled in a single array 
// (see TGeneticParameter** GP;).
//
// At start there are global parameters. 
// Global parameters are car setup parameters like wing angles 
// or robot parameters that are used for the whole track.
//
// This block is followed by one ore more parts defining local
// parameters. The definition of the part of the track that has
// to use a set of such local parameters (let's call it section) 
// is not contained here, just because we do not need it, 
// it has to be handled by the consumer (robot).
// 
// We have the number of parts stored here, so we can handle
// each group of parameters defined in a part based on the 
// index offset to the first parameter of the group and we 
// know the number of parameters per section and the number of 
// sections defined in a part.
// 
// To store these information we use a strucure per part.
//

//
// Forewarding of classes
//
class TGeneticParameterTOC;		// Table of content
class TGeneticParameterCounter;	// Local parameter counter
class TGeneticParameter;		// Optimization parameter

//
// Structure to bundle handling data of parts or parameters.
// Used to allow the definition of more than one part of parameters 
// that are defined for subsections of the track as USR or DanDroid do.
//  
typedef struct genPart
{
	int Offset;		// Index offset of first parameter
	int NbrOfSect;	// Number of sections in this part
	int Count;		// Number of parameters per section
	bool Active;	// Block active
} tgenPart;

//
// Global data for control of parameter optimization
//
// This structure contains all values from the current race 
// we need for genetic parameter optimization
//
// First section is a set of data to access the correct setup files
// Second section contains strategic parameters for the learning
// Third section provides some counters to access the genetic parameters
// Next is a set of race results stored here to control the selection
// Lasr contains the buffers for stings used
//
// At the end there is an placeholder array ([1]) to
// easily getting the address of the list of parts defined.
//
typedef struct genResult
{
	// Pointer to the current car
	tCarElt* car;			// Pointer to car data

	// Basic data to access setups
	void* Handle;			// Handle to carsetup file
	char* TrackName;		// name of the track selected
	char* CarType;			// car type to create path to setup file
	char* RobotName;		// robot name to create path to setup

	// Strategic data, car setup depending on optimization state
    int Type;				// 0: race; 1: qualifying
	float MaxFuel;			// Default = 60
	float TotalWeight;		// Total of parameters individual weight
	bool First;				// First race with unchanged parameters
	bool Second;			// Second race initializing optimization

	// Race result data
	int DamagesTotal;		// Total of damages taken in race
	int LastDamagesTotal;	// Last total
	double WeightOfDamages;	// Factor to weight damages as time penalties

	// In case we handle a set of different race types
	double QualifyingLapTime;	// Laptime while qualifying
	double RaceLapTime;			// Laptime while race

	double BestTotalLapTime;	// Best laptime in the current race
	double LastTotalLapTime;	// Last best value

	double WeightedBestLapTime;	    // Best laptime increased by time penalty for damages
    double LastWeightedBestLapTime;	// Last best value

	double BestLapTime;			// Best laptime
    double LastBestLapTime;		// Last best value

	double TopSpeed;			// Max velocity
	double LastTopSpeed;		// Last max value

	double MinSpeed;			// Min velocity
    double LastMinSpeed;		// Last min value

	// Buffers for strings
	char TrackNameBuffer[4096];	// Buffer for trackname 
	char CarTypeBuffer[4096];	// Buffer for car type
	char RobotNameBuffer[4096];	// Buffer for robotname

	// Counters
	int NextIdx;      // Index to next parameter
	int NbrOfParts;   // Number of parts used

	// Parts
    tgenPart* Part;   // Pointer to first part structure

	// Parameters
	TGeneticParameter** GP; // Pointer to first parameter

} tgenResult;

//
// Class TGeneticParameter
//
// This class holds the meta data of a generic parameter
// used to describe the access to the parameter in the setup files
// and the range allowed for variation while optimization.
//
class TGeneticParameter
{
  public:
	TGeneticParameter();  // Default construtor
	TGeneticParameter     // Constructor
	(
		void* MetaDataFile,		// File handle to get the data
		float MinDef,			// Defines the minimum allowed value
		float ValDef,			// Defines the default value
		float MaxDef,			// Defines the maximum allowed value
		const char *Label,		// Gives a short label for console output
		const char *Section,	// Section to be used reading in the setup file
		const char *Parameter,	// Name of data line used reading in the setup file 
		const char *Unit,		// Defines the unit used for the value
		float ParamWeight,		// Weight of parameter for random selection
		float ParamScale,		// Scales the random variation of the value
		float ParamRound,		// Defines the rounding 
								// (to be able to write it in xml without loss of information)
		bool TwoSided = false	// If there are left and right parameters to be set

	);

	virtual ~TGeneticParameter(); // Destructor

	void DisplayParameter(); // Display parameter definitions at console
	void DisplayStatistik(); // Display statistics at console

	int Set(	// Write meta data to the configuration file
		const char* Part, int Index); 
	int Get(	// Read meta data from the configuration file
		const char* Part, int Index); 

	int GetVal(	// Read initial value from setup file
		void* SetupHandle, int Index = 0); 
	int SetVal(	// Write data to car setup file 
		void* SetupHandle, int Index = 0);

  public:
	void* Handle;	// Handle to read/write data to/from car setup file

    bool Active;	// Allow random selection while optimization
	float Min;		// Min allowed
	float Max;		// Max allowed
	float Val;		// Current value

	float LastVal;	// Last value
	float OptVal;	// Value used getting the best race result till now

	float Def;		// Default value to use if not specified

	float Weight;	// Weigth/TotalWeight = propability to be selected
	float Range;	// Range (Max - Min)
	float Scale;	// Scale random variation
	float Round;	// Define rounding to avoid minimal parameter changes
					// that cannot be stored in the xml file

	// Statistics
	int Tries;		// Number of selections 
	int Changed;	// Number of successfull variations
	bool Selected;	// Parameter is in current selection 
					// (to avoid multiple variations)

	char *oLabel;  
	char *oSection;
	char *oParameter;
	char *oUnit;

	bool LeftRight;

	// Common data (class variables)
	static tgenResult* MyResults;	// Pointer to structore with all data
									// needed for optimization
};


//
// Class Genetic Parameter Counter
//
// TGeneticParameterCounter
//
// Meta data of counters used to define the number of sections in parts
// defining how to get it from the setupfile
//
class TGeneticParameterCounter
{
  public:
	TGeneticParameterCounter();			// Default constructor
	TGeneticParameterCounter			// Constructor
	(
		void* MetaDataFile,				// Handle to read the data from
		const char* Label = NULL,		// Short label for console output
		const char* Section = NULL,		// Section to pick up data in the xml file
		const char* Parameter = NULL,	// Name of data line to use 
		int DefCount = 0				// Default value
	);

	virtual ~TGeneticParameterCounter(); // Destructor

	int Set(int Index);	// Write meta data to configuration file 
	int Get(int Index);	// Read meta data from configuration file 

  public:
	void* Handle;		// Handle to read/write data

    bool Active;		// Allow selection of the part while optimization
	int Def;			// Default value to use if not specified
	int Count;			// Current value

	char *oLabel;
	char *oSection;
	char *oParameter;

};


//
// Class Genetic Parameter Table of Content
//
// TGeneticParameterTOC
//
// Table of contents of an xml file for genetic parameters meta data
//
class TGeneticParameterTOC
{
  public:
	TGeneticParameterTOC();			// Default constructor
	TGeneticParameterTOC			// Constructor
	(
		void* MetaDataFile,			// Handle to read /write data
		int NbrOfGlobalParams = 0,	// Number of global genetic parameters
		int MbrOfParamsGroups = 0,	// Number of local parameters groups
		float WeightDamages = 1.0,	// Weight of damages as time penalty
		bool GetInitialVal = true   // Read initial value from setup
	);

	virtual ~TGeneticParameterTOC(); // Destructor

	int Set(); // Write table of content to configuration file 
	int Get(); // Read table of content from configuration file 

  public:
	void* Handle;					// Handle to read /write data
	int GlobalParamCount;			// Number of global genetic parameters
	int ParamsGroupCount;			// Number of local parameters groups
	float WeightOfDamages;	        // Weight of damages as time penalty
	bool GetInitialVal;				// Read initial value from setup

};
//==========================================================================*

#endif /* __GENETIC_H__ */

