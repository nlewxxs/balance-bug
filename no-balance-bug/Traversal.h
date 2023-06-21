#ifndef Traversal_h
#define Traversal_h

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <stack>
#include <map>

#define XDIST 100
#define YDIST 100
#define ERRMARGIN 5
#define DEBUG true

#define OFFSET_DISTANCE 0.2

struct Node {
    int x;
    int y;

    bool operator<(const Node& other) const {
        return (x*x + y*y) < (other.x*other.x + other.y*other.y);
    }

    float distance(const Node& other) {
        return sqrt((x-other.x)*(x-other.x) + (y-other.y)*(y-other.y));
    }

    bool same(const Node& other) {
        bool xSame = (x >= (other.x - 0.1)) && (x <= (other.x + 0.1));
        bool ySame = (y >= (other.y - 0.1)) && (y <= (other.y + 0.1));
        return xSame && ySame;
    }

    Node(int _x = 0, int _y = 0)
        : x(_x), y(_y) {}
};
struct Edge {
    Node* curr;
    Node* prev;
    float angle;
    int   length;

    Edge(Node* _curr = NULL, Node* _prev = NULL, int _angle = 0, int _length = 0)
        : curr(_curr), prev(_prev), angle(_angle), length(_length) {}
};
struct Range {
    int min;
    int max;
};
struct ColourAngle {
    float red;
    float blue;
    float yellow;

    ColourAngle(float _red = 0, float _blue = 0, float _yellow = 0)
        : red(_red), blue(_blue), yellow(_yellow) {}
};
struct CoordAngle {
    float cBR;
    float cBY;
    float cRY;

    CoordAngle(float _cBR = 0, float _cBY = 0, float _cRY = 0)
        : cBR(_cBR), cBY(_cBY), cRY(_cRY) {}
};
struct AngleBool {
    bool aBR;
    bool aBY;
    bool aRY;

    AngleBool(bool _aBR = false, bool _aBY = false, bool _aRY = false)
        : aBR(_aBR), aBY(_aBY), aRY(_aRY) {}
};

// INPUTS 
// float currAngle
// double totalDistance


// Output
// bool  start = false;
// float targetAngle = 0;
// float targetDistance = 0;


// INSIDE
double prevNodeDistance = 0;


class MazeUtils {
    public:
        bool isBlocked(float angle);
        void blockAngle(float angle);
        AngleBool detectAnomalie (const CoordAngle &input);
        Node triangulate (const ColourAngle &input, const float &offset);
        Node nodeCoords(ColourAngle beaconAngles);
        void nodeScanner(float startAngle, float endAngle,bool& isRed,bool& isBlue,bool& isYellow, bool& currAngle);
    private:
        ColourAngle beaconAngles;
        std::vector<float> pathAngles;

        std::vector<Range> blockedRanges;

        std::map<Node, std::vector<float>> nodePathAngles;
        std::map<Node, std::stack<float>> nodePathStack;
        std::map<Node, float> nodeBackPath;

        std::stack<Node> nodeStack;
        std::stack<float> pathStack;
};

#endif
