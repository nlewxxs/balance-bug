#include <vector>
#include "CLASSIFY_IMAGE.h"

const int leftWallBounds[4]  = {0,240,320,480};
const int rightWallBounds[4] = {320,240,640,480};

const int leftPathBounds[4]   = {0,280,320,320};
const int middlePathBounds[4] = {240,240,400,480};
const int rightPathBounds[4]  = {320,280,640,320};

// all below should be false
bool isEnd;
bool isNode;
bool isPath;

bool isClear;
bool leftWall;
bool rightWall;

bool leftTurn;
bool rightTurn;


bool image::outsideBounds(const int (&box)[4], const int (&bound)[4]){
    bool outX = (box[2] < bound[0]) || (box[0] > bound[2]);
    bool outY = (box[3] < bound[1]) || (box[1] > bound[3]);
    return outX || outY;
}

bool image::hasWall(const int (&lower)[4], const int (&upper)[4], const int (&bound)[4]) {
    // {xMin, yMin, xMax, yMax}
    bool lowerWall  = ( (lower[3] >= bound[3]) && (lower[1] == 360) );
    bool upperWall  = ( (upper[1] <= bound[1]) && (upper[3] == 360) );
    bool lowerCross = ( (lower[0] <= bound[2]) && (lower[2] >= bound[0]));
    bool upperCross = ( (upper[0] <= bound[2]) && (upper[2] >= bound[0]));
    return (lowerWall && upperWall && lowerCross && upperCross);
}
