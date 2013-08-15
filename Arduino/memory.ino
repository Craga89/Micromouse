boolean wallsHorizontal[AREA_WALLS];
boolean wallsVertical[AREA_WALLS] = { true };

void getAdjacentWall(int &row, int &col, int dir) {
	int subject = corner == 3 ? 1 : -1;
		
	switch(dir) {
		case 0: row = row + 1; break;
		case 1: col = col + subject; break;
		case 2: row = row - 1; break;
		case 3: col = col - subject; break;
	}
}

int calculateWallIndex(int row, int col, int dir) {
	int index;

	// Up or down
	if(dir == 0 || dir == 2) {
		index = rowColtoZ(
			dir == 0 ? row : row - 1, col
		);
	}

	// Right or left
	else if(dir == 1 || dir == 3) {
		index = colRowtoZ(
			corner == 3 ? 
				(dir == 1 ? col : col - 1) : 
				(dir == 3 ? col : col - 1),
			row
		);
	}

	return index;
}

boolean wallExists(int row, int col, int dir) {
	if(corner == UNDEFINED) { return false; }

	int index = calculateWallIndex(row, col, dir);
	boolean *arr = dir == 0 || dir == 2 ? wallsHorizontal : wallsVertical;
	
	return (index < 0 || index > AREA_WALLS) ? true : arr[index];
}

void setWall(int row, int col, int dir, boolean state) {
	if(corner == UNDEFINED) { return; }

	int index = calculateWallIndex(row, col, dir);
	boolean *arr = dir == 0 || dir == 2 ? wallsHorizontal : wallsVertical;
	
	// Ensure we don't go outside the walls range
	if(index < 0 || index > AREA_WALLS) { return; }

	// Set the value
	arr[index] = state;
}
