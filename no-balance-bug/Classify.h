#ifndef Classify_h
#define Classify_h

#include <Arduino.h>
#include <stdio.h>

struct classifyElement{
    bool isEnd;
    bool isNode;
    bool isPath;

    bool isClear;
    bool leftWall;
    bool rightWall;

    bool leftTurn;
    bool rightTurn;

    classifyElement(bool _isEnd = false, bool _isNode = false, bool _isPath = false, bool _isClear = false, bool _leftWall = false, bool _rightWall = false, bool _leftTurn = false, bool _rightTurn = false)
        : isEnd(_isEnd), isNode(_isNode), isPath(_isPath), isClear(_isClear), leftWall(_leftWall), rightWall(_rightWall), leftTurn(_leftTurn), rightTurn(_rightTurn) {}
};

class Image {
    public:
        bool outsideBounds(const int box[4], const int bound[4]);
        bool outsideBoundsMiddle(const int box[4], const int bound[4]);
        bool hasWall(const int lowerL[4], const int lowerR[4], const int upperL[4], const int upperR[4],const int xLR, const int bound[4], bool L);
        void debugInfo();
        classifyElement classify(int grid[12][4]); 
    private:
        classifyElement info;
};

#endif
