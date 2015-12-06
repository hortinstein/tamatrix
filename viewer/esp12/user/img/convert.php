#!/usr/bin/php
<?php

echo "char imgNo[10][24*32/4]={\n";

for ($ix=0; $ix<10; $ix++) {
	$im=imagecreatefrompng($ix.".png");
	$first=true;
	echo "\t{";
	for ($yy=0; $yy<32; $yy++) {
		if ($yy>=16) $y=$yy-16; else $y=31-$yy;
		for ($x=0; $x<24; $x+=4) {
			$b=0;
			for ($z=0; $z<4; $z++) {
				$c=imagecolorsforindex($im, imagecolorat($im, $x+$z, $y));
				$b<<=2;
				$b|=((255-$c["green"])>>6);
			}
			if (!$first) echo ", ";
			$first=false;
			printf("0x%02X", $b);
		}
		if ($y!=31) echo "\n\t";
	}
	if ($ix!=9) echo "},\n"; else echo "}\n";
}
echo "};\n";


?>



