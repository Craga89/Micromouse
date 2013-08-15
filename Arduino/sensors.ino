// Threshold values
#define FRONT_DISABLE_THRESHOLD 800
#define FRONT_WALL_THRESHOLD 270
#define STOP_THRESHOLD 930
#define SIDE_WALL_THRESHOLD 370
#define SIDE_OPENING_THRESHOLD 150

static int left_prev = 0;
static int right_prev = 0;
static boolean use_front = true;
static unsigned long last_sense_time = 0;
static unsigned long last_sense_time2 = 0;

void initializeSensors() {
	pinMode(FRONT_LEFT, INPUT);
	pinMode(FRONT_RIGHT, INPUT);
	pinMode(FRONT, INPUT);
}

void processSensors() {
	return;
	
	unsigned long sense_time = millis();

	if(senseValue(FRONT) > STOP_THRESHOLD && analogRead(FRONT_LEFT) > STOP_THRESHOLD && analogRead(FRONT_RIGHT) > STOP_THRESHOLD && lastAction == ACTION_MOVE) {
		Serial.println("STOP");
		Serial.println( stepper_left.distanceToGo() );
		enabled = false;
		stop(true);
	}
	
	return;

	// Every 20ms (sensor change min)...
	if(sense_time - last_sense_time > 20) {
		// Check to see if we're too close for the front sensor to be reliable
		//use_front = front < FRONT_DISABLE_THRESHOLD;

		// Store the current time as the last sense time
		last_sense_time = sense_time;
	}

	if(sense_time - last_sense_time2 > 700) {
		// Compare the two left/right sensors with their previous values
		int left_cur = analogRead(FRONT_RIGHT);
		int right_cur = analogRead(FRONT_LEFT);
		int left_diff = abs(left_prev - left_cur);
		int right_diff = abs(right_prev - right_cur);
		
		// If there's a big drop in either side sensor, we've gone past a wall... calibrate
		if(left_prev + right_prev > 0 && (left_diff > SIDE_OPENING_THRESHOLD || right_diff> SIDE_OPENING_THRESHOLD)) {
			//print("WALL APPEARED ON ONE OF THE SIDES!!!!");
			//enabled = false;
		}

		// Store the current side sensor values
		left_prev = left_cur;
		right_prev = right_cur;
		
		last_sense_time2 = sense_time;
	}
}

void sense(boolean &north, boolean &east, boolean &south, boolean &west) {
	int row = curRow, col = curCol;
	getAdjacentWall(row, col, direction);
				
	boolean sensors[] = {
		senseWall(FRONT),
		senseWall(FRONT_LEFT),
		false,
		senseWall(FRONT_RIGHT)
	};

	// Determine absolute wall values
	int shift = turns % 4;
	if(turns > 0) { shift = 4 - shift; }
	north = sensors[ (0 + shift) % 4 ];
	east = sensors[ (1 + shift) % 4 ];
	south = sensors[ (2 + shift) % 4 ];
	west = sensors[ (3 + shift) % 4 ];

	if(corner == UNDEFINED && (!east || !west)) {
		corner = !east ? 3 : 1;
		print("WE ARE IN THE " + String(corner == 1 ? "RIGHT" : "LEFT") + " CORNER!");
		
		// Store side walls for all cells we've visisted before corner detection
		for(int i = 0; i < curRow + 1; i++) {
			setWall(i, curCol, 1, true);
			setWall(i, curCol, 3, true);
		}
	}
	
	// If a corner is known... proceed!
	if(corner != UNDEFINED) {
		// Remember any walls that we see
		if(DEBUG & 2) {
			print("STORING WALL RESULTS FOR " + String(row) + "/" + String(col) + "...");
		}
		if(north) { setWall(row, col, 0, true); }
		if(east) { setWall(row, col, 1, true); }
		if(south) { setWall(row, col, 2, true); }
		if(west) { setWall(row, col, 3, true); }
	}
	
	// Debug messages
	if(DEBUG_PRINT & 2) {
		int values[] = {
			senseValue(FRONT),
			senseValue(FRONT_LEFT),
			0,
			senseValue(FRONT_RIGHT)
		};
		
		startGroup("SENSOR DEBUG");
			print("TURNS " + String(turns));
			print( "NORTH INDEX " + String( (0 + shift) % 4 ) );
			print( "EAST INDEX " + String( (1 + shift) % 4 ) );
			print( "SOUTH INDEX " + String( (3 + shift) % 4 ) );
			
			startGroup("RELATIVE");
				print(String(values[0]));
				print(String(values[1]));
				print(String(values[3]));
			endGroup();
							
			startGroup("ABSOLUTE");
				if(north) { print("WALL TO THE NORTH"); }
				if(east) { print("WALL TO THE EAST"); }
				if(south) { print("WALL TO THE SOUTH"); }
				if(west) { print("WALL TO THE WEST"); }
			endGroup();
		endGroup();
	}
					
	// Floodfill
	floodfill();
}

// Detect if there is a wall or no wall
boolean senseWall(int pin) {
	int value = senseValue(pin);
	
	if(pin == FRONT) {
		return value > FRONT_WALL_THRESHOLD; 
	}
	else {
		return value > SIDE_WALL_THRESHOLD;
	}
}

int senseValue(int pin) {
	// Ensure we don't read the front sensor if within a given range, as the readings are inaccurate
	// Instead, we take an average of the two side sensors
	//if(pin == FRONT && !use_front) {
	//	return (readSensorAnalog(FRONT_LEFT) + readSensorAnalog(FRONT_RIGHT)) / 2;
	//}
	
	return readSensorAnalog(pin);
	
	//if Avg side sensor greater than 850 stop!
}

int readSensorAnalog(int pin) {
	int sortedValues[NUM_READS];
	int i, j = 0, k;

	for(i = 0; i < NUM_READS; i++) {
		int value = analogRead(pin);

		if(value < sortedValues[0] || i == 0) {
			j = 0;
		}
		else {
			for(j = 1; j < i; j++) {
				if(sortedValues[j - 1] <= value && sortedValues[j] >= value ) {
					break;
				}
			}
		}

		for(k = i; k > j; k--) {
			sortedValues[k] = sortedValues[k - 1];
		}
		sortedValues[j] = value;
				}

	int returnVal = 0;
	for(i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
		returnVal += sortedValues[i];
	}

	return int(returnVal / 10);
}
