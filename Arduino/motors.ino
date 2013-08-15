#include <AccelStepper.h>

AccelStepper stepper_left(AccelStepper::DRIVER, STEP_L, DIR_L);
AccelStepper stepper_right(AccelStepper::DRIVER, STEP_R, DIR_R);

int pos = 3600;
boolean disabled = true;

void initializeMotors() {
	// Set pin modes
	pinMode(DIR_L, OUTPUT);
	pinMode(STEP_L, OUTPUT);
	pinMode(MS1_L, OUTPUT);
	pinMode(MS2_L, OUTPUT);
	pinMode(SLEEP_L, OUTPUT);
	pinMode(RESET_L, OUTPUT);
	pinMode(ENABLE_L, OUTPUT);
	pinMode(PFD_L, OUTPUT);
	pinMode(DIR_R, OUTPUT);
	pinMode(STEP_R, OUTPUT);
	pinMode(MS1_R, OUTPUT);
	pinMode(MS2_R, OUTPUT);
	pinMode(SLEEP_R, OUTPUT);
	pinMode(RESET_R, OUTPUT);
	pinMode(ENABLE_R, OUTPUT);
	pinMode(PFD_R, OUTPUT);
		   
	// Set Mode type to 8 (1,1 both pins)
	digitalWrite(MS1_L, HIGH);
	digitalWrite(MS2_L, HIGH);
	digitalWrite(MS1_R, HIGH);
	digitalWrite(MS2_R, HIGH);

	// Setup other pins
	digitalWrite(SLEEP_L, HIGH);
	digitalWrite(RESET_L, HIGH);
	digitalWrite(PFD_L, HIGH);
	digitalWrite(SLEEP_R, HIGH);
	digitalWrite(RESET_R, HIGH);
	digitalWrite(PFD_R, HIGH);

	// Set forward direction
	setWheelDirection(0);
	disableMotors(false);
	sleepDrivers(false);
	
	// Set max speed
	stepper_left.setSpeed(700.0);
	stepper_left.setMaxSpeed(1000.0);
	stepper_right.setSpeed(700.0);
	stepper_right.setMaxSpeed(1000.0);
}

boolean processMotors() {
	int left_distance = stepper_left.distanceToGo();
	int right_distance = stepper_right.distanceToGo();

	if(left_distance >= 0) { stepper_left.runSpeed(); }
	if(right_distance >= 0) { stepper_right.runSpeed(); }

	return left_distance > 0 || right_distance > 0;
}

void setWheelDirection(int dir) {
	switch(dir) {
		// Foward
		case 0:
			stepper_left.setPinsInverted(true, false);
			stepper_right.setPinsInverted(false, false);
		break;
				
		// Left
		case 1:
			stepper_left.setPinsInverted(true, false);
			stepper_right.setPinsInverted(true, false);
		break;
		
		// Back
		case 2:
			stepper_left.setPinsInverted(false, false);
			stepper_right.setPinsInverted(true, false);
		break;
		
		// Right
		case 3:
			stepper_left.setPinsInverted(false, false);
			stepper_right.setPinsInverted(false, false);
		break;
	}
}

// Turn on/off the motor drivers
void sleepDrivers(boolean sleep) {
	print(sleep ? "SLEEP" : "AWAKE");
	digitalWrite(SLEEP_L, sleep ? LOW : HIGH);
	digitalWrite(SLEEP_R, sleep ? LOW : HIGH);
}

// Enable or disable motors
void disableMotors(boolean disabledState) {
	if(disabledState != disabled) {
		print( disabledState ? "DISABLED" : "ENABLED" );
		digitalWrite(ENABLE_L, disabledState ? HIGH : LOW);
		digitalWrite(ENABLE_R, disabledState ? HIGH : LOW);
		disabled = disabledState;
	}
}

int setValue = 50;

// Generic PWM output
void pwm(int steps, float speed) {
	stepper_left.move( steps );
	stepper_right.move( steps );
	stepper_left.setSpeed( speed );
	stepper_right.setSpeed( speed );
}

// Move forward or backward a certain number of steps
void move(int steps, boolean foward){
	setWheelDirection(forward ? 0 : 2);
	pwm(steps, speed);
	
	lastAction = ACTION_MOVE;
}  

// Rotate left or right a certain number of degrees
void rotate(int deg, boolean left) {
	// If turning 270 degrees, just go 90 the other direction
	if(deg % 270 == 0) {
		left = !left; deg = 90;
	}
	setWheelDirection( left ? 1 : 3 );
	
	print("ROTATING " + String(deg) + " degrees to the " + String(left ? "left" : "right"));

	// Calculate number of steps and asyncronously send PWM signals
	float fract = float(360.0) / float(deg);
	int steps = int(STEPS_360 / fract);
	pwm( steps, speed );
	
	// Set last action to rotation
	lastAction = ACTION_ROTATE;
}

// Stop the motors
void stop(boolean reset) {
	stepper_left.stop();
	stepper_right.stop();
	enabled = true;
	
	// Also reset the current position if needed
	if(reset) {
		stepper_left.setCurrentPosition(0.0);
		stepper_right.setCurrentPosition(0.0);
	}
}

// Move it - NEEDS EDITING
void moveTo(int row, int col) {
	// Store the current values and set new pos
	lastCol = curCol;
	lastRow = curRow;
	curCol = col;
	curRow = row;

	move(REVS_SQUARE, true);
}
