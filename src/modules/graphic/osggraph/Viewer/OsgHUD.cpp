/***************************************************************************

    file        : OsgHUD.cpp
    created     : Sun Nov 23 20:12:19 CEST 2014
    copyright   : (C) 2014 by Xavier Bertaux
    email       : Xavier Bertaux
    version     : $Id$
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/RenderInfo>
#include <osg/Projection>

#include <osgDB/WriteFile>

#include <osgText/Text>

#include <osg/TextureRectangle>
#include <osgDB/ReadFile>
#include <osg/TexMat>

#include <osg/BlendFunc>

#include "OsgHUD.h"
#include "OsgNodeMask.h"
#include "OsgMain.h"
#include "tgfclient.h"

#include "forcefeedback.h"
extern TGFCLIENT_API ForceFeedbackManager forceFeedback;


#include <sstream>
#include <iomanip> //setprecision

//string constants for dashboard
static std::string strEmpty;
static std::string strBrakeRep("F/R Brake Rep.");
static std::string strFrontARB("Front ARB");
static std::string strRearARB("Rear ARB");
static std::string strFDiffMSB("F Pow Max Slip");
static std::string strFDiffCMSB("F Coa Max Slip");
static std::string strRDiffMSB("R Pow Max Slip");
static std::string strRDiffCMSB("R Coa Max Slip");
static std::string strCDiffMSB("C Pow Max Slip");
static std::string strCDiffCMSB("C Coa Max Slip");
static std::string strFuel("Fuel");
static std::string strRepair("Repair");
static std::string strTireSet("New tires");
static std::string strFrontWing("Front wing");
static std::string strRearWing("Rear wing");
static std::string strPenalty("Next pit type");

static osg::Vec4 colorRed(1.0, 0.0, 0.0, 1.0);
static osg::Vec4 colorYellow(1.0, 0.878, 0.0, 1.0);
static osg::Vec4 colorCyan(0.31, 0.968, 0.933, 1.0);

float prevSteerAngle = 0.0f;

std::map<std::string,osgText::Text* > hudTextElements;
std::map<std::string,int> hudElementsVisibilityStatus;
int hudElementsVisibilityStatusEnabled = 1;


osg::Vec3 calculatePosition(osg::BoundingBox mybb, const std::string &objPoint,
osg::BoundingBox bb,  const std::string &referenceObjPoint,

float verticalModifier, float horizontalModifier){
    /*
    Possible positioning values:
    tl  //top left
    tr  //top right
    tc  //top center
    bl  //bottom left
    br  //bottom right
    bc  //bottom center
    ml  //middle left
    mr  //middle right
    mc  //middle center
    */

    float vPoint=0;
    float hPoint=0;
    float vSign = 0;

    //my starting point
    osg::Vec3 position = osg::Vec3(0.0f,0.0f,0.0f);

    //ref object
    //vertical
    if(referenceObjPoint.find('t')==0){
        vPoint += bb.yMax();
        vSign = 1;
    }else if(referenceObjPoint.find('b')==0){
        vPoint += bb.yMin();
        vSign = -1;
    }else if(referenceObjPoint.find('m')==0){
        vPoint += (bb.yMax() - bb.yMin())/2;
        vSign = 1;
    }

    //horizontal
    if(referenceObjPoint.find('l')==1){
        hPoint += bb.xMin();
    }else if(referenceObjPoint.find('r')==1){
        hPoint += bb.xMax();
    }else if(referenceObjPoint.find('c')==1){
        hPoint += (bb.xMax() - bb.xMin())/2;
    }


    //my obj /*todo check medium vertical alignment*/
    //vertical
    if(objPoint.find('t')==0){
        vPoint -= (mybb.yMax() - mybb.yMin()) * vSign;//height
    }else if(objPoint.find('b')==0){
        //do nothing
    }else if(objPoint.find('m')==0){
        vPoint -= (mybb.yMax() - mybb.yMin()) * vSign/2;
    }

    //horizontal
    if(objPoint.find('l')==1){
        //nothing to do
    }else if(objPoint.find('r')==1){
        hPoint -= (mybb.xMax() - mybb.xMin());//width
    }else if(objPoint.find('c')==1){
        hPoint -= (mybb.xMax() - mybb.xMin())/2;
    }

    //modifier
    hPoint += horizontalModifier;
    vPoint += verticalModifier;

    // apply the modifiers
    position += osg::Vec3(hPoint,vPoint,0.0f);

    return position;
}

#ifdef HUDDEBUG
OSGPLOT::OSGPLOT( float positionX,
                    float positionY,
                    float width,
                    float height,
                    float maxValue,
                    float minValue,
                    float timeFrame,
                    float referenceLineAtValue,
                    const std::string &Xdata,
                    const std::string &Ydata,
                    const std::string &title)
{
    //initialize variables
    this->positionX = positionX;
    this->positionY = positionY;
    this->width = width;
    this->height = height;
    this->maxValue = maxValue;
    this->minValue = minValue;
    this->timeFrame = timeFrame;
    this->referenceLineAtValue = referenceLineAtValue;
    this->Xdata = Xdata;
    this->Ydata = Ydata;
    this->title = title;

    this->osgGroup = new osg::Group;

    this->osgMainPlotLineGeometry = new osg::Geometry();
    this->osgMainPlotLineVertices =  new osg::Vec3Array(2);

    this->osgReferencePlotLineGeometry = new osg::Geometry();
    this->osgReferencePlotLineVertices =  new osg::Vec3Array(2);

    osgText::Text* osgTitle = new  osgText::Text;

    this->dataPoints =  new osg::Vec3Array(0);

    //draw the background of the chart
    this->drawBackground();

    //prepare the geode for the "osgReferencePlotLine"
    {
        osg::Geode* geode = new osg::Geode;

        // pass the created vertex array to the points geometry object.
        this->osgReferencePlotLineGeometry->setVertexArray(this->osgReferencePlotLineVertices);
        this->osgReferencePlotLineGeometry->setUseDisplayList (false);
        this->osgReferencePlotLineGeometry->setUseVertexBufferObjects(true);
        this->osgReferencePlotLineGeometry->setDataVariance(osg::Object::DYNAMIC); /*?needed?*/

        // set the same color for the reference plot line
        osg::Vec4Array* plotColors = new osg::Vec4Array;
        plotColors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
        this->osgReferencePlotLineGeometry->setColorArray(plotColors, osg::Array::BIND_OVERALL);

        // set the normal
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
        this->osgReferencePlotLineGeometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

        // tell osg to draw our geometry as lines
        this->osgReferencePlotLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,this->osgReferencePlotLineVertices->size()));

        // disable lighting (light is always on) and enalbe transparency
        osg::StateSet* stateset = osgReferencePlotLineGeometry->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        // add the points geometry to the geode.
        geode->addDrawable(this->osgReferencePlotLineGeometry);
        this->osgGroup->addChild(geode);
    }

    //prepare the geode for the "osgMainPlotLine"
    {
        osg::Geode* geode = new osg::Geode;

        // pass the created vertex array to the points geometry object.
        this->osgMainPlotLineGeometry->setVertexArray(this->osgMainPlotLineVertices);
        this->osgMainPlotLineGeometry->setDataVariance(osg::Object::DYNAMIC); /*?needed?*/

        this->osgMainPlotLineGeometry->setUseDisplayList (false);

        // set the same color for the whole plot line
        osg::Vec4Array* plotColors = new osg::Vec4Array;
        plotColors->push_back(osg::Vec4(0.0f,0.0f,0.0f,0.5f));
        this->osgMainPlotLineGeometry->setColorArray(plotColors, osg::Array::BIND_OVERALL);

        // set the normal
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
        this->osgMainPlotLineGeometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

        // tell osg to draw our geometry as lines
        this->osgMainPlotLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,this->osgMainPlotLineVertices->size()));

        // disable lighting (light is always on) and enalbe transparency
        osg::StateSet* stateset = osgMainPlotLineGeometry->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        // add the points geometry to the geode.
        geode->addDrawable(this->osgMainPlotLineGeometry);
        this->osgGroup->addChild(geode);
    }

    //prepare the geode for the title
    {
        osg::Geode* geode = new osg::Geode;

        //color
        osg::Vec4 color = osg::Vec4(0.0f,0.0f,0.0f,0.9f);
        osgTitle->setColor(color);

        std::string fontFileUrl = "/vera/Vera.ttf";
        std::string fontsMainDirectory = GetDataDir();
        fontsMainDirectory = fontsMainDirectory+"data/fonts";
        fontFileUrl = fontsMainDirectory+fontFileUrl;
        osgTitle->setFont(fontFileUrl);

        //font resolution
        osgTitle->setFontResolution(200,200);

        //set the font size
        osgTitle->setCharacterSize(20);
        osgTitle->setAlignment(osgText::Text::LEFT_BOTTOM_BASE_LINE );

        //asign the position
        osgTitle->setPosition(osg::Vec3(this->positionX+5.0f, this->positionY+height+5.0f, 0.0f));

        //GfLogInfo("OSGHUD: Plot Title: %s \n", this->title.c_str());
        //GfLogInfo("OSGHUD: Position: %f %f \n", this->positionX+100.0f, this->positionY+100);

        osgTitle->setText(this->title);
        osgTitle->setNodeMask(1);

        geode->addDrawable(osgTitle);
        this->osgGroup->addChild(geode);
    }
}

OSGPLOT::~OSGPLOT()
{
}

void OSGPLOT::setNodeMask(int mask)
{
    for (int i = 0; i < this->osgGroup->getNumChildren(); i++)
        this->osgGroup->getChild(i)->setNodeMask(mask);
}
osg::ref_ptr <osg::Group> OSGPLOT::getGroup()
{
    return (*this->osgGroup).asGroup();
}
void OSGPLOT::update(tSituation *s, const SDFrameInfo* frameInfo,
                        const tCarElt *currCar)
{
    //get x value
    float x = 0;
    if(this->Xdata == "time") x = (float)GfTimeClock();

    //get y value
    float y = 0;
    if(this->Ydata == "fps")                    y = (float)frameInfo->fInstFps;
    else if(this->Ydata == "carspeed")          y = (float)currCar->_speed_x * 3.6;
    else if(this->Ydata == "fpsavverrange")     y = (float)frameInfo->fAvgFps;
    else if(this->Ydata == "carbracketemp")     y = (float)currCar->_brakeTemp(0);
    else if(this->Ydata == "forcefeedback")     y = fabs((float)forceFeedback.force);

    //get z value
    float z = 0.1f;

    //add the new point
    this->appendDataPoint(x,y,z);

    //redraw
    this->recalculateDrawnPoint();
}

void OSGPLOT::appendDataPoint(float x, float y, float z)
{
    //add the new element (as last of our vector)
    this->dataPoints->push_back(osg::Vec3(x, y, z));
}

void OSGPLOT::recalculateDrawnPoint()
{
    //recalculate main plot line
    {

        //find max and min values for our plot
        //just draw point that are in our range of time
        for(osg::Vec3Array::iterator it = this->dataPoints->begin(); it != this->dataPoints->end();)
        {
            if((*it).x() <= (GfTimeClock() - this->timeFrame) || (*it).x() <= 0)
            {
                it = this->dataPoints->erase(it);

            }
            else
            {
                //find max
                if ((*it).y() > this->maxValue)
                {
                    this->maxValue = (float)(*it).y();
                }

                //find min
                if ((*it).y() < this->minValue)
                {
                    this->minValue = (float)(*it).y();
                }

                ++it;
            }
        }

        //cicle trounght data point and calculate correct display position
        int counter = 0;
        this->osgMainPlotLineVertices->resize(this->dataPoints->size());

        for(osg::Vec3Array::iterator it = this->dataPoints->begin(); it != this->dataPoints->end(); ++it, counter++) {

            //copy data
            (*this->osgMainPlotLineVertices)[counter].set(
                ((float)(*it).x() - (GfTimeClock() - this->timeFrame))* (this->width / this->timeFrame),
                (float)(*it).y(),
                (float)(*it).z()
            );

            //scale to fit plot area
            (*this->osgMainPlotLineVertices)[counter].set(
                (*this->osgMainPlotLineVertices)[counter].x(),
                ((*this->osgMainPlotLineVertices)[counter].y() - this->minValue) / (this->maxValue-this->minValue) * this->height,
                (*this->osgMainPlotLineVertices)[counter].z()
            );

            //move to correct position
            (*this->osgMainPlotLineVertices)[counter].set(
                (*this->osgMainPlotLineVertices)[counter].x() + this->positionX,
                (*this->osgMainPlotLineVertices)[counter].y() + this->positionY,
                (*this->osgMainPlotLineVertices)[counter].z()
            );

        }

        //pass the new vertices to the geometry
        this->osgMainPlotLineGeometry->setVertexArray(this->osgMainPlotLineVertices);

        //update the drawing instructions (for the different number of vertices)
        this->osgMainPlotLineGeometry->removePrimitiveSet(0,1);
        this->osgMainPlotLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,this->osgMainPlotLineVertices->size()));

    }

    //recalculate reference plot line
    {
        // note, anticlockwise ordering.
        osg::Vec3 myCoords[] =
        {
            osg::Vec3(
                this->positionX,
                ((this->referenceLineAtValue - this->minValue) / (this->maxValue - this->minValue) * this->height) + this->positionY,
                0.1f
            ),
            osg::Vec3(
                this->positionX + this->width,
                ((this->referenceLineAtValue - this->minValue) / (this->maxValue-this->minValue) * this->height) + this->positionY,
                0.1f
            ),
        };

        int numCoords = sizeof(myCoords)/sizeof(osg::Vec3);

        this->osgReferencePlotLineVertices = new osg::Vec3Array(numCoords,myCoords);

        //tell osg that our vertices has changed
        this->osgReferencePlotLineVertices->dirty();
        this->osgReferencePlotLineGeometry->setVertexArray(this->osgReferencePlotLineVertices);
    }
}

void OSGPLOT::drawBackground()
{
    // create Geometry object
    osg::Geode* geode = new osg::Geode;
    osg::Geometry* bgGeometry = new osg::Geometry();

    // create the vertices of the geometry
    /*
     * the numbers rappresent the order of the vertices iside the vec array (took me some time to figure out the order was anticlock wise)
     * x and y are the axes
     * assuming this square is the screen
     *
     *      4_______3
     *      |       |
     *    y |       |
     *      |       |
     *      1_______2
     *          x
    * */
    osg::Vec3 myCoords[] =
    {
           osg::Vec3(this->positionX, this->positionY, 0.0f),
           osg::Vec3(this->positionX + this->width, this->positionY, 0.0f),
           osg::Vec3(this->positionX + this->width, this->positionY + this->height, 0.0f),
           osg::Vec3(this->positionX, this->positionY+this->height, 0.0f),
    };

        int numCoords = sizeof(myCoords)/sizeof(osg::Vec3);
       osg::Vec3Array* vertices = new osg::Vec3Array(numCoords,myCoords);

       // pass the created vertex array to the points geometry object.
       bgGeometry->setVertexArray(vertices);

       // apply the same color to the whole geometry
       osg::Vec4Array* colors = new osg::Vec4Array;
       colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,0.5f));
       bgGeometry->setColorArray(colors, osg::Array::BIND_OVERALL);

       // setup normals
       osg::Vec3Array* normals = new osg::Vec3Array;
       normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
       bgGeometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

       // tell osg to draw this geometry as quads
       bgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,numCoords));

       // disable lighting (light is always on) and enable transparency
       osg::StateSet* stateset = bgGeometry->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
       stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
       stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

       // add the points geometry to the geode.
       geode->addDrawable(bgGeometry);

      // add the geode to the graph group
      this->osgGroup->addChild(geode);
}
#endif

// TODO[START]: move this to utils? /src/modules/graphic/osggraph/Utils
void split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;

    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::string formatLaptime(tdble sec, int sgn)
{
    std::ostringstream lapTimeString;

    if(sec < 0)
    {
        lapTimeString << "-";
    }
    else
    {
        lapTimeString << "+";
    }

    sec = abs(sec);

    const int m = (int)(sec / 60.0);
    sec -= 60 * m;
    const int s = (int)(sec);
    sec -= s;
    const int ms = (int)floor(sec * 1000.0);

    //minutes
    if( m < 10)
    {
        lapTimeString << "0";
    }

    lapTimeString << m;

    lapTimeString << ":";

    //seconds
    if( s < 10)
    {
        lapTimeString << "0";
    }

    lapTimeString << s;

    lapTimeString << ".";

    //decimals
    if( ms < 100)
    {
        lapTimeString << "0";
    }

    if( ms < 10)
    {
        lapTimeString << "0";
    }

    lapTimeString << ms;

    return lapTimeString.str();
}

void changeImageSize(osg::Geometry *geom,
                        float newSize/*where 1.0 full image size (width or height) and 0.0 no size (width or height)*/,
                        const std::string &resizeFrom/*left|right|top|bottom == this is the place that will be fixed(not modified), the other one ill be moved to fit the new size*/,
                        float hudScale)
{
    osg::TextureRectangle* texture;

    //get the texture data of this object
    texture = dynamic_cast<osg::TextureRectangle*>(geom->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));

    //get the image from the texture data
    osg::Image* img;
    img = texture->getImage();

    //get image dimensions
    float imgWidth = img->s() * hudScale;
    float imgHeight = img->t() * hudScale;

    //adapt the geometry
    {
        //osg::Vec3Array* vertices = new osg::Vec3Array;
        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());

        /*
         * how vertices are arranged:
         *      3_______2
         *      |       |
         *    y |       |
         *      |       |
         *      0_______1
         *          x
         *
         * [vertices(0-3)][0]=x
         * [vertices(0-3)][1]=y
        * */

        //change the width
        if(resizeFrom =="left")
        {
            //change right vertices
            (*vertices)[1][0] = (*vertices)[0][0]+(imgWidth * newSize);
            (*vertices)[2][0] = (*vertices)[0][0]+(imgWidth * newSize);
        }
        else if(resizeFrom =="right")
        {
            //change left vertices
            (*vertices)[0][0] = (*vertices)[1][0]-imgWidth+(imgWidth * (1.0f-newSize));
            (*vertices)[3][0] = (*vertices)[1][0]-imgWidth+(imgWidth * (1.0f-newSize));
        }
        else if(resizeFrom =="top")
        {
            //change bottom vertices
            (*vertices)[0][1] = (*vertices)[2][1]-imgHeight+(imgHeight * (1.0f-newSize));
            (*vertices)[1][1] = (*vertices)[2][1]-imgHeight+(imgHeight * (1.0f-newSize));
        }
        else if(resizeFrom =="bottom")
        {
            //change top vertices
            (*vertices)[2][1] = (*vertices)[0][1]+(imgHeight * newSize);
            (*vertices)[3][1] = (*vertices)[0][1]+(imgHeight * newSize);
        }

        vertices->dirty();

        geom->setVertexArray(vertices);
    }

    //adapt the texture
    {
        //osg::Vec2Array* texcoords = new osg::Vec2Array(4);

        osg::Vec2Array* texcoords = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(0));

        if(resizeFrom =="left")
        {
            (*texcoords)[1][0]= newSize;
            (*texcoords)[2][0]= newSize;
        }
        else if(resizeFrom =="right")
        {
            (*texcoords)[0][0] = 1.0f - newSize;
            (*texcoords)[3][0] = 1.0f - newSize;
        }
        else if(resizeFrom =="top")
        {
            (*texcoords)[0][1] = 1.0f - newSize;
            (*texcoords)[1][1] = 1.0f - newSize;
        }
        else if(resizeFrom =="bottom")
        {
            (*texcoords)[2][1] = newSize;
            (*texcoords)[3][1] = newSize;
        }

        geom->setTexCoordArray(0,texcoords);
    }
}

void changeImagePosition(osg::Geometry *geom,
                        float newX,
                        float newY,
                        float hudScale)
{
    osg::TextureRectangle* texture;

    //get the texture data of this object
    texture = dynamic_cast<osg::TextureRectangle*>(geom->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));

    //get the image from the texture data
    osg::Image* img;
    img = texture->getImage();

    //get image dimensions
    float imgWidth = img->s() * hudScale;
    float imgHeight = img->t() * hudScale;

    //adapt the geometry
    {
        //osg::Vec3Array* vertices = new osg::Vec3Array;
        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());

        /*
         * how vertices are arranged:
         *      3_______2
         *      |       |
         *    y |       |
         *      |       |
         *      0_______1
         *          x
         *
         * [vertices(0-3)][0]=x
         * [vertices(0-3)][1]=y
        * */

        //change the position
        (*vertices)[0][0] = newX;
        (*vertices)[0][1] = newY;

        (*vertices)[1][0] = newX+imgWidth;
        (*vertices)[1][1] = newY;

        (*vertices)[2][0] = newX+imgWidth;
        (*vertices)[2][1] = newY+imgHeight;
        
        (*vertices)[3][0] = newX;
        (*vertices)[3][1] = newY+imgHeight;

        vertices->dirty();

        geom->setVertexArray(vertices);
    }
}

void changeImageAlpha(osg::Geometry *geom,
                        float newAlpha/*where 1.0 fully visible and 0.0 completely hidden*/
)
{
    // assign colors
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,newAlpha);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

}

// TODO[END]: move this to utils? /src/modules/graphic/osggraph/Utils

SDHUD::SDHUD()
{
    //_cameraHUD = new osg::Camera;

    //initialize some vars
    this->lastCar = NULL;
    this->hudScale = 1.0f;
}

void SDHUD::CreateHUD(int scrH, int scrW)
{
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    this->camera = new osg::Camera;

    // set the projection matrix
    this->camera->setProjectionMatrix(osg::Matrix::ortho2D(0,scrW,0,scrH));

    // set the view matrix
    this->camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    this->camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    this->camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    this->camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    this->camera->setAllowEventFocus(false);

    //calculate optimal hud scale (choose the minor scale from the vertical and horizontal scale)
    float scaleH = (float)scrH/1024;
    float scaleW = (float)scrW/1280;

    if(scaleH < scaleW){
        this->hudScale = scaleH;
    }
    else
    {
        this->hudScale = scaleW;
    }

    GfLogDebug("OSGHUD: Hud Scale is: %f\n", this->hudScale);

    //generate the hud from the relative xml file
    this->camera->addChild(this->generateHudFromXmlFile(scrH, scrW));
    
//    GfuiAddKey(0, 'z', "do somethig with the OSG widgets", (void*)0, this::ToggleHUD2, NULL);

}

void
SDHUD::DispDebug(const tSituation *s, const SDFrameInfo* frame)
{
}  // grDispDebug

void SDHUD::Refresh(tSituation *s, const SDFrameInfo* frameInfo,
                        const tCarElt *currCar)
{
    CarData &data = carData[currCar];

    // reset timers if car changes
    if (currCar != lastCar)
    {
        data.oldSector = currCar->_currentSector;
        data.oldLapNumber = currCar->_laps;
        data.laptimeFreezeTime = 0.0;
        data.timeDiffFreezeTime = 0.0;
        lastCar = currCar;
    }

#ifdef HUDDEBUG
    //update all the graphs

    typedef std::map<std::string,OSGPLOT* >::iterator it_type;
    for(it_type iterator = this->hudGraphElements.begin(); iterator != this->hudGraphElements.end(); ++iterator)

    {
           //iterator->first = key
           //iterator->second = value
           iterator->second->update(s,frameInfo,currCar);
    }

#endif

//board
    tCarElt *firstAheadCar;
    tCarElt *secondAheadCar;
    tCarElt *firstBehindCar;
    tCarElt *secondBehindCar;

    std::vector<tCarElt *> boardCars;

    // get pointers for previous and behind cars from us
    if (currCar->_pos > 2)
    {
        secondAheadCar = s->cars[currCar->_pos - 3];
        boardCars.push_back(secondAheadCar);
    }
    if (currCar->_pos > 1) {
        firstAheadCar = s->cars[currCar->_pos - 2];
        boardCars.push_back(firstAheadCar);
    }
    //always add our car
    boardCars.push_back(s->cars[currCar->_pos-1]);

    if (currCar->_pos < s->_ncars )
    {
        firstBehindCar = s->cars[currCar->_pos + 0];
        boardCars.push_back(firstBehindCar);
    }

    if (currCar->_pos < (s->_ncars-1) )
    {
        secondBehindCar = s->cars[currCar->_pos + 1];
        boardCars.push_back(secondBehindCar);
    }

    //hide all board slots... we will enable later what we will use
    for (int id = 1; id <= 5; id++)
    {
        std::ostringstream mapKey;
        //hide board number
        mapKey.str("");
        mapKey << "board-player" << id << "-background";
        this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
        mapKey.str("");
        mapKey << "board-player" << id << "-background-first";
        this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
        mapKey.str("");
        mapKey << "board-player" << id << "-background-current";
        this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);

        mapKey.str("");
        mapKey << "board-player" << id << "-number";
        hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);

        mapKey.str("");
        mapKey << "board-player" << id << "-name";
        hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);

        mapKey.str("");
        mapKey << "board-player" << id << "-timediff";
        hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
    }

    int id = 0;
    for(std::vector<tCarElt *>::iterator car = boardCars.begin(); car != boardCars.end(); ++car) {

        std::ostringstream mapKey;
        id++;

        mapKey.str("");
        mapKey << "board-player" << id << "-background";
        this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_ALL);

        //select special background for current and/or first player
        if ((*car) == currCar)
        {
            this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
            mapKey << "-current";
            this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_ALL);
        }
        else if ((*car)->_pos == 1)
        {
            this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
            mapKey << "-first";
            this->hudImgElements[mapKey.str()]->setNodeMask(NODE_MASK_ALL);
        }

        std::ostringstream position;
        position.str("");
        position << (*car)->_pos;
        mapKey.str("");
        mapKey << "board-player" << id << "-number";
        hudTextElements[mapKey.str()]->setText(position.str());
        hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_ALL);

        //update board names texts
        mapKey.str("");
        mapKey << "board-player" << id << "-name";
        hudTextElements[mapKey.str()]->setText((*car)->_name);
        hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_ALL);

        //update time diff texts
        mapKey.str("");
        mapKey << "board-player" << id << "-timediff";

        if( data.oldLapNumber != currCar->_laps)
        {
            data.timeDiffFreezeTime = GfTimeClock();
        }

        if ( GfTimeClock() < (data.timeDiffFreezeTime + data.timeDiffFreezeCountdown))
        {
            data.oldLapNumber = currCar->_laps;

            std::ostringstream tempStr;
            tempStr.str("");
            if((*car)->_laps == currCar->_laps)
            {
                tempStr << formatLaptime(((*car)->_curTime - currCar->_curTime),1);
            }
            else if ((*car)->_laps > currCar->_laps)
            {
                tempStr << ((*car)->_laps - currCar->_laps);
                tempStr << " laps";
            }

            hudTextElements[mapKey.str()]->setText(tempStr.str());
            hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_ALL);
            //hide time diff for our car
            if ((*car) == currCar)
            {
                hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
            }
        }
        else
        {
            hudTextElements[mapKey.str()]->setNodeMask(NODE_MASK_NONE);
        }
    }

//position box
    std::ostringstream temp;
    temp.str("");
    temp << s->_ncars;
    hudTextElements["position-container-totalplayers"]->setText(temp.str());
    temp.str("");
    temp << currCar->_pos;
    hudTextElements["position-container-currentposition"]->setText(temp.str());


//laps box
    temp.str("");
    temp << s->_totLaps;
    hudTextElements["lap-container-totallaps"]->setText(temp.str());
    temp.str("");
    temp << currCar->_laps;
    hudTextElements["lap-container-currentlap"]->setText(temp.str());


//laptime
    if (data.oldSector != currCar->_currentSector)
    {
        data.laptimeFreezeTime = GfTimeClock();
        data.oldSector = currCar->_currentSector;

        if (currCar->_currentSector == 0)
        {
            hudTextElements["laptime-last-time"]->setText(formatLaptime(currCar->_lastLapTime,0));

            if (currCar->_lastLapTime == currCar->_bestLapTime)
            {
                this->hudImgElements["laptime-last-background-green"]->setNodeMask(NODE_MASK_ALL);
                this->hudImgElements["laptime-last-background-normal"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-grey"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-violet"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-red"]->setNodeMask(NODE_MASK_NONE);
            }
            else
            {
                this->hudImgElements["laptime-last-background-normal"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-grey"]->setNodeMask(NODE_MASK_ALL);
                this->hudImgElements["laptime-last-background-violet"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-green"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-red"]->setNodeMask(NODE_MASK_NONE);
            }
        }
        else
        {
            float currentPrevSectorSplitTime = currCar->_curSplitTime[currCar->_currentSector - 1]; // our time in the sector we have "just" run over
            float bestPrevSectorSplitTime = currCar->_bestSplitTime[currCar->_currentSector-1]; // the best split time of the sector we are in this moment

            if (currentPrevSectorSplitTime < bestPrevSectorSplitTime)
            {
                this->hudImgElements["laptime-last-background-normal"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-grey"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-violet"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-green"]->setNodeMask(NODE_MASK_ALL);
                this->hudImgElements["laptime-last-background-red"]->setNodeMask(NODE_MASK_NONE);
            }
            else
            {
                this->hudImgElements["laptime-last-background-normal"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-grey"]->setNodeMask(NODE_MASK_ALL);
                this->hudImgElements["laptime-last-background-violet"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-green"]->setNodeMask(NODE_MASK_NONE);
                this->hudImgElements["laptime-last-background-red"]->setNodeMask(NODE_MASK_NONE);
            }
        }
    }

    if (GfTimeClock() > (data.laptimeFreezeTime + data.laptimeFreezeCountdown))
    {
        //remember our current sector
        data.oldSector = currCar->_currentSector;

        temp.str("");
        temp << "S" << (data.oldSector+1);
        hudTextElements["laptime-sector-description"]->setText(temp.str());

        this->hudImgElements["laptime-last-background-normal"]->setNodeMask(NODE_MASK_ALL);
        this->hudImgElements["laptime-last-background-grey"]->setNodeMask(NODE_MASK_NONE);
        this->hudImgElements["laptime-last-background-violet"]->setNodeMask(NODE_MASK_NONE);
        this->hudImgElements["laptime-last-background-green"]->setNodeMask(NODE_MASK_NONE);
        this->hudImgElements["laptime-last-background-red"]->setNodeMask(NODE_MASK_NONE);

        //show laptime
        hudTextElements["laptime-last-time"]->setText(formatLaptime(currCar->_curLapTime,0));

        //on the last sector show the total lap time
        if (currCar->_currentSector == currCar->_nbSectors-1)
        {
            hudTextElements["laptime-best-time"]->setText(formatLaptime(currCar->_bestLapTime,0));
        }
        else
        {
            tdble bestSplitTime = currCar->_bestSplitTime[currCar->_currentSector]; // the best split time of the sector we are in this moment
            hudTextElements["laptime-best-time"]->setText(formatLaptime(bestSplitTime,0));
        }
    }

//fuel
    temp.str("");
    temp << std::fixed << std::setprecision(1) << currCar->_fuel;
    hudTextElements["fuel-info-quantity-number"]->setText(temp.str());

    temp.str("");

    //when we have done at least one lap calculate remaining fuel
    if (currCar->_laps > data.carLaps && currCar->_laps > 1)
    {
        float fuelConsumpionPerLap = currCar->_fuelTotal / (float)(currCar->_laps-1);
        data.remainingFuelForLaps = currCar->_fuel / fuelConsumpionPerLap;
        data.carLaps = currCar->_laps;
    }

    //if we have fuel for more than one lap display how many
    if (data.remainingFuelForLaps > 0 )
    {
        temp << std::fixed << std::setprecision(0) << data.remainingFuelForLaps;
    }
    else
    {
        temp << "---";
    }

    hudTextElements["fuel-info-laps-number"]->setText(temp.str());

    float carFuel = (float)((float)currCar->_fuel / (float)currCar->_tank);

    //update fuel bar
    changeImageSize(this->hudImgElements["fuel-icon-empty"], 1.0-carFuel, "top", this->hudScale);
    changeImageSize(this->hudImgElements["fuel-icon-full"], carFuel, "bottom", this->hudScale);


//abs
//tcs
//TODO: speed limiter

    bool abs = false;   // Show ABS indicator?
    bool tcs = false;   // Show TCS indicator?
    bool spd = false;   // Show speed limiter indicator?

    // Parse control messages if they include ABS / TCS / SPD
    for (int i = 0; i < 4; i++)
    {
        if (currCar->ctrl.msg[i])
        {
            abs = abs || strstr(currCar->ctrl.msg[i], "ABS");
            tcs = tcs || strstr(currCar->ctrl.msg[i], "TCS");
            spd = spd || strstr(currCar->ctrl.msg[i], "Speed Limiter On");
        }
    }

    this->hudImgElements["abs-icon"]->setNodeMask(abs ? NODE_MASK_ALL : NODE_MASK_NONE);
    this->hudImgElements["tcs-icon"]->setNodeMask(tcs ? NODE_MASK_ALL : NODE_MASK_NONE);

//gear
    // show gear: "N" for neutral, "R" for retro, "gearnumber" for all the others
    temp.str("");
    if(currCar->_gear == 0)
    {
        temp << "N";
    }
    else if (currCar->_gear == -1)
    {
        temp << "R";
    }
    else
    {
        temp << currCar->_gear;
    }

    hudTextElements["gear-number"]->setText(temp.str());

//speed
    temp.str("");
    //get speed in km/h
    temp << (int)(currCar->_speed_x * 3.6);
    hudTextElements["speed-number"]->setText(temp.str());


//damage
    float carDamage = (float)((currCar->_dammage) / (float)s->_maxDammage);
    changeImageSize(this->hudImgElements["engine-icon"], 1.0-carDamage, "top", this->hudScale);
    changeImageSize(this->hudImgElements["engine-icon-damaged"], carDamage, "bottom", this->hudScale);


//rpm
    float rpmWidth = 1.0 / currCar->_enginerpmMax * currCar->_enginerpm;
    changeImageSize(this->hudImgElements["rpm-on"], rpmWidth, "left", this->hudScale);
    changeImageSize(this->hudImgElements["rpm-off"], 1.0-rpmWidth, "right", this->hudScale);

// dash items
    std::string &description = strEmpty;
    if (currCar->_dashboardActiveItem < currCar->_dashboardInstantNb)
    {
        const tDashboardItem *item = &(currCar->_dashboardInstant[currCar->_dashboardActiveItem]);
        temp.str("");
        switch (item->type)
        {
        case DI_BRAKE_REPARTITION:
            description = strBrakeRep;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        case DI_FRONT_ANTIROLLBAR:
            description = strFrontARB;
            temp << std::fixed << std::setprecision(1) << (item->setup->value / 1000.0) << " kN/m";
            break;
        case DI_REAR_ANTIROLLBAR:
            description = strRearARB;
            temp << std::fixed << std::setprecision(1) << (item->setup->value / 1000.0) << " kN/m";
            break;
        case DI_FRONT_DIFF_MAX_SLIP_BIAS:
            description = strFDiffMSB;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        case DI_FRONT_DIFF_COAST_MAX_SLIP_BIAS:
            description = strFDiffCMSB;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        case DI_REAR_DIFF_MAX_SLIP_BIAS:
            description = strRDiffMSB;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        case DI_REAR_DIFF_COAST_MAX_SLIP_BIAS:
            description = strRDiffCMSB;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        case DI_CENTRAL_DIFF_MAX_SLIP_BIAS:
            description = strCDiffMSB;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        case DI_CENTRAL_DIFF_COAST_MAX_SLIP_BIAS:
            description = strCDiffCMSB;
            temp << std::fixed << std::setprecision(1) << (100.0 * item->setup->value) << " %";
            break;
        }
        hudTextElements["dash-items-type"]->setText(description);
        hudTextElements["dash-items-value1"]->setColor(colorRed);
        hudTextElements["dash-items-value1"]->setText(temp.str());
        hudTextElements["dash-items-value2"]->setText(std::string());
    }
    else
    {
        const tDashboardItem *item = &(currCar->_dashboardRequest[currCar->_dashboardActiveItem - currCar->_dashboardInstantNb]);
        std::ostringstream value1;
        std::ostringstream value2;
        switch (item->type)
        {
        case DI_FUEL:
            description = strFuel;
            value1 << std::fixed << std::setprecision(1) << item->setup->desired_value << " l";
            value2 << std::fixed << std::setprecision(1) << item->setup->value << " l";
            break;
        case DI_REPAIR:
            description = strRepair;
            value1 << std::fixed << std::setprecision(1) << item->setup->desired_value;
            value2 << std::fixed << std::setprecision(1) << item->setup->value;
            break;
        case DI_TYRE_SET:
            description = strTireSet;
            if (item->setup->desired_value > 0.9)
            {
                value1 << "YES";
            }
            else
            {
                value1 << "NO";
            }
            break;
        case DI_FRONT_WING_ANGLE:
            description = strFrontWing;
            value1 << std::fixed << std::setprecision(1) << RAD2DEG(item->setup->desired_value);
            value2 << std::fixed << std::setprecision(1) << RAD2DEG(item->setup->value);
            break;
        case DI_REAR_WING_ANGLE:
            description = strRearWing;
            value1 << std::fixed << std::setprecision(1) << RAD2DEG(item->setup->desired_value);
            value2 << std::fixed << std::setprecision(1) << RAD2DEG(item->setup->value);
            break;
        case DI_PENALTY:
            description = strPenalty;

            if (item->setup->desired_value > 0.9)
            {
                value1 << "PENALTY";
            }
            else
            {
                value1 << "REPAIR";
            }
            break;
        }
        hudTextElements["dash-items-type"]->setText(description);
        hudTextElements["dash-items-value1"]->setColor(colorYellow);
        hudTextElements["dash-items-value1"]->setText(value1.str());
        hudTextElements["dash-items-value2"]->setColor(colorCyan);
        hudTextElements["dash-items-value2"]->setText(value2.str());
    }

// driver inputs 
    changeImageSize(this->hudImgElements["driverinput-clutch"], currCar->ctrl.clutchCmd, "bottom", this->hudScale);
    changeImageSize(this->hudImgElements["driverinput-brake"], currCar->ctrl.brakeCmd, "bottom", this->hudScale);
    changeImageSize(this->hudImgElements["driverinput-throttle"], currCar->ctrl.accelCmd, "bottom", this->hudScale);
    
    const osg::Vec3d axis(0, 0, 1);
    //wheel
    
    double angle = currCar->ctrl.steer * 4;
    
    osg::Vec3d center = this->hudImgRotableElements["driverinput-wheel"]->getBound().center();//Save the center point coordinates of the object first
    osg::Matrix curMatrix = this->hudImgRotableElements["driverinput-wheel"]->getMatrix();
    
    curMatrix *= osg::Matrix::translate(-center);//Move the object to the origin of the world coordinate system
    curMatrix *= osg::Matrix::rotate(-prevSteerAngle, axis);//rotate
    curMatrix *= osg::Matrix::rotate(angle, axis);//rotate
    curMatrix *= osg::Matrix::translate(center);//move back to the original position of the object
    
    this->hudImgRotableElements["driverinput-wheel"]->setMatrix(curMatrix);

    prevSteerAngle = angle;
    
// tires temps


    for (int i = 0; i < 4; i++) { //for each tires
        
        std::ostringstream tireName;
        switch(i) {
            case 0:
                tireName << "fr-";
            break;
            case 1:
                tireName << "fl-";
            break;
            case 2:
                tireName << "rr-";
            break;
            case 3:
                tireName <<  "rl-";
            break;
            default:
                tireName << "";
        }
        
        for (int h = 0; h < 3; h++) { //for each part of the tire (in-mid-out)
            float currentTemp = 0;
            std::string tirePartName;
            switch(h) {
                case 0:
                    tirePartName = "in";
                    currentTemp = currCar->_tyreT_in(i);
                break;
                case 1:
                    tirePartName = "mid";
                    currentTemp = currCar->_tyreT_mid(i);
                break;
                case 2:
                    tirePartName = "out";
                    currentTemp = currCar->_tyreT_out(i);
                break;
                default:
                    tirePartName = "";
            }
            
            std::ostringstream tireNameCold;
            std::ostringstream tireNameOptimal;
            std::ostringstream tireNameHot;
            
            tireNameCold << "tire-" << tireName.str().c_str() << tirePartName.c_str() << "-cold";
            tireNameOptimal << "tire-" << tireName.str().c_str() << tirePartName.c_str() << "-optimal";
            tireNameHot << "tire-" << tireName.str().c_str() << tirePartName.c_str() << "-hot";

            float optimalAlpha = 0.0f;
            float hotAlpha = 0.0f;

            float tempOptimal = currCar->_tyreT_opt(i);
            float tempMaxCold = tempOptimal - ( tempOptimal * 10 / 100 ); //temp at witch we will conside the tire maximun cold
            float tempMaxHot = tempOptimal + ( tempOptimal * 10 / 100 ); //temp at witch we will conside the tire maximun hot


            changeImageAlpha(this->hudImgElements[tireNameCold.str().c_str()], 1.0f);
            optimalAlpha = (currentTemp-tempMaxCold) / (tempOptimal-tempMaxCold);
            if (optimalAlpha > 1.0f){
                optimalAlpha = 1.0f;
            }
            if (optimalAlpha < 0.0f){
                optimalAlpha = 0.0f;
            }
            changeImageAlpha(this->hudImgElements[tireNameOptimal.str().c_str()], optimalAlpha);
            
            
            hotAlpha = (tempMaxHot-currentTemp) / (tempMaxHot-tempOptimal);
            if (hotAlpha > 1.0f){
                hotAlpha = 1.0f;
            }
            if (hotAlpha < 0.0f){
                hotAlpha = 0.0f;
            }

            changeImageAlpha(this->hudImgElements[tireNameHot.str().c_str()], 1.0-hotAlpha);
        }
        //temps string only do this for middle temps?
        temp.str("");
        //internally the tire temp is in KELVIN
        int tireTempInCelsius = currCar->_tyreT_mid(i)- 273.15;
        temp << tireTempInCelsius << " C";

        std::ostringstream tireNameText;
        tireNameText << "tire-" << tireName.str().c_str()  << "temps";
        hudTextElements[tireNameText.str().c_str()]->setText(temp.str());
    
    }

// tire wear
    changeImageSize(this->hudImgElements["tire-degradation-fr-on"], currCar->_tyreTreadDepth(0), "bottom", this->hudScale);
    changeImageSize(this->hudImgElements["tire-degradation-fl-on"], currCar->_tyreTreadDepth(1), "bottom", this->hudScale);
    changeImageSize(this->hudImgElements["tire-degradation-rr-on"], currCar->_tyreTreadDepth(2), "bottom", this->hudScale);
    changeImageSize(this->hudImgElements["tire-degradation-rl-on"], currCar->_tyreTreadDepth(3), "bottom", this->hudScale);

//tire slip
    float slip = 0.0f;
    slip = currCar->_wheelSlipNorm(0)/currCar->_wheelSlipOpt(0);
    changeImageAlpha(this->hudImgElements["tire-fr-slip"], slip);
    slip = currCar->_wheelSlipNorm(1)/currCar->_wheelSlipOpt(1);
    changeImageAlpha(this->hudImgElements["tire-fl-slip"], slip);
    slip = currCar->_wheelSlipNorm(2)/currCar->_wheelSlipOpt(2);
    changeImageAlpha(this->hudImgElements["tire-rr-slip"], slip);
    slip = currCar->_wheelSlipNorm(3)/currCar->_wheelSlipOpt(3);
    changeImageAlpha(this->hudImgElements["tire-rl-slip"], slip);

//gforces
    osg::BoundingBox gforcegraphbb =hudImgElements["gforces-graph"]->getBoundingBox();
    osg::BoundingBox gforcedotbb = hudImgElements["gforces-dot"]->getBoundingBox();
    osg::Vec3f position = calculatePosition(gforcedotbb,"mc",gforcegraphbb,"mc", 0.0f, 0.0f);
    changeImagePosition(
        this->hudImgElements["gforces-dot"],
        gforcegraphbb.xMin()+position.x()+currCar->_DynGC.acc.y * 5 * 1,//horizontal
        gforcegraphbb.yMin()+position.y()+currCar->_DynGC.acc.x * 5 * -1,//vertical
        this->hudScale
    );

// debug info
    temp.str("");
    temp << "FPS: " << frameInfo->fAvgFps << " (Inst: " << frameInfo->fInstFps << ")";
    hudTextElements["debug-info"]->setText(temp.str());


}

void SDHUD::ToggleHUD()
{
    if (hudElementsVisibilityStatusEnabled > 0){
        hudElementsVisibilityStatus["boardWidget"] =        (int)this->hudWidgets["boardWidget"]->getNodeMask();
        hudElementsVisibilityStatus["racepositionWidget"] = (int)this->hudWidgets["racepositionWidget"]->getNodeMask();
        hudElementsVisibilityStatus["racelapsWidget"] =     (int)this->hudWidgets["racelapsWidget"]->getNodeMask();
        hudElementsVisibilityStatus["laptimeWidget"] =      (int)this->hudWidgets["laptimeWidget"]->getNodeMask();
        hudElementsVisibilityStatus["carinfoWidget"] =      (int)this->hudWidgets["carinfoWidget"]->getNodeMask();
        hudElementsVisibilityStatus["carstatusWidget"] =    (int)this->hudWidgets["carstatusWidget"]->getNodeMask();
        hudElementsVisibilityStatus["driverinputWidget"] =  (int)this->hudWidgets["driverinputWidget"]->getNodeMask();
        hudElementsVisibilityStatus["driverinput-wheel"] =  (int)this->hudImgRotableElements["driverinput-wheel"]->getNodeMask();
        hudElementsVisibilityStatus["debugWidget"] =        (int)this->hudWidgets["debugWidget"]->getNodeMask();
        hudElementsVisibilityStatus["dashitemsWidget"] =    (int)this->hudWidgets["dashitemsWidget"]->getNodeMask();
#ifdef HUDDEBUG
        hudElementsVisibilityStatus["graphWidget"] =        (int)this->hudWidgets["graphWidget"]->getNodeMask();
#endif

        this->hudWidgets["boardWidget"]->setNodeMask(0);
        this->hudWidgets["racepositionWidget"]->setNodeMask(0);
        this->hudWidgets["racelapsWidget"]->setNodeMask(0);
        this->hudWidgets["laptimeWidget"]->setNodeMask(0);
        this->hudWidgets["carinfoWidget"]->setNodeMask(0);
        this->hudWidgets["carstatusWidget"]->setNodeMask(0);
        this->hudWidgets["driverinputWidget"]->setNodeMask(0);
        this->hudImgRotableElements["driverinput-wheel"]->setNodeMask(0);
        this->hudWidgets["debugWidget"]->setNodeMask(0);
        this->hudWidgets["dashitemsWidget"]->setNodeMask(0);
#ifdef HUDDEBUG
        this->hudWidgets["graphWidget"]->setNodeMask(0);
        this->setGraphNodeMask(0);
#endif
        hudElementsVisibilityStatusEnabled = 0;
    }else{
        this->hudWidgets["boardWidget"]->setNodeMask(hudElementsVisibilityStatus["boardWidget"]);
        this->hudWidgets["racepositionWidget"]->setNodeMask(hudElementsVisibilityStatus["racepositionWidget"]);
        this->hudWidgets["racelapsWidget"]->setNodeMask(hudElementsVisibilityStatus["racelapsWidget"]);
        this->hudWidgets["laptimeWidget"]->setNodeMask(hudElementsVisibilityStatus["laptimeWidget"]);
        this->hudWidgets["carinfoWidget"]->setNodeMask(hudElementsVisibilityStatus["carinfoWidget"]);
        this->hudWidgets["carstatusWidget"]->setNodeMask(hudElementsVisibilityStatus["carstatusWidget"]);
        this->hudWidgets["driverinputWidget"]->setNodeMask(hudElementsVisibilityStatus["driverinputWidget"]);
        this->hudImgRotableElements["driverinput-wheel"]->setNodeMask(hudElementsVisibilityStatus["driverinput-wheel"]);
        this->hudWidgets["debugWidget"]->setNodeMask(hudElementsVisibilityStatus["debugWidget"]);
        this->hudWidgets["dashitemsWidget"]->setNodeMask(hudElementsVisibilityStatus["dashitemsWidget"]);
#ifdef HUDDEBUG
        this->hudWidgets["graphWidget"]->setNodeMask(hudElementsVisibilityStatus["graphWidget"]);
        this->setGraphNodeMask(hudElementsVisibilityStatus["graphWidget"]);
#endif
        hudElementsVisibilityStatusEnabled = 1;
    }
}
void SDHUD::ToggleHUDboard()
{
    this->hudWidgets["boardWidget"]->setNodeMask(1 - this->hudWidgets["boardWidget"]->getNodeMask());
    
    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/boardWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["boardWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");
}
void SDHUD::ToggleHUDraceinfo()
{
    this->hudWidgets["racepositionWidget"]->setNodeMask(1 - this->hudWidgets["racepositionWidget"]->getNodeMask());
    this->hudWidgets["racelapsWidget"]->setNodeMask(1 - this->hudWidgets["racelapsWidget"]->getNodeMask());

    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/racepositionWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["racepositionWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    
    //save the current status in the config file
    path.assign("widgets/racelapsWidget");
    value = this->hudWidgets["racelapsWidget"]->getNodeMask();
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
 
    //write
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");

}
void SDHUD::ToggleHUDlaptime()
{
    this->hudWidgets["laptimeWidget"]->setNodeMask(1 - this->hudWidgets["laptimeWidget"]->getNodeMask());
    
    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/laptimeWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["laptimeWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");
}
void SDHUD::ToggleHUDcarinfo()
{
    this->hudWidgets["carinfoWidget"]->setNodeMask(1 - this->hudWidgets["carinfoWidget"]->getNodeMask());
    this->hudWidgets["carstatusWidget"]->setNodeMask(1 - this->hudWidgets["carstatusWidget"]->getNodeMask());
    
    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/carinfoWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["carinfoWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    
    path.assign("widgets/carstatusWidget");
    value = this->hudWidgets["carstatusWidget"]->getNodeMask();
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);

    //write the file
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");
    
}
void SDHUD::ToggleHUDdriverinput()
{
    this->hudWidgets["driverinputWidget"]->setNodeMask(1 - this->hudWidgets["driverinputWidget"]->getNodeMask());
    this->hudImgRotableElements["driverinput-wheel"]->setNodeMask(this->hudWidgets["driverinputWidget"]->getNodeMask());

    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/driverinputWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["driverinputWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");

}
void SDHUD::ToggleHUDdebug()
{
    //toggle the visibility
    this->hudWidgets["debugWidget"]->setNodeMask(1 - this->hudWidgets["debugWidget"]->getNodeMask());
    
    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/debugWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["debugWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");

}

void SDHUD::ToggleHUDdashitems()
{
    //toggle the visibility
    this->hudWidgets["dashitemsWidget"]->setNodeMask(1 - this->hudWidgets["dashitemsWidget"]->getNodeMask());
    
    //save the current status in the config file
    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path= "widgets/dashitemsWidget";
    std::string attribute= "enabled";
    int value = this->hudWidgets["dashitemsWidget"]->getNodeMask();
    
    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");

}

#ifdef HUDDEBUG
void SDHUD::setGraphNodeMask(int mask)
{
    for (std::map<std::string, OSGPLOT *>::iterator it = hudGraphElements.begin(); it != hudGraphElements.end(); ++it)
    {
        it->second->setNodeMask(mask);
    }
}

void SDHUD::ToggleHUDgraph()
{
    //toggle the visibility
    this->hudWidgets["graphWidget"]->setNodeMask(1 - this->hudWidgets["graphWidget"]->getNodeMask());

    int value = this->hudWidgets["graphWidget"]->getNodeMask();
    setGraphNodeMask(value);

    //save the current status in the config file
    std::string configFileUrl = GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    std::string path = "widgets/graphWidget";
    std::string attribute = "enabled";

    //read the config file, update the value and write it back
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
    GfParmSetNum(paramHandle, path.c_str(), attribute.c_str(), NULL, (int)value);
    GfParmWriteFile(NULL, paramHandle, "osghudconfig");
    }
#endif

osg::ref_ptr <osg::Group> SDHUD::generateHudFromXmlFile(int scrH, int scrW)
{
    this->osgGroupHud = new osg::Group;
    osg::ref_ptr<osg::Group> osgGroupWidgets = new osg::Group;

    /*
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osgGroupHud->addChild(geode);

    // turn lighting off for the text and disable depth test to ensure it's always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    */
    this->osgGroupHud->addChild(osgGroupWidgets);

    //screen bounding box
    osg::BoundingBox screenBB;
    screenBB.expandBy(osg::Vec3(0.0f,0.0f,0.0f));
    screenBB.expandBy(osg::Vec3(scrW,scrH,0.0f));

    std::string mainSection= "widgets";
    std::string widgetsSectionPath;
    std::string widgetsSectionName= "widgets";
    std::string sectionPath;
    std::string subSectionPath;
    std::string subSectionName;

    std::string configFileUrl= GetLocalDir();
    configFileUrl.append("config/osghudconfig.xml");
    // int paramValue = 0;

    //open the file
    void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);


    //GfLogInfo("OSGHUD: Try to find all mains sections: %s \n", subSectionName.c_str());

    //cicle throught the widget sections
    if (GfParmListSeekFirst(paramHandle, mainSection.c_str()) == 0)
    {
        do
        {
            widgetsSectionName = GfParmListGetCurEltName(paramHandle, mainSection.c_str());
            widgetsSectionPath = mainSection + "/" + widgetsSectionName;

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->setName(widgetsSectionName);
            osgGroupWidgets->addChild(geode);
            this->hudWidgets[widgetsSectionName.c_str()] = geode;

            // turn lighting off for the text and disable depth test to ensure it's always ontop.
            osg::StateSet* stateset = geode->getOrCreateStateSet();
            stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

            //read enabled disabled status for this widget from the config file
            int widgetEnabled = GfParmGetNum (paramHandle, widgetsSectionPath.c_str(),"enabled", "",0);

            //cicle throught each element of the widget
            if (GfParmListSeekFirst(paramHandle, widgetsSectionPath.c_str()) == 0)
            {
                do
                {
                        subSectionName = GfParmListGetCurEltName(paramHandle, widgetsSectionPath.c_str());
                        subSectionPath = widgetsSectionPath + "/" + subSectionName;

                        //get a list of the params in this section
                        std::vector<std::string> paramsInSection = GfParmListGetParamsNamesList(paramHandle, subSectionPath.c_str());
                        std::string type = GfParmGetStr (paramHandle, subSectionPath.c_str(),"type", "no default value" );

                        /* ============================
                             CREATE OSG TEXT
                           ============================*/
                        if (type == "text" )
                        {
                            //read data into local variables
                            const std::string &elementId =     subSectionName;
                            std::string textStr =         GfParmGetStr (paramHandle, subSectionPath.c_str(),"text", "" );
                            std::string fontFileUrl =     GfParmGetStr (paramHandle, subSectionPath.c_str(),"fontFileUrl", "" );
                            std::string colorString =     GfParmGetStr (paramHandle, subSectionPath.c_str(),"color", "" );
                            float fontSize =             GfParmGetNum (paramHandle, subSectionPath.c_str(),"fontSize", "",0 ) * this->hudScale;
                            std::string textAlign =     GfParmGetStr (paramHandle, subSectionPath.c_str(),"textAlign", "" );

                            std::string positionRefObj =             GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-refObj", "" );
                            std::string positionRefObjPoint =         GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-refObjPoint", "tl" );
                            std::string positionMyPoint =             GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-myPoint", "tl" );
                            float positionVerticalModifier =         GfParmGetNum (paramHandle, subSectionPath.c_str(),"position-verticalModifier", "",0 ) * this->hudScale;
                            float positionHorizontalModifier =         GfParmGetNum (paramHandle, subSectionPath.c_str(),"position-horizontalModifier", "",0 ) * this->hudScale;

                            GfLogDebug("OSGHUD: Generate text object: %s \n", elementId.c_str());

                            //create the osg::text object
                            osgText::Text* text = new  osgText::Text;

                            //add the text obj to our pool (we willl need it later)
                            hudTextElements[elementId] = text;

                            //extract and apply the color
                            {
                                std::vector<std::string> colorStringVector = split(colorString, '#');

                                osg::Vec4 color = osg::Vec4(
                                    std::atof(colorStringVector[0].c_str()),
                                    std::atof(colorStringVector[1].c_str()),
                                    std::atof(colorStringVector[2].c_str()),
                                    std::atof(colorStringVector[3].c_str())
                                );

                                text->setColor(color);
                            }

                            //set the font
                            {
                                std::string fontsMainDirectory = GetDataDir();
                                fontsMainDirectory = fontsMainDirectory+"data/fonts";
                                fontFileUrl = fontsMainDirectory+fontFileUrl;
                                text->setFont(fontFileUrl);

                                //font resolution
                                text->setFontResolution(200,200);

                                //set the font size
                                text->setCharacterSize(fontSize);
                            }

                            //set alignement
                            if (textAlign=="")
                            {
                                text->setAlignment(osgText::Text::LEFT_BOTTOM_BASE_LINE );
                            }
                            else if (textAlign=="LEFT_BOTTOM")
                            {
                                text->setAlignment(osgText::Text::LEFT_BOTTOM_BASE_LINE );
                            }
                            else if (textAlign=="RIGHT_BOTTOM")
                            {
                                text->setAlignment(osgText::Text::RIGHT_BOTTOM_BASE_LINE );
                            }

                            //set the text string
                            text->setText(textStr);

                            //set the position
                            //find the referenceObj pointer and then get his bounding box
                            osg::BoundingBox refObjBb;

                            if ( hudTextElements.find(positionRefObj) != hudTextElements.end() )
                            {
                                refObjBb = hudTextElements[positionRefObj]->getBoundingBox();
                            }
                            else if ( this->hudImgElements.find(positionRefObj) != this->hudImgElements.end() )
                            {
                                refObjBb = this->hudImgElements[positionRefObj]->getBoundingBox();
                            }
#ifdef HUDDEBUG
                            else if ( this->hudGraphElements.find(positionRefObj) != this->hudGraphElements.end() )
                            {
                                //refObjBb = this->hudGraphElements[positionRefObj]->getBoundingBox();
                            }
#endif
                            else
                            {
                                GfLogDebug("OSGHUD: No (valid) reference object given for the current element alignement: Assuming Screen!\n");
                                refObjBb = screenBB;
                            }

                            //calculate the positioning
                            osg::Vec3 position = calculatePosition(text->getBoundingBox(),positionMyPoint,refObjBb,positionRefObjPoint, positionVerticalModifier, positionHorizontalModifier);

                            //asign the position
                            text->setPosition(position);

                            //TODO: enable shadows for texts?
                            //text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT); //OUTLINE //DROP_SHADOW_BOTTOM_RIGHT
                            /*
                            text->setBackdropOffset(0.07f);
                            color = osg::Vec4(0.57f,0.57f,0.57f,0.6f);
                            text->setBackdropColor(color);
                            */

                            //add the text to the hud
                            geode->addDrawable( text );

                        }
                        else if( type == "image" || type == "imagerotable")
                        {
                            /* ============================
                                 CREATE OSG IMAGE
                               ============================*/
                            //read data into local variables
                            const std::string &elementId =             subSectionName;
                            std::string url =                         GfParmGetStr (paramHandle, subSectionPath.c_str(),"url", "" );

                            std::string positionRefObj =             GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-refObj", "" );
                            std::string positionRefObjPoint =         GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-refObjPoint", "tl" );
                            std::string positionMyPoint =             GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-myPoint", "tl" );
                            float positionVerticalModifier =         GfParmGetNum (paramHandle, subSectionPath.c_str(),"position-verticalModifier", "",0 ) * this->hudScale;
                            float positionHorizontalModifier =         GfParmGetNum (paramHandle, subSectionPath.c_str(),"position-horizontalModifier", "",0 ) * this->hudScale;

                            GfLogDebug("OSGHUD: Generate image object: %s \n", elementId.c_str());

                            //start preparing the image
                            std::string filename = GetDataDir();
                            filename = filename+url;

                            //get the bounding box
                            osg::BoundingBox bb;

                            for(unsigned int i=0;i<geode->getNumDrawables();++i)
                            {
                                bb.expandBy(geode->getDrawable(i)->getBoundingBox());
                            }

                            //check that the image file exist
                            if (!GfFileExists(filename.c_str()))
                            {
                                GfLogError ("OSGHUD: Specified image file does not exist: %s.\n", filename.c_str());
                            }

                            osg::Image* img = osgDB::readImageFile(filename);

                            //correct the image size to match the hud scale
                            float imgWidth = img->s() *  this->hudScale;
                            float imgHeight = img->t() * this->hudScale;

                            // create geometry
                            osg::Geometry* geom = new osg::Geometry;
                            this->hudImgElements[elementId] =  geom;

                            //set the position
                            //find the referenceObj pointer and then get his bounding box
                            osg::BoundingBox refObjBb;
                            if ( hudTextElements.find(positionRefObj) != hudTextElements.end() )
                            {
                                refObjBb = hudTextElements[positionRefObj]->getBoundingBox();
                            }
                            else if ( this->hudImgElements.find(positionRefObj) != this->hudImgElements.end() )
                            {
                                refObjBb = this->hudImgElements[positionRefObj]->getBoundingBox();
                            }
#ifdef HUDDEBUG
                            else if ( this->hudGraphElements.find(positionRefObj) != this->hudGraphElements.end() )
                            {
                                //refObjBb = this->hudGraphElements[positionRefObj]->getBoundingBox();
                            }
#endif
                            else
                            {
                                GfLogDebug("OSGHUD: No (valid) reference object given for the current element alignement: Assuming Screen!\n");
                                refObjBb = screenBB;
                            }

                            //get object bounding box
                            osg::BoundingBox myObjBb;
                            myObjBb.expandBy(osg::Vec3(0.0f,0.0f,0.0f));
                            myObjBb.expandBy(osg::Vec3(imgWidth,imgHeight,0.0f));

                            //calculate the positioning
                            osg::Vec3 position = calculatePosition(myObjBb,positionMyPoint,refObjBb,positionRefObjPoint, positionVerticalModifier, positionHorizontalModifier);

                            //asign the position
                            float positionLeft =   position.x();
                            float positionBottom = position.y();

                            //create the vertices for the image geometry and assign them
                            osg::Vec3Array* vertices = new osg::Vec3Array;
                            float depth = 0.0f-0.1f;
                            vertices->push_back(osg::Vec3( positionLeft            ,positionBottom,depth)); //bottom left
                            vertices->push_back(osg::Vec3( positionLeft+imgWidth,positionBottom,depth)); //bottom right
                            vertices->push_back(osg::Vec3( positionLeft+imgWidth,positionBottom+imgHeight     ,depth)); //top right
                            vertices->push_back(osg::Vec3( positionLeft            ,positionBottom+imgHeight     ,depth)); //topleft
                            geom->setVertexArray(vertices);

                            // texture the geometry and apply material
                            // calculate textures coordinates
                            osg::Vec2Array* texcoords = new osg::Vec2Array(4);
                            (*texcoords)[0].set(0.0f, 0.0f);
                            (*texcoords)[1].set(1.0f, 0.0f);
                            (*texcoords)[2].set(1.0f, 1.0f);
                            (*texcoords)[3].set(0.0f, 1.0f);
                            geom->setTexCoordArray(0,texcoords);

                            // calculate normals
                            osg::Vec3Array* normals = new osg::Vec3Array(1);
                            (*normals)[0].set(0.0f,-1.0f,0.0f);
                            geom->setNormalArray(normals);
                            geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

                            // assign colors
                            osg::Vec4Array* colors = new osg::Vec4Array(1);
                            (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
                            geom->setColorArray(colors);
                            geom->setColorBinding(osg::Geometry::BIND_OVERALL);

                            // draw the vertices as quads
                            geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

                            // disable display list so our modified tex coordinates show up
                            geom->setUseDisplayList(false);

                            // setup texture
                            osg::TextureRectangle* texture = new osg::TextureRectangle;

                            texture->setImage(img);

                            // setup stateset
                            osg::StateSet* state = geom->getOrCreateStateSet();
                            state->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

                            //Conversely, disable write depth cache,
                            //Make objects behind transparent polygons visible
                            //OSG draws transparent polygons first, then draws opaque polygons
                            osg :: Depth * imgDepth = new osg :: Depth;
                            imgDepth-> setWriteMask (false);
                            state-> setAttributeAndModes (imgDepth, osg :: StateAttribute :: ON);

                            // setup material
                            osg::TexMat* texmat = new osg::TexMat;
                            texmat->setScaleByTextureRectangleSize(true);
                            state->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

                            //enable gl_blending (for texture transparency)
                            state->setMode(GL_BLEND, osg::StateAttribute::ON);
                            osg::BlendFunc* blend = new osg::BlendFunc;
                            blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_DST_ALPHA);

                            // turn off lighting (light always on)
                            state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

                            if (type == "image")
                            {
                                //add the image geometry to the hud
                                geode->addDrawable(geom);
                            }
                            else if(type == "imagerotable")
                            {
                                osg::MatrixTransform* transform = new osg::MatrixTransform;
                                const double angle = 0.0;
                                const osg::Vec3d axis(0, 0, 1);
                                transform->setMatrix(osg::Matrix::rotate(angle, axis)); 
                                transform->addChild(geom);
                                this->osgGroupHud->addChild(transform);
                                this->hudImgRotableElements[elementId] =  transform;
                                if(widgetEnabled>0){
                                    this->hudImgRotableElements["driverinput-wheel"]->setNodeMask(1);
                                }else{
                                    this->hudImgRotableElements["driverinput-wheel"]->setNodeMask(0);
                                }
                            }
                        }
                        else if( type == "graph")
                        {
#ifdef HUDDEBUG
                            /* ============================
                                 CREATE OSG GRAPH
                               ============================*/
                            //read data into local variables
                            const std::string &elementId = subSectionName;

                            //positioning variables
                            std::string positionRefObj =       GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-refObj", "" );
                            std::string positionRefObjPoint =  GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-refObjPoint", "tl" );
                            std::string positionMyPoint =      GfParmGetStr (paramHandle, subSectionPath.c_str(),"position-myPoint", "tl" );
                            float positionVerticalModifier =   GfParmGetNum (paramHandle, subSectionPath.c_str(),"position-verticalModifier", "",0 ) * this->hudScale;
                            float positionHorizontalModifier = GfParmGetNum (paramHandle, subSectionPath.c_str(),"position-horizontalModifier", "",0 ) * this->hudScale;

                            //graph variables
                            float width =                      GfParmGetNum (paramHandle, subSectionPath.c_str(),"width", "",0 );
                            float height =                     GfParmGetNum (paramHandle, subSectionPath.c_str(),"height", "",0 );
                            float maxValue =                   GfParmGetNum (paramHandle, subSectionPath.c_str(),"maxValue", "",0 );
                            float minValue =                   GfParmGetNum (paramHandle, subSectionPath.c_str(),"minValue", "",0 );
                            float timeFrame =                  GfParmGetNum (paramHandle, subSectionPath.c_str(),"timeFrame", "",0 );
                            float referenceLineAtValue =       GfParmGetNum (paramHandle, subSectionPath.c_str(),"referenceLineAtValue", "",0 );
                            std::string Xdata =                GfParmGetStr (paramHandle, subSectionPath.c_str(),"Xdata", "" );
                            std::string Ydata =                GfParmGetStr (paramHandle, subSectionPath.c_str(),"Ydata", "" );
                            std::string title =                GfParmGetStr (paramHandle, subSectionPath.c_str(),"title", "" );

                            GfLogDebug("OSGHUD: Generate graph object: %s \n", elementId.c_str());

                            //calculate position
                            //find the referenceObj pointer and then get his bounding box
                            osg::BoundingBox refObjBb;
                            if ( hudTextElements.find(positionRefObj) != hudTextElements.end() )
                            {
                                refObjBb = hudTextElements[positionRefObj]->getBoundingBox();
                            }
                            else if ( this->hudImgElements.find(positionRefObj) != this->hudImgElements.end() )
                            {
                                refObjBb = this->hudImgElements[positionRefObj]->getBoundingBox();
                            }

                            else if ( this->hudGraphElements.find(positionRefObj) != this->hudGraphElements.end() )
                            {
                                //refObjBb = this->hudGraphElements[positionRefObj]->getBoundingBox();
                            }

                            else
                            {
                                GfLogDebug("OSGHUD: No (valid) reference object given for the current element alignement: Assuming Screen!\n");
                                refObjBb = screenBB;
                            }
                            
                            //calculate our bounding box
                            osg::BoundingBox plotBB;
                            plotBB.expandBy(osg::Vec3(0.0f,0.0f,0.0f));
                            plotBB.expandBy(osg::Vec3(width,height,0.0f));

                              //calculate the positioning
                            osg::Vec3 position = calculatePosition(plotBB,positionMyPoint,refObjBb,positionRefObjPoint, positionVerticalModifier, positionHorizontalModifier);

                            float positionX = position.x();
                            float positionY = position.y();

                            //istantiate the graph
                            this->hudGraphElements[elementId] = new OSGPLOT(
                                positionX,
                                positionY,
                                width,
                                height,
                                maxValue,
                                minValue,
                                timeFrame,
                                referenceLineAtValue,
                                Xdata,
                                Ydata,
                                title
                            );

                            this->osgGroupHud->addChild(this->hudGraphElements[elementId]->getGroup());
                            this->hudGraphElements[elementId]->setNodeMask(widgetEnabled);
#endif
                        }
                        else
                        {
                        /* ============================
                             NO MORE HUD TYPE OPTIONS...??
                           ============================*/
                            GfLogDebug("OSG-HUD object type not recognized: %s", type.c_str());
                        }

                } while (GfParmListSeekNext(paramHandle, widgetsSectionPath.c_str()) == 0);
            }
            
            //only display the widget element if the widget is enabled
            if(widgetEnabled > 0){
                geode->setNodeMask(1);
            }else{
                geode->setNodeMask(0);
            }

        } while (GfParmListSeekNext(paramHandle, mainSection.c_str()) == 0);
    }

    //release the xml file
    GfParmReleaseHandle(paramHandle);

    //return the hud object
    return (*this->osgGroupHud).asGroup();
}

SDHUD::~SDHUD()
{
    //TODO: check we may have something more to clean up
    //do some cleanup
    hudTextElements.clear();
    this->hudImgElements.clear();
#ifdef HUDDEBUG
    this->hudGraphElements.clear();
#endif
}
