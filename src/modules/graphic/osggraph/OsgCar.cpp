#include <osg/MatrixTransform>

#include "OsgLoader.h"
#include "OsgCar.h"
#include "OsgMath.h"

osg::ref_ptr<osg::Node> SDCar::loadCar(tCarElt *car)
{
    this->car = car;
   // static const char* pszTexFileExt = ".png";
    static const int nMaxTexPathSize = 4096;
    char buf[nMaxTexPathSize];
    int index;
    int selIndex;
   // ssgEntity *carEntity;
   // ssgSelector *LODSel;
    /* ssgBranchCb		*branchCb; */
   // ssgTransform *wheel[4];
  //  int nranges;
  //  int i, j;
    void *handle;
    const char *param;
    int lg;
    char path[256];
    
    strncpy(car->_masterModel, GfParmGetStr(car->_carHandle, SECT_GROBJECTS, PRM_TEMPLATE, ""), MAX_NAME_LEN - 1);
	car->_masterModel[MAX_NAME_LEN - 1] = 0;
   // grssgLoaderOptions options;
  //  sgVec3 lightPos;
   // int lightNum;
  //  const char *lightType;
    //int lightTypeNum;

    //TRACE_GL("loadcar: start");

    /*if (!CarsAnchorTmp) {
        CarsAnchorTmp = new ssgBranch();
    }*/

    index = car->index;	/* current car's index */
    handle = car->_carHandle;

    /* Initialize board */
    //grInitBoardCar(car);

    /* Schedule texture mapping if we are using a custom skin and/or a master 3D model */
    const bool bMasterModel = strlen(car->_masterModel) != 0;

    GfOut("[gr] Init(%d) car %s for driver %s index %d\n", index, car->_carName, car->_modName, car->_driverIndex);
	GfOut("[gr] Init(%d) car %s MasterModel name\n", index, car->_masterModel);

    lg = 0;
    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%d/%s;",
                   GfLocalDir(), car->_modName, car->_driverIndex, car->_carName);
    if (bMasterModel)
        lg += snprintf(buf + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%d/%s;",
                       GfLocalDir(), car->_modName, car->_driverIndex, car->_masterModel);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%d;",
                   GfLocalDir(), car->_modName, car->_driverIndex);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%s;",
                   GfLocalDir(), car->_modName, car->_carName);
    if (bMasterModel)
        lg += snprintf(buf + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%s;",
                       GfLocalDir(), car->_modName, car->_masterModel);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "%sdrivers/%s;",
                   GfLocalDir(), car->_modName);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "drivers/%s/%d/%s;",
                   car->_modName, car->_driverIndex, car->_carName);
    if (bMasterModel)
        lg += snprintf(buf + lg, nMaxTexPathSize - lg, "drivers/%s/%d/%s;",
                       car->_modName, car->_driverIndex, car->_masterModel);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "drivers/%s/%d;",
                   car->_modName, car->_driverIndex);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "drivers/%s/%s;",
                   car->_modName, car->_carName);
    if (bMasterModel)
        lg += snprintf(buf + lg, nMaxTexPathSize - lg, "drivers/%s/%s;",
                       car->_modName, car->_masterModel);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "drivers/%s;", car->_modName);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "cars/%s;", car->_carName);
    if (bMasterModel)
        lg += snprintf(buf + lg, nMaxTexPathSize - lg, "cars/%s;", car->_masterModel);

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "data/objects;");

    lg += snprintf(buf + lg, nMaxTexPathSize - lg, "data/textures");

    /* loading raw car level 0*/
    selIndex = 0; 	/* current selector index */
    snprintf(buf, nMaxTexPathSize, "%s.ac",
             bMasterModel ? car->_masterModel : car->_carName); /* default car 3D model file */
    snprintf(path, 256, "%s/%s/1", SECT_GROBJECTS, LST_RANGES);
    param = GfParmGetStr(handle, path, PRM_CAR, buf);

    std::string strPath = GetDataDir();
    if (bMasterModel)
    	sprintf(buf, "cars/%s/%s.acc", car->_masterModel, car->_masterModel);
    else
        sprintf(buf, "cars/%s/%s.acc", car->_carName, car->_carName);
        
    strPath+=buf;
    osgLoader loader;
    std::string strTPath = GetDataDir();
    sprintf(buf, "drivers/%s/%d/", car->_modName, car->_driverIndex);
    strTPath += buf;
    GfOut("Chemin Textures : %s\n", strTPath.c_str());
    loader.AddSearchPath(strTPath);
    osg::Node *pCar = loader.Load3dFile(strPath, true);

    osg::Vec3 p;

    p[0] = car->_pos_X;//+ car->_drvPos_x;
    p[1] = car->_pos_Y;//+car->_drvPos_y;
    p[2] = car->_pos_Z;//+car->_drvPos_z;

    osg::ref_ptr<osg::MatrixTransform> transform1 = new osg::MatrixTransform;
    transform1->addChild(pCar);

    GfOut("loaded car %d",pCar);
    this->car_branch = transform1;
    return this->car_branch;
}


void SDCar::updateCar()
{
    osg::Vec3 p;

    p[0] = car->_pos_X;//+ car->_drvPos_x;
    p[1] = car->_pos_Y;//+car->_drvPos_y;
    p[2] = car->_pos_Z;//+car->_drvPos_z;

    osg::Matrix mat(car->_posMat[0][0],car->_posMat[0][1],car->_posMat[0][2],car->_posMat[0][3],
                    car->_posMat[1][0],car->_posMat[1][1],car->_posMat[1][2],car->_posMat[1][3],
                    car->_posMat[2][0],car->_posMat[2][1],car->_posMat[2][2],car->_posMat[2][3],
                    car->_posMat[3][0],car->_posMat[3][1],car->_posMat[3][2],car->_posMat[3][3]);

    this->car_branch->setMatrix(mat);
}

SDCars::SDCars(void)
{
	cars_branch = new osg::Group;
}

SDCars::~SDCars(void)
{
}

void SDCars::addSDCar(SDCar * car)
{
    the_cars.insert(the_cars.end(),car);
}

osg::ref_ptr<osg::Node> SDCars::loadCars(tSituation * pSituation)
{
    tSituation *s = pSituation;
    this->situation = pSituation;
    for (int i = 0; i < s->_ncars; i++) 
    {
        tCarElt* elt = s->cars[i];
        SDCar * car = new SDCar;
        this->addSDCar(car);
        this->cars_branch->addChild(car->loadCar(elt));

    }
    
    return cars_branch;
}

void SDCars::updateCars()
{
    std::vector<SDCar *>::iterator it;
    for(it = the_cars.begin(); it!= the_cars.end(); it++)
    {
        (*it)->updateCar();
    }
}

