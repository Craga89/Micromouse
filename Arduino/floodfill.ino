#include <QueueList.h>

int floodfillArr[AREA] = {};

void initializeFloodfill() {
	for(int i = 0; i < AREA; i++) {
		floodfillArr[i] = UNDEFINED;
	}
}
		
void floodfill() {
	QueueList <int> queue;
	int i = 0;

	// Clean the array
	initializeFloodfill();
	
	// Push middle of maze onto queue
	queue.push( rowColtoZ( floor(LENGTH / 2), floor(LENGTH / 2) ) );
	
	if(DEBUG_PRINT & 4) { startGroup("FLOODFILL"); }
		
	while(!queue.isEmpty()) {
		int x = queue.pop();
		int z = x & 255;
		
		int row, col;
		zToRowCol( z, row, col );
		
		int val = (x >> 8) & 255;
		int curVal = floodfillArr[ z ];

		// Floodfill this cell
		if((curVal != UNDEFINED && curVal <= val) || z < 0 || z > AREA) { continue; }
		floodfillArr[ z ] = val;

		// Check the cell to the north
		if(!wallExists(row, col, 0)) {
			queue.push( rowColtoZ(row + 1, col) | ((val + 1) << 8) );
		}

		// Check the cel to the east
		if(!wallExists(row, col, 1)) {
			queue.push( rowColtoZ(row, col + 1) | ((val + 1) << 8) );
		}

		// Check the cell to the south
		if(!wallExists(row, col, 2)) {
			queue.push( rowColtoZ(row - 1, col) | ((val + 1) << 8) );
		}

		// Check the cell to the west
		if(!wallExists(row, col, 3)) {
			queue.push( rowColtoZ(row, col - 1) | ((val + 1) << 8) );
		}

		i++;
	}

	if(DEBUG_PRINT & 4) {
		for(int i = 0; i < 9; i++) {
			print( 
				String(floodfillArr[ rowColtoZ(i, 0) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 1) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 2) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 3) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 4) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 5) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 6) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 7) ]) + "\t" + 
				String(floodfillArr[ rowColtoZ(i, 8) ])
			);
		}
		print(String("FLOODFILLED ") + String(i));
		endGroup();
	}
}

int getFloodfillValue(int row, int col) {
	return floodfillArr[ rowColtoZ(row, col) ];
}
