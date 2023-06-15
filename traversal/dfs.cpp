#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <map>
#include <stack>
#include <math.h>

// Consts
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

// 20cm distance ahead
const float offsetDistance = 20; 

const int leftWallBounds[4]  = {0,240,320,420};
const int rightWallBounds[4] = {320,240,640,420};

const int middlePathBounds[4] = {260,240,380,480};
const int leftPathBounds[4]   = {0,280,320,320};
const int rightPathBounds[4]  = {320,280,640,320};

// Input
bool isRed    = false;
bool isBlue   = false;
bool isYellow = false;

float currAngle = 0;
int totalDistance = 0;

// Output
bool  start = false;
float targetAngle = 0;
float targetDistance = 0;

// Internal
bool isEnd  = false;
bool isNode = false;
bool isPath = false;

bool isClear   = false;
bool leftWall  = false;
bool rightWall = false;

bool leftTurn  = false;
bool rightTurn = false;

double prevNodeDistance = 0;

// [Red, Blue, Yellow]
std::vector<float> beaconAngles = {-1, -1, -1};
std::vector<float> pathAngles;

std::vector<Range> blockedRanges;

std::map<Node, std::vector<float>> nodePathAngles;
std::map<Node, std::stack<float>> nodePathStack;
std::map<Node, float> nodeBackPath;

std::stack<Node> nodeStack;
std::stack<float> pathStack;

// Classify
bool outsideBounds(std::vector<int> box, const int bound[4]) {
    // {xMin, yMin, xMax, yMax}
    bool outX = (box[2] < bound[0]) || (box[0] > bound[2]);
    bool outY = (box[3] < bound[1]) || (box[1] > bound[3]);
    return outX || outY;
}
bool isWall(std::vector<int> lowerL, std::vector<int> lowerR, std::vector<int> upperL, std::vector<int> upperR, int xLR, const int bound[4]) {
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
    
    // std::cout<<"lowerWallL: "<<lowerWallL<<" lowerWallR: "<<lowerWallR<<" upperWallL: "<<upperWallL<<" upperWallR: "<<upperWallR<<" lowerAcross: "<<lowerAcross;
    // std::cout<<" upperAcross: "<<upperAcross<<" connL: "<<connL<<" connR: "<<connR<<std::endl;
    return ( (connL || connR) && lowerAcross && upperAcross );
}
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
    if (isWall(grid[8], grid[9], grid[4], grid[5], 160, leftWallBounds)) {
        std::cout << "LEFT WALL"<<std::endl;
        leftWall = true;
    } else {
        leftWall = false;
    }

    // Right Wall
    if (isWall(grid[10], grid[11], grid[6], grid[7], 480, rightWallBounds)) {
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

// Traversal
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
void nodeScanner(float startAngle, float endAngle) {
    float pathStart    = -1;
    float beaconStart  = -1;

    bool scanningPath   = isClear; //isPath
    bool scanningRed    = isRed;
    bool scanningBlue   = isBlue;
    bool scanningYellow = isYellow;

    int cycle = 0;
    int rotation = 0;
    int prevAngle = -1;

    // Testing
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
    // Testin
    //while (currAngle + cycle < endAngle) {
        bool scanningBeacon = scanningRed || scanningBlue || scanningYellow;

        // USE NIKS FUNCTION 
        // GET MAZE GRID
        // classifyMazeElement(grid);

        if (currAngle < prevAngle) { cycle += 360; }
        // check for full rotation
        if ( (currAngle + cycle) >= (startAngle + 360) ) {
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
            std::cout<<"EDGE RED"<<std::endl; 
            // Pos Edge
            if (isRed && !rotation) { beaconStart = currAngle; }
            // Neg Edge
            else if (beaconStart != -1) {
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
            std::cout<<"EDGE BLUE"<<std::endl; 
            // Pos Edge
            if (isBlue && !rotation) { beaconStart = currAngle; }
            // Neg Edge
            else if (beaconStart != -1) {
                float angle;
                if (currAngle < beaconStart) { angle = fmod((beaconStart + currAngle),360) / 2; }
                else { angle = float(beaconStart + currAngle) / 2;  }
                std::cout<<"BLUE:   "<<beaconStart<<" -> "<<currAngle<<" = "<<angle<<std::endl;
                beaconAngles[1] = angle;
                if (!scanningRed) { beaconStart = -1; }
            }
            scanningBlue = isBlue;
        }
        // Yellow
        if (scanningYellow != isYellow) {
            std::cout<<"EDGE YELLOW"<<std::endl;
            // Pos Edge
            if (isYellow) { beaconStart = currAngle; }
            // Neg Edge
            else if (beaconStart != -1) {
                float angle;
                if (currAngle < beaconStart) { angle = fmod((beaconStart + currAngle),360) / 2; }
                else { angle = float(beaconStart + currAngle) / 2;  }
                std::cout<<"YELLOW: "<<beaconStart<<" -> "<<currAngle<<" = "<<angle<<std::endl;
                beaconAngles[2] = angle;
                if (!scanningRed) { beaconStart = -1; }
            }
            scanningYellow = isYellow;
        }
        prevAngle = (int)currAngle;
    }
}
Node nodeCoords(std::vector<float>) {
    // triangulate(beaconAngles);
    // use dead reckoning - nodeStack.top(); + totalDistance - prevNodeDistance
    // weight both calculations
    return Node(0,0);
}
void backTrack() {

}
void nodeResponse() {
    // Stop Rover
    start = false;
    // Store startAngle
    float startAngle = currAngle;
    // Set blocked paths
    // !turn strict (miss paths but no crash) || wall loose (wont miss paths but may crash)
    // Error with isEnd blocking as it may occur in slight turns/when not straight
    if (isEnd && !leftWall && !rightWall) { blockAngle(startAngle); }
    if (!rightTurn) { blockAngle(startAngle + 90); }
    if (!leftTurn)  { blockAngle(startAngle + 270); }
    // Move forward by offset ~ 20cm
    targetDistance = offsetDistance;
    // Wait until moved distance
    // Delay(1000);
    // Run nodeScanner
    targetAngle = startAngle + 405;
    nodeScanner(startAngle,startAngle + 405);
    // Calc coordinates
    Node currNode = nodeCoords(beaconAngles);
    // Add currNode to stack
    nodeStack.push(currNode);
    // IMPORTANT
    // Check if node similar to previously stored node, if true backtrack and remove that angle from the node
    // Backtrack + add Edge if revisiting Node + skip sendning Coords
    // if coords + pathAngles similar enough { backTrack }
    for (auto& pair : nodePathAngles) { 
        if (currNode.same(pair.first) && (pathAngles.size() == pair.second.size())) {
            bool same = true;

            std::vector<float> sorted1 = pathAngles;
            std::vector<float> sorted2 = pair.second;

            std::sort(sorted1.begin(), sorted1.end());
            std::sort(sorted2.begin(), sorted2.end());

            for (std::size_t i = 0; i < sorted1.size(); ++i) {
                if (std::abs(sorted1[i] - sorted2[i]) > 39) {
                    same = false;
                    break;
                }
            }

            if (same) { 
                // SEND EDGE TO SERVER + REMOVE EDGE FROM STACK
                backTrack(); 
                return;
            }
        }
    }
    // 
    // calc edge distance (total - prev) weight with coord1 - coord2
    //
    // Send Coords to Server
    //
    // Send Edge with startAngle and previous node in stack
    // 
    // Set prevNodeDistance
    prevNodeDistance = totalDistance;
    //
    // Backtrack if no paths available
    if (pathAngles.size() <= 1) { backTrack(); return; }
    // Add to Node-Path maps
    nodePathAngles[currNode] = pathAngles;
    // Disregard (startAngle+180)+-39 for nodePathStack
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
}
void setupDfs() {
    float startAngle = currAngle;

    // RUN CODE TWICE RESET AT startAngle -- later 
    targetAngle = currAngle + 405;
    // Might need classify inside nodeScanner not parallel
    nodeScanner(currAngle, targetAngle);
    Node currNode = nodeCoords(beaconAngles);
    // TAKE AVERAGE TWO RUNS IF DISCREPENCY RUN AGAIN -- later

    // Add currNode to top of stack
    nodeStack.push(currNode);

    // Send currNode to server API e.g. 1,1;
    // Call HTTP GET function -- communication
    // String serverPath = serverName + "/Nodes/Add?SessionId=1&NodeId=2&XCoord="+std::to_string(currNode.x)+"&YCoord="+std::to_string(currNode.y);
    // http.begin(serverPath); 

    // Add paths to node map for debugging
    nodePathAngles[currNode] = pathAngles;

    // Add ALL paths to path stack -- figure out how thats gunna work with backtracking
    for (auto& angle : pathAngles) {
        pathStack.push(angle);
        // Don't explicitly need but maybe to check if completed backtracking
        nodeBackPath[currNode] = -1;
    }
    nodePathStack[currNode] = pathStack;
    while (!pathStack.empty()) { pathStack.pop(); }

    // Check if no available angles
    if (pathAngles.size() == 0) {
        std::cout<<"No Paths from Node: "<<currNode.x<<","<<currNode.y<<std::endl;
        // MAYBE MOVE TOWARDS START ANGLE IF SMART
        // SEND KILL SIGNAL IDK RN
        nodeStack.pop();
    }

    // Classify
    // Depends on classify either start or nodeResponse
}
void dfs() {
    Node currNode = nodeStack.top();
    std::stack<float> currPathStack = nodePathStack[currNode];

    if (currPathStack.empty()) { backTrack(); return; }
    else {
        // Face first path
        targetAngle = nodePathStack[currNode].top();
        nodePathStack[currNode].pop();
        // Wait till turned

        // USE NIKS FUNCTION
        std::vector<std::vector<int>> grid = {};
        classifyMazeElement(grid);

        if (isClear) { 
            while (!isNode) { 
                //GET GRID
                start = true;
                classifyMazeElement(grid);
            }
            nodeResponse();
        }
        else { start = false; /*END*/ }
        // else if (isPath) { start = true; }

        
    }
    // Store init angle
    // classifyMazeElement();
    //
    // Set initial blocked angles e.g. 45+6...180-6
}
void run() {
    setupDfs();

    // LOOP while nodeStack isnt empty
    while (!nodeStack.empty()) { 
        dfs();
    }
}

int main() { return 0; }
