

function Visualize(maze, solver) {
	// Store maze
	this.maze = maze;
	this.solver = solver;

	// Size of individual cells
	this.cellSize = 40;

	// Setup element references
	this.elems = {
		maze: $('#actual').css('width', (this.cellSize + 4) * this.maze.length),
		known: $('#known').css('width', (this.cellSize + 4) * this.maze.length),
		floodfill: $('#floodfill').css('width', (this.cellSize + 4) * this.maze.length),
		thoughts: $('#thoughts')
	};

	// Render the grid
	this.render();
}

_.extend(Visualize.prototype, {
	render: function() {
		// Create the maze interface
		this.maze.each(function(value, v) {
			var classes = [
				'block',
				(value & 8) && 'wall-3' || '',
				(value & 4) && 'wall-2' || '',
				(value & 2) && 'wall-1' || '',
				(value & 1) && 'wall-0' || ''
			].join(' ') + ' blank';

			// Add to absolute maze
			$('<div/>', {
				'class': classes,
				'css': { lineHeight: this.cellSize+'px' },
				'width': this.cellSize,
				'height': this.cellSize,
				'text': this.maze.zToRowCol(v),
				'title': this.maze.zToRowCol(v) + ' (' + v + ')'
			})
			.appendTo(this.elems.maze)

			// Add known maze
			.clone().attr({
				'class': 'block blank',
				'title': this.maze.zToRowColRelative(v) + ' (' + this.maze.rowColtoZ( this.maze.zToRowColRelative(v) ) + ')'
			})
			.text( this.maze.zToRowColRelative(v))
			.appendTo(this.elems.known)

			// Add floodfill maze
			.clone().attr({
				'class': 'block blank',
				'title': this.maze.relativify(this.maze.zToRowCol(v)) + ' (' + this.maze.rowColtoZ(this.maze.relativify(this.maze.zToRowCol(v))) + ')'
			})
			.html( this.solver.floodfillArr[ v ] || '&nbsp;' )
			.appendTo(this.elems.floodfill);

		}, this);

		// Create mouse element(s)
		$('<div/>', { 'class': 'mouse' })
		.appendTo( this.elems.maze )
		.clone().appendTo( this.elems.floodfill )
		.clone().appendTo( this.elems.known );
		this.elems.mouse = $('.maze .mouse');
	},

	rotate: function(dir, turns) {
		var z = this.maze.rowColtoZ( this.maze.relativify([this.lastRow, this.lastCol]) ),
			current = $([
				$('.block', this.elems.known).removeClass('current').eq( z ).addClass('current')[0],
				$('.block', this.elems.maze).removeClass('current').eq( z ).addClass('current')[0],
				$('.block', this.elems.floodfill).removeClass('current').eq( z ).addClass('current')[0],
			]),
			degrees;

		// Calculate degree turns needed (turn 270 turns into -90 degree turns too, its quicker after all!)
		this._degrees = (this._degrees || 0) + (dir === 3 ? 1 : -1) * (turns % 3 === 0 ? -90 : turns * 90);

		// Update transform angle
		this.elems.mouse.css({
			'-webkit-transform': 'rotate(' + this._degrees + 'deg)',
			'-moz-transform': 'rotate(' + this._degrees + 'deg)',
			'-ms-transform': 'rotate(' + this._degrees + 'deg)',
			'-o-transform': 'rotate(' + this._degrees + 'deg)',
			'transform': 'rotate(' + this._degrees + 'deg)',
		}, 200);

		// Redraw maze
		this.redraw();
	},

	move: function(row, col) {
		// Highlight the current square visually
		var z = this.maze.rowColtoZ(this.maze.relativify([row, col])),
			current = $([
				$('.block', this.elems.known)[z],
				$('.block', this.elems.maze)[z],
				$('.block', this.elems.floodfill)[z],
			]),
			nextStack = [];

		// Update transform angle
		this.elems.mouse.css( current.position() )
			.removeClass('top right bottom left').addClass(
				col > this.lastCol ? 'right' : col < this.lastCol ? 'left' :
				row > this.lastRow ? 'top' : row < this.lastRow ? 'bottom' : ''
			);

		// Output current relative and absolute position
		this.elems.maze.prev().children('span').text( this.maze.relativify([row, col]).join(',')  );
		this.elems.known.prev().children('span').text( [row,col].join(',') );

		// Redraw maze
		this.redraw();

		// Store it
		this.lastRow = row; 
		this.lastCol = col;
	},

	_floodfillColor: function(value) {
		var val = Math.round(255 * value / this.solver.floodfillMax);
		return 'rgb(' + [0,255-val,0].join(',') + ')';
	},

	redraw: function() {
		// Update the current and neighbouring cell walls
		_.each(_.range(this.maze.area), function(z) {
			var abs = this.maze.zToRowCol(z),
				rowCol = this.maze.relativify(abs),
				absZ = this.maze.rowColtoZ(rowCol),
				known = $('.block', this.elems.known).eq( absZ ),
				flood = $('.block', this.elems.floodfill).eq( absZ )

			// Update known wall classes
			known.add(flood).addClass(
				_.map( _.range(4), function(i) {
					return this.solver.existWall(abs[0], abs[1], i) && 'wall-'+i || '';
				}, this)
				.join(' ')
			);

			// Update floodfill values and colours
			var val = this.solver.floodfillValue.apply(this.solver, abs);
			flood.attr('data-val', val || '0').html(val || '0').css('color', this._floodfillColor(val));

		}, this);
	},

	think: function() {
		console.log.apply(console, arguments);
	}
});
