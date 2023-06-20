#include "Classify.h"

const int leftWallBounds[4]  = {0,240,320,480};
const int rightWallBounds[4] = {320,240,640,480};

const int leftPathBounds[4]   = {0,280,320,320};
const int middlePathBounds[4] = {240,240,400,480};
const int rightPathBounds[4]  = {320,280,640,320};

// all below should be false

bool Image::outsideBounds(const int box[4], const int bound[4]){
    bool outX = (box[2] < bound[0]) || (box[0] > bound[2]);
    bool outY = (box[3] < bound[1]) || (box[1] > bound[3]);
    return outX || outY;
}

bool Image::hasWall(const int lowerL[4], const int lowerR[4], const int upperL[4], const int upperR[4],const int xLR, const int bound[4]) {
    // {xMin, yMin, xMax, yMax}
    bool lowerWallL   = ( (lowerL[3] >= bound[3]) && (lowerL[1] <= 361) ) || // wall in range
                        ( (lowerR[0] <= xLR+1) && (lowerL[2] >= xLR-1) &&    // overlap on x axis
                          (lowerL[3] >= lowerR[1]) &&                        // overlap on y axis
                          (lowerR[3] >= bound[3]) && (lowerL[1] <= 361) );   // wall in range

    bool lowerWallR   = ( (lowerR[3] >= bound[3]) && (lowerR[1] <= 361) ) || // wall in range
                        ( (lowerR[0] <= xLR+1) && (lowerL[2] >= xLR-1) &&    // overlap on x axis
                          (lowerR[3] >= lowerL[1]) &&                        // overlap on y axis
                          (lowerL[3] >= bound[3]) && (lowerR[1] <= 361) );   // wall in range

    bool upperWallL   = ( (upperL[1] <= bound[1]) && (upperL[3] >= 359) ) || // wall in range
                        ( (upperR[0] <= xLR+1) && (upperL[2] >= xLR-1) &&    // overlap in x axis
                          (upperL[3] >= upperR[1]) &&                        // overlap in y axis
                          (upperR[1] <= bound[1]) && (upperL[3] >= 359) );   // wall in range

    bool upperWallR   = ( (upperR[1] <= bound[1]) && (upperR[3] >= 359) ) || // wall in range
                        ( (upperR[0] <= xLR+1) && (upperL[2] >= xLR-1) &&    // overlap in x axis
                          (upperR[3] >= upperL[1]) &&                        // overlap in y axis
                          (upperL[1] <= bound[1]) && (upperR[3] >= 359) );   // wall in range

    bool lowerAcross = ( (lowerL[0] <= bound[2]) || lowerR[0] <= bound[2] ) && ( (lowerR[2] >= bound[0]) || (lowerL[2] >= bound[0]) );
    bool upperAcross = ( (upperL[0] <= bound[2]) || upperR[0] <= bound[2] ) && ( (upperR[2] >= bound[0]) || (upperL[2] >= bound[0]) );

    bool connL = upperWallL && lowerWallL && (lowerL[0] <= upperL[2]) && (lowerL[2] >= upperL[0]);
    bool connR = upperWallR && lowerWallR && (lowerR[0] <= upperR[2]) && (lowerR[2] >= upperR[0]);
    
    return ( (connL || connR) && lowerAcross && upperAcross );
}

void Image::debugInfo() {    
    char tmp[64];
    // if ( info.isNode && info.isPath ) { std::cout << "BIG ERROR, DETECTING BOTH PATH & NODE" };
    sprintf(tmp, "isNode:     %d isEnd:       %d isPath:      %d",info.isNode,info.isEnd,info.isPath);
    Serial.println(tmp);
    sprintf(tmp, "isClear:    %d leftTurn:    %d rightTurn:   %d",info.isClear,info.leftTurn,info.rightTurn);
    Serial.println(tmp);
    sprintf(tmp, "leftWall:   %d rightWall:   %d",info.leftWall,info.rightWall);
    Serial.println(tmp);
}

// FUNCTION DOESNT NEED IF ELSE -- later fix
classifyElement Image::classify(int grid[12][4]) {
    // Left Turn
    if (outsideBounds(grid[0], leftPathBounds) &&
        outsideBounds(grid[1], leftPathBounds)) {
        // std::cout << "LEFT JUNCTION"
        info.leftTurn = true;
    } else {
        info.leftTurn = false;
    }
    // Right Turn
    if (outsideBounds(grid[3], rightPathBounds) &&
        outsideBounds(grid[2], rightPathBounds)) {
        // std::cout << "RIGHT JUNCTION"
        info.rightTurn = true;
    } else {
        info.rightTurn = false;
    }

    // Path or End
    if (outsideBounds(grid[1],  middlePathBounds) &&
        outsideBounds(grid[2],  middlePathBounds) &&
        outsideBounds(grid[5],  middlePathBounds) &&
        outsideBounds(grid[6], middlePathBounds) ){
        // std::cout << "PATH AHEAD"
        info.isClear = true;
        info.isEnd  = false;
    } else {
        // std::cout << "DEAD END/ PATH BLOCKED"
        info.isClear = false;
        info.isEnd  = true;
    }

    // Left Wall
    if (hasWall(grid[4], grid[5], grid[0], grid[1], 160, leftWallBounds)) {
        // std::cout << "LEFT WALL"
        info.leftWall = true;
    } else {
        info.leftWall = false;
    }

    // Right Wall
    if (hasWall(grid[6], grid[7], grid[2], grid[3], 480, rightWallBounds)) {
        // std::cout << "RIGHT WALL"
        info.rightWall = true;
    } else {
        info.rightWall = false;
    }

    info.isNode = info.isEnd   || info.leftTurn || info.rightTurn;
    info.isPath = info.isClear && info.leftWall && info.rightWall;
    
    return info;
}
