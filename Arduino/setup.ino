/*
	1 = SENSORS
	2 = ROTATE
	3 = FORWARD
*/
#define DEBUG 0

/*
	1 = ENABLED
	2 = SENSORS
	4 = FLOODFILL 
*/
#define DEBUG_PRINT 0 //3

// Define program constants
#define UNDEFINED -10
#define LENGTH 9
#define AREA 81
#define AREA_WALLS 9 * (9 - 1) - 1

// Action constants
#define ACTION_MOVE 1
#define ACTION_ROTATE 2

// Motor specific
#define MODE_TYPE 8
#define REVS_FULL 1600 // 1600 steps in one full wheel revolution
#define REVS_SQUARE 1665 // No of steps to move a square (~17.5 cm) (1641) (1684) (18cm) (1664)
//With 1684 moving ahead by 1.5cm 

// 2756 seems to give perfect turns
#define STEPS_360 2757 // No of steps to rotate 360 degrees (2804) (2800) (2739)
#define DEGREE_STEPS STEPS_360 / float(360.0) // How many steps per degree

// Sensor specific
#define NUM_READS 50
#define FRONT A2
#define FRONT_LEFT A0
#define FRONT_RIGHT A1

// Digital inputs
#define SWITCH 13

// Digital outputs
#define DIR_L 39		// Grey
#define STEP_L 9		// Yellow (marked)
#define MS1_L 43		// Blue
#define MS2_L 45		// Green
#define SLEEP_L 47		// White 
#define RESET_L 49		// Brown
#define ENABLE_L 51	        // Orange
#define PFD_L 53		// Purple

//2nd Stepper Motor 
#define DIR_R 38		// Grey
#define STEP_R 8		// Yellow
#define MS1_R 42		// Blue
#define MS2_R 44		// Green
#define SLEEP_R 46		// White 
#define RESET_R 48		// Brown
#define ENABLE_R 50		// Orange
#define PFD_R 52		// Purple

// Memory (position, direction etc)
int lastCol = 0;
int lastRow = 0;
int curCol = 0;
int curRow = 0;
int corner = UNDEFINED;

int turns = 0;
int direction = 0;
int lastDirection = 0;

int lastAction = 0;
float speed = 980.0;

void setup() {
	// Setup serial out
	Serial.begin(9600);

	// Initialise pin modes
	pinMode(SWITCH, INPUT);

	// Initialize motors
	initializeFloodfill();
	initializeSensors();
	initializeMotors();

	// Sense  walls in north straight away
	boolean north, east, south, west;
	sense(north, east, south, west);
	
	// Attach start button to interrupt handler
	attachInterrupt(SWITCH, toggle, CHANGE);
}

// Interrupt handler - Turns on/off motor drivers when switch is changed
static boolean enabled = false;
static unsigned long last_enable_time = 0;

void toggle() {
	unsigned long enable_time = millis();
	
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if(enable_time - last_enable_time > 200) {
		enabled = digitalRead(SWITCH);
		print("ENABLED? " + String(enabled ? "YES" : "NO"));
		sleepDrivers( !enabled );
		//delay(1000);
	}
	
	last_enable_time = enable_time;
}

void loop() {
	// Move the motors and if currently moving... return
	if(!enabled || process()) { return; }

	if(DEBUG) {
		switch(DEBUG) {
					case 1: debug_sensors(); break;
					case 2: turn(); break;
					case 3: forward(); break;
		}
		
		return;
	}        

	int nextVal = AREA, nextRow, nextCol;
	boolean north, east, south, west; 
	
	// If we'eve reached the center of the maze...
	if(getFloodfillValue(curRow, curCol) == 0) {
		enabled = false; return;
	}

	// If we just rotated... sense first to update floodfill
	if(lastAction == 2 || corner == UNDEFINED) {
		sense(north, east, south, west);
	}
	
	if(corner == UNDEFINED) {
		print("NO CORNER KNOWN.... MOVE FORWARD FOR NOW");
		nextRow = curRow + 1; nextCol = curCol;
		moveTo(nextRow, nextCol);
		return;
	}
	
	// Start group
	startGroup("PROCESSING NEXT MOVE");
	
	// Look al all 4 sides to the current cell and determine the lowest floodfill value...
	for(int dir = 0; dir < 4; dir++) {
		boolean hasWall = wallExists(curRow, curCol, dir);

		int row = curRow, col = curCol;
		getAdjacentWall(row, col, dir);

		int val = getFloodfillValue(row, col);

		if(!hasWall && (val == nextVal && dir == direction || val < nextVal)) {
			nextRow = row;
			NextCol = col;
			nextVal = val;
		}
	}
	print(
		"MOVING TO (" + String(nextRow) + "/" + String(nextCol) + ") WITH VALUE " + nextVal +
		" FROM (" + String(curRow) + "/" + String(curCol) + ")"
	);
	
	// Determine the new direction
	int newDirection = curCol == nextCol ? ( curRow < nextRow ? 0 : 2 ) : 
		corner == 3 ? ( curCol < nextCol ? 1 : 3 ) : ( curCol < nextCol ? 3 : 1 );

	// If we need to rotate... go!
	if(newDirection != direction) {
		startGroup("ROTATION");
		print("NEW DIRECTION IS " + String(newDirection));

		// Keep track of the number of turns and set direction
		turns -= direction - newDirection;
		rotate(abs(direction - newDirection) * 90, direction - newDirection > 0);
		direction = newDirection;
		
		endGroup();
	}
	
	// No rotation? Sense and move
	else {
		// Sense the forward blocks surrounding area using sensors
		sense(north, east, south, west);

		// Check for a dead end (but make sure it isn't the center!)
		if(getFloodfillValue(nextRow, nextCol) < 1 || (north + east + south + west) < 3) {
			moveTo(nextRow, nextCol);
		}
		else {
			print("DEAD END. DON'T MOVE.");
		}
	}
	
	endGroup();
}

boolean process() {
	// Continually process sensor values
	processSensors();
	
	// Process motors
	return processMotors();
}
