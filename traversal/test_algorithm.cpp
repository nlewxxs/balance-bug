#include "algorithm.cpp"
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>  

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

#define TESTPATHS "./testcases/"
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
        for (int i = 0; i < 16; i++) {
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

std::vector<std::string> getFolders(const std::string path){
    filepath myPath("./testcases/");
    std::vector<std::string> tstFolders;
    for (const filepath& dirEntry : recursive_directory_iterator(myPath)){
        if(dirEntry.string().find("/tst.") == std::string::npos){
            tstFolders.push_back(dirEntry.string());
            std::cout << "Found Directory: " << dirEntry << std::endl;
        }
    }
    return tstFolders;
}

void createBoundsGrid (const std::string filepath, testOutput &out){
    out.coords.boundingBoxes.clear();
    std::vector<int> subCoordinates;
    std::vector<std::vector<std::string>> content;
	std::vector<std::string> row;
	std::string line, word;
    int ctr = 0;

    std::string fname = filepath + "/tst.csv";

    std::fstream file (fname, std::ios::in);
	if(file.is_open())
	{
		while(getline(file, line))
		{
			row.clear();
 
			std::stringstream str(line);
 
			while(getline(str, word, ','))
				row.push_back(word);
			    content.push_back(row);
		}
	}
	else
		std::cout<<"Could not open" << fname << "\n";

        for(int i=0;i<content.size();i++)
        {
            for(int j=0;j<content[i].size();j++)
            {   
                std::cout << "J is " << j << std::endl;
                switch (i){
                    case 0:
                        std::cout << "GOT: " << content[i][j] << std::endl;
                        if(j==0){
                            out.coords.currAngle = std::stof(content[i][j]);
                        } else {
                            subCoordinates.push_back(std::stoi(content[i][j]));
                            ctr++;
                            if(ctr == 4){
                                ctr = 0;
                                out.coords.boundingBoxes.push_back(subCoordinates);
                                subCoordinates.clear();
                            }
                        }
                        break;
                    
                    case 1:
                        switch (j){
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
            std::cout<<"\n";
        }
}

void testAlgorithm(const testOutput &boundsGrid, const std::string path){
    drawGrid(boundsGrid.coords.boundingBoxes);

    classifyMazeElement(boundsGrid.coords.boundingBoxes);
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
    
    currAngle = boundsGrid.coords.currAngle;
    nodeScanner(280,280+1205);

    if(DEBUGOUTPUTS){
        printDebug();
    }
}


int main() {
    std::vector<std::string> tstFolders = getFolders(TESTPATHS);
    testOutput test;

    for (const auto& tstFile : tstFolders){
        std::cout << "Now Testing Directory: " << tstFile << std::endl;
        createBoundsGrid(tstFile, test);
        test.coords.printGrid();
        testAlgorithm(test, tstFile);
    }
    
    
}