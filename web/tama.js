
var lastseq=0;


function drawTama(id, tama) {
	var ctx;
	if (id==0) ctx=$("tamalcd1").getContext("2d");
	if (id==1) ctx=$("tamalcd2").getContext("2d");
	ctx.fillStyle="#efffe0";
	ctx.fillRect(0,0,480,320);
	var p=0;
	for (y=0; y<32; y++) {
		for (x=0; x<48; x++) {
			var c=tama.pixels.substr(p++, 1);
			if (c=='A') ctx.fillStyle="#efffe0";
			if (c=='B') ctx.fillStyle="#A0B090";
			if (c=='C') ctx.fillStyle="#707058";
			if (c=='D') ctx.fillStyle="#102000";
			ctx.fillRect(x*10, y*10, 9, 9);
		}
	}
}

function updateTama() {
	var myReq=new Request.JSON({
		method: 'get',
		url: 'gettama.php',
		onSuccess: function(resp) {
			lastseq=resp.lastseq;
			for (var i=0; i<resp.tama.length; i++) {
				drawTama(resp.tama[i].id, resp.tama[i]);
			}
			setTimeout("updateTama()", 100);
		}
	});
	myReq.get({"lastseq": lastseq});
}


window.addEvent('domready', function() {
	updateTama();
});
