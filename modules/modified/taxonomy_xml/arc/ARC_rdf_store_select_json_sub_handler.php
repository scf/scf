<?php
/*
appmosphere RDF classes (ARC): http://www.appmosphere.com/en-arc

Copyright (c) 2006 appmosphere web applications, Germany. All Rights Reserved.
This work is distributed under the W3C(r) Software License [1] in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231

ns sk                       http://www.appmosphere.com/ns/site_kit#
ns doap                     http://usefulinc.com/ns/doap#
sk:className                ARC_rdf_store_select_json_sub_handler
doap:name                   ARC RDF Store SELECT query JSON result sub-handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles SPARQL SELECT queries with JSON result type
//release
doap:created                2006-04-10
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-10: release 0.1.0
*/

class ARC_rdf_store_select_json_sub_handler {

	var $version="0.1.0";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_select_json_sub_handler(&$api){
		$this->__construct($api);
	}

	/*					*/

	function decode_mysql_utf8_bugs($val=""){
		return $this->api->decode_mysql_utf8_bugs($val);
	}

	/*					*/
	
	function get_json_result($args=""){
		$nl="\n";
		$jsonp=(isset($args["result_type_args"]["jsonp"])) ? $args["result_type_args"]["jsonp"] : false;
		$code="";
		$code.=($jsonp) ? $jsonp.'({' : '{';
		$code.=$this->get_json_result_head($args);
		$code.=",".$this->get_json_result_body($args);
		$code.=($jsonp) ? $nl.'})' : $nl.'}';
		return array("result"=>$code, "error"=>"");
	}
	
	/*					*/	

	function get_json_result_head($args=""){
		$ind="  ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";

		$jsoni=(isset($args["result_type_args"]["jsoni"])) ? $args["result_type_args"]["jsoni"] : false;
		$jsonc=(isset($args["result_type_args"]["jsonc"])) ? $args["result_type_args"]["jsonc"] : false;

		$code="";
		$code.=$nl.$ind.'head: {';
		$code.=$nl.$ind2.'vars: [';
		$var_code="";
		$vars=($jsoni) ? $this->get_jsoni_result_vars($jsoni, 1) : (($jsonc) ? $this->get_jsonc_result_vars($jsonc, 1) : $args["infos"]["result_vars"]);
		foreach($vars as $cur_var){
			$cur_var=preg_match("/^([^:]+)\:(.+)$/", $cur_var, $matches) ? $matches[2] : $cur_var;
			$var_code.=($var_code) ? ', "'.$cur_var.'"' : '"'.$cur_var.'"';
		}
		$code.=$var_code.']';
		$code.=$nl.$ind.'}';
		return $code;
	}
	
	/*					*/	

	function get_jsonc_result_vars($t="", $vars_only=false){
		if(!preg_match("/^[0-9a-z _\-\(\),]+$/i", $t)){
			return array("");
		}
		$t=str_replace(" ", "", $t);
		$t=str_replace("(", '"=>array(', $t);
		$t=str_replace(",", ',"', $t);
		$t=str_replace("(", '(', $t);
		$t=preg_replace("/([^)]),/", '\\1",', $t);
		$t=preg_replace("/([^)])$/", '\\1"', $t);
		$t='$t_struct=array("'.$t.');';
		@eval($t);
		if(!is_array($t_struct)){
			return array("");
		}
		$result=array();
		foreach($t_struct as $k=>$v){
			$cur_var=is_array($v) ? $k : $v;
			$result[]=($vars_only) ? $cur_var : array("var"=>$cur_var, "compact"=>!is_array($v));
		}
		return $result;
	}

	function get_jsoni_result_vars($t="", $vars_only=false){
		$t_struct=$this->get_jsoni_struct($t);
		$result=array();
		foreach($t_struct as $k=>$v){
			$cur_var=is_array($v) ? $k : $v;
			$result[]=($vars_only) ? $cur_var : array("var"=>$cur_var, "compact"=>!is_array($v));
		}
		return $result;
	}

	/*					*/	

	function get_jsoni_struct($t=""){
		if(!preg_match("/^[0-9a-z _\-\(\)]+$/i", $t)){
			return array("");
		}
		/* mark aliases */
		$t=preg_replace("/ as /i", ":", $t);
		/* parse index template */
		$t=str_replace(" ", "", $t);
		$t=str_replace("(", '"=>array(', $t);
		$t=str_replace(",", ',"', $t);
		$t=str_replace("(", '("', $t);
		$t=preg_replace("/([^)]),/", '\\1",', $t);
		$t=preg_replace("/([^)])\)/", '\\1")', $t);
		$t=preg_replace("/([^)])$/", '\\1"', $t);
		$t='$t_struct=array("'.$t.');';
		@eval($t);
		return (is_array($t_struct)) ? $t_struct : array("");
	}
	
	function get_jsoni_indexes($t=""){
		$t_struct=$this->get_jsoni_struct($t);
		$this->jsoni_indexes=array();
		foreach($t_struct as $k=>$v){
			$cur_var=is_array($v) ? $k : $v;
			$cur_key=$cur_var;
			$this->jsoni_indexes[]=$cur_key;
			if(is_array($v)){
				$this->calc_jsoni_indexes(array("prefix"=>$cur_key, "entry"=>$v));
			}
		}
		return $this->jsoni_indexes;
	}

	function calc_jsoni_indexes($args=""){
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}/* prefix, entry */
		foreach($entry as $k=>$v){
			$cur_var=is_array($v) ? $k : $v;
			$cur_key=$prefix." ".$cur_var;
			if(!in_array($cur_key, $this->jsoni_indexes)){
				$this->jsoni_indexes[]=$cur_key;
			}
			if(is_array($v)){
				$this->calc_jsoni_indexes(array("prefix"=>$cur_key, "entry"=>$v));
			}
		}
	}

	/*					*/
	
	function get_json_result_body($args=""){
		$ind="  ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";

		$jsoni=(isset($args["result_type_args"]["jsoni"])) ? $args["result_type_args"]["jsoni"] : false;
		$jsonc=(isset($args["result_type_args"]["jsonc"])) ? $args["result_type_args"]["jsonc"] : false;
		$distinct=(isset($args["infos"]["distinct"])) ? $args["infos"]["distinct"] : false;
		$ordered=(isset($args["infos"]["order_conditions"])) ? $args["infos"]["order_conditions"] : false;
		$code="";
		$code.=$nl.$ind.'results: {';
		/* distinct */
		$code.=$nl.$ind2.'distinct: '.($distinct ? 'true' : 'false');
		/* ordered */
		$code.=",".$nl.$ind2.'ordered: '.($ordered ? 'true' : 'false');
		/* compact */
		$code.=",".$nl.$ind2.'compact: '.($jsonc ? 'true' : 'false');
		/* indexed */
		$code.=",".$nl.$ind2.'indexed: '.($jsoni ? 'true' : 'false');
		if($jsoni){
			/* index */
			$code.=",".$nl.$ind2.'index: {';
			$code.=$this->get_jsoni_result_body_index($args);
			$code.=$nl.$ind2.'}';
		}
		else{
			/* bindings */
			$code.=",".$nl.$ind2.'bindings: [';
			$code.=$this->get_json_result_body_bindings($args);
			$code.=$nl.$ind2.']';
		}
		$code.=$nl.$ind.'}';
		return $code;
	}

	function get_json_result_body_bindings($args=""){
		$ind="  ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";
		$jsonc=(isset($args["result_type_args"]["jsonc"])) ? $args["result_type_args"]["jsonc"] : false;
		$row=(isset($args["row"])) ? $args["row"] : false;
		$rs = (isset($args["rs"])) ? $args["rs"] : false;
		$enc_vals=$this->config["encode_values"];
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$code="";
		if(!$row){
			return $code;
		}
		$result_vars=$args["infos"]["result_vars"];
		$result_vars_jsonc=(count($result_vars)==1) ? $result_vars[0]."()" : join("(),", $result_vars)."()";
		$jsonc_t=($jsonc) ? $jsonc : $result_vars_jsonc;
		$vars=$this->get_jsonc_result_vars($jsonc_t);
		$api =& $this->api;
		$api_helper =& $this->api->get_api_helper();
		do {
			/* result */
			$code.=(strlen($code)) ? "," : "";
			$code.=$nl.$ind3.'{';
			$vars_code="";
			foreach($vars as $cur_t_var){
				$cur_var=$cur_t_var["var"];
				$compact=$cur_t_var["compact"];
				if(isset($row[$cur_var])){
					$cur_type=(isset($row[$cur_var."__type"])) ? $row[$cur_var."__type"] : 0;
					$val=($enc_vals) ? rawurldecode($row[$cur_var]) : $row[$cur_var];
					$vars_code.=(strlen($vars_code)) ? ",".$nl : "";
					$vars_code.=$ind4.'"'.$cur_var.'": ';
					$vars_code.=($compact) ? "" : '{';
					if(!isset($row[$cur_var."__type"]) || ($row[$cur_var."__type"]==0)){
						$vars_code.=($compact) ? '"'.$val.'"' : ' type: "uri", value: "'.$val.'"';
					}
					elseif($row[$cur_var."__type"]==1){
						$vars_code.=($compact) ? '"'.substr($val, 2).'"' : ' type: "bnode", value: "'.substr($val, 2).'"';
					}
					else{
						$val=$this->decode_mysql_utf8_bugs($val);
						$val=($fix_utf8) ? $api_helper->escape_js_string($api_helper->adjust_utf8_string($val)) : $api_helper->escape_js_string($val);
						$val_type="literal";
						$vars_code_addition="";
						if(isset($row[$cur_var."__lang"]) && ($lang=$row[$cur_var."__lang"])){
							$vars_code_addition.= ($compact) ? ', "'.$cur_var.'__lang": "'.$lang.'"' : ', "xml:lang": "'.$lang.'"';
						}
						elseif(isset($row[$cur_var."__dt"]) && ($dt=$row[$cur_var."__dt"])){
							$dt_val=($conv_id) ? $api->get_val("CONV('".$dt."', 16, 10)") : $api->get_val("'".$dt."'");
							if($dt_val){
								$val_type="typed-literal";
								$vars_code_addition.= ($compact) ? ', "'.$cur_var.'__datatype": "'.htmlspecialchars($dt_val).'"' : ', "datatype": "'.htmlspecialchars($dt_val).'"';
							}
						}
						$vars_code.=($compact) ? '"'.$val.'"'.$vars_code_addition : ' type: "'.$val_type.'"'.$vars_code_addition.', value: "'.$val.'"';
					}
					$vars_code.=($compact) ? "": ' }';
				}
			}
			$code.=$nl.$vars_code;
			/* /result */
			$code.=$nl.$ind3.'}';
			$row=mysql_fetch_array($rs);
		} while ($row);
		return $code;
	}

	function get_jsoni_result_body_index($args=""){
		$ind="  ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";
		$jsoni=(isset($args["result_type_args"]["jsoni"])) ? $args["result_type_args"]["jsoni"] : false;
		$row=(isset($args["row"])) ? $args["row"] : false;
		$rs = (isset($args["rs"])) ? $args["rs"] : false;
		$enc_vals=$this->config["encode_values"];
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$code="";
		if(!$row){
			return $code;
		}
		$result_vars=$args["infos"]["result_vars"];
		$api =& $this->api;
		$api_helper =& $this->api->get_api_helper();
		if($row){
			/* index creation */
			$idxs=$this->get_jsoni_indexes($jsoni);
			$val_idxs=array();
			do {
				$cur_vals=array();
				foreach($result_vars as $cur_var){
					if(isset($row[$cur_var])){
						$cur_type=(isset($row[$cur_var."__type"])) ? $row[$cur_var."__type"] : 0;
						$val=($enc_vals) ? rawurldecode($row[$cur_var]) : $row[$cur_var];
						if($cur_type==0){
							$cur_vals[$cur_var."__type"]="uri";
						}
						elseif($cur_type==1){
							$cur_vals[$cur_var."__type"]="bnode";
							$val=substr($val, 2);
						}
						elseif($cur_type > 1){
							$cur_vals[$cur_var."__type"]="literal";
							$val=$this->decode_mysql_utf8_bugs($val);
							$val=($fix_utf8) ? $api_helper->escape_js_string($api_helper->adjust_utf8_string($val)) : $api_helper->escape_js_string($val);
							if(isset($row[$cur_var."__lang"]) && ($lang=$row[$cur_var."__lang"])){
								$cur_vals[$cur_var."__lang"]=$lang;
							}
							elseif(isset($row[$cur_var."__dt"]) && ($dt=$row[$cur_var."__dt"])){
								$dt_val=($conv_id) ? $api->get_val("CONV('".$dt."', 16, 10)") : $api->get_val("'".$dt."'");
								if($dt_val){
									$cur_vals[$cur_var."__dt"]=htmlspecialchars($dt_val);
									$cur_vals[$cur_var."__type"]="typed-literal";
								}
							}
						}
						$cur_vals[$cur_var]=$val;
					}
				}
				/* index */
				foreach($idxs as $cur_idx){
					$parts=(strpos($cur_idx, " ")) ? explode(" ", $cur_idx) : array($cur_idx);
					$cur_val_idx =& $val_idxs;
					for($i=0,$i_max=count($parts);$i<$i_max;$i++){
						$cur_part=$parts[$i];
						if($cur_part){
							$cur_part_alias=$cur_part;
							if(preg_match("/^([^:]+)\:(.+)$/", $cur_part, $matches)){
								$cur_part=$matches[1];
								$cur_part_alias=$matches[2];
							}
							if(!array_key_exists($cur_part_alias, $cur_val_idx)){
								$cur_val_idx[$cur_part_alias]=array();
							}
							$cur_val_idx =& $cur_val_idx[$cur_part_alias];
							if(isset($cur_vals[$cur_part])){
								$cur_key=rawurlencode($cur_vals[$cur_part]);
								$cur_key.=" type:".$cur_vals[$cur_part."__type"];
								if(isset($cur_vals[$cur_part."__lang"]) && ($cur_lang=$cur_vals[$cur_part."__lang"])){
									$cur_key.=" lang:".rawurlencode($cur_lang);
								}
								elseif(isset($cur_vals[$cur_part."__dt"]) && ($cur_dt=$cur_vals[$cur_part."__dt"])){
									$cur_key.=" datatype:".rawurlencode($cur_dt);
								}
								if(!array_key_exists($cur_key, $cur_val_idx)){
									$cur_val_idx[$cur_key]=array();
								}
								$cur_val_idx =& $cur_val_idx[$cur_key];
							}
						}
					}
				}
				$row=mysql_fetch_array($rs);
			} while ($row);
			/* extract result idxs from idxs array */
			$result_idxs=array();
			$result_idx="";
			foreach($idxs as $cur_idx){
				if(!$result_idx || (($cur_idx!=$result_idx) && (strpos($cur_idx, $result_idx)===0))){
					$result_idx=$cur_idx;
				}
				else{
					$result_idxs[]=$result_idx;
					$result_idx=$cur_idx;
				}
			}
			$result_idxs[]=$result_idx;
			/* generate json */
			for($i=0,$i_max=count($result_idxs);$i<$i_max;$i++){
				$cur_idx=$result_idxs[$i];
				$parts=(strpos($cur_idx, " ")) ? explode(" ", $cur_idx) : array($cur_idx);
				$sibling_parts=array();
				do {
					$has_sibling=false;
					if(($i<$i_max-1) && ($next_idx=$result_idxs[$i+1])){
						$next_parts=(strpos($next_idx, " ")) ? explode(" ", $next_idx) : array($next_idx);
						if($next_parts[0]==$parts[0]){
							$sibling_parts[]=$next_parts;
							$has_sibling=true;
							$i++;
						}
					}
				} while ($has_sibling);
				$code.=($i>0) ? "," : "";
				$code.=$this->get_jsoni_code(array("my_ind"=>$ind3, "parts"=>$parts, "sibling_parts"=>$sibling_parts, "val_idx"=>$val_idxs));
			}
		}
		return $code;
	}

	/*					*/	
	
	function get_jsoni_code($args=""){
		$ind="  ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}/* my_ind, parts, sibling_parts, val_idx */
		$code="";
		$sub_parts=array_slice($args["parts"], 1);
		$cur_part=$args["parts"][0];
		$cur_part_alias=$cur_part;
		if(preg_match("/^([^:]+)\:(.+)$/", $cur_part, $matches)){
			$cur_part=$matches[1];
			$cur_part_alias=$matches[2];
		}
		/* siblings */
		$sub_sibling_parts=array();
		$sibling_sibling_parts=array();
		if(isset($args["sibling_parts"])){
			foreach($args["sibling_parts"] as $cur_sibling_parts){
				$cur_sibling_sub_parts=array_slice($cur_sibling_parts, 1);
				if($sub_parts[0] && ($sub_parts[0]==$cur_sibling_sub_parts[0])){
					$sub_sibling_parts[]=$cur_sibling_sub_parts;
				}
				else{
					$sibling_sibling_parts[]=$cur_sibling_sub_parts;
				}
			}
		}
		if(count($sub_parts)){
			$code.=$nl.$my_ind.$cur_part_alias.': [';
			if($val_idx=$args["val_idx"][$cur_part_alias]){
				$added=false;
				foreach($val_idx as $k=>$v){
					$code.=($added)? "," : "";
					$code.=$nl.$my_ind.$ind.'{';
					$val_parts=explode(" ", $k);
					/* val */
					$code.=$nl.$my_ind.$ind2.'value: "'.rawurldecode($val_parts[0]).'"';
					/* type, lang, datatype */
					for($i=1,$i_max=count($val_parts);$i<$i_max;$i++){
						$val_part=$val_parts[$i];
						if(preg_match("/^([^\:]+)\:(.*)$/", $val_part, $matches)){
							$val_part_1=$matches[1];/* type|lang|datatype */
							$val_part_2=rawurldecode($matches[2]);
							$code.=",".$nl.$my_ind.$ind2.$val_part_1.': "'.$val_part_2.'"';
						}
					}
					/* child elements */
					if(strlen($sub_parts[0])){
						$code.=",".$nl.$my_ind.$ind2.'index: {';
						$code.=$this->get_jsoni_code(array("my_ind"=>$my_ind.$ind3, "parts"=>$sub_parts, "sibling_parts"=>$sub_sibling_parts, "val_idx"=>$val_idx[$k]));
						$code.=$nl.$my_ind.$ind2.'}';
					}
					/* sibling elements */
					foreach($sibling_sibling_parts as $cur_sibling_parts){
						$code.=",".$this->get_jsoni_code(array("my_ind"=>$my_ind.$ind2, "parts"=>$cur_sibling_parts, "val_idx"=>$val_idx[$k]));
					}
					$code.=$nl.$my_ind.$ind.'}';
					$added=true;
				}
			}
			$code.=$nl.$my_ind.']';
		}
		else{
			$code.=$nl.$my_ind.$cur_part_alias.': [';
			if(isset($val_idx[$cur_part_alias]) && ($val_idx=$val_idx[$cur_part_alias])){
				$added=false;
				foreach($val_idx as $k=>$v){
					$code.=($added)? "," : ""; 
					$val_parts=(strpos($k, " ")) ? explode(" ", $k) : array($k);
					/* val */
					$code.=$nl.$my_ind.$ind.'"'.rawurldecode($val_parts[0]).'"';
					$added=true;
				}
			}
			$code.=$nl.$my_ind.']';
		}
		return $code;
	}

	/*					*/

}

?>