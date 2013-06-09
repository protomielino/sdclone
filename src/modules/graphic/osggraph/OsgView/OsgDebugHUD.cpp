#include<osg/Camera>
#include <osgDB/ReadFile>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>

#include "OsgDebugHUD.h"



SDDebugHUD::SDDebugHUD(){

    osg::Geometry* geom = osg::createTexturedQuadGeometry(
        osg::Vec3(), osg::Vec3(0.5f,0.0f,0.0f), osg::Vec3(0.0f,0.7f,0.0f),
    0.0f,0.0f,1.0f,1.0f);
    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    quad->addDrawable( geom );

    HUD_camera = new osg::Camera;
    HUD_camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    HUD_camera->setRenderOrder( osg::Camera::POST_RENDER );
    HUD_camera->setProjectionMatrix(osg::Matrix::ortho2D(-1, 1, -1, 1));
    HUD_camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    HUD_camera->addChild( quad );
    HUD_camera->setNodeMask(0);
}

void SDDebugHUD::setTexture(osg::ref_ptr<osg::Texture2D> map){
    osg::StateSet* stateset = HUD_camera->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0,map);
}

void SDDebugHUD::setTexture(osg::ref_ptr<osg::TextureCubeMap> map){
    osg::StateSet* stateset = HUD_camera->getOrCreateStateSet();
    osg::ref_ptr<osg::Texture2D> lm = new osg::Texture2D;
    lm->setTextureSize( 256, 256 );
    lm->setInternalFormat( GL_RGB );
    lm->setDataVariance(osg::Object::DYNAMIC);
    lm->setImage(map->getImage(3));
    stateset->setTextureAttributeAndModes( 0,lm);
}

void SDDebugHUD::toggleHUD(){
    HUD_camera->setNodeMask(1-HUD_camera->getNodeMask());
}

SDDebugHUD::~SDDebugHUD(){
}
