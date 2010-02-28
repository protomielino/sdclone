#pragma once
#include "network.h"
#include <vector>

class RobotXml
{
public:
	RobotXml();
	
	bool CreateRobotFile(const char*pRobotName,std::vector<Driver> &vecDriver);
	bool ReadRobotDrivers(const char*pRobotName,std::vector<Driver> &vecDrivers);
};

