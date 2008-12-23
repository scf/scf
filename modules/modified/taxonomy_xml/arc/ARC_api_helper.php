<?php
/*
appmosphere RDF classes (ARC): http://www.appmosphere.com/en-arc

Copyright © 2005 appmosphere web applications, Germany. All Rights Reserved.
This work is distributed under the W3C® Software License [1] in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231

ns sk                       http://www.appmosphere.com/ns/site_kit#
ns doap                     http://usefulinc.com/ns/doap#
sk:className                ARC_api_helper
doap:name                   ARC API Helper
doap:homepage               http://www.appmosphere.com/en-arc_api
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              ARC API Helper
//release
doap:created                2006-10-01
doap:revision               0.1.1
//changelog
sk:releaseChanges           2006-03-19: release 0.1.0
                            2006-10-01: revision 0.1.1
                                        - made store vars optional ("enable_vars" setting, default: false)
*/

class ARC_api_helper {

	var $version="0.1.1";
	var $table_lock="off";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_api_helper(&$api){
		$this->__construct($api);
	}

	/*					*/
	
	function get_mtime(){
		list($msec, $sec) = explode(" ", microtime());
		return ((float)$msec + (float)$sec);
	}

	/*					*/
	
	function encode_mysql_utf8_bugs($val=""){
		$val=str_replace(utf8_encode("Ü"), "&#arc220;", $val);
		$val=str_replace(utf8_encode("ß"), "&#arc223;", $val);
		$val=str_replace(utf8_encode("ö"), "&#arc246;", $val);
		$val=str_replace(utf8_encode("ü"), "&#arc252;", $val);
		return $val;
	}

	function decode_mysql_utf8_bugs($val=""){
		$val=str_replace("&#arc220;", utf8_encode("Ü"), $val);
		$val=str_replace("&#arc223;", utf8_encode("ß"), $val);
		$val=str_replace("&#arc246;", utf8_encode("ö"), $val);
		$val=str_replace("&#arc252;", utf8_encode("ü"), $val);
		return $val;
	}

	function adjust_utf8_string($val=""){
		if(utf8_decode($val)==$val){
			return $val;
		}
		$val=(strpos(utf8_decode(str_replace("?", "", $val)), "?")===false) ? utf8_decode($val) : $val;
		$length=strlen($val);
		$new_val="";
		$tmp="";
		$do_enc=true;
		for($i=0,$i_max=$length;$i<$i_max;$i++){
			$is_special=false;
			$cur_char=$val{$i};
			$enc_char=rawurlencode($cur_char);
			$new_char="";
			if($cur_char==$enc_char){
			}
			elseif($enc_char=="%B4"){
				$cur_char.="'";
			}
			else{
				$utf_char=utf8_encode($cur_char);
				$bytes=strlen($utf_char);
				if($bytes>1){
					$tmp.=$cur_char;
					$is_special=true;
					$cur_char="";
				}
				else{
					$cur_char=$utf_char;
				}
			}
			if(!$is_special){
				$new_val.= ($tmp) ? $this->adjust_utf8_char($tmp).$cur_char : $cur_char;
				$tmp="";
			}
		}
		/* remaining tmp ? */
		if($tmp){
			$new_val.=$this->adjust_utf8_char($tmp);
		}
		return ($do_enc) ? utf8_encode($new_val) : $new_val;
	}

	function adjust_utf8_char($val=""){
		$char_dec=hexdec(rawurlencode($val));
		if($char_dec > 14835840){
			return '&#'.($char_dec-14835840).';';
		}
		$result="";
		switch($val){
			case "Â»":
				$result.='»';break;
			case "Ã©":
				$result.='é';break;
			case "Ã¨":
				$result.='è';break;
			case rawurldecode("%F5"):
				$result.="ö";/* %F6 */break;
			case rawurldecode("%C3%89"):
				$result.='É';break;
			default:
				$result.=$val;
		}
		return $result;
	}

	function escape_js_string($val=""){
		$val=str_replace("%0D", '\\r', rawurlencode($val));
		$val=str_replace("%0A", '\\n', $val);
		$val=str_replace("%22", '\"', $val);
		return preg_replace("/([^\\\])\"/", '\\1\"', rawurldecode($val));
	}

	/*					*/

	function get_hash_int_id($val="", $sql=false){
		return $sql ? "CONV('".substr(md5($val), 0, 15)."', 16, 10)" : substr(md5($val), 0, 15);
	}
	
	function get_hash_md5_id($val="", $sql=false){
		$result="";
		if($val==""){
			return $sql ? "'X_by1-I3bB@@hJ2Tz--=9'" : "X_by1-I3bB@@hJ2Tz--=9";
		}
		$val=md5($val);
		$parts=array(substr($val, 0, 11), substr($val, 11, 11), substr($val, 22));
		$alpha='0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !?()+,-.@;=[]_{}';
		$base=strlen($alpha);
		for($i=0;$i<3;$i++){
			$cur_part=$parts[$i];
			$cur_int=hexdec($cur_part);
			$cur_result="";
			for($j=floor(log10($cur_int)/log10($base));$j>=0;$j--){
				$pos=floor($cur_int/pow($base,$j));
				$cur_result.=$alpha{$pos};
				$cur_int-=($pos*pow($base, $j));
			}
			$result.=sprintf("%07s", $cur_result);
		}
		return $sql ? "'".$result."'" : $result;
	}
	
	function get_hash_sha1_id($val="", $sql=false){
		$result="";
		if($val==""){
			return $sql ? "'ZSK!7vEP6q[fX +VKiH+s6(a,e'" : "ZSK!7vEP6q[fX +VKiH+s6(a,e";
		}
		$val=sha1($val);
		$parts=array(substr($val, 0, 11), substr($val, 11, 11), substr($val, 22, 11), substr($val, 33));
		$alpha='0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !?()+,-.@;=[]_{}';
		$base=strlen($alpha);
		for($i=0;$i<4;$i++){
			$cur_part=$parts[$i];
			$cur_int=hexdec($cur_part);
			$cur_result="";
			for($j=floor(log10($cur_int)/log10($base));$j>=0;$j--){
				$pos=floor($cur_int/pow($base,$j));
				$cur_result.=$alpha{$pos};
				$cur_int-=($pos*pow($base, $j));
			}
			$result.=($i==3) ? sprintf("%05s", $cur_result) : sprintf("%07s", $cur_result);
		}
		return $sql ? "'".$result."'" : $result;
	}
	
	function get_incr_int_id($val=""){
		/* check existence */
		$tbl_1=$this->config["prefix"]."_id2val";
		$col_val = (isset($this->config["encode_values"]) && $this->config["encode_values"]) ? rawurlencode($val) : mysql_real_escape_string($val);
		if(($rs=mysql_query("SELECT id FROM ".$tbl_1." WHERE T1.val='".$col_val."'")) && mysql_num_rows($rs)){
			$row=mysql_fetch_array($rs);
			return $row["id"];
		}
		else{
			$q1=mysql_query("INSERT IGNORE INTO ".$tbl_1." (val) VALUES('".$col_val."')");
			if($q2=mysql_query("SELECT LAST_INSERT_ID() AS id")){
				$row=mysql_fetch_array($q2);
				return $row["id"];
			}
		}
	}
	
	/*					*/

	function get_prop_table_infos(){
		if(!isset($this->prop_table_infos)){
			$this->prop_table_infos=array();
			if(isset($this->config["prop_tables"]) && is_array($this->config["prop_tables"])){
				$tbl_base=$this->config["prefix"]."_triple";
				foreach($this->config["prop_tables"] as $cur_prop_tbl){
					foreach($cur_prop_tbl["props"] as $cur_prop){
						$tbl_name=($cur_prop_tbl["prop_type"]=="obj")? $tbl_base."_op_".$cur_prop_tbl["name"] : $tbl_base."_dp_".$cur_prop_tbl["name"];
						$this->prop_table_infos[$cur_prop]=array("tbl"=>$tbl_name, "type"=>$cur_prop_tbl["prop_type"]);
					}
				}
			}
		}
		return $this->prop_table_infos;
	}
	
	/*					*/
		
	function get_base_tables(){
    $r = ($this->config["store_type"]=="split") ? array("triple_dp", "triple_op", "triple_dup", "id2val") : array("triple", "triple_dup", "id2val");
    return (isset($this->config['enable_vars']) && $this->config['enable_vars']) ? array_merge($r, array('id2val')) : $r;
	}

	function get_prop_tables(){
		$result=array();
		$prop_tables=(isset($this->config["prop_tables"]) && is_array($this->config["prop_tables"])) ? $this->config["prop_tables"] : array();
		foreach($prop_tables as $cur_tbl){
			$result[]=($cur_tbl["prop_type"]=="dt") ? "triple_dp_".$cur_tbl["name"] : "triple_op_".$cur_tbl["name"];
		}
		return $result;
	}

	function get_triple_tables(){
		return ($this->config["store_type"]=="split") ? array_merge(array("triple_dp", "triple_op"), $this->get_prop_tables()) : array("triple");
	}

	function get_tables(){
		return array_merge($this->get_prop_tables(), $this->get_base_tables());
	}
	
	/*					*/

	function lock_tables($custom=""){
		if($this->table_lock=="off"){
			$tbls=$this->get_tables();
			$prefix=$this->config["prefix"];
			$code="";
			foreach($tbls as $cur_tbl){
				$code.=strlen($code) ? ", " : "LOCK TABLES ";
				$code.=$prefix."_".$cur_tbl." WRITE";
			}
			if(is_array($custom)){
				foreach($custom as $cur_tbl){
					$code.=strlen($code) ? ", " : "LOCK TABLES ";
					$code.=$cur_tbl." WRITE";
				}
			}
			$tmp=mysql_query("FLUSH TABLES");
			$tmp=mysql_query($code);
			$this->table_lock="on";
		}
	}
	
	function unlock_tables(){
		$tmp=mysql_query("UNLOCK TABLES");
		$this->table_lock="off";
	}

	/*					*/
	
	function get_default_ifp_iris(){
		$air="http://www.daml.org/2001/10/html/airport-ont#";
		$foaf="http://xmlns.com/foaf/0.1/";
		$skos="http://www.w3.org/2004/02/skos/core#";
		$owl="http://www.w3.org/2002/07/owl#";
		return array(
			$air."iataCode",
			$foaf."mbox",
			$foaf."mbox_sha1sum",
			$foaf."homepage",
			$foaf."weblog",
			$skos."subjectIndicator",
			$owl."sameAs"
		);
	}
	
	function get_default_fp_iris(){
		$foaf="http://xmlns.com/foaf/0.1/";
		$skos="http://www.w3.org/2004/02/skos/core#";
		$owl="http://www.w3.org/2002/07/owl#";
		return array(
			$foaf."primaryTopic",
			$owl."sameAs"
		);
	}
	
	/*					*/
	/*					*/
	
	function arg($name="", $multi=false, $mthd=false){
		$mthd=strtolower($mthd);
		if($multi){
			$qs = "";
			if(!$mthd || ($mthd=="post")){
				$qs = @file_get_contents('php://input');
			}
			if(!$qs){
				$qs = "&".@$_SERVER["QUERY_STRING"]."&";
			}
			if(preg_match_all("/\&".$name."=([^\&]*)/", $qs, $matches)){
				foreach($matches[1] as $i=>$val){
					$matches[1][$i]=stripslashes($val);
				}
				return $matches[1];
			}
			return array();
		}
		if($mthd=="post"){
			return isset($_POST[$name]) ? stripslashes($_POST[$name]) : false;
		}
		if($mthd=="get"){
			return isset($_GET[$name]) ? stripslashes($_GET[$name]) : false;
		}
		return (isset($_POST[$name])) ? stripslashes($_POST[$name]) : ((isset($_GET[$name])) ? stripslashes($_GET[$name]) : false);
	}

	/*					*/

}

?>