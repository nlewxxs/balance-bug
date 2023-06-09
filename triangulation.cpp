#include <exception>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <cmath>

#define xDist 100
#define yDist 100

//TODO FIX X VALUE BASED OFF THE MATHS OF THE Y VALUE


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

    std::cout << modifiedAngle.blue * (180/M_PI) << ", " << modifiedAngle.red * (180/M_PI) << ", " << modifiedAngle.yellow * (180/M_PI) << ", " << (tan(modifiedAngle.blue)) << std::endl;

    //x-triangulation
    xBR = (yDist * tan(modifiedAngle.blue)) / (tan(modifiedAngle.blue) + tan(modifiedAngle.red));
    xBY = ((tan(modifiedAngle.blue) * (xDist / 2)) - (yDist * tan(modifiedAngle.blue) * tan(modifiedAngle.yellow))) / (tan(modifiedAngle.blue) - tan(modifiedAngle.yellow));
    xRY = xDist - ((yDist * tan(modifiedAngle.red)) / (tan(modifiedAngle.yellow) + tan(modifiedAngle.red)));

    //y-triangulation
    yBR = yDist - (yDist / (tan(modifiedAngle.blue) + tan(modifiedAngle.red)));
    // yBY = (((yDist * tan(modifiedAngle.blue)) - (xDist /2)) / (tan(modifiedAngle.yellow) + tan(modifiedAngle.blue)));
    yBY = ((yDist * tan(modifiedAngle.red)) - (xDist / 2)) / (tan(modifiedAngle.red) + tan(modifiedAngle.yellow));
    yRY = ((yDist * tan(modifiedAngle.red)) - (xDist / 2)) / (tan(modifiedAngle.red) + tan(modifiedAngle.yellow));

    // if  (yBY < 0) {
    //     yBY = 0- yBY;
    // }

    // if  (yBR < 0) {
    //     yBR = 0- yBR;
    // }

    // if  (yRY < 0) {
    //     yRY = 0- yRY;
    // }

    std::cout << xBR << ", " << xBY << ", " << xRY << std::endl;
    std::cout << yBR << ", " << yBY << ", " << yRY << std::endl;


    output.x = (xBR + xBY + xRY) / 3;
    output.y = (yBR + yBY + yRY) / 3;
    
    std::cout << output.x << ", " << output.y << std::endl;

    return output;
}





int main() {
    // Angle testAngle = {
    //     180,          //yellow
    //     45.0,           //red
    //     315.0,          //blue
    // };

    // Angle testAngle = {
    //     138.814,          //yellow
    //     54.782,           //red
    //     345.964,          //blue
    // };

    Angle testAngle = {
        221.186,          //yellow
        14.036,           //red
        305.218,          //blue
    };

    float testOffset = 0;

    Node testOutput;

    testOutput = triangulate(testAngle, testOffset);

    std::cout << testOutput.x << ", " << testOutput.y << std::endl;
}