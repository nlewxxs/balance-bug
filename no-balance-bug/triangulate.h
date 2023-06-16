#ifndef triangulate_h
#define triangulate_h

///////////////////////////////////////////
///////////       MACROS        ///////////
///////////////////////////////////////////

#define XDIST 100
#define YDIST 100
#define ERRMARGIN 5

///////////////////////////////////////////
///////////     DEFINITIONS     ///////////
///////////////////////////////////////////

//warning, BR conflicts with a macro in esp32 xtensa config, so have to rename to BR_Beacons...

struct Node {
    int x;
    int y;

    Node(int _x = 0, int _y = 0)
        : x(_x), y(_y) {}
};

struct Angle {
    float yellow;
    float red;
    float blue;

    Angle(float _yellow = 0, float _red = 0, float _blue = 0)
        : yellow(_yellow), red(_red), blue(_blue) {}
};

struct CoordAngle {
    float BR_Beacons;
    float BY_Beacons;
    float RY_Beacons;

    CoordAngle(float _BR_Beacons = 0, float _BY_Beacons = 0, float _RY_Beacons = 0)
        : BR_Beacons(_BR_Beacons), BY_Beacons(_BY_Beacons), RY_Beacons(_RY_Beacons) {}
};

struct AngleBool {
    bool aBR_Beacons;
    bool aBY_Beacons;
    bool aRY_Beacons;

    AngleBool(bool _aBR_Beacons = false, bool _aBY_Beacons = false, bool _aRY_Beacons = false)
        : aBR_Beacons(_aBR_Beacons), aBY_Beacons(_aBY_Beacons), aRY_Beacons(_aRY_Beacons) {}
};

#endif
