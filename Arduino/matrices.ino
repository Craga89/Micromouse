int maze[AREA];

int get(int row, int col) {
	return maze[ rowColtoZ(row, col) ];
}

// X/Y coordiante linearizing methods
int rowColtoZ(int row, int col) {
	return abs(LENGTH * row + col);
}

int colRowtoZ(int row, int col) {
	return abs(LENGTH * col + row);
}

void zToRowCol(int z, int &row, int &col) {
	row = floor(z / LENGTH);
	col = z % LENGTH;
}
