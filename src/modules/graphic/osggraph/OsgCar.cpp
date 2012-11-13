#include <osg/MatrixTransform>



#include "OsgLoader.h"
#include "OsgCar.h"
#include "OsgMath.h"


/*float * flatten(float mat[4][4]){
    float res[16];
    for(int i=0;i<16;i++){
        res[i] = mat[i/4][i%4];
        GfOut("%d ",mat[i/4][i%4]);
        if(i%4==3)
           GfOut("\n");
    }
     GfOut("________\n");
    return res;

}*/



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

  //  ssgModelPath(buf);
  //  ssgTexturePath(buf);

    /* loading raw car level 0*/
    selIndex = 0; 	/* current selector index */
    snprintf(buf, nMaxTexPathSize, "%s.ac",
             bMasterModel ? car->_masterModel : car->_carName); /* default car 3D model file */
    snprintf(path, 256, "%s/%s/1", SECT_GROBJECTS, LST_RANGES);
    param = GfParmGetStr(handle, path, PRM_CAR, buf);
    //grCarInfo[index].LODThreshold[selIndex] = GfParmGetNum(handle, path, PRM_THRESHOLD, NULL, 0.0);
    //GfOut("HEHEHHEHEHEHHEHHEH %s\n",path);
    //carEntity = grssgCarLoadAC3D(param, NULL, index);
    std::string strPath = GetDataDir();
    sprintf(buf, "cars/%s/%s.acc", car->_carName, car->_carName);
    strPath+=buf;
    osgLoader loader;
    //GfOut("Chemin Textures : %s\n", m_strTexturePath.c_str());
    //loader.AddSearchPath(m_strTexturePath);
    //osg::Node *pCar = loader.Load3dFile("/usr/local/share/games/speed-dreams-2/cars/mp1-cavallo-tr06/mp1-cavallo-tr06-lod1.acc");
    osg::Node *pCar = loader.Load3dFile(strPath);

    osg::Vec3 p;

    p[0] = car->_pos_X;//+ car->_drvPos_x;
    p[1] = car->_pos_Y;//+car->_drvPos_y;
    p[2] = car->_pos_Z;//+car->_drvPos_z;
   // osgXformPnt3(p, car->_posMat);


    osg::ref_ptr<osg::MatrixTransform> transform1 = new osg::MatrixTransform;
   /* transform1->setMatrix( osg::Matrix::translate(p[0],p[1], p[2]) );*/
   // transform1->setMatrix( osg::Matrix(flatten(car->pub.posMat)) );
    transform1->addChild(pCar);
    //osgcars->addChild(transform1.get());
    //GfOut("LE POINTEUR %d\n",osgcars.get());
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

    osg::Matrix Ry = osg::Matrix::rotate(car->_yaw,osg::Z_AXIS);
    osg::Matrix Rp = osg::Matrix::rotate(-car->_pitch,osg::X_AXIS);
    osg::Matrix Rr = osg::Matrix::rotate(car->_roll,osg::Y_AXIS);
    osg::Matrix T = osg::Matrix::translate(p[0],p[1], p[2]);
    Rr.mult(Ry,Rr);
    Rp.mult(Rr,Rp);
    T.mult(Rp,T);
    this->car_branch->setMatrix(T);
    //this->car_branch->setMatrix(osg::Matrix(flatten(car->pub.posMat)));
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

