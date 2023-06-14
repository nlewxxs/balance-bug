#include "algorithm.cpp"
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>  
#include<algorithm>

#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
// #else
// #   define ASSERT(condition, message) do { } while (false)

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
using filepath = std::filesystem::path;

#define TESTPATHS "./samples/"
#define DEBUGOUTPUTS true

struct classifyElement{
    bool isEnd;
    bool isNode;
    bool isPath;

    bool isClear;
    bool leftWall;
    bool rightWall;

    bool leftTurn;
    bool rightTurn;

    // classifyElement(bool _isEnd = false, bool _isNode = false, bool _isPath = false, bool _isClear = false, bool _leftWall = false, bool _rightWall = false, bool _leftTurn = false, bool _rightTurn = false)
    //     : isEnd(_isEnd), isNode(_isNode), isPath(_isPath), isClear(_isClear), leftWall(_leftWall), rightWall(_rightWall), leftTurn(_leftTurn), rightTurn(_rightTurn) {}
};

struct imageInfo{
    std::vector<std::vector<int>> boundingBoxes;
    float currAngle;

    void printGrid() {
        std::cout<<"\nCURRENT GRID BOX"<<std::endl;
        std::cout<<"{ xMin, yMin, xMax, yMax }"<<std::endl;
        for (int i = 0; i < 12; i++) {
            int arrLength = 0;
            std::cout<<"{";
            for (int j = 0; j < 4; j++) {
                if ( j != 0 ) { std::cout<<" "; }
                std::cout<<boundingBoxes[i][j];
                if ( j != 3 ) { std::cout<<","; }
                arrLength += std::to_string(boundingBoxes[i][j]).length();
                //std::cout<<"i: "<<i<<"j: "<<j<<"value: "<<grid[i][j];
            }
            std::cout<<"}";
            if ( (i+1)%4 == 0 && i != 0) { std::cout<<std::endl; }
            else {
                std::cout<<",";
                for (int i = arrLength; i < 13; i++) {
                    std::cout<<" ";
                }
            }
        }
    }
};

struct testOutput{
    imageInfo coords;
    classifyElement classifiedOut;
};

std::vector<std::string> getFolders(const std::string path) {
    filepath myPath(TESTPATHS);
    std::vector<std::string> tstFolders;
    for (const filepath& dirEntry : recursive_directory_iterator(myPath)){
        if(dirEntry.string().find("/tst.") == std::string::npos){
            tstFolders.push_back(dirEntry.string());
            std::cout << "Found Directory: " << dirEntry << std::endl;
        }
    }
    return tstFolders;
}

void createBoundsGrid (const std::string filepath, testOutput &out) {
    out.coords.boundingBoxes.clear();
    std::vector<int> subCoordinates;
    std::vector<std::vector<std::string>> content;
	std::vector<std::string> row;
	std::string line, word;
    int ctr = 0;

    std::string fname = filepath + "/tst.csv";

    std::fstream file (fname, std::ios::in);
    if(file.is_open()) {
		while(getline(file, line)) {
			row.clear();
			std::stringstream str(line);

			while(getline(str, word, ','))
				row.push_back(word);
			    content.push_back(row);
		}
	} else { std::cout<<"Could not open" << fname << "\n"; }

    for(int i=0;i<content.size();i++) {
        for(int j=0;j<content[i].size();j++) {   
            switch (i) {
                case 0:
                    if(j==0) {
                        out.coords.currAngle = std::stof(content[i][j]);
                    } else {
                        subCoordinates.push_back(std::stoi(content[i][j]));
                        ctr++;
                        if(ctr == 4) {
                            out.coords.boundingBoxes.push_back(subCoordinates);
                            subCoordinates.clear();
                            ctr = 0;
                        }
                    }
                    break;
                case 1:
                    switch (j) {
                        case 0:
                            out.classifiedOut.isEnd = (content[i][j] == "true") ? true : false;
                            break;
                        case 1:
                            out.classifiedOut.isNode = (content[i][j] == "true") ? true : false;
                            break;
                        case 2:
                            out.classifiedOut.isPath = (content[i][j] == "true") ? true : false;
                            break;
                        case 3:
                            out.classifiedOut.isClear = (content[i][j] == "true") ? true : false;
                            break;
                        case 4:
                            out.classifiedOut.leftWall = (content[i][j] == "true") ? true : false;
                            break;
                        case 5:
                            out.classifiedOut.rightWall = (content[i][j] == "true") ? true : false;
                            break;
                        case 6:
                            out.classifiedOut.leftTurn = (content[i][j] == "true") ? true : false;
                            break;
                        case 7:
                            out.classifiedOut.rightTurn = (content[i][j] == "true") ? true : false;
                            break;
                        default:
                            assert(true == false);
                            std::cout << "Error with the target outputs" << std::endl;
                    }
                    break;
                default: 
                    assert(true == false);
                    std::cout << "Too Many lines in the test file" << std::endl;
            }                
        }
    }
}

void testAlgorithm(const testOutput &boundsGrid, const std::string path){
    // drawGrid(boundsGrid.coords.boundingBoxes);
    classifyMazeElement(boundsGrid.coords.boundingBoxes);

    if(DEBUGOUTPUTS){
        printDebug();
    }

    std::string errorOut;
    
    errorOut = "Failed: isEnd in " + path + ", expected: " + ((boundsGrid.classifiedOut.isEnd) ? "true" : "false") + ", got: " + ((isEnd) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.isEnd == isEnd, errorOut);
    errorOut = "Failed: isNode in " + path + ", expected: " + ((boundsGrid.classifiedOut.isNode) ? "true" : "false") + ", got: " + ((isNode) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.isNode == isNode, errorOut);
    errorOut = "Failed: isPath in " + path + ", expected: " + ((boundsGrid.classifiedOut.isPath) ? "true" : "false") + ", got: " + ((isPath) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.isPath == isPath, errorOut);
    errorOut = "Failed: isClear in " + path + ", expected: " + ((boundsGrid.classifiedOut.isClear) ? "true" : "false") + ", got: " + ((isClear) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.isClear == isClear, errorOut);
    errorOut = "Failed: leftWall in " + path + ", expected: " + ((boundsGrid.classifiedOut.leftWall) ? "true" : "false") + ", got: " + ((leftWall) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.leftWall == leftWall, errorOut);
    errorOut = "Failed: rightWall in " + path + ", expected: " + ((boundsGrid.classifiedOut.rightWall) ? "true" : "false") + ", got: " + ((rightWall) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.rightWall == rightWall, errorOut);
    errorOut = "Failed: leftTurn in " + path + ", expected: " + ((boundsGrid.classifiedOut.leftTurn) ? "true" : "false") + ", got: " + ((leftTurn) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.leftTurn == leftTurn, errorOut);
    errorOut = "Failed: rightTurn in " + path + ", expected: " + ((boundsGrid.classifiedOut.rightTurn) ? "true" : "false") + ", got: " + ((rightTurn) ? "true" : "false");
    ASSERT(boundsGrid.classifiedOut.rightTurn == rightTurn, errorOut);

    std::cout << "Passed: " << path << std::endl;
}

void testNodeScanner(float startAngle, float endAngle) {
    float pathStart    = -1;
    float beaconStart  = -1;

    bool scanningPath   = isClear; //isPath
    bool scanningRed    = isRed;
    bool scanningBlue   = isBlue;
    bool scanningYellow = isYellow;

    int cycle = 0;
    int rotation = 0;
    int prevAngle = -1;

    blockedRanges.push_back({((int)currAngle+141)%360, ((int)currAngle+219)%360});

    // Testing
    std::vector<std::string> tstFolders = getFolders(TESTPATHS);
    std::sort(tstFolders.begin(),tstFolders.end());
    testOutput test;

    for (const auto& tstFile : tstFolders){
        std::cout<<"\n";
        std::cout<<"Img Path: "<<tstFile<<std::endl;
        createBoundsGrid(tstFile, test);
        // test.coords.printGrid();
        classifyMazeElement(test.coords.boundingBoxes);
        currAngle = test.coords.currAngle;
        std::cout<<"Current Image Angle: "<<currAngle<<std::endl;

        // modulo
        if (currAngle >= 360) { currAngle -= 360; }

        // expected test inputs
        if (currAngle < 31) {
        } else if (currAngle < 40) {
        } else if (currAngle < 125) {
            isRed   = false;
        } else if (currAngle < 205) {
        } else if (currAngle < 223) {
            isBlue = true;
        } else if (currAngle < 268) {
            isBlue  = false;
        } else if (currAngle < 280) {
        } else if (currAngle < 342) {
        } else if (currAngle < 352) {
            isYellow = true;
        } else {
            isRed    = true;
            isYellow = false;
        }
        // Testing

        bool scanningBeacon = scanningRed || scanningBlue || scanningYellow; // check for modulo/overflow
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

int main() {
    std::vector<std::string> tstFolders = getFolders(TESTPATHS);
    std::sort(tstFolders.begin(),tstFolders.end());
    testOutput test;

    for (const auto& tstFile : tstFolders){
        std::cout<<"\n";
        // std::cout<<tstFile<<std::endl;
        createBoundsGrid(tstFile, test);
        test.coords.printGrid();
        testAlgorithm(test, tstFile);
    }

    // MAKE SURE FIRST ANGLE IS EQUAL TO THIS IN FILES
    // blockedRanges.push_back({0,20});
    // blockedRanges.push_back({129,231});
    // testNodeScanner(280,280+405);

    // if(DEBUGOUTPUTS){
    //     printDebug();
    // }
}
