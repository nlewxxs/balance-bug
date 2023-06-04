// #include <ArduinoEigenDense.h>

// using namespace Eigen;

// placeholder values for now - get from MATLAB auto-tuning
int Kp_heading = 40;
int Ki_heading = 0; // PD controller for heading
int Kd_heading = 1;
int Kp_position = 40;
int Ki_position = 0; // PD controller for position
int Kd_position = 1;
int Kp_tilt = 40;
int Ki_tilt = 1;
int Kd_tilt = 1;

int balanceCenter = 90; // whatever tilt value is balanced

int P_bias, T_bias, H_bias = 0;

double H_derivative, H_out, P_derivative, P_out, T_derivative, T_out;
double H_error[2], P_error[2], T_error[2] = {0, 0}; //stores current and previous value for derivative calculation
double H_integral[2], P_integral[2], T_integral[2] = {0, 0}; //stores current and previous value for integral calculation
int iteration_time;
int oldMillis = 0;


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

  // how to define matrices using eigen for when we do LQR
  // MatrixXf m1(2,4);
  // m1 << 1, 2, 3, 4, 5, 6, 7, 8;
  
  // MatrixXf m2(4,3);
  // m2 << 9, 3, 2, 6, 4, 2, 6, 7, 3, 1, 4, 2;

  // MatrixXf outm = m1 * m2;

}

void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println("test");

  iteration_time = millis() - oldMillis;

  // position control first
  P_error[1] = getPosSetpoint() - getPosition();
  P_integral[1] = P_integral[0] + P_error[1] * iteration_time; // 1 is current, 0 is old
  P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  P_out = Kp_position*P_error[1] + Ki_position*P_integral[1] + Kd_position*P_derivative + P_bias;
  // make this output positive to move forwards, negative to move backwards

  P_integral[0] = P_integral[1]; // time shift integral readings after out calculated
  P_error[0] = P_error[1];

  // balance control
  T_error[1] = balanceCenter + P_out - getTilt(); // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  T_integral[1] = T_integral[0] + T_error[1] * iteration_time; // 1 is current, 0 is old
  T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  T_out = Kp_position*T_error[1] + Ki_position*T_integral[1] + Kd_position*T_derivative + T_bias;

  T_integral[0] = T_integral[1]; // time shift integral readings after out calculated
  T_error[0] = T_error[1];

  // heading control to offset the wheels for rotation
  H_error[1] = getHeadingSetpoint() - getHeading();
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time; // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading*H_error[1] + Ki_heading*H_integral[1] + Kd_heading*H_derivative + H_bias;

  H_integral[0] = H_integral[1]; // time shift integral readings after out calculated
  H_error[0] = H_error[1];


  double leftWheelDrive = T_out - H_out;
  double rightWheelDrive = T_out + H_out;

  Serial.print("Currently at\nPosition: ");
  Serial.println(getPosition());
  Serial.print("Tilt: ");
  Serial.println(getTilt());
  Serial.print("Heading: ");
  Serial.println(getHeading());
  Serial.print("Control outputs\nPosition: ");
  Serial.println(P_out);
  Serial.print("Tilt: ");
  Serial.println(T_out);
  Serial.print("Heading: ");
  Serial.println(H_out);

}

double getTilt() {
  // tilt of inverted pendulum
  return 95.0;
}

double getPosition() { //unsure of how this reading will work - needs to be 1D (as in just x)
  // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?
  return 10;
}

double getPosSetpoint() {
  // output from the imaging program to tell us where to be
  return 0;
}

double getHeading() { // probably range from 0->2pi
  return 0.6;
}

double getHeadingSetpoint() {
  // output from the imaging program to tell us where to face
  return 0;
}





