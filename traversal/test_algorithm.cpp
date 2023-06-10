#include "algorithm.cpp"
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
using filepath = std::filesystem::path;

#define TESTPATHS "./testcases/"
#define DEBUGOUTPUTS true

struct extractedCoords{
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

void createBoundsGrid (const std::string filepath, extractedCoords &out){
    out.boundingBoxes.clear();
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
                if((i == 0) && j==0){
                    out.currAngle = std::stof(content[i][j]);
                } else {
                    std::cout<<content[i][j]<<", ";
                    subCoordinates.push_back(std::stoi(content[i][j]));
                    ctr++;
                    if(ctr == 4){
                        ctr = 0;
                        out.boundingBoxes.push_back(subCoordinates);
                        subCoordinates.clear();
                    }
                }
                
            }
            std::cout<<"\n";
        }
}

void testAlgorithm(const extractedCoords &boundsGrid){
    drawGrid(boundsGrid.boundingBoxes);

    classifyMazeElement(boundsGrid.boundingBoxes);

    isClear = true;
    isRed = true;
    currAngle = boundsGrid.currAngle;
    nodeScanner(280,280+1205);

    if(DEBUGOUTPUTS){
        printDebug();
    }
}


int main() {
    std::vector<std::string> tstFolders = getFolders(TESTPATHS);
    extractedCoords testCoords;



    for (const auto& tstFile : tstFolders){
        std::cout << "Now Testing Directory: " << tstFile << std::endl;
        createBoundsGrid(tstFile, testCoords);
        testCoords.printGrid();
        testAlgorithm(testCoords);
    }
    
    
}