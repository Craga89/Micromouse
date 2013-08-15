#include <StackList.h>

/*
	 PRINT HELPERS
*/
String _printPrefix = "";
StackList <String> _printGroups;

void print(String output) {
	if(DEBUG_PRINT > 0 || DEBUG > 0) { 
		Serial.print(_printPrefix + output + (true ? "\n" : ""));
	}
}

void clearScreen() {
	for(int i = 0; i < 90; i++) { Serial.println(); }
}

void startGroup(String name) {
	// Add tab indentation and output name      
	print("============= " + name + " ===============");
	_printPrefix += "  ";
	_printGroups.push( name );
}

void endGroup() {
	_printPrefix = _printPrefix.substring(0, _printPrefix.length() - 2);
	print("============= END " + _printGroups.pop() + " =============");
	print("");
}
/*
	END PRINT HELPERS
*/

void debug_sensors() {
	int pins[] = { FRONT, FRONT_LEFT, FRONT_RIGHT };

	// Check Sensors
	boolean north, east, south, west;
	sense(north, east, south, west);
	
	startGroup("Checking Sensors...");
	print( String("NORTH: ") + (north ? String("WALL ") : String("NO WALL ")) + String(senseValue(FRONT)) );
	print( String("EAST: ") + (east ? String("WALL ") : String("NO WALL ")) + String(senseValue(FRONT_LEFT)) );
	print( String("WEST: ") + (west ? String("WALL ") : String("NO WALL ")) + String(senseValue(FRONT_RIGHT)) );
	print( String(senseValue(FRONT_LEFT) - senseValue(FRONT_RIGHT)) );
	print( String(senseValue(FRONT)) + " " + String(senseValue(FRONT_RIGHT)) + " " + String(senseValue(FRONT_LEFT)));
	endGroup();

	delay(1000);
	clearScreen();
}

void turn() {
	float deg = 90;
	delay(2000);
	
	print("Turning Left 360 degrees...");
	rotate(deg, true); // Left
}

void forward(){
	delay(3000);
	if(enabled) { move(REVS_SQUARE, true); }
}

