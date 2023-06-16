#include "Traversal.h"
#include "Classify.h"

Image imgInfo;
MazeUtils mazeInfo;

void setup() {
    int grid[12][4] = { {4, 122, 159, 205},   {160, 130, 301, 239}, {419, 136, 458, 146}, {497, 153, 637, 239},
                        {97, 307, 159, 359},  {160, 240, 259, 318}, {0, 0, 0, 0},         {560, 240, 620, 359},
                        {4, 360, 113, 460},   {0, 0, 0, 0},         {0, 0, 0, 0},         {607, 360, 637, 422} 
    };
    imgInfo.classify(grid);
    imgInfo.debugInfo();
}

void loop() {}
