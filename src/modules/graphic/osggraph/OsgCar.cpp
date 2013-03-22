/***************************************************************************

    file                 : OsgCar.cpp
    created              : Mon Aug 21 18:24:02 CEST 2012
    copyright            : (C) 2012 by Gaetan André
    email                : gaetan.andre@gmail.com
    version              : $Id$

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osg/Program>
#include <portability.h>

#include "OsgLoader.h"
#include "OsgCar.h"
#include "OsgMath.h"
#include "OsgScreens.h"
#include "OsgRender.h"
#include "OsgMain.h"
#include "OsgSky.h"



static osg::ref_ptr<osg::Program> program ;

class SDCarShader{
private :
    osg::Node *pCar;
    osg::StateSet* stateset;
    osg::Uniform * diffuseMap;
    osg::Uniform * viewPoint;
    osg::Uniform * specularColor;
    osg::Uniform * lightVector;
    osg::Uniform * lightPower;
    osg::Uniform * ambientColor;
    osg::Uniform * shininess;

public :
    static void initiateShaderProgram(){
        std::string TmpPath = GetDataDir();
        osg::ref_ptr<osg::Shader> vertShader =
                new osg::Shader( osg::Shader::VERTEX);
        osg::ref_ptr<osg::Shader> fragShader =
                new osg::Shader( osg::Shader::FRAGMENT);
        vertShader->loadShaderSourceFromFile(TmpPath+"/data/shaders/car.vert");
        fragShader->loadShaderSourceFromFile(TmpPath+"/data/shaders/car.frag");
        program = new osg::Program;
        program->addShader( vertShader.get() );
        program->addShader( fragShader.get() );
    }

    SDCarShader(osg::Node *car){
        pCar= car;
        stateset = pCar->getOrCreateStateSet();
        stateset->setAttributeAndModes(program);

        diffuseMap = new osg::Uniform("diffusemap", 0 );
        stateset->addUniform(diffuseMap);
        viewPoint = new osg::Uniform("pv",osg::Vec3());
        stateset->addUniform(viewPoint);
        specularColor = new osg::Uniform("specularColor", osg::Vec4(0.8,0.8,0.8,1.0));
        stateset->addUniform(specularColor);
        lightVector = new osg::Uniform("lightvector",osg::Vec3());
        stateset->addUniform(lightVector);
        lightPower = new osg::Uniform("lightpower",osg::Vec4());
        stateset->addUniform(lightPower);
        ambientColor =new osg::Uniform("ambientColor",osg::Vec4());
        stateset->addUniform(ambientColor);
        shininess = new osg::Uniform("smoothness", 128.0f);
        stateset->addUniform(shininess);
    }

    void update(){


        SDRender * ren = (SDRender *)getRender();
        osg::Vec3f sun_pos= ren->getSky()->getSun()->getSunPosition();
        osg::Vec4f sun_color = ren->getSky()->get_sun_color();
        osg::Vec4f scene_color = ren->getSky()->get_scene_color();

        SDScreens * scr= (SDScreens *)getScreens();
        osg::Vec3 c = scr->getActiveView()->getCameras()->getSelectedCamera()->getCameraPosition();
        osg::Matrix modelview = scr->getActiveView()->getOsgCam()->getViewMatrix();
        osg::Vec4 v = osg::Vec4(c.x(),c.y(),c.z(),1.0);
        osg::Vec4 pv = v*modelview;
        osg::Vec4 lv = osg::Vec4(sun_pos.x(),sun_pos.y(),sun_pos.z(),0.0);
        lv =lv*modelview;


        this->viewPoint->set(osg::Vec3(pv.x(),pv.y(),pv.z()));
        this->lightVector->set(osg::Vec3(lv.x(),lv.y(),lv.z()));
        this->lightPower->set(sun_color);
        this->ambientColor->set(scene_color*0.5);

        //osg::StateSet* stateset = pCar->getOrCreateStateSet();
        //stateset->setAttributeAndModes( program.get() );
        /*   stateset->addUniform(new osg::Uniform("diffusemap", 0 ));
            stateset->addUniform(new osg::Uniform("pv", osg::Vec3(pv.x(),pv.y(),pv.z())));
            stateset->addUniform(new osg::Uniform("specularColor", osg::Vec4(1.0,1.0,1.0,1.0)));
            stateset->addUniform(new osg::Uniform("lightvector", osg::Vec3(lv.x(),lv.y(),lv.z())));
            stateset->addUniform(new osg::Uniform("lightpower", osg::Vec4(4.0*sun_color.r(),4.0*sun_color.g(),4.0*sun_color.b(),4.0)));
            stateset->addUniform(new osg::Uniform("ambientColor", scene_color));*/
        //stateset->addUniform(new osg::Uniform("smoothness", 25.0f));

    }
};

osg::ref_ptr<osg::Node>
SDCar::loadCar(tCarElt *car)
{
    this->car = car;
    static const int nMaxTexPathSize = 4096;
    char buf[nMaxTexPathSize];
    int index;
    void *handle;
    //const char *param;
    int lg =0;
    char path[256];
    
    osgLoader loader;
    std::string TmpPath = GetDataDir();
    std::string strTPath;
    
    strncpy(car->_masterModel, GfParmGetStr(car->_carHandle, SECT_GROBJECTS, PRM_TEMPLATE, ""), MAX_NAME_LEN - 1);
    car->_masterModel[MAX_NAME_LEN - 1] = 0;

    index = car->index;	/* current car's index */
    handle = car->_carHandle;

    /* Initialize board */
    /* Schedule texture mapping if we are using a custom skin and/or a master 3D model */
    const bool bMasterModel = strlen(car->_masterModel) != 0;

    GfOut("[gr] Init(%d) car %s for driver %s index %d\n", index, car->_carName, car->_modName, car->_driverIndex);
    GfOut("[gr] Init(%d) car %s MasterModel name\n", index, car->_masterModel);

    snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/%d/",
             GfLocalDir(), car->_modName, car->_driverIndex);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/%s/",
             GfLocalDir(), car->_modName, car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    if (bMasterModel)
    {
        snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/%s/",
                 GfLocalDir(), car->_modName, car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }

    snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/",
             GfLocalDir(), car->_modName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "drivers/%s/%d/%s/",
             car->_modName, car->_driverIndex, car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    if (bMasterModel)
    {
        lg += snprintf(buf, nMaxTexPathSize, "drivers/%s/%d/%s/",
                       car->_modName, car->_driverIndex, car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }
    
    snprintf(buf, nMaxTexPathSize, "drivers/%s/%d/",
             car->_modName, car->_driverIndex);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "drivers/%s/%s/",
             car->_modName, car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    if (bMasterModel)
    {
        snprintf(buf, nMaxTexPathSize, "drivers/%s/%s/",
                 car->_modName, car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }

    snprintf(buf, nMaxTexPathSize, "drivers/%s/", car->_modName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "cars/models/%s/", car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    if (bMasterModel)
    {
        snprintf(buf, nMaxTexPathSize, "cars/models/%s/", car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }

    snprintf(buf, nMaxTexPathSize, "data/objects/");
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "data/textures/");
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    /* loading raw car level 0*/
    //  selIndex = 0; 	/* current selector index */
    snprintf(buf, nMaxTexPathSize, "%s.ac",
             bMasterModel ? car->_masterModel : car->_carName); /* default car 3D model file */
    snprintf(path, 256, "%s/%s/1", SECT_GROBJECTS, LST_RANGES);
    // param = GfParmGetStr(handle, path, PRM_CAR, buf);

    std::string strPath = GetDataDir();
    if (bMasterModel)
        sprintf(buf, "cars/models/%s/%s.acc", car->_masterModel, car->_masterModel);
    else
        sprintf(buf, "cars/models/%s/%s.acc", car->_carName, car->_carName);

    strPath+=buf;
    GfOut("Chemin Textures : %s\n", strTPath.c_str());
    osg::Node * pCar = loader.Load3dFile(strPath, true);

    osg::ref_ptr<osg::MatrixTransform> transform1 = new osg::MatrixTransform;
    transform1->addChild(pCar);

    GfOut("HERRE0\n");

    SDCarShader::initiateShaderProgram();
    this->shader = new SDCarShader(pCar);


    GfOut("HERRE10\n");

    GfOut("loaded car %d",pCar);
    this->car_branch = transform1;

    this->car_branch->addChild(wheels.initWheels(car,handle));
    
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

    wheels.updateWheels();

    this->car_branch->setMatrix(mat);

    shader->update();
}

SDCars::SDCars(void)
{
    cars_branch = new osg::Group;
}

SDCars::~SDCars(void)
{
    for(unsigned i=0;i<the_cars.size();i++){
        delete the_cars[i];
    }
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
