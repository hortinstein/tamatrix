
var lastseq=0;
var tamaCanvas=Array();


function drawTama(id, tama) {
	var ctx;
	if (tamaCanvas.length<=id) {
		var c=new Element('canvas', {
			'height': 160,
			'width': 240,
			'class': 'tamalcd'
		});
		var d=new Element('div', {
			'class': 'tamahex'
		});
		c.inject(d);
		d.inject($("tamas"));
		if ((id%5)==2) {
			var e=new Element('div', {
				'class': 'tamaoffset'
			});
			e.inject($("tamas"));
		}
		
		tamaCanvas[id]=c;
		ctx=tamaCanvas[id].getContext("2d");
		ctx.fillStyle="#efffe0";
		ctx.fillRect(0,0,240,160);
	}
	ctx=tamaCanvas[id].getContext("2d");
	var p=0;
	for (y=0; y<32; y++) {
		for (x=0; x<48; x++) {
			var c=tama.pixels.substr(p++, 1);
			if (c=='A') ctx.fillStyle="#efffe0";
			if (c=='B') ctx.fillStyle="#A0B090";
			if (c=='C') ctx.fillStyle="#707058";
			if (c=='D') ctx.fillStyle="#102000";
			ctx.fillRect(x*5, y*5, 4, 4);
		}
	}
}

function updateTama() {
	var myReq=new Request.JSON({
		method: 'get',
		url: 'gettama.php',
		onSuccess: function(respJson, respTxt) {
			setTimeout("updateTama()", 100);
			if (respTxt!="") { //yeah, dirty hack
				lastseq=respJson.lastseq;
					for (var i=0; i<respJson.tama.length; i++) {
					drawTama(respJson.tama[i].id, respJson.tama[i]);
				}
			}
		},
		onError: function(text, err) {
			//Just try again in a while.
			setTimeout("updateTama()", 500);
		}
	});
	myReq.get({"lastseq": lastseq});
}


window.addEvent('domready', function() {
	updateTama();
});
