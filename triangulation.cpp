#include <iostream>
#include <cmath>


///////////////////////////////////////////
///////////       MACROS        ///////////
///////////////////////////////////////////

#define XDIST 100
#define YDIST 100
#define DEBUG true

///////////////////////////////////////////
///////////     DEFINITIONS     ///////////
///////////////////////////////////////////

struct Node {
    int x;
    int y;

    Node(int _x = 0, int _y = 0)
        : x(_x), y(_y) {}
};

struct Angle {
    float yellow;
    float red;
    float blue;

    Angle(float _yellow = 0, float _red = 0, float _blue = 0)
        : yellow(_yellow), red(_red), blue(_blue) {}
};

///////////////////////////////////////////
///////////      Functions      ///////////
///////////////////////////////////////////

Node triangulate (const Angle &input, const float &offset) {
    //declarations
    Angle modifiedAngle;
    Node output;

    float xBR;
    float xBY;
    float xRY;

    float yBR;
    float yBY;
    float yRY;

    //modify the modifiedAngle
    modifiedAngle.blue = (360 - (input.blue - offset)) * (M_PI/180);
    modifiedAngle.red  = (input.red - offset) * (M_PI/180);
    modifiedAngle.yellow = (180 - input.yellow - offset) * (M_PI/180);


    //x-triangulation
    xBR = (YDIST * tan(modifiedAngle.blue)) / (tan(modifiedAngle.blue) + tan(modifiedAngle.red));
    xBY = ((tan(modifiedAngle.blue) * (XDIST / 2)) - (YDIST * tan(modifiedAngle.blue) * tan(modifiedAngle.yellow))) / (tan(modifiedAngle.blue) - tan(modifiedAngle.yellow));
    xRY = (((XDIST / 2) * tan(modifiedAngle.red)) + (XDIST * tan(modifiedAngle.yellow)) - (YDIST * tan(modifiedAngle.red) * tan(modifiedAngle.yellow))) / (tan(modifiedAngle.yellow) + tan(modifiedAngle.red));

    //y-triangulation
    yBR = YDIST - (YDIST / (tan(modifiedAngle.blue) + tan(modifiedAngle.red)));
    yBY = ((YDIST * tan(modifiedAngle.red)) - (XDIST / 2)) / (tan(modifiedAngle.red) + tan(modifiedAngle.yellow));
    yRY = ((YDIST * tan(modifiedAngle.red)) - (XDIST / 2)) / (tan(modifiedAngle.red) + tan(modifiedAngle.yellow));

    output.x = (xBR + xBY + xRY) / 3;
    output.y = (yBR + yBY + yRY) / 3;
    
    if (DEBUG) {
        std::cout << "XCoords: " << xBR << ", " << xBY << ", " << xRY << std::endl;
        std::cout << "YCoords: " << yBR << ", " << yBY << ", " << yRY << std::endl;
        std::cout << "Angles: " << modifiedAngle.blue * (180/M_PI) << ", " << modifiedAngle.red * (180/M_PI) << ", " << modifiedAngle.yellow * (180/M_PI) << ", " << (tan(modifiedAngle.blue)) << std::endl;
        std::cout << "Outputs: " << output.x << ", " << output.y << std::endl;
    }
    return output;
}





int main() {
    Angle testAngle = {
        180,          //yellow
        45.0,           //red
        315.0,          //blue
    };

    // Angle testAngle = {
    //     138.814,          //yellow
    //     54.782,           //red
    //     345.964,          //blue
    // };

    // Angle testAngle = {
    //     221.186,          //yellow
    //     14.036,           //red
    //     305.218,          //blue
    // };

    float testOffset = 0;

    Node testOutput;

    testOutput = triangulate(testAngle, testOffset);

    std::cout << testOutput.x << ", " << testOutput.y << std::endl;
}