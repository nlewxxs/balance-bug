#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <stack>
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

// INPUTS
bool isRed = false;
bool isBlue = false;
bool isYellow = false;

int currAngle = 0;
int totalDistance = 0;

std::vector<std::vector<int>> boundsGrid; // [ [#, xMin, yMin, xMax, yMax] ]

// OUTPUTS
bool stop = true;
bool start = false;
int targetDistance = 0;
int targetAngle = 0;

struct Node {
    int x;
    int y;

    Node(int _x = 0, int _y = 0)
        : x(_x), y(_y) {}
};

struct Edge {
    Node* curr;
    Node* prev;
    int angle;
    int length;
    
    Edge(Node* _curr = NULL, Node* _prev = NULL, int _angle = 0, int _length = 0)
        : curr(_curr), prev(_prev), angle(_angle), length(_length) {}
};

Node node; // [x,y]
Edge edgeNodes; // [currNode, prevNode]

// DEBUG OUTPUTS
bool isEnd = false;
bool isNode = false;
bool isPath = false;

bool leftWall = false;
bool rightWall = false;

bool leftTurn = false;
bool rightTurn = false;
// INTERNAL
std::stack<Node> nodeStack;
std::stack<Edge> edgeStack;

double prevNodeDistance = 0;
std::map<Node, std::vector<int>> nodePathAngles; // [ [Node,[Angles]] ]
// std::vector<std::pair<Node, std::vector<int>>> nodePathAngles; // [ [Node,[Angles]] ]


// CONSTANTS

//double leftWallMaxX = 400;
//double leftWallMinX = 240;
double leftWallMinY = 240;
double leftWallMaxY = 480;
//std::vector<int> leftWallBounds = {xmin,240,xmax,480}

//double rightWallMaxX = 640;
//double rightWallMinX = 480;
double rightWallMinY = 240;
double rightWallMaxY = 480;
//std::vector<int> rightWallBounds = {xmin,240,xmax,480}

//double middlePathMinX = 240;
//double middlePathMaxX = 400;
//double middlePathMinY = 240; 
//double middlePathMaxY = 480; 
std::vector<int> middlePathBounds = {240,240,400,480};

//double leftPathMinX = 0;
//double leftPathMaxX = 160;
//double leftPathMinY = 280;
//double leftPathMaxY = 320;
std::vector<int> leftPathBounds = {0,280,280,320};


//double rightPathMinX = 480;
//double rightPathMaxX = 640;
//double rightPathMinY = 280;
//double rightPathMaxY = 320;
std::vector<int> rightPathBounds = {480,280,640,320};


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
            for (arrLength; arrLength < 12; arrLength++) { 
                std::cout<<" ";
            }
        }
    }
}


void drawGrid(std::vector<std::vector<int>> grid){
    for (int i = 0; i <= 480; i+= 20) {
        for (int j = 0; j <= 640; j+= 10) {
            bool isHorizontal = false;
            bool isVertical = false;
            bool isCorner = false;
            for (const auto& coord : grid){
                if ((i == coord[1] || i == coord[3]) && (j == coord[0] || j == coord[2])) {
                    isCorner = true;
                    break;
                }
                if ((i == coord[1] || i == coord[3]) && j >= coord[0] && j<=coord[2]) {
                    isHorizontal = true;
                    break;
                }
                if ((j == coord[0] || j==coord[2]) && i>=coord[1] && i<=coord[3]) {
                    isVertical = true;
                    break;
                }
          }
            if (isCorner) { std::cout<<"+"; } 
            else if (isHorizontal) { std::cout<<"-"; }
            else if (isVertical) { std::cout<<"¦"; }
            else { std::cout<< " "; }
        } 
        std::cout << std::endl;
    }
}


void printDebug() {
    if ( isNode && isPath ) { std::cout << "BIG ERROR, DETECTING BOTH PATH & NODE"<<std::endl; } 
    std::cout<<"isNode: "<<isNode<<" isEnd: "<<isEnd<<" leftTurn: "<<leftTurn<<" rightTurn: "<<rightTurn<<" isPath: "<<isPath<<std::endl;
    std::cout<<"leftWall: "<<leftWall<<" rightWall: "<<rightWall<<std::endl;
}


bool outsideBounds(std::vector<int> box, std::vector<int> bound) {
    // {xMin, yMin, xMax, yMax}
    bool outX = (box[2] < bound[0]) || (box[0] > bound[2]);
    bool outY = (box[3] < bound[1]) || (box[1] > bound[3]);
    return outX || outY;
}

bool isWall(std::vector<int> lower, std::vector<int> upper, int min, int max) {
    // {xMin, yMin, xMax, yMax}
    bool lowerWall = ( (lower[1] == 360) && (lower[3] >= max) );
    bool upperWall = ( (upper[1] <= min) && (upper[3] == 360) );
    return (lowerWall && upperWall);
}

// FIGURE OUT LATER FOR LANES
bool encompassesBounds() {return false;}

void classifyMazeElement(std::vector<std::vector<int>> grid) {

    // Left Turn
    if (outsideBounds(grid[8], leftPathBounds)) {
        std::cout << "LEFT JUNCTION"<<std::endl; 
        leftTurn = true;
    } else {
        leftTurn = false;
    }
    // Right Turn
    if (outsideBounds(grid[8], leftPathBounds)) {
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
        isPath = true;
        isEnd = false;
    }  else {
        std::cout << "DEAD END/ PATH BLOCKED"<<std::endl;
        isPath = false;
        isEnd = true;
    }

    // Wall on Left
    // leftWallMinY => leftWallBounds[1]
    // leftWallMaxY => leftWallBounds[3]
    // Then check if within x axis
    if (isWall(grid[12], grid[8], leftWallMinY, leftWallMaxY)) { 
        std::cout << "LEFT WALL"<<std::endl;
        leftWall = true;
    } else {
        leftWall = false;
    }

    // Wall on Right
    // rightWallMinY => rightWallBounds[1]
    // rightWallMaxY => rightWallBounds[3]
    // Then check if within x axis
    if (isWall(grid[15], grid[11], rightWallMinY, rightWallMaxY)) { 
        std::cout << "RIGHT WALL"<<std::endl;
        rightWall = true;
    } else {
        rightWall = false;
    }

    isPath = isPath && leftWall && rightWall; 
    isNode = isEnd  || leftTurn || rightTurn;

    if (!isPath && !isNode) {
        std::cout<<"Currently at Node, looking for paths..."<<std::endl;
    }
//    std::cout<<leftLaneLength<<leftWallRangeY<<leftLane<<std::endl;
//    std::cout<<rightLaneLength<<rightWallRangeY<<rightLane<<std::endl;

} 

int main() {
    // [1] is a dummy for now
    boundsGrid = { 
        {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
        {0,140,130,240} , {240,120,320,160}, {320,140,410,240}, {500,120,640,200},
        {0,260,60,360}  , {0,0,0,0}        , {430,240,460,360}, {0,0,0,0},
        {0,0,0,0}       , {0,0,0,0}        , {450,360,480,440}, {480,420,530,480},
    };

//    printGrid(boundsGrid);
    drawGrid(boundsGrid);

    classifyMazeElement(boundsGrid);

    printDebug();
//    printOutputs();
//    if ( boundsGrid[][]

}


// GRIDS
//
// GRID TEST 3
// {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
// {10,120,80,240} , {0,0,0,0}        , {0,0,0,0}        , {510,240,610,120},
// {90,240,160,360}, {0,0,0,0}        , {460,340,480,360}, {480,240,520,360},
// {0,0,0,0}       , {160,360,320,470}, {320,360,480,470}, {0,0,0,0},
//
// PASS 
//
// GRID TEST 4
// {0,0,640,480}   , {0,0,0,0}        , {0,0,0,0}        , {0,0,0,0},
// {0,140,130,240} , {240,120,320,160}, {320,140,410,240}, {500,120,640,200},
// {0,260,60,360}  , {0,0,0,0}        , {430,240,460,360}, {0,0,0,0},
// {0,0,0,0}       , {0,0,0,0}        , {450,360,480,440}, {480,420,530,480},
//
// UNSURE (Might need to seperate looking for path vs checking for node function)
//
//
