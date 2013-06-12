class SDCarShader{
private :

    osg::ref_ptr<osg::Program> program ;

    osg::ref_ptr<osg::Node> pCar;
    osg::ref_ptr<osg::StateSet> stateset;
    osg::ref_ptr<osg::Uniform> diffuseMap;
    osg::ref_ptr<osg::Uniform> reflectionMap;
    osg::ref_ptr<osg::Uniform> specularColor;
    osg::ref_ptr<osg::Uniform> lightVector;
    osg::ref_ptr<osg::Uniform> lightPower;
    osg::ref_ptr<osg::Uniform> ambientColor;
    osg::ref_ptr<osg::Uniform> shininess;


public :
    SDCarShader(osg::Node *car);
    void update(osg::Matrixf view);
};
