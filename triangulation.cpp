#include <exception>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <cmath>

#define xDist 100
#define yDist 100

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
    modifiedAngle.yellow = (input.yellow - offset - 180) * (M_PI/180);

    //x-triangulation
    xBR = (yDist * tan(modifiedAngle.blue)) / (tan(modifiedAngle.blue) + tan(modifiedAngle.red));
    xBY = (yDist * tan(modifiedAngle.red)) / (tan(modifiedAngle.yellow) + tan(modifiedAngle.blue));
    xRY = xDist - ((yDist * tan(modifiedAngle.red)) / (tan(modifiedAngle.yellow) + tan(modifiedAngle.red)));

    //y-triangulation
    yBR = yDist - (yDist / (tan(modifiedAngle.blue) + tan(modifiedAngle.red)));
    yBY = yDist - (xDist / (tan(modifiedAngle.yellow) + tan(modifiedAngle.blue)));
    yRY = xDist / (tan(modifiedAngle.yellow) + tan(modifiedAngle.red));

    std::cout << xBR << ", " << xBY << ", " << xBR << std::endl;


    output.x = (xBR + xBY + xRY) / 3;
    output.y = (yBR + yBY + yRY) / 3;
    
    return output;
}





int main() {
    Angle testAngle = {
        225.0,          //yellow
        45.0,           //red
        315.0,          //blue
    };

    float testOffset = 0;

    Node testOutput;

    testOutput = triangulate(testAngle, testOffset);

    std::cout << testOutput.x << ", " << testOutput.y << std::endl;
}