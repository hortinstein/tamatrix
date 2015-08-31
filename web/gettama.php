<?php
//header("Content-Type: text/json");

$lastseq=0;
if (isset($_REQUEST["lastseq"])) $lastseq=$_REQUEST["lastseq"];

$shm=shmop_open(7531, "a", 0644, 1024*8);
if ($shm===FALSE) die("Can't open ipc shm key");

$ret=new StdClass();


$hdr=unpack("Lcurrseq/Lnotama", shmop_read($shm, 0, 8));
$ret->notama=$hdr["notama"];
$ret->lastseq=$hdr["currseq"];
$ret->tama=array();
if ($hdr["currseq"]<=$lastseq) {
	//No update needed.
	echo json_encode($ret);
	return;
}

$n=0;
for ($i=0; $i<$hdr["notama"]; $i++) {
	$off=$i*(32*48+6)+8;
	$shdr=unpack("Llastseq", shmop_read($shm, $off, 4));
//	var_dump($shdr);
	if ($shdr["lastseq"]>$lastseq) {
		$data=unpack("c*", shmop_read($shm, $off+4, 32*48));
//		var_dump($data);
		$st="";
		for ($x=0; $x<32*48; $x++) $st.=chr($data[$x+1]+65);
		$ret->tama[$n]=new StdClass();
		$ret->tama[$n]->id=$i;
		$ret->tama[$n]->pixels=$st;
		$ret->tama[$n]->icons=unpack("S", shmop_read($shm, $off+12+32*48, 2));
		$n++;
	}
}

echo json_encode($ret);
?>