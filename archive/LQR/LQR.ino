#include <ArduinoEigenDense.h>
using namespace Eigen;
#include <Wire.h>
#include "mpu6050.h"

#define N 50 // how many loops we are using to find the optimal value

MatrixXf A(3,3); // states are position, tilt, heading
MatrixXf B(3,2); // two control inputs (L and R wheels) relating to 3 states
MatrixXf R(2,2); // control input cost matrix
MatrixXf Q(3,3); // state cost matrix

MatrixXf actualState(3,1);
MatrixXf desiredState(3,1);
MatrixXf error(3,1);

MatrixXf P[N+1]; // array of matrices P (unknown matrix used for finding optimal solution)
MatrixXf K[N]; // constant (for solution to DARE -> u = -k*e)
MatrixXf u[N]; // control inputs

double iteration_time;
unsigned long oldMillis = 0;

int balanceCenter = 0;

mpu6050 mpu = mpu6050();

void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  // how to define matrices using eigen for when we do LQR
  // MatrixXf m1(2,4);
  // m1 << 1.5, 2, 3, 4, 5, 6, 7, 8;
  // MatrixXf m2(4,3);
  // m2 << 9, 3, 2, 6, 4, 2, 6, 7, 3, 1, 4, 2;
  // MatrixXf outm = m1 * m2;

  // P << 1,1,1,1,1,1,1,1,1;
  // MatrixXf test[N];
  // test[0] = P;
  // test[1] = P;
  // Serial.println(test[1](1,1));

  // Serial.print(outm(0,0));
  // Serial.print(" ");
  // Serial.print(outm(0,1));
  // Serial.print(" ");
  // Serial.print(outm(0,2));
  // Serial.println(" ");
  // Serial.print(outm(1,0));
  // Serial.print(" ");
  // Serial.print(outm(1,1));
  // Serial.print(" ");
  // Serial.print(outm(1,2));
  
  A <<  1, 0, 0,
        0, 1, 0,
        0, 0, 1; // identity - system won't change without inputs (hopefully this is accurate - potentially pitch will change due to gravity but can't say if this will be pos or neg so set to 1 for now)
  R <<  5, 0,
        0, 5; //diagonal, each value corresponds to a control input cost (first for L second for R - larger value means more costly, low effort required)
  Q <<  2, 0, 0,
        0, 25, 0,
        0, 0, 1.2; //diagonal, each value corresponds to state importance (balance most important hence set the largest)
  updateB();

  desiredState << 0, balanceCenter, 0; // setpoints for no movement
  actualState << 0, 0, 0; // initial conditions

  mpu.init();

  mpu.calibrate();
  delay(20);
}

void updateB() { //B changes each loop. Expresses how the system changes from t-1 to t due to the control inputs

  //  --  --  --     position/tilt/heading dependence on left wheel input
  //  --  --  --     position/tilt/heading dependence on right wheel input - this will need to be modelled with transfer functions and then us comparing force to wheel input

  B <<  0.5, 0.5,
        -2, -2,
        1, -1; // position and tilt will depend equally on each wheel (sum A/2 + B/2 and get the total wheel output)
                                // heading depends positively on left wheel and negatively on right wheel
                                // position++ with wheels, tilt inversely so
}

void loop() {
  mpu.update();

  // actualState(0,0) = getPosition();
  actualState(1,0) = mpu.getPitch();
  actualState(2,0) = mpu.getYaw();

  // Serial.print(mpu.getRawXGyro());
  // Serial.print(" / ");
  // Serial.print(mpu.getRawYGyro());
  // Serial.print(" / ");
  // Serial.print(mpu.getRawZGyro());
  // Serial.print(" / ");
  // Serial.println(mpu.getRawZGyro());

  
  // actualState(1,0) = 23;
  // actualState(2,0) = -6;



  // unsigned long iteration_time_temp = (millis() - oldMillis);
  // iteration_time = iteration_time_temp / 1000;
  double iteration_time = (millis() - oldMillis) / 1000.0;
  oldMillis = millis();

  error = actualState - desiredState;


  // for (int j = 0; j < Q.rows(); j++) {
  //   for (int k = 0; k < Q.cols(); k++) {
  //     Serial.print(Q.inverse()(j,k));
  //     Serial.print(" ");
  //   }
  //   Serial.println();
  // }




  // Serial.print("  Error 0: ");
  // Serial.print(actualState(0,0));
  // Serial.print(" - ");
  // Serial.print(desiredState(0,0));
  // Serial.print(" = ");
  // Serial.print(error(0,0));
  // Serial.print("  Error 1: ");
  // Serial.print(actualState(1,0));
  // Serial.print(" - ");
  // Serial.print(desiredState(1,0));
  // Serial.print(" = ");
  // Serial.print(error(1,0));
  // Serial.print("  Error 2: ");
  // Serial.print(actualState(2,0));
  // Serial.print(" - ");
  // Serial.print(desiredState(2,0));
  // Serial.print(" = ");
  // Serial.print(error(2,0));
  // Serial.print("  Error: ");
  // Serial.print(error(0,0));
  // Serial.print(" / ");
  // Serial.print(error(1,0));
  // Serial.print(" / ");
  // Serial.print(error(2,0));

  // MatrixXf empty(3,3);
  // empty << 0,0,0,0,0,0,0,0,0;

  // for (int i = 0; i < N; i++) {
  //   P[i] = Zero(); // 3x3
  //   K[i] = Zero(); // 2x3
  //   u[i] = Zero(); // 2x1
  // }

  P[N] = Q;
  for (int i = N; i > 0; i--) {
    P[i-1] = Q + A.transpose()*P[i]*A - A.transpose()*P[i]*B*(B.transpose()*P[i]*B + R).inverse()*B.transpose()*P[i]*A;
    // Serial.print("P[");
    // Serial.print(i-1);
    // Serial.println("]:");
    // for (int j = 0; j < P[i-1].rows(); j++) {
    //   for (int k = 0; k < P[i-1].cols(); k++) {
    //     Serial.print(P[i-1](j,k));
    //     Serial.print(" ");
    //   }
    //   Serial.println();
    // }
  }

  // Serial.print("  Error: ");
  // Serial.print(error(0,0));
  // Serial.print(" / ");
  // Serial.print(error(1,0));
  // Serial.print(" / ");
  // Serial.print(error(2,0));

  for (int i = 0; i < N; i++) {
    K[i] = (R + B.transpose()*P[i]*B).inverse()*B.transpose()*P[i]*A;
    u[i] = -K[i]*error;
    // Serial.print("K[");
    // Serial.print(i);
    // Serial.println("]:");
    // for (int j = 0; j < K[i].rows(); j++) {
    //   for (int k = 0; k < K[i].cols(); k++) {
    //     Serial.print(K[i](j,k));
    //     Serial.print(" ");
    //   }
    //   Serial.println();
    // }
    // Serial.print("u[");
    // Serial.print(i);
    // Serial.println("]:");
    // for (int j = 0; j < u[i].rows(); j++) {
    //   for (int k = 0; k < u[i].cols(); k++) {
    //     Serial.print(u[i](j,k));
    //     Serial.print(" ");
    //   }
    //   Serial.println();
    // }
  }

  Serial.print("Pitch: ");
  Serial.print(actualState(1,0));
  Serial.print("Heading: ");
  Serial.print(actualState(2,0));
  Serial.print("  Wheel inputs: ");
  Serial.print(u[N-1](0,0));
  Serial.print(" / ");
  Serial.println(u[N-1](1,0));
  // Serial.print("  Desired: ");
  // Serial.print(desiredState(0,0));
  // Serial.print(" / ");
  // Serial.print(desiredState(1,0));
  // Serial.print(" / ");
  // Serial.print(desiredState(2,0));
  // Serial.print("  Actual: ");
  // Serial.print(actualState(0,0));
  // Serial.print(" / ");
  // Serial.print(actualState(1,0));
  // Serial.print(" / ");
  // Serial.print(actualState(2,0));
  // Serial.print("  Error: ");
  // Serial.print(error(0,0));
  // Serial.print(" / ");
  // Serial.print(error(1,0));
  // Serial.print(" / ");
  // Serial.println(error(2,0));

  // delay(100);
  // Serial.end();
}

double getPosition() { //unsure of how this reading will work - needs to be 1D (as in just x)
  // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?
  return 0;
}

double getPosSetpoint() {
  // output from the imaging program to tell us where to be
  return 0;
}

double getHeadingSetpoint() {
  // output from the imaging program to tell us where to face
  return 0;
}


