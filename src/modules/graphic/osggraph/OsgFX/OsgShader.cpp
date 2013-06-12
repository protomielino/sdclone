#include "OsgRender.h"

#include <osg/Program>
#include <osg/Node>
#include <osg/Uniform>
#include <raceman.h>

#include "OsgMain.h"
#include "OsgShader.h"
#include "OsgSky/OsgSky.h"

SDCarShader::SDCarShader(osg::Node *car){

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

    pCar= car;
    stateset = pCar->getOrCreateStateSet();
    stateset->setAttributeAndModes(program);

    diffuseMap = new osg::Uniform("diffusemap", 0 );
    stateset->addUniform(diffuseMap);
    reflectionMap = new osg::Uniform("reflectionmap", 2 );
    stateset->addUniform(reflectionMap);
    specularColor = new osg::Uniform("specularColor", osg::Vec4(0.8f,0.8f,0.8f,1.0f));
    stateset->addUniform(specularColor);
    /*lightVector = new osg::Uniform("lightvector",osg::Vec3());
    stateset->addUniform(lightVector);
    lightPower = new osg::Uniform("lightpower",osg::Vec4());
    stateset->addUniform(lightPower);
    ambientColor =new osg::Uniform("ambientColor",osg::Vec4());
    stateset->addUniform(ambientColor);*/
    lightVector = stateset->getOrCreateUniform("lightvector",osg::Uniform::FLOAT_VEC3);
    lightPower = stateset->getOrCreateUniform("lightpower",osg::Uniform::FLOAT_VEC4);
    ambientColor = stateset->getOrCreateUniform("ambientColor",osg::Uniform::FLOAT_VEC4);
    shininess = new osg::Uniform("smoothness", 300.0f);
    stateset->addUniform(shininess);
}

void SDCarShader::update(osg::Matrixf view){

    SDRender * ren = (SDRender *)getRender();
    osg::Vec3f sun_pos= ren->getSky()->getSun()->getSunPosition();
    osg::Vec4f sun_color = ren->getSky()->get_sun_color();
    osg::Vec4f scene_color = ren->getSceneColor();

       /* GfOut("Sun Position : %f %f %f\n",sun_pos._v[0],sun_pos._v[1],sun_pos._v[2]);
        GfOut("Sun Color : %f %f %f %f\n",sun_color._v[0],sun_color._v[1],sun_color._v[2],sun_color._v[3]);
        GfOut("Scene Color : %f %f %f %f\n",scene_color._v[0],scene_color._v[1],scene_color._v[2],scene_color._v[3]);
    */



    osg::Vec4f lv = osg::Vec4(sun_pos.x(),sun_pos.y(),sun_pos.z(),0.0f);
    lv = lv*view;

        /*
        GfOut("View Point : %f %f %f\n",pv.x(),pv.y(),pv.z());
        GfOut("Light Vector  : %f %f %f\n",lv.x(),lv.y(),lv.z());
  */
       //GfOut("View Point : %f %f %f\n",pv.x(),pv.y(),pv.z());
    //  GfOut("Scene Color : %f %f %f %f\n",scene_color._v[0],scene_color._v[1],scene_color._v[2],scene_color._v[3]);

    osg::Uniform::Type t1 = lightVector->getType();
    osg::Uniform::Type t2 = lightPower->getType();
    osg::Uniform::Type t3 = ambientColor->getType();

       // GfOut("LV (vec3) %s, LP (vec4) %s, AC(vec4), %s\n",
         //     osg::Uniform::getTypename(t1),osg::Uniform::getTypename(t2),osg::Uniform::getTypename(t3));



    lightVector->set(osg::Vec3f(lv.x(),lv.y(),lv.z()));
    lightPower->set(sun_color);
    ambientColor->set(scene_color);

}

