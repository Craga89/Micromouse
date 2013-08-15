function Solver(maze) {
	_.bindAll(this, 'process');

	// Store maze
	this.maze = maze;

	// Wall storage
	this.wallsHorizontal = [];
	this.wallsVertical = [];

	// Coordinates and direction
	this.lastRow = 0;
	this.lastCol = 0;
	this.curCol = 0;
	this.curRow = 0;
	this.direction = 0;
	this.corner = null;
	this.lastAction = 0;
		
	// Direction strings
	this.dirStrings = [ 'north', 'east', 'south', 'west' ];

	// Setup floodfill array and floodfill it
	this.floodfillArr = new Array(maze.area);
	this.floodfillMax = 0;
	this.floodfill();

	// Insantiate visualizer
	this.visual = new Visualize(maze, this);

	// Ense initial walls
	this.sense();

	// Update the visualisation
	this.visual.move(this.curRow, this.curCol, this);
}

_.extend(Solver.prototype, {
	// Start solving the maze autonomously!
	start: function(speed) {
		var _this = this;

		// Store speed
		this.speed = speed;

		// Start timer
		(function timer() {
			!_this.inCenter && _this.process();

			clearTimeout(_this._timer);
			_this._timer = setTimeout(timer, _this.speed);
		}());
	},

	// Set the speed of the mouse and reset timer
	setSpeed: function(speed) {
		this.speed = speed;

		clearTimeout(this._timer);
		this._timer = setTimeout(_.bind(this.start, this, speed), speed);
	},

	// Simulated sensor retrieval - uses direction
	// calculated in move()
	sense: function() {
		var forward = this.adjacentWall(this.curRow, this.curCol, this.direction),
			abs = this.maze.relativify(forward),
			value = this.maze.get(abs),
			values;

		values = [
			0 + !!(value & 1), // Top
			0 + !!(value & 2), // Right
			0 + !!(value & 4), // Bottom
			0 + !!(value & 8) // Left
		];

		if(this.corner == undefined && (!values[1] || !values[3])) {
			this.corner = !values[1] ? 3 : 1;
			this.visual.think('DETECT A SIDE OPENING... STARTED AT CORNER ' + this.corner);

			_.each(_.range(0, this.curRow + 1), function(i) {
				this.setWall(i, this.curCol, 1, true);
				this.setWall(i, this.curCol, 3, true);
			}, this);
		}

		// Store the values in memory
		if(this.corner != undefined) {
			_.each(values, function(state, dir) {
				this.setWall(forward[0], forward[1], dir, state)
			}, this);


			this.visual.think('I can go ' + _.compact(_.map( _.range(4) , function(i) {
				return this.existWall(forward[0], forward[1], i, true) ? null : this.dirStrings[i];
			}, this)).join(' or ') + ' there');
		}

		// Floodfill
		this.floodfill();

		return values;
	},

	// Sets the new coordinates of the mouse, keeping track of its
	// previous location in the maze
	move: function(row, col) {
		// Store the current values and set new pos
		this.lastCol = this.curCol;
		this.lastRow = this.curRow;
		this.curCol = col;
		this.curRow = row;

		// Move the visualisation
		this.visual.move(row, col);

		// Set last action
		this.lastAction = 1;
	},

	// Determines and stores the new direction of the mouse
	// based on direction and number of turns
	rotate: function(dir, turns) {
		this.direction = Math.abs((this.direction + (dir === 1 ? -1 : 1) * turns) % 4);
		this.visual.think('ROTATING ' + (dir === 1 ? 'LEFT' : 'RIGHT') + ' ' + turns + ' turns (new direction: ' + this.direction + ')');

		this.visual.rotate(dir, turns);

		this.lastAction = 2;
	},

	// Celebration when getting to the middle of the maze. Turn around continuously!
	celebrate: function() {
		var _this = this, i = 1;

		// Set flag
		this.inCenter = true;

		// Start timer
		(function timer() {
			_this.inCenter && _this.visual.rotate(3, 1);

			clearTimeout(_this._celebrateTimer);
			_this._celebrateTimer = setTimeout(timer, 250);
		}());
	},

	// Returns a wall index
	_wallIndex: function(row, col, dir) {
		// Up or down
		if(dir === 0 || dir === 2) {
			var whichRow = dir === 0 ? row : row - 1,
				index = this.maze.length * whichRow + col;
		}

		// Right or left
		else if(dir === 1 || dir === 3) {
			var whichCol = this.corner === 3 ?
					(dir === 1 ? col : col - 1) : 
					(dir === 3 ? col : col - 1)
				index = this.maze.length * whichCol + row;
		}

		return index;
	},

	// Check if a wall exists in a particular direction relative to a cell
	existWall: function(row, col, dir) {
		if(this.corner == undefined) { return false; }

		var arr = this[ dir === 0 || dir === 2 ? 'wallsHorizontal' : 'wallsVertical' ],
			area = this.maze.length * (this.maze.length - 1) - 1,
			index = this._wallIndex(row, col, dir);

		return index < 0 || index > area ? true : !!arr[index];
	},

	// Set the precense of a wall in a particular direction relative to a cell
	setWall: function(row, col, dir, state) {
		if(this.corner == undefined) { return; }

		var arr = this[ dir === 0 || dir === 2 ? 'wallsHorizontal' : 'wallsVertical' ],
			area = this.maze.length * (this.maze.length - 1) - 1,
			index = this._wallIndex(row, col, dir);

		if(index < 0 || index > area) { return; }

		// Set the value
		arr[index]  = !!state;
	},

	// Return a row/col array of a cell in a particular direction to a cell
	adjacentWall: function(row, col, dir) {
		var subject = this.corner === 3 ? 1 : -1;

		switch(dir) {
			case 0: return [row + 1, col]; break;
			case 1: return [row, col + subject]; break;
			case 2: return [row - 1, col]; break;
			case 3: return [row, col - subject]; break;
		}
	},

	// Process and perform the next move
	process: function() {
		this.visual.think('============= PROCESSING NEXT MOVE... =============');

		var nextRow, nextCol, nextDir,
			nextVal = this.maze.area;

		if(this.floodfillValue(this.curRow, this.curCol) === 0) {
			this.celebrate();
			this.visual.think('FOUND THE CENTER OF THE MAZE! CELEBRATION TIME!'); return;
		}

		// If last movement was a rotation... sense first
		if(this.lastAction === 2 || this.corner == undefined) {
			this.sense();
		}

		// If no this.corner
		if(!this.corner) {
			this.visual.think('CORNER NOT KNOWN YET... MOVE FORWARD UNTIL WE CAN DEDUCE');
			nextRow = this.curRow + 1; nextCol = this.curCol;
			this.move(nextRow, nextCol);
			return;
		}

		// Determine the next square to go to using floodfill values
		_.each( _.range(4) , function(dir) {
			var hasWall = this.existWall(this.curRow, this.curCol, dir),
				adjacent = this.adjacentWall(this.curRow, this.curCol, dir);
				val = this.floodfillValue(adjacent[0], adjacent[1]);

			if(!hasWall && (val === nextVal && dir === this.direction || val < nextVal)) {
				nextRow = adjacent[0];
				nextCol = adjacent[1];
				nextVal = val;
			}
		}, this);
		this.visual.think('DECISION: Moving to', nextRow, nextCol, 'with value', nextVal);

		// Determine new direction of travel based on next row/col
		var newDirection = this.curCol === nextCol ? ( this.curRow < nextRow ? 0 : 2 ) : 
			this.corner === 3 ? ( this.curCol < nextCol ? 1 : 3 ) : ( this.curCol < nextCol ? 3 : 1 );
		
		// Rotate if needed
		if(newDirection !== this.direction) {
			this.rotate( this.direction - newDirection > 0 ? 1 : 3, Math.abs(this.direction - newDirection) );
		}

		else {
			// Sense the surrounding area and store results
			var sensors = this.sense();

			// Not a dead end? Move to the new cell
			if(!this.floodfillValue(nextRow, nextCol) || _.reduce(sensors, function(memo, num){ return memo + num; }, 0) < 3) {
				this.move(nextRow, nextCol);
			}

			// If this is a dead end (has walls on more than 2 sides)... re-evaluate
			else {
				this.visual.think('DEAD END. Reprocessing.');
				this.process();
			}
		}
	},

	// Floodfill algorithm (Queue based approach)
	floodfill: function() {
		var queue, x, z, rowCol, val, curVal, hasWall, dir;

		// Reset floodfill array
		this.floodfillArr = [];

		// Create new queue
		queue = [
			this.maze.rowColtoZ([
				Math.floor(this.maze.length / 2) , Math.floor(this.maze.length / 2)
			])
		];

		// Whilst the queue isn't empty
		while(queue.length) {
			// Grab the first value of the queue
			x = queue.shift();

			// Grab Z coordinate and value from value using bitwise
			z = x & 255;
			val = (x >> 8) & 255;

			// Convert Z coordiante into row/col and get current value
			rowCol = this.maze.zToRowCol(z);
			curVal = this.floodfillArr[z];

			// Check we're within the bounds of the maze
			if((curVal != undefined && curVal <= val) || z < 0 || z > this.maze.area) { continue; }

			// Floodfill this cell and store max value (for visuals)
			this.floodfillArr[z] = val;
			if(this.floodfillMax < val) { this.floodfillMax = val; }

			// For every 90 degree direction, floodfill
			for(dir = 0; dir < 4; dir++ ) {
				adjRowCol = this.adjacentWall(rowCol[0], rowCol[1], dir);
				hasWall = this.existWall(rowCol[0], rowCol[1], dir);

				!hasWall && queue.push(
					this.maze.rowColtoZ(adjRowCol) | (val + 1) << 8
				);
			}
		}

	},

	// Return the floodfill value for a particular row/col cell
	floodfillValue: function(row, col) {
		return this.floodfillArr[ this.maze.rowColtoZ([row, col]) ];
	}
});
