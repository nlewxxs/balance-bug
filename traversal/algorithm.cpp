#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <math.h>

/*
INPUTS
    SPI
        [1...16][x,y][max,min]
        [red,yellow,blue][x,y]
    MPU
        [current_angle (deg)]
        [distance (total)]
OUTPUTS
    ESP
        [stop,start (move)]
        [target_angle (deg)]
    SERVER
        [x,y (Node)]
        [ax,ay,bx,by (Edge)][distance][angle]
*/

///////////////////////////////////////////
///////////     DEFINITIONS     ///////////
///////////////////////////////////////////

struct Node {
    int x;
    int y;

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

const float offsetDistance = 0.1;

//double leftWallMinX = 240;
//double leftWallMinY = 240;
//double leftWallMaxX = 400;
//double leftWallMaxY = 480;
const int leftWallBounds[4] = {0,240,320,480};

//double rightWallMinX = 480;
//double rightWallMinY = 240;
//double rightWallMaxX = 640;
//double rightWallMaxY = 480;
const int rightWallBounds[4] = {320,240,640,480};

//double middlePathMinX = 240;
//double middlePathMinY = 240;
//double middlePathMaxX = 400;
//double middlePathMaxY = 480;
const int middlePathBounds[4] = {240,240,400,480};

//double leftPathMinX = 0;
//double leftPathMinY = 280;
//double leftPathMaxX = 160;
//double leftPathMaxY = 320;
const int leftPathBounds[4] = {0,280,320,320};


//double rightPathMinX = 480;
//double rightPathMinY = 280;
//double rightPathMaxX = 640;
//double rightPathMaxY = 320;
const int rightPathBounds[4] = {320,280,640,320};


///////////////////////////////////////////
///////////       INPUTS        ///////////
///////////////////////////////////////////

bool isRed    = false;
bool isBlue   = false;
bool isYellow = false;

float currAngle = 0;
int totalDistance = 0;

// [ [xMin, yMin, xMax, yMax] ]
std::vector<std::vector<int>> boundsGrid;

///////////////////////////////////////////
///////////       OUTPUTS       ///////////
///////////////////////////////////////////

//bool stop  = true;
bool start = false;

float targetAngle = 0;
float targetDistance = 0;

// [x,y]
Node node;

// [currNode, prevNode, angle, length]
Edge edgeNodes;

///////////////////////////////////////////
///////////    DEBUG OUTPUTS    ///////////
///////////////////////////////////////////

bool isEnd  = false;
bool isNode = false;
bool isPath = false;

bool isClear   = false;
bool leftWall  = false;
bool rightWall = false;

bool leftTurn  = false;
bool rightTurn = false;

///////////////////////////////////////////
///////////      INTERNAL       ///////////
///////////////////////////////////////////

double prevNodeDistance = 0;

std::vector<float> pathAngles;
std::vector<float> beaconAngles = {-1, -1, -1}; // [Red, Blue, Yellow]

std::vector<Range> blockedRanges;

std::map<Node, std::vector<float>> nodePathAngles; // [ [Node,[Angles]] ]

std::stack<Node> nodeStack;
std::stack<float> pathStack;

///////////////////////////////////////////
///////////      PRINTING       ///////////
///////////////////////////////////////////

void printGrid(std::vector<std::vector<int>> grid) {
    std::cout<<"{ xMin, yMin, xMax, yMax }"<<std::endl;
    for (int i = 0; i < 16; i++) {
        int arrLength = 0;
        std::cout<<"{";
        for (int j = 0; j < 4; j++) {
            if ( j != 0 ) { std::cout<<" "; }
            std::cout<<grid[i][j];
            if ( j != 3 ) { std::cout<<","; }
            arrLength += std::to_string(grid[i][j]).length();
            //std::cout<<"i: "<<i<<"j: "<<j<<"value: "<<grid[i][j];
        }
        std::cout<<"}";
        if ( (i+1)%4 == 0 && i != 0) { std::cout<<std::endl; }
        else {
            std::cout<<",";
            for (int arrLength; arrLength < 12; arrLength++) {
                std::cout<<" ";
            }
        }
    }
}

void drawGrid(std::vector<std::vector<int>> grid){
    for (int i = 0; i <= 480; i+= 20) {
        for (int j = 0; j <= 640; j+= 10) {
            bool isCorner = false;
            bool isVertical = false;
            bool isHorizontal = false;
            for (const auto& coord : grid){
                if ((i == coord[1] || i == coord[3]) && (j == coord[0] || j == coord[2])) {
                    isCorner = true;
                    break;
                }
                if ((j == coord[0] || j == coord[2]) && i >= coord[1] && i <= coord[3]) {
                    isVertical = true;
                    break;
                }
                if ((i == coord[1] || i == coord[3]) && j >= coord[0] && j <= coord[2]) {
                    isHorizontal = true;
                    break;
                }
          }
            if (isCorner) { std::cout<<"+"; }
            else if (isVertical) { std::cout<<"¦"; }
            else if (isHorizontal) { std::cout<<"-"; }
            else { std::cout<< " "; }
        }
        std::cout << std::endl;
    }
}

void printDebug() {
    if ( isNode && isPath ) { std::cout << "BIG ERROR, DETECTING BOTH PATH & NODE"<<std::endl; }
    std::cout<<"\nisNode:   "<<isNode<<" isEnd:     "<<isEnd<<" isPath:    "<<isPath<<std::endl;
    std::cout<<"isClear:  "<<isClear<<" leftTurn:  "<<leftTurn<<" rightTurn: "<<rightTurn<<std::endl;
    std::cout<<"leftWall: "<<leftWall<<" rightWall: "<<rightWall<<std::endl;

    std::cout<<"\nPrevious Node Distance: "<<prevNodeDistance<<std::endl;

    std::cout<<"\nPath Angles:"<<std::endl;
    for (auto &angle : pathAngles) {
        std::cout<<" > "<<angle<<std::endl;
    }
    std::cout<<"\nBeacon Angles:"<<std::endl;
    std::cout<<" > Red:    "<<beaconAngles[0]<<std::endl;
    std::cout<<" > Blue:   "<<beaconAngles[1]<<std::endl;
    std::cout<<" > Yellow: "<<beaconAngles[2]<<std::endl;

    std::cout<<"\nBlocked Angles:"<<std::endl;
    for (auto &range : blockedRanges) {
        std::cout<<" > min: "<<range.min<<" max: "<<range.max<<std::endl;
    }

    std::cout<<"\nNode Path Map"<<std::endl;
    for (auto &pair : nodePathAngles) {
        std::cout<<" Node: " <<pair.first.x<<","<<pair.first.y<<std::endl;
        for (auto &angle : pair.second) {
            std::cout<<"  > "<<angle<<std::endl;
        }
    }

    std::cout<<"\nNode Stack:"<<std::endl;
    std::stack<Node> tempNodeStack = nodeStack;
    while (!tempNodeStack.empty()) {
        std::cout<< " > "<<tempNodeStack.top().x<<","<<tempNodeStack.top().y;
        tempNodeStack.pop();
    }

    std::cout<<"\nPath Stack:"<<std::endl;
    std::stack<float> tempPathStack = pathStack;
    while (!tempPathStack.empty()) {
        std::cout<< " > "<<tempPathStack.top();
        tempPathStack.pop();
    }
}

///////////////////////////////////////////
//////////   CLASSIFY ELEMENT   ///////////
///////////////////////////////////////////

bool outsideBounds(std::vector<int> box, const int bound[4]) {
    // {xMin, yMin, xMax, yMax}
    bool outX = (box[2] < bound[0]) || (box[0] > bound[2]);
    bool outY = (box[3] < bound[1]) || (box[1] > bound[3]);
    return outX || outY;
}

bool isWall(std::vector<int> lower, std::vector<int> upper, const int bound[4]) {
    // {xMin, yMin, xMax, yMax}
    bool lowerWall  = ( (lower[3] >= bound[3]) && (lower[1] == 360) );
    bool upperWall  = ( (upper[1] <= bound[1]) && (upper[3] == 360) );
    bool lowerCross = ( (lower[0] <= bound[2]) && (lower[2] >= bound[0]));
    bool upperCross = ( (upper[0] <= bound[2]) && (upper[2] >= bound[0]));
    return (lowerWall && upperWall && lowerCross && upperCross);
}

// FIGURE OUT LATER FOR LANES
bool encompassesBounds() {return false;}

void classifyMazeElement(std::vector<std::vector<int>> grid) {
    // Left Turn
    if (outsideBounds(grid[8], leftPathBounds) &&
        outsideBounds(grid[9], leftPathBounds)) {
        std::cout << "LEFT JUNCTION"<<std::endl;
        leftTurn = true;
    } else {
        leftTurn = false;
    }
    // Right Turn
    if (outsideBounds(grid[11], rightPathBounds) &&
        outsideBounds(grid[10], rightPathBounds)) {
        std::cout << "RIGHT JUNCTION"<<std::endl;
        rightTurn = true;
    } else {
        rightTurn = false;
    }

    // Path or End
    if (outsideBounds(grid[9],  middlePathBounds) &&
        outsideBounds(grid[10], middlePathBounds) &&
        outsideBounds(grid[13], middlePathBounds) &&
        outsideBounds(grid[14], middlePathBounds) ){
        std::cout << "PATH AHEAD"<<std::endl;
        isClear = true;
        isEnd  = false;
    } else {
        std::cout << "DEAD END/ PATH BLOCKED"<<std::endl;
        isClear = false;
        isEnd  = true;
    }

    // Left Wall
    // Check if within x axis later
    if (isWall(grid[12], grid[8], leftWallBounds) ||
        isWall(grid[13], grid[9], leftWallBounds)) {
        std::cout << "LEFT WALL"<<std::endl;
        leftWall = true;
    } else {
        leftWall = false;
    }

    // Right Wall
    // Check if within x axis later
    if (isWall(grid[15], grid[11], rightWallBounds) ||
        isWall(grid[14], grid[10], rightWallBounds)) {
        std::cout << "RIGHT WALL"<<std::endl;
        rightWall = true;
    } else {
        rightWall = false;
    }

    isNode = isEnd   || leftTurn || rightTurn;
    isPath = isClear && leftWall && rightWall;

    if (!isPath && !isNode) {
        std::cout<<"Currently at Node, looking for paths..."<<std::endl;
    }
}

///////////////////////////////////////////
///////////    MAZE TRAVERSAL   ///////////
///////////////////////////////////////////

bool isBlocked(float angle) {
    for (Range range : blockedRanges) {
        if ((angle >= range.min) && (angle <= range.max)) {
            return true;
        }
    }
    return false;
}

// endAngle = startAngle + 359
void nodeScanner(float startAngle, float endAngle) {
    float pathStart    = -1;
    float beaconStart  = -1;

    bool scanningPath   = isClear; //isPath
    bool scanningRed    = isRed;
    bool scanningBlue   = isBlue;
    bool scanningYellow = isYellow;

    int cycle = 0;
    int rotation = 0;
    // int for storage space
    int prevAngle = -1;

    // Testing
    //blockedRanges.push_back({0,20});
    //blockedRanges.push_back({129,231});
    blockedRanges.push_back({((int)currAngle+135)%360, ((int)currAngle+205)%360});
    std::cout<<"\n";
    for (float currAngle = startAngle; currAngle <= (endAngle - cycle); currAngle+=0.39) {
        // modulo
        if (currAngle >= 360) { currAngle -= 360; }
        // expected test inputs
        if ((currAngle >= 0) && (currAngle <= 31)) {
            isPath  = true;
            isClear = true;
        } else if (currAngle <= 69) {
            isRed = true;
            isPath  = false;
            isClear = false;
        } else if (currAngle <= 125) {
            isRed   = false;
        } else if (currAngle <= 205) {
            isPath  = true;
            isClear = true;
        } else if (currAngle <= 223) {
            isBlue = true;
        } else if (currAngle <= 268) {
            isPath  = false;
            isClear = false;
            isBlue  = false;
        } else if (currAngle <= 280) {
            isPath  = true;
            isClear = true;
        } else if (currAngle <= 342) {
            isPath  = false;
            isClear = false;
        } else if (currAngle <= 352) {
            isYellow = true;
        } else {
            isRed    = true;
            isPath   = true;
            isClear  = true;
            isYellow = false;
        }
    // Testing
    //while (currAngle < endAngle) {
        bool scanningBeacon = scanningRed || scanningBlue || scanningYellow;

        if (currAngle < prevAngle) { cycle += 360; }

        if ( (currAngle == startAngle) && cycle ) {
            rotation += 1;
            pathStart   = scanningPath   ? pathStart   : -1;
            beaconStart = scanningBeacon ? beaconStart : -1;
        }

        prevAngle = (int)currAngle;

        // Path - try isPath for more accuracy
        if (scanningPath != isClear) {
            // Pos Edge
            if (isClear && !rotation) { pathStart = currAngle; }
            // Neg Edge
            else if (pathStart != -1) {
                // pathEnd = currAngle;
                float angle;
                if (currAngle < pathStart) { angle = fmod((pathStart + currAngle),360) / 2; }
                else { angle = float(pathStart + currAngle) / 2;  }

                std::cout<<"PATH:   "<<pathStart<<" -> "<<currAngle<<" = "<<angle<<std::endl;

                if (!isBlocked(angle)) { pathAngles.push_back(angle); }
                else { std::cout<<"PATH BLOCKED: "<<angle<<std::endl; }
            }
            scanningPath = isClear;
        }
        // Red
        if (scanningRed != isRed) {
            // Pos Edge
            if (isRed && !rotation) { beaconStart = currAngle; }
            // Neg Edge
            else if (beaconStart != -1) {
                // beaconEnd = currAngle;
                float angle;
                if (currAngle < beaconStart) { angle = fmod((beaconStart + currAngle),360) / 2; }
                else { angle = float(beaconStart + currAngle) / 2;  }

                std::cout<<"RED:    "<<beaconStart<<" -> "<<currAngle<<" = "<<angle<<std::endl;

                beaconAngles[0] = angle;
            }
            scanningRed = isRed;
        }
        // Blue
        if (scanningBlue != isBlue) {
            // Pos Edge
            if (isBlue && !rotation) { beaconStart = currAngle; }
            // Neg Edge
            else if (beaconStart != -1) {
                float angle;
                if (currAngle < beaconStart) { angle = fmod((beaconStart + currAngle),360) / 2; }
                else { angle = float(beaconStart + currAngle) / 2;  }

                std::cout<<"BLUE:   "<<beaconStart<<" -> "<<currAngle<<" = "<<angle<<std::endl;

                beaconAngles[1] = angle;
            }
            scanningBlue = isBlue;
        }
        // Yellow
        if (scanningYellow != isYellow) {
            // Pos Edge
            if (isYellow) { beaconStart = currAngle; }
            // Neg Edge
            else if (beaconStart != -1) {
                float angle;
                if (currAngle < beaconStart) { angle = fmod((beaconStart + currAngle),360) / 2; }
                else { angle = float(beaconStart + currAngle) / 2;  }

                std::cout<<"YELLOW: "<<beaconStart<<" -> "<<currAngle<<" = "<<angle<<std::endl;
                beaconAngles[2] = angle;
            }
            scanningYellow = isYellow;
        }
    }
}

void nodeResponse() {

}

Node nodeCoords() {
    // use beaconAngles

    return {0,0};
}

void backTrack() {

}

void dfs() {
    // START IN CORNER
    // RUN CODE TWICE
    targetAngle = currAngle + 405;
    // Set initial blocked angles e.g. 45+6...180-6
    nodeScanner(currAngle, targetAngle);
    Node currNode = nodeCoords();

    // TAKE AVERAGE TWO RUNS IF DISCREPENCY RUN AGAIN
    // Send currNode to server API e.g. 1,1;

    if (pathAngles.size() != 0) {
        targetAngle = pathAngles[0];
    } else {
        std::cout<<"No Paths from Node: "<<currNode.x<<","<<currNode.y<<std::endl;
        // SEND KILL SIGNAL IDK RN
    }




}

int main() {
    // [1] is a dummy for now
    boundsGrid = {
    {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {0,140,130,240} , {240,120,320,160}, {320,140,410,240}, {500,120,640,200},
    {0,260,60,360}  , {0,0,0,0}        , {430,240,460,360}, {0,0,0,0},
    {0,0,0,0}       , {0,0,0,0}        , {450,360,480,480}, {480,420,530,480},
    };
//    printGrid(boundsGrid);
    drawGrid(boundsGrid);

    classifyMazeElement(boundsGrid);

    isClear = true;
    isRed = true;
    currAngle = 280;
    nodeScanner(280,280+405);

    printDebug();
//    printOutputs();

}


/* GRIDS

GRID TEST 1.

    {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {0,0,0,0}       , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {40,240,120,360}, {0,0,0,0}        , {0,0,0,0}        , {520,240,600,360},
    {40,360,120,480}, {0,0,0,0}        , {0,0,0,0}        , {520,360,600,480}

    PASS

GRID TEST 2.
    {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {0,0,0,0}       , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {40,240,120,360}, {300,240,320,360}, {0,0,0,0}        , {520,240,600,360},
    {40,360,120,480}, {0,0,0,0}        , {0,0,0,0}        , {520,360,600,480}

    PASS

GRID TEST 3

    {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {10,120,80,240} , {0,0,0,0}        , {0,0,0,0}        , {510,120,610,240},
    {90,240,160,360}, {0,0,0,0}        , {460,340,480,360}, {480,240,520,360},
    {0,0,0,0}       , {160,360,320,470}, {320,360,480,470}, {0,0,0,0},

    PASS

GRID TEST 4
    {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
    {0,140,130,240} , {240,120,320,160}, {320,140,410,240}, {500,120,640,200},
    {0,260,60,360}  , {0,0,0,0}        , {430,240,460,360}, {0,0,0,0},
    {0,0,0,0}       , {0,0,0,0}        , {450,360,480,440}, {480,420,530,480},

    UNSURE (Might need to seperate looking for path vs checking for node function)

GRID TEST 5

    {0,0,640,480}   , {0,0,0,0} , {0,0,0,0} , {0,0,0,0},
    {0,0,0,0}       , {0,0,0,0} , {0,0,0,0} , {0,0,0,0},
    {40,240,120,360}, {160,240,480,260} , {0,0,0,0} , {520,240,600,360},
    {40,360,120,480}, {0,0,0,0} , {0,0,0,0} ,{520,360,600,480}

    PASS
*/
