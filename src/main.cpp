#include "main.h"

#define LEFT_MOTOR_A_PORT 1
#define LEFT_MOTOR_B_PORT 3
#define LEFT_MOTOR_C_PORT 2 //reversed

#define RIGHT_MOTOR_A_PORT 13 //not reversed
#define RIGHT_MOTOR_B_PORT 11
#define RIGHT_MOTOR_C_PORT 12

#define INERTIAL_PORT 15

#define INTAKE_PORT 10

#define HIGH_STAKES_PORT 4

#define EXT_ADI_SMART_PORT 9
#define CLAMP_PORT 'a'
#define FLAG_PORT 'd'

pros::Motor Intake(INTAKE_PORT);
pros::Motor HighStakes(HIGH_STAKES_PORT);

pros::MotorGroup LeftDriveSmart({LEFT_MOTOR_A_PORT, LEFT_MOTOR_B_PORT, -LEFT_MOTOR_C_PORT}); //Creates a motor group with forwards ports 1 & 4 and reversed port 7
pros::MotorGroup RightDriveSmart({-RIGHT_MOTOR_A_PORT, RIGHT_MOTOR_B_PORT, -RIGHT_MOTOR_C_PORT}); //Creates a motor group with forwards port 2 and reversed port 6 and 5
pros::Imu DrivetrainInertial(INERTIAL_PORT);
//DrivetrainInertial.reset();
pros::MotorGroup smartdrive ({LEFT_MOTOR_A_PORT, LEFT_MOTOR_B_PORT, -LEFT_MOTOR_C_PORT, -RIGHT_MOTOR_A_PORT, RIGHT_MOTOR_B_PORT, -RIGHT_MOTOR_C_PORT, INERTIAL_PORT});
pros::ADIDigitalOut Clamp ({CLAMP_PORT});
pros::ADIDigitalOut Flag ({FLAG_PORT});
pros::IMU Inertial({INERTIAL_PORT});


bool flagState = true;
bool clampState = false;
bool intakeState = false;

void ToggleClamp() {
    clampState = !clampState;          // Toggle the state
    Clamp.set_value(clampState);       // Update the digital output
    pros::delay(200);                // Delay for debouncing
}
void ToggleFlag() {
    flagState = !flagState;          // Toggle the state
    Flag.set_value(flagState);       // Update the digital output
    pros::delay(200);                // Delay for debouncing
}

void ToggleIntake() {
    intakeState = !intakeState;
    if (intakeState) {
        pros::screen::print(TEXT_SMALL, 3, "Ssetting to 200");
        Intake.move_velocity(200);
    }
    else {
        pros::screen::print(TEXT_SMALL, 3, "Ssetting to 0");
        Intake.move_velocity(0);
    }
}

void BuzzingTask() {
    pros::Controller Controller1(pros::E_CONTROLLER_MASTER);
    while (true) {
        if (intakeState) {
            Controller1.rumble(".");  // Short rumble to simulate continuous buzzing
        }
        pros::delay(15); // Delay to control buzzing frequency
    }
}



/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
	pros::lcd::register_btn1_cb(on_center_button);
    pros::Task buzzingTask(BuzzingTask);
}


/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
enum Direction {clockwise, counterclockwise};
void TurnDegrees(pros::IMU& inertial, Direction dir, int degrees) {
    int initial = Inertial.get_heading();
    int targetdeg;

    if (dir == clockwise) {
        targetdeg = (initial + degrees) % 360;
        LeftDriveSmart.move_velocity(-20);
        RightDriveSmart.move_velocity(20);

        while (inertial.get_heading() < targetdeg) {
            pros::delay(5);
        }
    } else if (dir==counterclockwise) { 
        targetdeg = 360-degrees;
        
        RightDriveSmart.move_velocity(-20);
        LeftDriveSmart.move_velocity(20);

        while (inertial.get_heading() > targetdeg || inertial.get_heading() < 5) {
            pros::delay(5);
        }
    }

    // Stop the motors
    LeftDriveSmart.move_velocity(0);
    RightDriveSmart.move_velocity(0);
}

void autonomous() {
    //rotate high stakes automatically
    HighStakes.move_relative(-200, 50);

    ToggleFlag();
    Inertial.reset();
    pros::delay(2000); // Allow time for reset

    
    //turn to correct angle
    TurnDegrees(Inertial, clockwise, 15);
    
    //left drivetrain and right drivetrain move velocity -200, driving backward
    LeftDriveSmart.move_velocity(23);
    RightDriveSmart.move_velocity(23);
    pros::delay(5000);
    LeftDriveSmart.move_velocity(0);
    RightDriveSmart.move_velocity(0);

    ToggleClamp();
    ToggleIntake();

    TurnDegrees(Inertial, Direction::clockwise, 70);

    LeftDriveSmart.move_velocity(-50);
    RightDriveSmart.move_velocity(-50);
    pros::delay(1500);
    LeftDriveSmart.move_velocity(0);
    RightDriveSmart.move_velocity(0);

    TurnDegrees(Inertial, Direction::clockwise, 120);

    LeftDriveSmart.move_velocity(-50);
    RightDriveSmart.move_velocity(-50);

    ToggleIntake();
    //toggleclamp and then toggle intake
    //turn degrees, clockwise around 90
    //left drivetrain and right drivetrain move velocity 50, driving forward
    //turn degrees, clockwise around 150
    //left drivetrain and right drivetrain move velocity 100, driving forward to touch ladder
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
    pros::Controller Controller1(pros::E_CONTROLLER_MASTER);

    while(true) {
        // Calculate drivetrain motor velocities
        // Left joystick (up/down) for forward/backward (Axis3)
        // Right joystick (left/right) for turning (Axis1)
        int turn = Controller1.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X); //  Turning
        int forward = Controller1.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);  // Forward/backward

        // Compute motor speeds for arcade drive
        int drivetrainLeftSideSpeed = -(forward + turn); // Left motor speed
        int drivetrainRightSideSpeed = -(forward - turn);  // Right motor speed

        // Deadband logic to prevent small joystick movements from moving the robot
        const int deadband = 25; // Threshold for joystick input
        if (abs(drivetrainLeftSideSpeed) < deadband) {
            drivetrainLeftSideSpeed = 0;
        }
        if (abs(drivetrainRightSideSpeed) < deadband) {
            drivetrainRightSideSpeed = 0;
        }

        // Set motor velocities
        LeftDriveSmart.move_velocity((drivetrainLeftSideSpeed * 2));  // Adjust scaling as needed
        RightDriveSmart.move_velocity((drivetrainRightSideSpeed * 2));

        // Control Clamp and Flag using buttons
        if (Controller1.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)) {
            ToggleClamp();
        }
        if (Controller1.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
            ToggleFlag();
        }
        pros::screen::print(TEXT_SMALL, 3, "State: %3d", intakeState);
        // Control Intake using shoulder buttons (L1/L2)
        if (Controller1.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1)) {
            ToggleIntake();
        }  
        if (Controller1.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) {
            while (Controller1.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
                Intake.move_velocity(-200);
            }
            intakeState = !intakeState;
            Intake.move_velocity(0);
        }  
   

        
        if (Controller1.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            HighStakes.move_velocity(-100);  // Spin intake forward
        } else if (Controller1.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            HighStakes.move_velocity(100); // Spin intake backward
        } else {
            HighStakes.move_velocity(0);    // Stop intake
        }

        // Delay to prevent CPU overload
        pros::delay(20);
    }
    }
