#include <iostream>
#include <utility>
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

    bool operator<(const Node& other) const {
        return (x*x + y*y) < (other.x*other.x + other.y*other.y);
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

const float offsetDistance = 20; // offset of rover to camera 20cm

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
// std::vector<std::vector<int>> boundsGrid;

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
std::map<Node, std::stack<float>> nodePathStack; // [ [Node,[Angles]] ]

// std::stack<std::pair<Node,std::stack<float>>> nodeStackPathStack; // less storage but more complex

std::map<Node, float> nodeBackPath;

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
    /*
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
    */
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
    if (outsideBounds(grid[4], leftPathBounds) &&
        outsideBounds(grid[5], leftPathBounds)) {
        std::cout << "LEFT JUNCTION"<<std::endl;
        leftTurn = true;
    } else {
        leftTurn = false;
    }
    // Right Turn
    if (outsideBounds(grid[7], rightPathBounds) &&
        outsideBounds(grid[6], rightPathBounds)) {
        std::cout << "RIGHT JUNCTION"<<std::endl;
        rightTurn = true;
    } else {
        rightTurn = false;
    }

    // Path or End
    if (outsideBounds(grid[5],  middlePathBounds) &&
        outsideBounds(grid[6],  middlePathBounds) &&
        outsideBounds(grid[9],  middlePathBounds) &&
        outsideBounds(grid[10], middlePathBounds) ){
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
    if (isWall(grid[8], grid[4], leftWallBounds) ||
        isWall(grid[9], grid[5], leftWallBounds)) {
        std::cout << "LEFT WALL"<<std::endl;
        leftWall = true;
    } else {
        leftWall = false;
    }

    // Right Wall
    // Check if within x axis later
    if (isWall(grid[11], grid[7], rightWallBounds) ||
        isWall(grid[10], grid[6], rightWallBounds)) {
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

void blockAngle(float angle) {
    int min = ((int)angle - 39) % 360;
    int max = ((int)angle + 39) % 360;
    Range block = { min, max };
    blockedRanges.push_back(block);
}

// endAngle = startAngle + 405
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
    //blockedRanges.push_back({((int)currAngle+135)%360, ((int)currAngle+205)%360});
    // ^
    //blockAngle(startAngle+180);
    std::cout<<"\n";
    for (float currAngle = startAngle; currAngle <= (endAngle - cycle); currAngle+=0.4f) {
        // modulo
        if (currAngle >= 360) { currAngle -= 360; }
        // expected test inputs
        if ((currAngle >= 0) && (currAngle <= 31)) {
            isPath  = true;
            isClear = true;
        } else if (currAngle <= 69) {
            isRed   = true;
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
        bool scanningBeacon = scanningRed || scanningBlue || scanningYellow; // check for modulo/overflow
        if (currAngle < prevAngle) { cycle += 360; }
        // check for full rotation
        if ( (prevAngle < startAngle) && (currAngle >= startAngle) && cycle ) {
            rotation += 1;
            pathStart   = scanningPath   ? pathStart   : -1;
            beaconStart = scanningBeacon ? beaconStart : -1;
        }
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

                pathStart = -1;
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

                beaconStart = -1;
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

                beaconStart = -1;
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

                beaconStart = -1;
            }
            scanningYellow = isYellow;
        }

        prevAngle = (int)currAngle;
    }
}

Node nodeCoords() {
    // use beaconAngles
    // use dead reckoning

    return Node(0,0);
}

void backTrack() {

}

void nodeResponse() {
    // Store startAngle
    float startAngle = currAngle;
    // Set blocked paths
    if (isEnd)     { blockAngle(startAngle); }
    if (rightWall) { blockAngle(startAngle + 90); }
    if (leftWall)  { blockAngle(startAngle + 270); }
    // Move forward by offset ~ 20cm
    targetDistance = offsetDistance;
    // Wait until moved distance
    //
    // Run nodeScanner
    nodeScanner(startAngle,startAngle + 405);
    // Calc coordinates
    Node currNode = nodeCoords();
    // Send Coords to Server
    //
    // Send Edge with startAngle and previous node in stack
    //
    // Backtrack if no paths available
    if (pathAngles.size() <= 1) { backTrack(); }
    // Backtrack + add Edge if revisiting Node
    // if coords + pathAngles similar enough { backTrack }
    //
    // Add to Node-Path maps
    // std::pair< Node,std::vector<float>> pair = std::make_pair(currNode, pathAngles);
    nodePathAngles[currNode] = pathAngles;
    // testmap.insert(pair);

    //nodePathAngles.insert(pair);
    // Disregard (startAngle+180)+-39 for nodePathStack
    /*
    blockAngle(startAngle + 180);
    for (auto& angle : pathAngles) {
        if (!isBlocked(angle)) { pathStack.push(angle); }
        else {
            std::cout<<"PATH BACK: "<<angle<<std::endl;
            nodeBackPath[currNode] = angle;
        }
    }
    nodePathStack[currNode] = pathStack;
    while (!pathStack.empty()) {
        pathStack.pop();
    }
    blockedRanges = {};
    // get top of stack, pop it off, face it and move
    */ 
}

void setupDfs() {
    // RUN CODE TWICE
    targetAngle = currAngle + 405;
    // Might need classify inside nodeScanner not parallel
    nodeScanner(currAngle, targetAngle);
    Node currNode = nodeCoords(/*beaconAngles*/);
    // TAKE AVERAGE TWO RUNS IF DISCREPENCY RUN AGAIN

    // Send currNode to server API e.g. 1,1;

    // Add paths to node map for debugging
    // nodePathAngles[currNode] = pathAngles;

    // Pick angle
    if (pathAngles.size() != 0) {
        targetAngle = pathAngles[0];
    } else {
        std::cout<<"No Paths from Node: "<<currNode.x<<","<<currNode.y<<std::endl;
        // SEND KILL SIGNAL IDK RN
    }
    // Add ALL other paths to path stack -- figure out how thats gunna work with backtracking

    // Classify
    // Depends on classify either start or nodeResponse
}

void dfs() {
    // START IN CORNER
    setupDfs();

    // Store init angle
    // classifyMazeElement();
    //
    // Set initial blocked angles e.g. 45+6...180-6
}

void run() {
    setupDfs();

    dfs();
}

// int main() {
//     // [1] is a dummy for now
//     boundsGrid = {
//     {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
//     {0,140,130,240} , {240,120,320,160}, {320,140,410,240}, {500,120,640,200},
//     {0,260,60,360}  , {0,0,0,0}        , {430,240,460,360}, {0,0,0,0},
//     {0,0,0,0}       , {0,0,0,0}        , {450,360,480,480}, {480,420,530,480},
//     };
// //    printGrid(boundsGrid);
//     drawGrid(boundsGrid);

//     classifyMazeElement(boundsGrid);

//     isClear = true;
//     isRed = true;
//     currAngle = 280;
//     nodeScanner(280,280+1205);

//     printDebug();
// //    printOutputs();

// }


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
