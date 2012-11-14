#ifndef _OSGCAR_H_
#define _OSGCAR_H_

#include <raceman.h>
#include <osg/Group>
#include <vector>

class SDCar
{
    private :
        osg::ref_ptr<osg::MatrixTransform> car_branch;
        tCarElt *car;
    public :
        osg::ref_ptr<osg::Node> loadCar(tCarElt *car);
        void updateCar();
};

class SDCars
{
    private :
        std::vector<SDCar *> the_cars;
        osg::ref_ptr<osg::Group> cars_branch;
        tSituation * situation;

        void addSDCar(SDCar * car);

    public :

	SDCars(void);
	~SDCars(void);

        osg::ref_ptr<osg::Node> loadCars(tSituation * pSituation);
        void updateCars();
};


#endif /* _OSGCAR_H_ */
