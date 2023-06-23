#ifndef _CLASSIFY_IMAGE_H_
#define _CLASSIFY_IMAGE_H_

class image {
    public:
        bool outsideBounds(const int (&box)[4], const int (&bound)[4]);
        bool hasWall(const int (&lower)[4], const int (&upper)[4], const int (&bound)[4]);
        void classify(int grid[16][4]); 
};

#endif
