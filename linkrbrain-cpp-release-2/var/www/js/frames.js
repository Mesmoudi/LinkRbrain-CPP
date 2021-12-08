$('.frameset').resize(function(){

	var frameset = $(this);
	frameset.find('.row, .vBar, .hBar, .frame').css({position:'absolute'});
	var rows = frameset.find('.row');
	
	//	Resize the whole thing, depending on the screen
	var framesetWidth = frameset.outerWidth();
	var framesetHeight = frameset.outerHeight();
	var fontSize = Math.sqrt(framesetWidth * framesetHeight) / 80;
	$('<style type="text/css">').text('.frameset{font-size:' + fontSize + 'px}').appendTo('head');
	
	//	Determine frameset initial height
	var frameHeights = [];
	var frameHeightSum = 0;
	rows.each(function(y, row){	
		var rowHeight = 0;
		$(row).find('.frame').each(function(){
			var frame = $(this);
			rowHeight = Math.max(rowHeight, frame.outerHeight());
		});
		frameHeights.push(rowHeight);
		frameHeightSum += rowHeight;
	});
	
	//	Add bars & resize frames
	var top = 0;
	var yMax = rows.length;
	rows.each(function(y, row){
		var hBar = null;
		var hBarHeight = 0;
		var frameHeight = $(row).height();
		$(row).css({top:top+'px'});
		//	Horizontal separation bar
		if (y < yMax - 1){
			hBar = $('<div>')
				.appendTo(row)
				.addClass('hBar')
				.css({bottom:0});
			hBarHeight = hBar.outerHeight(true);
			// frameHeight -= hBarHeight;
			// hBar.width(framesetWidth);
		}
		//	Frame widths
		var frames = $(row).find('.frame');
		var frameWidths = [];
		var frameWidthSum = 0;
		$(row).find('.frame').each(function(){
			var frame = $(this);
			var frameWidth = frame.width();
			frameWidthSum += frameWidth;
			frameWidths.push(frameWidth);
		});
		//	Columns within the row
		var left = 0;
		var xMax = frames.length;
		frames.each(function(x){
			var frame = $(this);
			var frameWidth = framesetWidth / xMax;
			//	Vertical separation bar
			var vBar = null;
			if (x < xMax - 1){
				vBar = $('<div>')
					.insertAfter(frame)
					.addClass('vBar')
				vBarWidth = vBar.width();
				frameWidth -= vBarWidth;
			}
			//	The frame, finally!
			frame
				.width(frameWidth - frame.outerWidth() + frame.width())
				.height(frameHeight + frame.height() - frame.outerHeight())
				.data({x:x, y:y})
				.css({left:left+'px'});
			//	Left position incrementation			
			left += frameWidth;
			if (vBar){
				vBar.height(frameHeight);
				vBar.css({left:left+'px'});
				left += vBarWidth;
			}
		});
		//	Top position incrementation
		top += frameHeight + hBarHeight;
	});
	

	//	Horizontal resizing!
	frameset.find('.vBar').mousedown(function(e){
		var vBar = $(this);
		var vBarLeft = parseInt(vBar.css('left'));
		var x = e.pageX;
		//	Previous frame
		var prev = vBar.prev();
		var prevWidth = parseInt(prev.css('width'));
		//	Next frame
		var next = vBar.next();
		var nextLeft = parseInt(next.css('left'));
		var nextWidth = parseInt(next.css('width'));
		//	Events
		var overlay = $('<div>')
			.addClass('overlay')
			.css('cursor', vBar.css('cursor'))
			.appendTo(frameset);
		overlay.mousemove(function(e){
			var dx = e.pageX - x;
			vBar.css('left', (vBarLeft + dx) + 'px');
			// prev.width(prevWidth + dx).resize();
			prev.width(prevWidth + dx);
			next.css('left', (nextLeft + dx) + 'px');
			// next.width(nextWidth - dx).resize();
			next.width(nextWidth - dx);
		}).mouseup(function(){
			prev.resize();
			next.resize();
			overlay.remove();
		}).mouseout(function(){
			prev.resize();
			next.resize();
			overlay.remove();
		});		
	});
	

	//	Vertical resizing!
	frameset.find('.hBar').mousedown(function(e){
		var hBar = $(this);
		var y = e.pageY;
		//	Previous frames
		var prev = hBar.parent();
		prev = prev.add(prev.find('.frame,.vBar'));
		var prevHeight = [];
		prev.each(function(){
			prevHeight.push($(this).height());
		});
		//	Next frames
		var next = hBar.parent().next();
		var nextTop = parseInt(next.css('top'));
		next = next.add(next.find('.frame,.vBar'));
		var nextHeight = [];
		next.each(function(){
			nextHeight.push($(this).height());
		});
		//	Events
		var overlay = $('<div>')
			.addClass('overlay')
			.css('cursor', hBar.css('cursor'))
			.appendTo(frameset);
		overlay.mousemove(function(e){
			var dy = e.pageY - y;
			prev.each(function(i){
				// $(this).height(prevHeight[i] + dy).filter('.frame').resize();
				$(this).height(prevHeight[i] + dy);
			});
			next.each(function(i){
				$(this).height(nextHeight[i] - dy)
			// }).first().css({top: (nextTop + dy) + 'px'}).find('.frame').resize();
			}).first().css({top: (nextTop + dy) + 'px'});
		}).mouseup(function(){
			prev.filter('.frame').resize();
			next.find('.frame').resize();
			overlay.remove();
		}).mouseout(function(){
			prev.filter('.frame').resize();
			next.find('.frame').resize();
			overlay.remove();
		});		
	});

});
