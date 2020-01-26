#ifndef _PATHOFFSETS_H_
#define _PATHOFFSETS_H_

#include <string>
#include <vector>

#include "Vec2d.h"
#include "MyTrack.h"

class PathOffsets
{
private:
    std::string         _baseFilename;
    std::vector<double> _offsets;
    std::vector<double>	_times;
    int			        _lastSeg;
    Vec2d		        _lastPt;
    double		        _lastTime;

public:
    PathOffsets();
    ~PathOffsets();

    void    setBaseFilename( const char* pBaseFilename );
    void    update( const MyTrack& track, const tCarElt* pCar );
    void    save_springs( const MyTrack& track, int lap );
};

#endif // _PATHOFFSETS.H_
