#include <iostream>
#include <math.h>
#include "triangulate.h"


///////////////////////////////////////////
///////////      Functions      ///////////
///////////////////////////////////////////

// AngleBool detectAnomalie (const CoordAngle &input) {
//     bool aBR_BeaconsBY_Beacons, aBR_BeaconsRY_Beacons, aRY_BeaconsBY_Beacons = false;

//     if(abs((input.BR_Beacons - input.BY_Beacons)) > ERRMARGIN){
//         aBR_BeaconsBY_Beacons = true;
//     }
//     if(abs((input.BR_Beacons - input.RY_Beacons)) > ERRMARGIN){
//         aBR_BeaconsRY_Beacons = true;
//     }
//     if(abs((input.RY_Beacons - input.BY_Beacons)) > ERRMARGIN){
//         aRY_BeaconsBY_Beacons = true;
//     }


//     //TODO: TEST + FINISH IMPLEMENT
//     if(aBR_BeaconsBY_Beacons && aBR_BeaconsRY_Beacons && aRY_BeaconsBY_Beacons){
//         return AngleBool{false, false, false};
//     }
//     if(aBR_BeaconsBY_Beacons && aBR_BeaconsRY_Beacons && !aRY_BeaconsBY_Beacons){
//         return AngleBool{true, true, false};
//     }
//     if(aBR_BeaconsBY_Beacons && !aBR_BeaconsRY_Beacons && aRY_BeaconsBY_Beacons){
//         return AngleBool{true, false, true};
//     }
//     if(!aBR_BeaconsBY_Beacons && aBR_BeaconsRY_Beacons && aRY_BeaconsBY_Beacons){
//         return AngleBool{false, true, true};
//     }
//     if(!aBR_BeaconsBY_Beacons && !aBR_BeaconsRY_Beacons && aRY_BeaconsBY_Beacons){
//         return AngleBool{false, false, false};
//     }
//     if(!aBR_BeaconsBY_Beacons && aBR_BeaconsRY_Beacons && !aRY_BeaconsBY_Beacons){
//         return AngleBool{false, false, false};
//     }
//     if(aBR_BeaconsBY_Beacons && !aBR_BeaconsRY_Beacons && !aRY_BeaconsBY_Beacons){
//         return AngleBool{false, false, false};
//     }
//     return AngleBool{false, false, false};
// }

Node triangulate (const Angle &input, const float &offset) {
    //declarations
    Angle modifiedAngle;
    Node output;

    float xBR_Beacons;
    float xBY_Beacons;
    float xRY_Beacons;

    float yBR_Beacons;
    float yBY_Beacons;
    float yRY_Beacons;

    //modify the modifiedAngle
    modifiedAngle.blue = (360 - (input.blue - offset)) * (M_PI/180);
    modifiedAngle.red  = (input.red - offset) * (M_PI/180);
    modifiedAngle.yellow = (180 - input.yellow - offset) * (M_PI/180);


    //x-triangulation
    xBR_Beacons = (YDIST * tan(modifiedAngle.blue)) / (tan(modifiedAngle.blue) + tan(modifiedAngle.red));
    xBY_Beacons = ((tan(modifiedAngle.blue) * (XDIST / 2)) - (YDIST * tan(modifiedAngle.blue) * tan(modifiedAngle.yellow))) / (tan(modifiedAngle.blue) - tan(modifiedAngle.yellow));
    xRY_Beacons = (((XDIST / 2) * tan(modifiedAngle.red)) + (XDIST * tan(modifiedAngle.yellow)) - (YDIST * tan(modifiedAngle.red) * tan(modifiedAngle.yellow))) / (tan(modifiedAngle.yellow) + tan(modifiedAngle.red));

    //y-triangulation
    yBR_Beacons = YDIST - (YDIST / (tan(modifiedAngle.blue) + tan(modifiedAngle.red)));
    yBY_Beacons = ((YDIST * tan(modifiedAngle.red)) - (XDIST / 2)) / (tan(modifiedAngle.red) + tan(modifiedAngle.yellow));
    yRY_Beacons = ((YDIST * tan(modifiedAngle.red)) - (XDIST / 2)) / (tan(modifiedAngle.red) + tan(modifiedAngle.yellow));

    output.x = (xBR_Beacons + xBY_Beacons + xRY_Beacons) / 3;
    output.y = (yBR_Beacons + yBY_Beacons + yRY_Beacons) / 3;

    return output;
}

