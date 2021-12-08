$.fn.frameset = function(){

	var frameset = this;
	var rows = frameset.children('.row');
	
	//	Add bars
	rows.each(function(y, row){
		$(row).children('.frame').each(function(x, frame){
			if (x > 0){
				$('<div>').addClass('vBar').insertBefore(frame);
			}
		});
		if (y > 0){
			$('<div>').addClass('hBar').appendTo(row);
		}
	});
	
	//	Add mouse events on the bars
	rows.children('.hBar').mousedown(function(e){
		var y = e.pageY;
		var hBar = $(this);
		var row = hBar.parent();
		var rowTop = parseInt(row.css('top'));
		var rowHeight = row.height();
		var frames = row.children('.frame');
		var framesHeight = frames.height();
		var framesHeightMin = frames.children('.title').outerHeight(true);
		var previousRow = row.prev();
		var previousRowHeight = previousRow.height();
		var previousRowFrames = previousRow.children('.frame');
		var previousRowFramesHeight = previousRowFrames.height();
		var overlay = $('<div>')
		 .addClass('overlay').css('cursor', hBar.css('cursor'))
		 .appendTo(frameset);
		overlay.mousemove(function(e){
			var dy = e.pageY - y;
			if (previousRowFramesHeight + dy > framesHeightMin  &&  framesHeight - dy > framesHeightMin){
				previousRow.height(previousRowHeight + dy);
				previousRowFrames.height(previousRowFramesHeight + dy);
				frames.height(framesHeight - dy);
				row.height(rowHeight - dy).css({top : (rowTop + dy) + 'px'});
			}
		});
		overlay.mouseup(function(e){
			//	Remember the horizontal ratios
			var hBars = frameset.find('.hBar');
			var availableHeight = frameset.height() - hBars.length * hBars.outerHeight(true);
			rows.each(function(y){
				var row = $(this);
				var height = row.children('.frame:first').height();
				row.data('frameset-ratio', height / availableHeight);
			});
			//	Do other required stuff
			var callback;
			frames.framesetResizeCallback();
			previousRowFrames.framesetResizeCallback();
			overlay.remove();
		});
		overlay.mouseout(function(e){
			overlay.mouseup();
		});
	});
	rows.children('.vBar').mousedown(function(e){
		var x = e.pageX;
		var vBar = $(this);
		var vBarLeft = parseInt(vBar.css('left'));
		var vBarWidth = vBar.width();
		var previousFrame = vBar.prev();
		var previousFrameWidth = previousFrame.width();
		var frame = vBar.next();
		var frameLeft = parseInt(frame.css('left'));
		var frameWidth = frame.width();
		var overlay = $('<div>')
		 .addClass('overlay').css('cursor', vBar.css('cursor'))
		 .appendTo(frameset);
		overlay.mousemove(function(e){
			var dx = e.pageX - x;
			previousFrame.width(previousFrameWidth + dx);
			vBar.css({left : (vBarLeft + dx) + 'px'});
			frame.width(frameWidth - dx).css({left : (frameLeft + dx) + 'px'});
		});
		overlay.mouseup(function(e){
			//	Remember the horizontal ratios
			var row = frame.parent();
			var vBars = row.find('.vBar');
			row.children('.frame').each
			var availableWidth = frameset.width() - vBars.length * vBars.outerWidth(true);
			row.children('.frame').each(function(y){
				var frame = $(this);
				var width = frame.width();
				frame.data('frameset-ratio', width / availableWidth);
			});
			//	Do other required stuff
			previousFrame.framesetResizeCallback();
			frame.framesetResizeCallback();
			overlay.remove();
		});
		overlay.mouseout(function(e){
			overlay.mouseup();
		});
	});
	
	//	The end!
	return frameset;
};

$.fn.framesetResize = function(){
	var frameset = $(this);
	if (!frameset.hasClass('frameset')){
		return frameset;
	}
	var rows = frameset.children('.row');
	//
	//	Update heights
	//
	var hBars = rows.children('.hBar');
	var hBarHeight = hBars.length ? hBars.height() : 0;
	var availableHeight = frameset.height() - hBars.length * hBarHeight;
	var totalHeight = 0;
	//	Get ratios
	$(rows).each(function(){
		var row = $(this);
		var rowRatio = row.data('frameset-ratio');
		totalHeight += rowRatio ? 0 : row.children('.frame:first').outerHeight(true);
	});		
	var positionTop = 0;
	$(rows).each(function(y){
		var row = $(this);
		var rowRatio = row.data('frameset-ratio');
		var firstFrame = row.children('.frame:first');
		var height =
		 ( rowRatio
		 ? rowRatio
		 : (firstFrame.height() / totalHeight)
		 ) * availableHeight;
		var rowHeight = height + (y ? hBarHeight : 0);
		row.height(rowHeight).css({top : positionTop + 'px'})
		 .children('.frame').height(height - firstFrame.outerHeight(true) + firstFrame.height());
		positionTop += rowHeight;
	});
	//	Update width
	$(rows).each(function(){
		var row = $(this);
		var frames = row.children('.frame');
		var vBars = row.children('.vBar');
		var vBarWidth = vBars.length ? vBars.width() : 0;
		var availableWidth = frameset.width() - vBars.length * vBarWidth;
		var totalWidth = 0;
		frames.each(function(){
			totalWidth += $(this).outerWidth(true);
		});
		var positionLeft = 0;
		frames.each(function(x, frame){
			frame = $(frame);
			var frameRatio = frame.data('frameset-ratio');
			var width = 
			 ( frameRatio
			 ? frameRatio
			 : (frame.width() / totalWidth)
			 ) * availableWidth;
			frame.width(width - frame.outerWidth(true) + frame.width()).css({left : positionLeft + 'px'});
			positionLeft += width;
			if (x < vBars.length){
				var vBar = vBars.eq(x);
				vBar.css({left : positionLeft + 'px'});;
				positionLeft += vBar.width();
			}
		});
	});
	rows.children('.frame').framesetResizeCallback();
	return frameset;
};

$.fn.framesetResizeCallback = function(newCallback){
	var callback;
	this.each(function(f, frame){
		frame = $(frame);
		// console.log(f, frame.attr('class'), frame.data('frameset-resize-callback'));
		if (frame.hasClass('frame')){
			if (newCallback && $.isFunction(newCallback)){
				frame.data('frameset-resize-callback', newCallback);
			} else if ($.isFunction(callback = frame.data('frameset-resize-callback'))){
				callback();
			}
		}
	});
	return this;
};