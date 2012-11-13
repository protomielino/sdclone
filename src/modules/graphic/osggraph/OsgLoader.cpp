#include "OsgLoader.h"
//#include "ReaderWriterACC.h"
#include <tgf.hpp>

#include <tgf.hpp>
#include <osg/MatrixTransform>
#include <osg/Node>

osgLoader::osgLoader(void)
{
	m_pOpt = new osgDB::ReaderWriter::Options();
}

osgLoader::~osgLoader(void)
{
}

void osgLoader::AddSearchPath(std::string strPath)
{

		m_pOpt->getDatabasePathList().push_front(strPath);
}

osg::ref_ptr<osg::Image> osgLoader::LoadImageFile(std::string strFile)
{
	osg::ref_ptr<osg::Image> Image;
    std::string absFileName = osgDB::findDataFile(strFile, m_pOpt);
	if (absFileName.empty())
		return Image;

	Image = osgDB::readRefImageFile(absFileName, m_pOpt);
	return Image;
}

osg::Node *osgLoader::Load3dFile(std::string strFile)
{
	osg::Node *pNode = NULL;	
	std::string ext = osgDB::getFileExtension(strFile);
    if (ext == "acc" || ext == "ac")
	{
		//Use custom ACC file loader
		osgDB::ReaderWriter::ReadResult rr = m_ACCReader.readNode(strFile, m_pOpt);
       GfOut("le test %d \n",rr.validNode());
       if (rr.validNode()) {
            osg::Node * nod = rr.takeNode();
            osg::MatrixTransform * rot = new osg::MatrixTransform;
            osg::Matrix mat( 1.0f,  0.0f, 0.0f, 0.0f,
                             0.0f,  0.0f, 1.0f, 0.0f,
                             0.0f, -1.0f, 0.0f, 0.0f,
                             0.0f,  0.0f, 0.0f, 1.0f);
            rot->setMatrix(mat);
            rot->addChild(nod);
            return rot;
       }
        else{
            return NULL;}
	}
	else 
	{
		pNode = osgDB::readNodeFile(strFile, m_pOpt);
        GfOut("le test %d \n",pNode);
	}	
	
    osg::ref_ptr<osg::MatrixTransform> rot = new osg::MatrixTransform;
    osg::Matrix mat( 1.0f,  0.0f, 0.0f, 0.0f,
                     0.0f,  0.0f, 1.0f, 0.0f,
                     0.0f, -1.0f, 0.0f, 0.0f,
                     0.0f,  0.0f, 0.0f, 1.0f);
    rot->setMatrix(mat);
    rot->addChild(pNode);

	GfOut("le test %d \n",pNode);
    return rot.get();
}
