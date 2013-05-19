// 30 Oct 2002
// ACC3D loader for models generated by the AC3D modeller (www.ac3d.org)
// part of this source code were supplied by the AC3D project (Andy Colebourne)
// eg the basic parsing of an AC3D file.
// Conversion from AC3D scenegraph to OSG by GW Michel.

#include <vector>
#include <string>
#include <string.h>		// to removed on OSG 3.2
#include <iostream>
#include <stdlib.h>
#include <sstream>

#include <osg/GL>
#include <osg/GLU>

#include <osg/Math>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/Math>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/StateSet>
#include <osg/ShadeModel>
#include <osg/Math>
#include <osg/Notify>

#include <osgUtil/Tessellator>
#include <osgUtil/SmoothingVisitor>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/fstream>

#include "AccException.h"
#include "AccGeode.h"
#include "ReaderWriterACC.h"

using namespace osgDB;

class geodeVisitor : public osg::NodeVisitor
{ // collects geodes from scene sub-graph attached to 'this'
        public:
            geodeVisitor():
                osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

            ~geodeVisitor() { _geodelist.clear();}

            // one apply for each type of Node that might be a user transform
            virtual void apply(osg::Geode& geode)
                        {
                _geodelist.push_back(&geode);
            }

	    virtual void apply(osg::Group& gp)
			{
		traverse(gp);    // must continue subgraph traversal.
	    }

	    std::vector<const osg::Geode *> getGeodes() {return _geodelist;}

        protected:
            typedef std::vector<const osg::Geode *>    Geodelist;
            Geodelist  _geodelist;
};

ReaderWriterACC::ReaderWriterACC()
{
    supportsExtension("acc","Speed Dreams Database format");
        m_bCar = false;
        m_transparentGroup = NULL;
        m_opaqueGroup = NULL;
}

const char* ReaderWriterACC::className()
{
        return "ACC Speed Dreams Database Reader";
}

void ReaderWriterACC::SetCar(bool b)
{
        m_bCar = b;
}

osg::Node* ReaderWriterACC::readFile(std::istream& stream, const osgDB::ReaderWriter::Options* options)
{
    FileData fileData(options);
    osg::Group *pWorld = new osg::Group;
    osg::Matrix identityTransform;

	while ((stream.good())&&(!stream.eof()))
	{
		osg::Node* node = readObject(stream, fileData, identityTransform, TextureData());
		if (node)
			pWorld->addChild(node);
	}
    return pWorld;
}

osg::Node* ReaderWriterACC::readObject(std::istream& stream, FileData& fileData, const osg::Matrix& parentTransform, TextureData textureData)
{
	int textureId = 0;
	std::string texname0,texname1,texname2,texname3;
    // most of this logic came from Andy Colebourne (developer of the AC3D editor) so it had better be right!
    // The transform configured in this current object level
    osg::Matrix transform;
    // The vertex pool in this object
    osg::ref_ptr<VertexSet> vertexSet = new VertexSet;
        osg::ref_ptr<osg::Group> group = NULL;
    //osg::ref_ptr<osg::Group> group = new osg::Group;
    osg::Vec2 textureOffset(0, 0);
    osg::Vec2 textureRepeat(1, 1);
    float creaseAngle = 61;
        unsigned objectType = ac3d::ObjectTypeGroup;
        std::string strName;

    while (!stream.eof() && stream.good())
        {
        std::string token;
        stream >> token;

	if (token == "MATERIAL")
		{
	    MaterialData mat;
	    mat.readMaterial(stream);
	    fileData.addMaterial(mat);
	}
	else if (token == "OBJECT")
		{
	    std::string type;
	    stream >> type;

			if (!m_bCar)
			{
				group = new osg::Group();
				group->setDataVariance(osg::Object::STATIC);
			}
			else
			{
				group = m_opaqueGroup;
			}

	    if (type == "group")
			{
		objectType = ac3d::ObjectTypeGroup;

                        }
            else if (type == "world")
                        {
                objectType = ac3d::ObjectTypeGroup;
                                if (m_bCar)
                                {
                                        group = new osg::Group();
                                        //group->setDataVariance(osg::Object::STATIC);
                                        group->setName("World");
                                        m_transparentGroup = new osg::Group();
                                        m_transparentGroup->setName("Transparent");
                                        m_opaqueGroup = new osg::Group();
                                        m_opaqueGroup->setName("Opaque");
                                        group->addChild(m_opaqueGroup);
                                        group->addChild(m_transparentGroup);
                                }
                        }
                        else if (type == "poly")
                        {
                                   objectType = ac3d::ObjectTypeNormal;
                        }
            else
                objectType = ac3d::ObjectTypeNormal;
        }
        else if (token == "crease")
                {
            stream >> creaseAngle;
        }
        else if (token == "data")
                {
            int len;
            stream >> len;
            std::vector<char> tmp(len);
            stream.read(&(tmp[0]), len);
        }
        else if (token == "name")
                {
                        strName = readString(stream);
                        m_bBlockTransparent = false;

			if (m_bCar)
			{
		//bool b;
		/*if (strName == "GRILL1_s_0")
		    b = true;*/
		if (!strncmp(strName.c_str(), ":WI", 3) || !strncmp(strName.c_str(), "WI", 2))
				{
					group = m_transparentGroup;
					m_bBlockTransparent = false;
				}
				else
					m_bBlockTransparent = true;
			}
			else
			{
				if (group)
					group->setName(strName.c_str());
			}
	}
	else if (token == "texture")
		{
	    // read the texture name
	    std::string texname = readString(stream);

            // strip absolute paths
            if (texname[0] == '/' || (isalpha(texname[0]) && texname[1] == ':'))
                        {
                std::string::size_type p = texname.rfind('\\');
                if (p != std::string::npos)
                    texname = texname.substr(p+1, std::string::npos);
                p = texname.rfind('/');
                if (p != std::string::npos)
                    texname = texname.substr(p+1, std::string::npos);
            }
                if (texname == "empty_texture_no_mapping")
                                texname = "";

			if (!m_bCar)
			{
				switch (textureId)
				{
					case 0:
					texname0 = texname;
					break;

					case 1:
					texname1 = texname;
					break;

					case 2:
					texname2 = texname;
					break;

					case 3:
					texname3 = texname;
					break;
				}
			}
			else
			{
				if (textureId == 0)
					texname0 = texname;
			}
			textureId++;
	}
		else if (token == "base")
		{
			//TODO
		}
		else if (token == "tiled")
		{
			//TODO
		}
		else if (token == "skids")
		{
			//TODO
		}
		else if (token == "shad")
		{

                }
        else if (token == "texrep")
                {
            stream >> textureRepeat[0] >> textureRepeat[1];
        }
        else if (token == "texoff")
                {
            stream >> textureOffset[0] >> textureOffset[1];
        }
        else if (token == "rot")
                {
            for (unsigned n = 0; n < 3; ++n)
                for (unsigned m = 0; m < 3; ++m)
                    stream >> transform(m, n);
        }
        else if (token == "loc")
                {
            for (unsigned n = 0; n < 3; ++n)
                stream >> transform(3, n);
        }
        else if (token == "url")
                {
            std::string url;
            stream >> url;
            group->addDescription(url);
        }
        else if (token == "numvert")
                {
            osg::Matrix currentTransform = transform*parentTransform;

            unsigned num;
            stream >> num;

	    if (num != 0)
			{
		vertexSet->reserve(num);
				char line[256];
				stream.getline(line,256);

		for (unsigned n = 0; n < num; ++n)
				{
					stream.getline(line,256);
					std::stringstream ss(line);
					osg::Vec3 p,nm;
		    ss >> p[0] >> p[1] >> p[2];

					nm[0] = 0.0;
					nm[1] = 1.0;
					nm[2] = 0.0;

					ss >> nm[0] >> nm[1] >> nm[2];

                    vertexSet->addVertex(currentTransform.preMult(p));
                }
            }
        }
        else if (token == "numsurf")
                {
                        textureData = fileData.toTextureData(texname0,texname1,texname2,texname3,m_bBlockTransparent);

            unsigned num;
            stream >> num;

	    if (0 < num)
			{
		// list of materials required- generate one geode per material
		std::vector<Bins> primitiveBins(fileData.getNumMaterials());

		for (unsigned n = 0; n < num; ++n)
				{
		    std::string token;
		    stream >> token;

		    if (token != "SURF")
					{
            osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: expected SURF line while reading object \""
						<< group->getName() << "\"!" << std::endl;
			return group.release();
		    }

                    stream >> token;
                    unsigned flags = strtol(token.c_str(), NULL, 0);
                    //unsigned v = flags &0xF;
                    //bool bNotStrip = false;
                    /*if (v!=4)
                        bNotStrip = false;*/

                    stream >> token;
                    if (token != "mat")
                                        {
                        osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: expected mat line while reading object \""
                                                << group->getName() << "\"!" << std::endl;
                        return group.release();
                    }

                    // read the material index
                    unsigned matIdx;
                    stream >> matIdx;
                    if (primitiveBins.size() <= matIdx)
                                        {
                        osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: invalid material number while reading object \""
                                                << group->getName() << "\"1" << std::endl;
                        return group.release();
                    }

                    // now get the correct PrimitiveBin
                    PrimitiveBin* primitiveBin = 0;
                    primitiveBin = primitiveBins[matIdx].getOrCreatePrimitiveBin(flags, vertexSet.get());
                    if (!primitiveBin)
                                        {
                        osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: unexpected primitive flags while reading object \""
                                                << group->getName() << "\"2" << std::endl;
                        return group.release();
                    }

                    // read the refs
                    stream >> token;
                    if (token != "refs")
                                        {
                        osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: expected refs line while reading object \""
                                                << group->getName() << "\"3" << std::endl;
                        return group.release();
                    }

                    unsigned nRefs = 0;
                    stream >> nRefs;
                    if (!stream)
                                        {
                        osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: could not read number of refs while reading object \""
                                                << group->getName() << "\"4" << std::endl;
                        return group.release();
                    }

                    // in case this is an invalid refs count for this primitive
                    // read further, but do not store that primitive
                    bool acceptPrimitive = primitiveBin->beginPrimitive(nRefs);
                    //int UVCount = 1;

		    for (unsigned i = 0; i < nRefs; ++i)
					{
			// Read the vertex index
			unsigned index;
			stream >> index;
			if (vertexSet->size() <= index)
			{
                osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: invalid ref vertex index while reading object \""
						    << group->getName() << "\"5" << std::endl;
			    return group.release();
			}

			// Read the texture corrdinates
						osg::Vec2 texCoord,texCoord2,texCoord3;
						std::string strUVs;
						std::getline(stream,strUVs);

						char * pch;
						pch = strtok ((char*)strUVs.c_str()," ");
						texCoord[0] = atof(pch);
						pch = strtok (NULL, " ");
						texCoord[1] = atof(pch);
						pch = strtok (NULL, " ");

			//UVCount = 1;
						if (pch)
						{
							texCoord2[0] = atof(pch);
							pch = strtok (NULL, " ");
							texCoord2[1] = atof(pch);
							pch = strtok (NULL, " ");
			    //UVCount = 2;
						}

						if (pch)
						{
							texCoord3[0] = atof(pch);
							pch = strtok (NULL, " ");
							texCoord3[1] = atof(pch);
							pch = strtok (NULL, " ");
			    //UVCount = 3;
						}

			if (!stream)
						{
                osg::notify(osg::WARN) << "osgDB SPEED DREAMS reader: could not parse texture coords while reading object \""
						   << group->getName() << "\" setting to (0,0)" << std::endl;
			    stream.clear();
			    std::string dummy;
			    std::getline(stream, dummy);
			}

                        if (acceptPrimitive)
                        {
                            texCoord[0] = textureOffset[0] + texCoord[0]*textureRepeat[0];
                            texCoord[1] = textureOffset[1] + texCoord[1]*textureRepeat[1];

                            if (!primitiveBin->vertex(index, texCoord,texCoord2,texCoord3))
                            {
                                return group.release();
                            }
                        }
                    }

                    if (acceptPrimitive)
                    {
                        if (!primitiveBin->endPrimitive())
                        {
                            return group.release();
                        }
                    }
                }

                for (unsigned i = 0; i < primitiveBins.size(); ++i)
                    primitiveBins[i].finalize(group.get(), fileData.getMaterial(i), textureData);
            }
        }
        else if (token == "kids")
                {
                        osg::Node *k = NULL;

            unsigned num;
            stream >> num;
            if (num != 0)
                        {
                for (unsigned n = 0; n < num; n++)
                                {
                    k = readObject(stream, fileData, transform*parentTransform, textureData);
                    if (k == 0)
                                        {
                        osg::notify(osg::FATAL) << "osgDB SPEED DREAMS reader: error reading child object" << std::endl;
                        return group.release();
                    }
                    else
                                        {
                        osg::LightSource *ls = dynamic_cast<osg::LightSource*>(k);
                        if (ls)
                                                {
                            osg::StateSet* lightStateSet = group->getOrCreateStateSet();
                            group->setStateSet(lightStateSet);
                            group->setCullingActive(false);
                            ls->setStateSetModes(*lightStateSet, osg::StateAttribute::ON);
                        }

						if(!m_bCar)
							group->addChild(k);
		    }
		}
		//bool bFinished = true;
	    }
	    else if (objectType == ac3d::ObjectTypeLight)
			{ // add a light source to the scene 1 Nov 2003
		osg::Light* ac3dLight = fileData.getNextLight();
		osg::Matrix tt = transform*parentTransform;
		ac3dLight->setPosition(osg::Vec4(tt(3, 0), tt(3, 1), tt(3, 2), 1));
		ac3dLight->setDirection(osg::Matrix::transform3x3(osg::Vec3(0.0f, 0.0f, -1.0f), tt));
		ac3dLight->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
		ac3dLight->setDiffuse(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
		ac3dLight->setSpecular(osg::Vec4(1.0f,1.0f,0.5f,1.0f));

                osg::LightSource* ac3dLightSource = new osg::LightSource;
                ac3dLightSource->setDataVariance(osg::Object::STATIC);
                ac3dLightSource->setLight(ac3dLight);
                ac3dLightSource->setLocalStateSetModes(osg::StateAttribute::ON);

                // for some mad reason, you need to set this so that the light works.  WHY?
                return ac3dLightSource;
            }
            return group.release();
        }
                else
                {
                        std::string strUnknown = token;
            osg::notify(osg::WARN) << "osgDB SPEED DREAMS reader: unknown token refs line while reading object \"6"
                <<" token:"<< strUnknown << "\"" << std::endl;
                }

		if (!stream.good())
		{
	    //bool bBad = stream.bad();
	    //bool bFail = stream.fail();
	    //bool bEof = stream.eof();
        osg::notify(osg::WARN) << "osgDB SPEED DREAMS reader: token read error \"7"
		<<" token:"<< token << "\"" << std::endl;
		}
    }

	if (!stream.good())
	{
	//bool bBad = stream.bad();
	//bool bFail = stream.fail();
	//bool bEof = stream.eof();
	}

    return group.release();
}

osgDB::ReaderWriter::ReadResult ReaderWriterACC::readNode(const std::string& file,const Options* options)
{
    std::string ext = osgDB::getFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    // GWM added Dec 2003 - get full path name (change in osgDB handling of files).
    std::string fileName = osgDB::findDataFile( file, options );
    osg::notify(osg::INFO) << "osgDB SPEED DREAMS reader: starting reading \"" << fileName << "\"" << std::endl;

    // Anders Backmann - correct return if path not found
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // allocate per file data and start reading
    osgDB::ifstream fin;
    fin.open(fileName.c_str(), std::ios::in);
    if (!fin.is_open()) return ReadResult::FILE_NOT_FOUND;

    // code for setting up the database path so that internally referenced file are
    // searched for on relative paths.
    osg::ref_ptr<Options> local_opt;
    if (options)
        local_opt = static_cast<Options*>(options->clone(osg::CopyOp::DEEP_COPY_ALL));
    else
        local_opt = new Options;
    local_opt->getDatabasePathList().push_back(osgDB::getFilePath(fileName));

    ReadResult result = readNode(fin, local_opt.get());
    if (result.validNode())
        result.getNode()->setName(fileName);
    return result;
}

osgDB::ReaderWriter::ReadResult ReaderWriterACC::readNode(std::istream& fin, const Options* options)
{
    std::string header;
    fin >> header;
    if (header.substr(0, 4) != "AC3D")
        return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    return readFile(fin, options);
}

osgDB::ReaderWriter::WriteResult ReaderWriterACC::writeNode(const osg::Node& node,const std::string& fileName, const Options* /*options*/)
{
    std::string ext = osgDB::getFileExtension(fileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;
    geodeVisitor vs; // this collects geodes.
    std::vector<unsigned int>iNumMaterials;
    const_cast<osg::Node&>(node).accept(vs); // this parses the tree to streamd Geodes
    std::vector<const osg::Geode *> glist=vs.getGeodes();
    osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
    // Write out the file header
    std::vector<const osg::Geode *>::iterator itr;
    fout << "AC3Db" << std::endl;
    // output the Materials
    int iNumGeodesWithGeometry = 0;
    for (itr=glist.begin();itr!= glist.end();itr++)
    {
        iNumMaterials.push_back(const_cast<ac3d::Geode*>(static_cast<const ac3d::Geode*>(*itr))->ProcessMaterial(fout,itr-glist.begin()));
        unsigned int iNumDrawables = (*itr)->getNumDrawables();
        int iNumGeometries = 0;
        for (unsigned int i = 0; i < iNumDrawables; i++)
        {
            const osg::Drawable* pDrawable = (*itr)->getDrawable(i);
            if (NULL != pDrawable)
            {
                const osg::Geometry *pGeometry = pDrawable->asGeometry();
                if (NULL != pGeometry)
                    iNumGeometries++;
            }
        }
        if (iNumGeometries > 0)
            iNumGeodesWithGeometry++;
    }
    // output the Geometry
    unsigned int nfirstmat=0;
    fout << "OBJECT world" << std::endl;

    fout << "kids " << iNumGeodesWithGeometry << std::endl;
    for (itr=glist.begin();itr!= glist.end();itr++) {
        const_cast<ac3d::Geode*>(static_cast<const ac3d::Geode*>(*itr))->ProcessGeometry(fout,nfirstmat);
        nfirstmat+=iNumMaterials[itr-glist.begin()];
    }
    fout.close();
    return WriteResult::FILE_SAVED;
}

osgDB::ReaderWriter::WriteResult ReaderWriterACC::writeNode(const osg::Node& node,std::ostream& fout, const Options* opts)
{
    try
    {
        // write ac file.
        if(dynamic_cast<const osg::Group*>(&node))
                {
            const osg::Group *gp=dynamic_cast<const osg::Group*>(&node);
            const unsigned int nch=gp->getNumChildren();
            for (unsigned int i=0; i<nch; i++) {
                writeNode(*(gp->getChild(i)), fout, opts);
            }
        }
        else
            osg::notify(osg::WARN)<<"File must start with a geode "<<std::endl;
        fout.flush();
        return WriteResult::FILE_SAVED;
    }
    catch(ac3d::Exception e)
    {
        osg::notify(osg::WARN)<<"Error parsing OSG tree: "<< e.getError() << std::endl;
    }
    return WriteResult::FILE_NOT_HANDLED;

}

/// Returns a possibly quoted string given in the end of the current line in the stream
std::string readString(std::istream& stream)
{
    std::string s;
    stream >> std::ws;

    if (stream.peek() != '\"')
    {
        // Not quoted, just read the string
        stream >> s;
    }
    else
    {
        // look for quoted strings

        // throw away the quote
        stream.get();

        // extract characters until either an error happens or a quote is found
        while (stream.good())
        {
            std::istream::char_type c;
            stream.get(c);
            if (c == '\"')
                break;
            s += c;
        }
    }

    return s;
}

void setAlphaClamp(osg::StateSet* stateSet,float clamp)
{
	osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
	alphaFunc->setFunction(osg::AlphaFunc::GREATER,clamp);
	stateSet->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
}

void setTranslucent(osg::StateSet* stateSet)
{
    osg::BlendFunc* blendFunc = new osg::BlendFunc;
    blendFunc->setDataVariance(osg::Object::STATIC);
    blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);
    blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    stateSet->setAttribute(blendFunc);
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

unsigned int GetUVCount(unsigned flags)
{
	switch(flags)
	{
	case 0x14:
		return 1;
	case 0x24:
		return 2;
	case 0x34:
		return 2;
	case 0x44:
		return 2;
	}

	return 0;
}
