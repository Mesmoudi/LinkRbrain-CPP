var emptyHues = 0;
var hueStart = Math.random();
$('input.hue').each(function(){
	var input = $(this);
	var color = input.val();
	if (color == ''  ||  color == input.attr('title')){
		emptyHues++;
	}
});
$('input.hue').each(function(i){
	var input = $(this);
	var color = input.val();
	if (color == ''  ||  color == input.attr('title')){
		var hueValue = Math.floor(360 * (hueStart + i / emptyHues)) % 360;
		color = 'rgb(' + $.hsv2rgb(hueValue, 255, 255) + ')';
		input.val(color);
	}
});
$('input.hue').attr('spellcheck', false).change(function(){
	var input = $(this);
	var color = input.val();
	input.css(
	{	cursor		:	'pointer'
	,	background	:	color
	,	color		:	color
	});
}).focus(function(){
	$(this).blur();
}).mousedown(function(e){
	//	Find position relatively to the screen
	var input = $(this);
	var offset = input.offset();
	var doc = $(document);
	//	Show hue selection tool
	var palette = $('<div>').addClass('hue').append(
		$('<img>').attr('src', '/palette.png')	
	).appendTo(document.body).mousedown(function(e){
		var x = e.offsetX - parseInt(palette.css('paddingLeft'));
		var X = palette.width();
		var color = $.hsv2rgb(360 * x / X, 255, 255);
		input.val('rgb(' + color.join(',') + ')').change();
		palette.remove();
	});
	doc.scroll(function(){
		var left = offset.left - doc.scrollLeft() - 4;
		var top = offset.top - doc.scrollTop() + input.outerHeight() + 4;
		palette.css(
		{	left	:	left + 'px'
		,	top		:	top + 'px'
		});
	}).mousedown(function(){
		palette.remove();
	}).scroll();
	//	The end!
	e.preventDefault(e);
	return false;
}).change();