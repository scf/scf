<?php
/*
appmosphere RDF classes (ARC): http://www.appmosphere.com/en-arc

Copyright © 2004 appmosphere web applications, Germany. All Rights Reserved.
This work is distributed under the W3C® Software License [1] in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231

ns sk                       http://www.appmosphere.com/ns/site_kit#
ns doap                     http://usefulinc.com/ns/doap#
sk:className                ARC_ntriples_serializer
doap:name                   ARC N-Triples serializer
doap:homepage               http://www.appmosphere.com/en-arc_ntriples_serializer
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              A simple PHP N-Triples serializer for ARC triple arrays
//release
doap:created                2004-07-21
doap:revision               0.1.0
//changelog
sk:releaseChanges           -

*/

class ARC_ntriples_serializer{

	function ARC_ntriples_serializer($args=""){
		$this->spacer=" ";
		$this->linebreak="\r\n";
		if(is_array($args)){foreach($args as $k=>$v){$this->$k=$v;}}/* linebreak, spacer */
	}

	/*					*/

	function str2unicode_nfc($str){
		$result="";
		/* try to detect encoding */
		$tmp=str_replace("?", "", $str);
		if(strpos(utf8_decode($tmp), "?")===false){
			$str=utf8_decode($str);
		}
		for($i=0,$i_max=strlen($str);$i<$i_max;$i++){
			$nr=0;/* unicode dec nr */
			/* char */
			$char=$str[$i];
			/* utf8 binary */
			$utf8_char=utf8_encode($char);
			$bytes=strlen($utf8_char);
			if($bytes==1){
				/* 0####### (0-127) */
				$nr=ord($utf8_char);
			}
			elseif($bytes==2){
				/* 110##### 10###### = 192+x 128+x */
				$nr=((ord($utf8_char[0])-192)*64) + (ord($utf8_char[1])-128);
			}
			elseif($bytes==3){
				/* 1110#### 10###### 10###### = 224+x 128+x 128+x */
				$nr=((ord($utf8_char[0])-224)*4096) + ((ord($utf8_char[1])-128)*64) + (ord($utf8_char[2])-128);
			}
			elseif($bytes==4){
				/* 1111#### 10###### 10###### 10###### = 240+x 128+x 128+x 128+x */
				$nr=((ord($utf8_char[0])-240)*262144) + ((ord($utf8_char[1])-128)*4096) + ((ord($utf8_char[2])-128)*64) + (ord($utf8_char[3])-128);
			}
			/* result (see http://www.w3.org/TR/rdf-testcases/#ntrip_strings) */
			if($nr<9){/* #x0-#x8 (0-8) */
				$result.="\\u".sprintf("%04X",$nr);
			}
			elseif($nr==9){/* #x9 (9) */
				$result.='\t';
			}
			elseif($nr==10){/* #xA (10) */
				$result.='\n';
			}
			elseif($nr<13){/* #xB-#xC (11-12) */
				$result.="\\u".sprintf("%04X",$nr);
			}
			elseif($nr==13){/* #xD (13) */
				$result.='\t';
			}
			elseif($nr<32){/* #xE-#x1F (14-31) */
				$result.="\\u".sprintf("%04X",$nr);
			}
			elseif($nr<34){/* #x20-#x21 (32-33) */
				$result.=$char;
			}
			elseif($nr==34){/* #x22 (34) */
				$result.='\"';
			}
			elseif($nr<92){/* #x23-#x5B (35-91) */
				$result.=$char;
			}
			elseif($nr==92){/* #x5C (92) */
				$result.='\\';
			}
			elseif($nr<127){/* #x5D-#x7E (93-126) */
				$result.=$char;
			}
			elseif($nr<65536){/* #x7F-#xFFFF (128-65535) */
				$result.="\\u".sprintf("%04X",$nr);
			}
			elseif($nr<1114112){/* #x10000-#x10FFFF (65536-1114111) */
				$result.="\\U".sprintf("%08X",$nr);
			}
			else{
				/* other chars are not defined => ignore */
			}
		}
		return $result;
	}


	/*					*/

	function get_ntriples($triples){
		$spacer=$this->spacer;
		$linebreak=$this->linebreak;
		$result="";
		if(is_array($triples)){
			for($i=0,$i_max=count($triples);$i<$i_max;$i++){
				$cur_t=$triples[$i];
				/* s */
				$s=$cur_t["s"];
				$s_type=$s["type"];
				if($s_type==="uri"){
					$result.='<'.$this->str2unicode_nfc($s["uri"]).'>';
				}
				elseif($s_type==="bnode"){
					$result.=$s["bnode_id"];
				}
				$result.=$spacer;
				/* p */
				$p=$cur_t["p"];
				$result.='<'.$p.'>';
				$result.=$spacer;
				/* o */
				$o=$cur_t["o"];
				$o_type=$o["type"];
				if($o_type==="uri"){
					$result.='<'.$this->str2unicode_nfc($o["uri"]).'>';
				}
				elseif($o_type==="bnode"){
					$result.=$o["bnode_id"];
				}
				elseif($o_type==="literal"){
					$result.='"'.$this->str2unicode_nfc($o["val"]).'"';
					if($dt=$o["dt"]){
						$result.="^^<".$dt.">";
					}
					elseif($lang=$o["lang"]){
						$result.="@".$lang;
					}
				}
				$result.=" .".$linebreak;
			}
		}
		return $result;
	}
}
?>