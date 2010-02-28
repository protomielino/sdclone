#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

class CarData
{
public:
	std::string strName;
	std::string strCategory;
	std::string strRealCarName;
	std::string strXMLPath;
};

class CarInfo
{
public:
	CarInfo();


	std::string GetRealCarName(const char *pszName);
	void GetCarsInCategory(const char *pszCat,std::vector<std::string>&vecCars);
	void GetCarsInCategoryRealName(const char *pszCat,std::vector<std::string>&vecCars);
	void GetCategories(std::vector<std::string>&vecCats);
	CarData * GetCarData(const char *pszName);
	CarData * GetCarDataFromRealName(const char *pszRealName);
protected:
	void Init();
	bool m_bInit;

	std::vector<CarData> m_vecCarData;
	std::map<std::string,int> m_mapCarName;
	std::set<std::string> m_setCarCat;
};


extern CarInfo *GetCarInfo();
