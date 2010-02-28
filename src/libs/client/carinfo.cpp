#include "carinfo.h"
#include <tgfclient.h>


CarInfo g_carInfo;


CarInfo *GetCarInfo()
{
	return &g_carInfo;
}


CarInfo::CarInfo()
{
	m_bInit = false;
}

void CarInfo::Init()
{
	if (m_bInit)
		return;

	//Read in car info

    tFList	*files;
    tFList	*curFile;
	char buf[1024];
    void	*hdle;

    /* Load the category list */
    files = GfDirGetList("cars");
    curFile = files;
    if ((curFile != NULL) && (curFile->name[0] != '.')) 
	{
        do 
		{
	    curFile = curFile->next;
		std::string strName = strdup(curFile->name);
		
		sprintf(buf, "cars/%s/%s.xml", strName.c_str(),strName.c_str());
	    hdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	    if (!hdle) {
	        continue;
	    }
		CarData cardata;
		cardata.strName = strName;
		cardata.strCategory = GfParmGetStr(hdle,"Car","category", "");
		cardata.strRealCarName = GfParmGetName(hdle);
		cardata.strXMLPath = buf;
		m_vecCarData.push_back(cardata);

		m_mapCarName[strName] = m_vecCarData.size()-1;
		m_setCarCat.insert(cardata.strCategory);

		GfParmReleaseHandle(hdle);
		} 
		while (curFile != files);
	}
	
	GfDirFreeList(files, NULL, true, true);

	m_bInit = true;
}

std::string CarInfo::GetRealCarName(const char *pszName)
{
	Init();
	std::string strRealName;

	std::string strName = pszName;
	std::map<std::string,int>::iterator p;
	p = m_mapCarName.find(strName);
	if (p!=m_mapCarName.end())
	{
		CarData *pCar = &m_vecCarData[p->second];
		strRealName = pCar->strRealCarName;
	}

	return strRealName;
}

void CarInfo::GetCarsInCategory(const char *pszCat,std::vector<std::string>&vecCars)
{
	Init();
	std::string strCat = pszCat;

	for (unsigned i=0;i<m_vecCarData.size();i++)
	{
		if (strCat == "All")
			vecCars.push_back(m_vecCarData[i].strName);
		else if (m_vecCarData[i].strCategory == strCat)
			vecCars.push_back(m_vecCarData[i].strName);
		
	}
}

void CarInfo::GetCategories(std::vector<std::string>&vecCats)
{
	Init();

	std::set<std::string>::iterator p;
	p = m_setCarCat.begin();

	while (p!= m_setCarCat.end())
	{
		vecCats.push_back(*p);
		p++;
	}
}

void CarInfo::GetCarsInCategoryRealName(const char *pszCat,std::vector<std::string>&vecCars)
{
	Init();
	std::string strCat = pszCat;

	for (unsigned i=0;i<m_vecCarData.size();i++)
	{
		if (strCat == "All")
			vecCars.push_back(m_vecCarData[i].strRealCarName);
		else if (m_vecCarData[i].strCategory == strCat)
			vecCars.push_back(m_vecCarData[i].strRealCarName);
		
	}
}

CarData * CarInfo::GetCarDataFromRealName(const char *pszRealName)
{
	Init();

	for (unsigned i=0;i<m_vecCarData.size();i++)
	{
		if (m_vecCarData[i].strRealCarName==pszRealName)
		{
			return &m_vecCarData[i];
		}
	}

	return NULL;
}

CarData * CarInfo::GetCarData(const char *pszName)
{
	Init();
	std::string strName = pszName;

	std::map<std::string,int>::iterator p;
	p = m_mapCarName.find(strName);
	if (p!=m_mapCarName.end())
		return &m_vecCarData[p->second];
	
	return NULL;
}
