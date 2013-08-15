// Maze simulator object
function Maze(maze, startRow, startCol) {
	this.maze = _.flatten(maze);
	this.startRow = startRow;
	this.startCol = startCol;

	this.length = maze.length;
	this.area = this.maze.length;
}

_.extend(Maze.prototype, {
	// String helper
	toString: function() {
		return this.maze.toString();
	},

	// _.each() helper
	each: function(fn, context) {
		return _.each(this.maze, fn, context);
	},

	// Get a particular cell value
	get: function(rowCol) {
		return this.maze[ this.rowColtoZ(rowCol) ];
	},

	// X/Y coordiante (un)linearizing functions
	rowColtoZ: function(rowCol) {
		return this.length*rowCol[0]+rowCol[1];
	},
	zToRowCol: function(z) {
		return [ Math.floor(z / this.length), z % this.length ];
	},

	// Similar to above, but calls relativify first
	rowColtoZRelative: function(rowCol) {
		return this.relativify(this.rowColtoZ(rowCol));
	},
	zToRowColRelative: function(z) {
		return this.relativify([ Math.floor(z / this.length), z % this.length ]);
	},

	// Relativeify converter
	relativify: function(rowCol) {
		return [ Math.abs(this.startRow - rowCol[0]), Math.abs(this.startCol - rowCol[1]) ];
	}
});
