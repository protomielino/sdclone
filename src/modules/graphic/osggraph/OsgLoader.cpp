#include "OsgLoader.h"
//#include "ReaderWriterACC.h"

#include <tgf.hpp>

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
	if (ext == "acc")
	{
		//Use custom ACC file loader
		osgDB::ReaderWriter::ReadResult rr = m_ACCReader.readNode(strFile, m_pOpt);
	    if (rr.validNode()) 
			return rr.takeNode();
		else
			return NULL;
	}
	else 
	{
		pNode = osgDB::readNodeFile(strFile, m_pOpt);
	}	
	
	GfOut("le test %d \n",pNode);
	return pNode;
}
