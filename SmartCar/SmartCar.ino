#include <Smartcar.h>
#include <BluetoothSerial.h>

BluetoothSerial bluetooth;

const int speed = 70;     // 70% of the full speed
const int turnAngle = 75; // Degrees to turn

const int minObstacle = 20; // Minimum distance ahead to obstacle

// const int TRIGGER_PIN = 5; // Trigger signal for ultrasonic
// const int ECHO_PIN = 18; // Reads signal for ultrasonic

// FIXME: LINUS make certain this is the correct pin on the vehicle
const int SIDE_FRONT_PIN = A0; // Infrared reader

const unsigned long PULSES_PER_METER = 600; // Amount of odometer pulses per 1 meter
// const unsigned int MAX_DISTANCE = 100; // Max distance to measure with ultrasonic
const int GYROSCOPE_OFFSET = 37;

// SR04 front(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Ultrasonic sensor

GP2Y0A21 sideFrontIR(SIDE_FRONT_PIN); // Infrared sensor (12-78 cm distance reading)

GY50 gyro(GYROSCOPE_OFFSET); // Gyroscope sensorconst int rDegrees = 75;  // Degrees to turn right

DirectionlessOdometer leftOdometer(
    smartcarlib::pins::v2::leftOdometerPin, []() { leftOdometer.update(); }, PULSES_PER_METER); // Odometer sensor
DirectionlessOdometer rightOdometer(
    smartcarlib::pins::v2::rightOdometerPin, []() { rightOdometer.update(); }, PULSES_PER_METER);

BrushedMotor leftMotor(smartcarlib::pins::v2::leftMotorPins); // Motors
BrushedMotor rightMotor(smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

SmartCar car(control, gyro, leftOdometer, rightOdometer); // Initializing of the car

void setup()
{
    Serial.begin(9600);
    bluetooth.begin("Group 2 SmartCar");
    Serial.print("Ready to connect!");
}

float distance;
boolean atObstacle = false;
void driveWithAvoidance()
{
    distance = sideFrontIR.getMedianDistance();
    if (distance > 0 && distance <= minObstacle)
    {
        atObstacle = true;
        car.setSpeed(0);
        car.setAngle(0);
    }
    car.setSpeed(-10);
    delay(1000); // FIXME: Have the car move a bit backward before searching for new path
    //TODO: Add turnRight and turnLeft as separate methods (make papa francisco proud)

    while (atObstacle)
    {
        // Try turning right
        for (int i = car.getHeading(); i < turnAngle; i += 5)
        {
            // Increment steering to the right and check if space is free
            turnRight(i, 20);
            delay(100);
            if (distance > 65) //65 was used as 78 is the max range for the LIDAR sensor
            {
                atObstacle = false;
                break;
            }
            car.setAngle(0);
        }
        // Try turning left
        for (int i = car.getHeading(); i > turnAngle; i = 5)
        {
            // Increment steering to the right and check if space is free
            turnLeft(i, 20);
            delay(100);
            if (distance > 65) //65 was used as 78 is the max range for the LIDAR sensor
            {
                atObstacle = false;
                break;
            }
            car.setAngle(0);
        }
    }
}

void turnRight(int degrees, int turnSpeed = speed)
{
    car.setSpeed(turnSpeed);
    car.setAngle(degrees);
}

void turnLeft(int degrees, int turnSpeed = speed)
{
    car.setSpeed(turnSpeed);
    car.setAngle(degrees);
}

void driveForward(int speed)
{
    if (car.getSpeed < 0)
    {                              // Fixes motor overloading
        while (car.getSpeed < -20) // Used 20 as a threshold
        {
            car.setSpeed(car.getSpeed + 10);
        }
    }
    car.setSpeed(speed);
    car.setAngle(0);
}

void driveBackward(int speed)
{
    if (car.getSpeed > 0)
    {                             // Fixes motor overloading
        while (car.getSpeed > 20) // Used 20 as a threshold
        {
            car.setSpeed(car.getSpeed - 10);
        }
    }
    car.setAngle(0);
    car.setSpeed(speed);
}

void handleInput(char input)
{ // Handle serial input if there is any

    switch (input)
    {

    case 'a': // Automatic driving
        driveWithAvoidance();
        break;

    case 'l': // Turn Left
        turnLeft(-turnAngle);
        break;

    case 'r': // Turn Right
        turnRight(turnAngle);
        break;

    case 'f': // Forward
        driveForward(speed);
        break;

    case 'b': // Backwards
        driveBackward(-speed);
        break;

    case 'i': // Increases carspeed by 10%
        car.setSpeed(car.getSpeed() + 10);
        break;

    case 'q': // Decreases carspeed by 10%
        car.setSpeed(car.getSpeed() - 10);
        break;

    default:
        car.setSpeed(0);
        car.setAngle(0);
    }
}

void readBluetooth()
{
    while (bluetooth.available())
    {
        char input = bluetooth.read();
        handleInput(input);
    }
}

void loop()
{
    readBluetooth();
    driveWithAvoidance();
}