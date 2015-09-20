
var lastseq=0;
var tamaCanvas=Array();
var bigId=-1;

function showBigTama(id) {
	if (id!=-1) {
		bigId=id;
		var ctx=$("bigTama").getContext("2d");
		ctx.fillStyle="#efffe0";
		ctx.fillRect(0,0,720,480);
		$("ovl").style.visibility="visible";
		$("bigTama").style.visibility="visible";
		lastseq=0; //force update
	} else {
		$("ovl").style.visibility="hidden";
		$("bigTama").style.visibility="hidden";
	}
}

function drawTama(id, tama) {
	var ctx;
	if (tamaCanvas.length<=id) {
		var c=new Element('canvas', {
			'height': 160,
			'width': 240,
			'class': 'tamalcd'
		});
		c.addEvent('click', function(id) {
			showBigTama(id);
		}.bind(c,id));
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
		var ctx=tamaCanvas[id].getContext("2d");
		ctx.fillStyle="#efffe0";
		ctx.fillRect(0,0,240,160);
	}
	var ctx=tamaCanvas[id].getContext("2d");
	var bigCtx=$("bigTama").getContext("2d");
	var p=0;
	for (var y=0; y<32; y++) {
		for (var x=0; x<48; x++) {
			var c=tama.pixels.substr(p++, 1);
			var st;
			if (c=='A') st="#efffe0";
			if (c=='B') st="#A0B090";
			if (c=='C') st="#707058";
			if (c=='D') st="#102000";
			ctx.fillStyle=st;
			ctx.fillRect(x*5, y*5, 4, 4);
			if (bigId==id) {
				bigCtx.fillStyle=st;
				bigCtx.fillRect(x*15, y*15, 14, 14);
			}
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
	$("ovl").addEvent("click", function() {
		showBigTama(-1);
	});
	$("bigTama").addEvent("click", function() {
		showBigTama(-1);
	});
});
