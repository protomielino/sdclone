/***************************************************************************

    file                 : OsgCar.cpp
    created              : Mon Aug 21 18:24:02 CEST 2012
    copyright            : (C)2012 by Gaétan André, (C)2014 Xavier Bertaux
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

#include <osgUtil/Optimizer>
#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osg/Program>
#include <osg/Geode>
#include <osg/Geometry>
#include <portability.h>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <robottools.h>

#include "OsgLoader.h"
#include "OsgCar.h"
#include "OsgMath.h"
#include "OsgScreens.h"
#include "OsgRender.h"
#include "OsgMain.h"
#include "OsgSky.h"
#include "OsgShader.h"
#include "OsgReflectionMapping.h"

class SDRender;

SDCar::SDCar(void)
{
    car_root = new osg::Group;
}

SDCar::~SDCar(void)
{
    car_root->removeChildren(0, car_root->getNumChildren());
    car_root = NULL;

    //delete loader;

    //delete wheels;
    delete cockpit;
    delete driver;
    delete wing;
    delete wing3;
    delete shader;
    delete reflectionMapping;

    //free car;
}

osg::ref_ptr<osg::Node> SDCar::loadCar(tCarElt *car, bool tracktype, unsigned carshader)
{
    this->car = car;
    static const int nMaxTexPathSize = 4096;
    char buf[nMaxTexPathSize];
    int index;
    void *handle;
    const char *param;
    //const char *param;,car_branch->getMatrix()
    //int lg =0;
    int nranges = 0;
    rcvShadowMask = 0x1;
    castShadowMask = 0x2;
    char path[256];

#if 1
    osgLoader loader;

    std::string TmpPath = GetDataDir();
    std::string strTPath;
    
    /*strncpy(car->_masterModel, GfParmGetStr(car->_carHandle, SECT_GROBJECTS, PRM_TEMPLATE, ""), MAX_NAME_LEN - 1);
    car->_masterModel[MAX_NAME_LEN - 1] = 0;*/

    index = car->index;	/* current car's index */
    handle = car->_carHandle;

    /* Initialize board */
    /* Schedule texture mapping if we are using a custom skin and/or a master 3D model */
    //const bool bMasterModel = strlen(car->_masterModel) != 0;

    GfLogInfo("[gr] Init(%d) car %s for driver %s index %d\n", index, car->_carName, car->_modName, car->_driverIndex);
    GfLogInfo("[gr] Init(%d) car %s MasterModel name\n", index, car->_masterModel);

    snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/%d/",
             GfDataDir(), car->_modName, car->_driverIndex);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/%s/",
             GfDataDir(), car->_modName, car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    /*if (bMasterModel)
    {
        snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/%s/",
                 GfLocalDir(), car->_modName, car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }*/

    snprintf(buf, nMaxTexPathSize, "%sdrivers/%s/",
             GfDataDir(), car->_modName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "drivers/%s/%d/%s/",
             car->_modName, car->_driverIndex, car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    /*if (bMasterModel)
    {
        lg += snprintf(buf, nMaxTexPathSize, "drivers/%s/%d/%s/",
                       car->_modName, car->_driverIndex, car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }*/
    
    snprintf(buf, nMaxTexPathSize, "drivers/%s/%d/",
             car->_modName, car->_driverIndex);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "drivers/%s/%s/",
             car->_modName, car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    /*if (bMasterModel)
    {
        snprintf(buf, nMaxTexPathSize, "drivers/%s/%s/",
                 car->_modName, car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }*/

    snprintf(buf, nMaxTexPathSize, "drivers/%s/", car->_modName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "cars/models/%s/", car->_carName);
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);
    
    /*if (bMasterModel)
    {
        snprintf(buf, nMaxTexPathSize, "cars/models/%s/", car->_masterModel);
        strTPath = TmpPath+buf;
        loader.AddSearchPath(strTPath);
    }*/

    snprintf(buf, nMaxTexPathSize, "data/objects/");
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    snprintf(buf, nMaxTexPathSize, "data/textures/");
    strTPath = TmpPath+buf;
    loader.AddSearchPath(strTPath);

    /* loading raw car level 0*/
    //  selIndex = 0; 	/* current selector index */
    snprintf(buf, nMaxTexPathSize, "%s.ac",
             car->_carName); /* default car 3D model file */
    snprintf(path, 256, "%s/%s/1", SECT_GROBJECTS, LST_RANGES);
    // param = GfParmGetStr(handle, path, PRM_CAR, buf);

    std::string strPath = GetDataDir();

    /*if (bMasterModel)
        sprintf(buf, "cars/models/%s/%s.acc", car->_masterModel, car->_masterModel);
    else*/
        sprintf(buf, "cars/models/%s/%s.acc", car->_carName, car->_carName);

    strPath+=buf;

    std::string name = car->_carName;

    osg::ref_ptr<osg::Group> gCar = new osg::Group;
    gCar->setName("CAR");
    osg::ref_ptr<osg::Switch> pBody =new osg::Switch;
    pBody->setName("COCK");
    osg::ref_ptr<osg::Node> pCar = new osg::Node;
    osg::ref_ptr<osg::Switch> pWing = new osg::Switch;
    pWing->setName("WING");
    osg::ref_ptr<osg::MatrixTransform> pWing3 = new osg::MatrixTransform;
    pWing->setName("WINGREAR");
    osg::ref_ptr<osg::Switch> pDriver = new osg::Switch;
    pDriver->setName("DRIVER");
    osg::ref_ptr<osg::Group> pSteer = new osg::Switch;
    pSteer->setName("STEER");
    osg::ref_ptr<osg::Group> pHiSteer = new osg::Switch;
    pHiSteer->setName("HISTEER");

    strPath+=buf;
    GfLogInfo("Chemin Textures : %s\n", strTPath.c_str());

    pCar = loader.Load3dFile(strPath, true);
#if 0
            std::string pCar_path = GetLocalDir();
            pCar_path = pCar_path+name+".osg";
            osgDB::writeNodeFile( *pCar, pCar_path );
#endif

    GfLogInfo("Load Car ACC !\n");

    /*strPath+=buf;
    GfOut("Chemin Textures : %s\n", strTPath.c_str());
    osg::ref_ptr<osg::Node> pCar = loader.Load3dFile(strPath, true);*/

    snprintf(path, 256, "%s/%s", SECT_GROBJECTS, SECT_WING_MODEL);
    param = GfParmGetStr(handle, path, PRM_WING_1, NULL);
    if (param)
    {
        osg::ref_ptr<osg::Node> pWin1 = new osg::Node;
        osg::ref_ptr<osg::Node> pWin2 = new osg::Node;

        strPath = GetDataDir();

        sprintf(buf, "cars/models/%s/wingR1-src.ac", car->_carName);
        strPath+=buf;

        pWin1 = loader.Load3dFile(strPath, true);
        pWin1->setName("WING1");
        GfLogInfo("Load Wing1 ACC !\n");

        strPath = GetDataDir();
        sprintf(buf, "cars/models/%s/wingR2-src.ac", car->_carName);
        strPath+=buf;
        pWin2 = loader.Load3dFile(strPath, true);

        pWin2->setName("WING2");
        GfLogInfo("Load Wing2 ACC !\n");

        pWing->addChild(pWin1.get(), true);
        pWing->addChild(pWin2.get(), false);

        if (!tracktype)
            pWing->setSingleChildOn(0);
        else
            pWing->setSingleChildOn(1);
#if 0
    std::string pWing_path = GetLocalDir();
    pWing_path = pWing_path+"wing.osg";
    osgDB::writeNodeFile( *pWing, pWing_path );
#endif
    }

    osg::ref_ptr<osg::Switch> pWin3 = new osg::Switch;

    snprintf(path, 256, "%s/%s", SECT_GROBJECTS, LST_REARWING);
    nranges = GfParmGetEltNb(handle, path) + 1;
    if (nranges > 1)
    {
        osg::ref_ptr<osg::Node> rearwingBody1 = new osg::Node;
        osg::ref_ptr<osg::Node> rearwingBody2 = new osg::Node;
        osg::ref_ptr<osg::Node> rearwingBody3 = new osg::Node;

        strPath = GetDataDir();
        sprintf(buf, "cars/models/%s/%s-wing-05-src.ac", car->_carName, car->_carName);
        strPath+=buf;
        rearwingBody1 = loader.Load3dFile(strPath, true);

        strPath = GetDataDir();
        sprintf(buf, "cars/models/%s/%s-wing-15-src.ac", car->_carName, car->_carName);
        strPath+=buf;
        rearwingBody2 = loader.Load3dFile(strPath, true);

        strPath = GetDataDir();
        sprintf(buf, "cars/models/%s/%s-wing-ab-src.ac", car->_carName, car->_carName);
        strPath+=buf;
        rearwingBody3 = loader.Load3dFile(strPath, true);

        pWin3->addChild( rearwingBody1.get(), true);
        pWin3->addChild( rearwingBody2.get(), false);
        pWin3->addChild( rearwingBody1.get(), false);
        pWin3->setSingleChildOn(0);

#if 0
    std::string pWing3_path = GetLocalDir();
    pWing3_path = pWing3_path+"wing3.osg";
    osgDB::writeNodeFile( *pWin3, pWing3_path );
#endif
    }

    GfLogInfo("Rear Wing angle Loaded\n");

    snprintf(path, 256, "%s/%s", SECT_GROBJECTS, SECT_COCKPIT);
    param = GfParmGetStr(handle, path, PRM_MODELCOCKPIT, NULL);

    strPath = GetDataDir();
    osg::ref_ptr<osg::Group> cockpit = new osg::Group;
    osg::ref_ptr<osg::Node> cock = new osg::Node;

    if (param)
    {

        /*if (bMasterModel)
            sprintf(buf, "cars/models/%s/cockpit.ac", car->_masterModel);
        else*/
        sprintf(buf, "cars/models/%s/cockpit.ac", car->_carName);

        strPath +=buf;

        cock= loader.Load3dFile(strPath, true);
#if 0
            std::string pCockpit_path = GetLocalDir();
            pCockpit_path = pCockpit_path+"cockpit.osg";
            osgDB::writeNodeFile( *cock, pCockpit_path );
#endif

        GfLogTrace("Cockpit High Loading \n");

        // Load steer cockpit model if available
        snprintf(path, 256, "%s/%s", SECT_GROBJECTS, SECT_STEERWHEEL);
        param = GfParmGetStr(handle, path, PRM_SW_MODELHR, NULL);

        if (param)
        {
            osg::ref_ptr<osg::Node> steer_branch = new osg::Node;
            osg::ref_ptr<osg::MatrixTransform> steer = new osg::MatrixTransform;

            strPath = GetDataDir();

            /*if (bMasterModel)
                sprintf(buf, "cars/models/%s/histeer.acc", car->_masterModel);
            else*/
                sprintf(buf, "cars/models/%s/histeer.ac", car->_carName);

            strPath+=buf;
            steer_branch = loader.Load3dFile(strPath, true);
#if 0
            std::string pSteer_path = GetLocalDir();
            pSteer_path = pSteer_path+"histeer.osg";
            osgDB::writeNodeFile( *steer_branch, pSteer_path );
#endif

            double xpos = GfParmGetNum(handle, path, PRM_XPOS, NULL, 0.0);
            double ypos = GfParmGetNum(handle, path, PRM_YPOS, NULL, 0.0);
            double zpos = GfParmGetNum(handle, path, PRM_ZPOS, NULL, 0.0);
            double angl = GfParmGetNum(handle, path, PRM_SW_ANGLE, NULL, 0.0);
            double steerMovt = GfParmGetNum(handle, path, PRM_SW_MOVT, NULL, 1.0);

            osg::Matrix pos = osg::Matrix::translate(xpos, ypos, zpos);
            osg::Matrix rot = osg::Matrix::rotate(0.0, osg::X_AXIS, angl, osg::Y_AXIS, 0.0, osg::Z_AXIS );

            pos = rot * pos;
            steer->addChild(steer_branch.get());
            steer->setMatrix(pos);

            //cockpit->addChild(steer.get());
            GfLogTrace("Cockpit High Steer Loading \n");
        }

        /*if (!cock)
            pBody->setSingleChildOn(0);
        else*/
            //cockpit->setSingleChildOn(1);

    }
        cockpit->addChild(cock.get());
        pBody->addChild(cockpit.get(), false);
#if 0
        std::string pBody_path = GetLocalDir();
        pBody_path = pBody_path+"body-"+name+".osg";
        osgDB::writeNodeFile( *pBody, pBody_path );
#endif

        // separate driver models for animation according to steering wheel angle ...
        snprintf(path, 256, "%s/%s", SECT_GROBJECTS, LST_DRIVER);
        nranges = GfParmGetEltNb(handle, path) + 1;
        osg::ref_ptr<osg::Switch> driver = new osg::Switch;

        if (nranges > 1)
        {
            // We have at least one separate driver model to add...
            //osg::ref_ptr<osg::Node> driver_branch = new osg::Node;

            //grCarInfo[index].DRMSelector = DRMSel = new ssgSelector;
            //grCarInfo[index].carTransform->addKid(DRMSel);

            int selIndex = 0;
            std::string tmp = GetLocalDir();
            std::string driver_path;

            // add the drivers
            for (int i = 1; i < nranges; i++)
            {
                osg::ref_ptr<osg::Node> driver_branch = new osg::Node;
                osg::ref_ptr<osg::MatrixTransform> position = new osg::MatrixTransform;

                snprintf(buf, nMaxTexPathSize, "%s/%s/%d", SECT_GROBJECTS, LST_DRIVER, i);
                param = GfParmGetStr(handle, buf, PRM_DRIVERMODEL, "");
                //grCarInfo[index].DRMThreshold[selIndex] = GfParmGetNum(handle, buf, PRM_DRIVERSTEER, NULL, 0.0);

                tdble xpos = GfParmGetNum(handle, buf, PRM_XPOS, NULL, 0.0);
                tdble ypos = GfParmGetNum(handle, buf, PRM_YPOS, NULL, 0.0);
                tdble zpos = GfParmGetNum(handle, buf, PRM_ZPOS, NULL, 0.0);
                osg::Matrix pos = osg::Matrix::translate(xpos, ypos, zpos);

                driver_path = tmp+param;

                driver_branch = loader.Load3dFile(driver_path, true);

                position->addChild(driver_branch);
                driver->addChild(position);
                driver_path ="";

                selIndex++;
            }

            driver->setSingleChildOn(0);

            // select a default driver - steer value of 0.0 is desired...
            /*for (i = 1; i < nranges; i++)
            {
                if (grCarInfo[index].DRMThreshold[i-1] == 0.0f)
                {
                    DRMSel->select( grCarInfo[index].DRMSelectMask[i-1] );
                    break;
                }
            }
            if (i == nranges)
                DRMSel->select( grCarInfo[index].DRMSelectMask[0] );*/

        #if 0
            std::string pDriver_path = GetLocalDir();
            pDriver_path = pDriver_path+"driver.osg";
            osgDB::writeNodeFile( *driver, pDriver_path );
        #endif
        }

        gCar->addChild(pCar.get());
        gCar->addChild(pWing.get());
        gCar->addChild(pWin3.get());
        gCar->addChild(driver.get());
#else

    osg::ref_ptr<osg::Group> gCar = new osg::Group;
    gCar->setName("CAR");
    osg::ref_ptr<osg::Switch> pBody =new osg::Switch;
    pBody->setName("COCK");
    osg::ref_ptr<osg::Node> pCar = new osg::Node;
    osg::ref_ptr<osg::Switch> pWing = new osg::Switch;
    pWing->setName("WING");
    osg::ref_ptr<osg::MatrixTransform> pWing3 = new osg::MatrixTransform;
    pWing->setName("WINGREAR");
    osg::ref_ptr<osg::Switch> pDriver = new osg::Switch;
    pDriver->setName("DRIVER");
    osg::ref_ptr<osg::Group> pSteer = new osg::Switch;
    pSteer->setName("STEER");
    osg::ref_ptr<osg::Group> pHiSteer = new osg::Switch;
    pHiSteer->setName("HISTEER");

    std::string LocalPath = GetDataDir();

    osg::ref_ptr<osgDB::Options> options = new::osgDB::ReaderWriter::Options();
    //options = new osgDB::ReaderWriter::Options;

    snprintf(buf, 4096, "drivers/%s/%d/", car->_modName, car->_driverIndex);
    options->getDatabasePathList().push_back(LocalPath+buf);

    snprintf(buf, 4096, "cars/models/%s/", car->_carName);
    options->getDatabasePathList().push_back(LocalPath+buf);

    options->getDatabasePathList().push_back(LocalPath+"data/textures/");
    options->getDatabasePathList().push_back(LocalPath+"data/objects/");

    gCar->addChild( osgDB::readNodeFile("sc-cavallo-360.osg", options));

    options->getDatabasePathList().clear();
#endif

        pBody->addChild(gCar.get(), true);
        pBody->setSingleChildOn(1);

        osg::ref_ptr<osg::MatrixTransform> transform1 = new osg::MatrixTransform;
        transform1->addChild(pBody.get());
        transform1->setNodeMask(rcvShadowMask | castShadowMask);

        // GfOut("loaded car %d",pCar.get());
        this->car_branch = transform1.get();

        //wheels = new SDWheels;
        this->car_branch->addChild(wheels.initWheels(car,handle));
        /*this->car_branch->getOrCreateStateSet()->setMode
            ( GL_DEPTH_TEST, osg::StateAttribute::PROTECTED|osg::StateAttribute::ON );*/

        this->car_root = new osg::Group;
        car_root->addChild(car_branch);

        /*if (SHADOW_TECHNIQUE == 0)
        this->car_root->addChild(this->initOcclusionQuad(car));*/

        //car_root->setNodeMask(1);

        this->shader = new SDCarShader(pCar.get(), this);

        if (carshader)
            this->reflectionMappingMethod = REFLECTIONMAPPING_DYNAMIC;
        else
            this->reflectionMappingMethod = REFLECTIONMAPPING_OFF;

        this->reflectionMapping = new SDReflectionMapping(this);
        this->setReflectionMap(reflectionMapping->getReflectionMap());

        return this->car_root;
}

bool SDCar::isCar(tCarElt*c)
{
    return c==car;
}
SDReflectionMapping * SDCar::getReflectionMap()
{
    return reflectionMapping;
}

int SDCar::getReflectionMappingMethod()
{
    return reflectionMappingMethod;
}

tCarElt * SDCar::getCar()
{
    return car;
}

#define GR_SHADOW_POINTS 6
#define MULT 1.1
osg::ref_ptr<osg::Node> SDCar::initOcclusionQuad(tCarElt *car)
{
    osg::Vec3f vtx;
    osg::Vec2f tex;
    float x;
    int i;

    char buf[512];
    std::string TmpPath = GetDataDir();

    //  GfOut("\n################## LOADING SHADOW ###############################\n");

    std::string shadowTextureName = GfParmGetStr(car->_carHandle, SECT_GROBJECTS, PRM_SHADOW_TEXTURE, "");

    snprintf(buf, sizeof(buf), "cars/models/%s/", car->_carName);
    if (strlen(car->_masterModel) > 0) // Add the master model path if we are using a template.
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "cars/models/%s/", car->_masterModel);

    std::string dir = buf;
    shadowTextureName = TmpPath +dir+shadowTextureName;

    // GfOut("\n lepath = %s\n",shadowTextureName.c_str());

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    vtx._v[2] = 0.0;
    for (i = 0, x = car->_dimension_x * MULT / 2.0; i < GR_SHADOW_POINTS / 2;
         i++, x -= car->_dimension_x * MULT / (float)(GR_SHADOW_POINTS - 2) * 2.0)
    {
        vtx._v[0] = x;
        tex._v[0] = 1.0 - (float)i / (float)((GR_SHADOW_POINTS - 2) / 2.0);

        vtx._v[1] = -car->_dimension_y * MULT / 2.0;
        vertices->push_back(vtx);
        tex._v[1] = 0.0;
        texcoords->push_back(tex);

        vtx._v[1] = car->_dimension_y * MULT / 2.0;
        vertices->push_back(vtx);
        tex._v[1] = 1.0;
        texcoords->push_back(tex);
    }

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0.0f,0.0f, 1.0f) );

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );

    quad = new osg::Geometry;
    quad->setVertexArray( vertices.get() );
    quad->setNormalArray( normals.get() );
    quad->setNormalBinding( osg::Geometry::BIND_OVERALL );
    quad->setColorArray( colors.get() );
    quad->setColorBinding( osg::Geometry::BIND_OVERALL );
    quad->setTexCoordArray( 0, texcoords.get() );
    quad->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, vertices->size()) );

    quad->setDataVariance(osg::Object::DYNAMIC);

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(shadowTextureName);
    texture->setImage( image.get() );

    osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc;
    blendFunc->setFunction( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( quad.get() );

    osg::StateSet* stateset = root->getOrCreateStateSet();
    //stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
    stateset->setRenderBinDetails( 2, "DepthSortedBin");
    stateset->setTextureAttributeAndModes(0, texture.get() );
    stateset->setAttributeAndModes( blendFunc );
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN );

    shadowVertices = vertices;

    //  GfOut("\n################## LOADED SHADOW ###############################\n");

    return root.get();
}

void SDCar::deactivateCar(tCarElt *car)
{
    if(this->car == car)
    {
        this->car_root->setNodeMask(0);
    }
}

void SDCar::activateCar(tCarElt *car)
{
    if(this->car == car)
    {
        this->car_root->setNodeMask(1);
    }
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

    reflectionMapping->update();
    this->setReflectionMap(reflectionMapping->getReflectionMap());

    //ugly computation,
    /*if (SHADOW_TECHNIQUE == 0)
    {
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
        for(unsigned int i=0;i<shadowVertices->size();i++)
        {
            osg::Vec3 vtx = (*shadowVertices.get())[i];
            osg::Vec4 vtx_world = osg::Vec4(vtx,1.0f)*mat;
            vtx_world._v[2] = RtTrackHeightG(car->_trkPos.seg, vtx_world.x(), vtx_world.y()); //0.01 needed, we have to sort out why
            vertices->push_back(osg::Vec3(vtx_world.x(), vtx_world.y(), vtx_world.z()));
        }
        quad->setVertexArray(vertices);
    }*/
}

void SDCar::updateShadingParameters(osg::Matrixf modelview)
{
    shader->update(modelview);
}

void SDCar::setReflectionMap(osg::ref_ptr<osg::Texture> map)
{
   car_branch->getOrCreateStateSet()->setTextureAttributeAndModes(2, map, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
}

SDCars::SDCars(void)
{
    cars_branch = new osg::Group;
}

SDCars::~SDCars(void)
{
    for(unsigned i=0;i<the_cars.size();i++)
    {
        delete the_cars[i];
    }
}

void SDCars::addSDCar(SDCar * car)
{
    the_cars.insert(the_cars.end(),car);
}

void SDCars::loadCars(tSituation * pSituation, bool trackType)
{
    SDRender *rend = (SDRender *)getRender();
    unsigned carShader = rend->getShader();
    tSituation *s = pSituation;
    this->situation = pSituation;

    for (int i = 0; i < s->_ncars; i++)
    {
        tCarElt* elt = s->cars[i];
        SDCar * car = new SDCar;
        this->addSDCar(car);
        this->cars_branch->addChild(car->loadCar(elt, trackType, carShader));
    }
    
    return;;
}

void SDCars::deactivateCar(tCarElt *car)
{
    std::vector<SDCar *>::iterator it;
    for(it = the_cars.begin(); it!= the_cars.end(); it++)
    {
        (*it)->deactivateCar(car);
    }
}

void SDCars::activateCar(tCarElt *car)
{
    std::vector<SDCar *>::iterator it;

    for(it = the_cars.begin(); it!= the_cars.end(); it++)
    {
        (*it)->activateCar(car);
    }
}

SDCar *SDCars::getCar(tCarElt*car)
{
    std::vector<SDCar *>::iterator it;
    SDCar *res = new SDCar;

    for(it = the_cars.begin(); it!= the_cars.end(); it++)
    {
        if((*it)->isCar(car))
        {
            res = *it;
        }
    }

    return res;
}

void SDCars::updateCars()
{
    std::vector<SDCar *>::iterator it;

    for(it = the_cars.begin(); it!= the_cars.end(); it++)
    {
        (*it)->updateCar();
    }
}

void SDCars::updateShadingParameters(osg::Matrixf modelview)
{
    std::vector<SDCar *>::iterator it;

    for(it = the_cars.begin(); it!= the_cars.end(); it++)
    {
        (*it)->updateShadingParameters(modelview);
    }
}

void SDCars::unLoad()
{
    cars_branch->removeChildren(0, cars_branch->getNumChildren());
    cars_branch = NULL;
}
