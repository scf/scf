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
sk:className                ARC_rdf_store_construct_handler
doap:name                   ARC RDF Store CONSTRUCT query handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles SPARQL CONSTRUCT queries 
//release
doap:created                2006-04-19
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-19: release 0.1.0
*/

class ARC_rdf_store_construct_handler {

	var $version="0.1.0";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();

		$this->ns_prefixes=array();
		$this->ns_prefix_count=0;
	}
	
	function ARC_rdf_store_construct_handler(&$api){
		$this->__construct($api);
	}

	/*					*/
	
	function get_qname($iri=""){
		if(preg_match("/(.*[\/|#|\:])([^\/|#|\:]+)/", $iri, $matches)){
			$ns_iri=$matches[1];
			$local_name=$matches[2];
			if($ns_iri=="http://www.w3.org/1999/02/22-rdf-syntax-ns#"){
				return "rdf:".$local_name;
			}
			if(!array_key_exists($ns_iri, $this->ns_prefixes)){
				$this->ns_prefixes[$ns_iri]="ns".$this->ns_prefix_count;
				$this->ns_prefix_count++;
			}
			return $this->ns_prefixes[$ns_iri].":".$local_name;
		}
		return false;
	}

	/*					*/
	
	function decode_mysql_utf8_bugs($val=""){
		return $this->api->decode_mysql_utf8_bugs($val);
	}
	
	/*					*/
	
	function get_result($args=""){/* infos, result_type (rdfxml|n3|turtle|sql), result_type_args() */
		if(!isset($args["infos"])){
			return array("result"=>"", "error"=>"missing parameter 'infos'");
		}
		$args["result_type"]=(isset($args["result_type"])) ? $args["result_type"] : "rdfxml";
		$result_type=$args["result_type"];
		/* rewrite to sql */
		if(!$rewriter=$this->api->get_sparql2sql_rewriter()){
			return $this->api->get_default_error();
		}
		$sql=$rewriter->get_sql($args);
		if(is_array($sql) && ($error=$sql["error"])){
			return $sql;
		}
		if($result_type=="sql"){
			return array("result"=>$sql, "error"=>"");
		}
		/* run query */
		$t1=$this->api->get_mtime();
		if($this->config["store_type"]!="basic"){
			$this->api->create_merge_tables();
		}
		if(!$rs=mysql_query($sql)){
			return array("result"=>"", "error"=>mysql_error());
		}
		$t2=$this->api->get_mtime();
		$dur=$t2-$t1;
		/* result */
		$mthd="get_".$result_type."_result";
		if(method_exists($this, $mthd)){
			$args["rs"] =& $rs;
			$sub_result=$this->$mthd($args);
			@mysql_free_result($rs);
			return array(
				"result"=>$sub_result["result"],
				"error"=>$sub_result["error"],
				"headers"=>isset($sub_result["headers"]) ? $sub_result["headers"] : array(),
				"query_time"=>$dur
			);
		}
		@mysql_free_result($rs);
		/* unsupported result type */
		return array(
			"result"=>false,
			"error"=>"Unsupported result type '".rawurlencode($result_type)."'",
			"headers"=>array(),
			"query_time"=>$dur
		);
	}

	/*					*/
	
	function get_rdfxml_result($args=""){
		$ind=" ";
		$nl="\n";
		$code="";
		if(!isset($args["infos"]["template_triples"]["triples"])){
			return array("result"=>"", "error"=>"Could not construct graph.");
		}
		$tts=$args["infos"]["template_triples"]["triples"];
		$rs =& $args["rs"]; 
		$enc_vals=$this->config["encode_values"];
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$row_count=0;
		while($rs_row=mysql_fetch_array($rs)){
			$row_count++;
			$row=array();
			foreach($rs_row as $col=>$val){
				if(!is_numeric($col)){
					if(strpos($col, "__dt")!==false){
						$row[$col]=($conv_id) ? $this->api->get_val("CONV('".$val."', 16, 10)") : $this->api->get_val("'".$val."'");
					}
					elseif(isset($rs_row[$col."__type"]) && ($rs_row[$col."__type"]>1)){/* literal */
						$val=($enc_vals) ? rawurldecode($val) : $val;
						$val=$this->decode_mysql_utf8_bugs($val);
						$row[$col]=($fix_utf8) ? $this->api->adjust_utf8_string($val) : $val;
					}
					else{
						$row[$col]=($enc_vals) ? rawurldecode($val) : $val;
					}
					$cols[]=$col;
				}
			}
			$prev_s_type="";
			$prev_s_val="";
			foreach($tts as $tt){
				/* s */
				$s=$tt["s"];
				$s_type=$s["type"];
				if($s_type=="var"){
					$s_val=$row[$s["val"]];
					$s_val_type=$row[$s["val"]."__type"];
				}
				elseif($s_type=="bnode"){
					$s_val=$s["val"]."_".$row_count;
					$s_val_type="bnode";
				}
				else{
					$s_val=$s["val"];
					$s_val_type=$s_type;
				}
				if(($s_type!=$prev_s_type) || ($s_val!=$prev_s_val)){/* new s */
					$code.=($prev_s_type) ? $nl.$ind.'</rdf:Description>' : "";
					$prev_s_type=$s_type;
					$prev_s_val=$s_val;
					$code.=(strlen($code)) ? $nl.$nl.$ind.'<rdf:Description' : $ind.'<rdf:Description';
					$code.=(($s_type=="iri") || ($s_val_type==0)) ? ' rdf:about="'.$s_val.'"' : ' rdf:nodeID="'.substr($s_val, 2).'"';
					$code.='>';
				}
				/* p, o */
				$p=$tt["p"];
				$p_type=$p["type"];
				$p_val=($p_type=="var") ? $row[$p["val"]] : $p["val"];
				$o=$tt["o"];
				$o_type=$o["type"];
				if($o_type=="var"){
					$o_val=$row[$o["val"]];
					$o_val_lang=(isset($row[$o["val"]."__lang"])) ? $row[$o["val"]."__lang"] : "";
					$o_val_dt=(!$o_val_lang && isset($row[$o["val"]."__dt"])) ? $row[$o["val"]."__dt"] : "";
					$o_val_type=$row[$o["val"]."__type"];
				}
				elseif($o_type=="bnode"){
					$o_val=$o["val"]."_".$row_count;
					$o_val_type="bnode";
				}
				else{
					$o_val=$o["val"];
					$o_val_lang=isset($o["lang"]) ? $o["lang"] : "";
					$o_val_dt=(!$o_val_lang && isset($o["dt"])) ? $o["dt"] : "";
					$o_val_type=$o_type;
				}
				$p_qname=$this->get_qname($p_val);
				$code.=$nl.$ind.$ind.'<'.$p_qname;
				if(in_array($o_val_type, array("iri", "0"))){
					$code.=' rdf:resource="'.$o_val.'"/>';
				}
				elseif(in_array($o_val_type, array("bnode", 1))){
					$code.=' rdf:nodeID="'.substr($o_val, 2).'"/>';
				}
				else{/* literal */
					if($o_val_lang){
						$code.=' xml:lang="'.rawurlencode($o_val_lang).'"';
					}
					elseif($o_val_dt && ($o_val_dt!="http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral")){
						$code.=' rdf:datatype="'.htmlspecialchars($o_val_dt).'"';
					}
					if((strpos(trim($o_val), "<")===0) || ($o_val_dt=="http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral")){
						$code.=' rdf:parseType="Literal">'.$o_val;
					}
					elseif(htmlspecialchars(str_replace('"', "", $o_val)) != str_replace('"', "", $o_val)){/* <, >, & */
						$code.='><![CDATA['.$o_val.']]>';
					}
					else{
						$code.='>'.$o_val;
					}
					$code.='</'.$p_qname.'>';
				}
			}
			$code.=$nl.$ind.'</rdf:Description>';
		}
		/* rdf doc */
		$result=''.
			'<?xml version="1.0" encoding="UTF-8"?>'.
			$nl.'<rdf:RDF'.
			$nl.$ind.'xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"';
		'';
		foreach($this->ns_prefixes as $iri=>$prefix){
			$result.=$nl.$ind.'xmlns:'.$prefix.'="'.$iri.'"';
		}
		$result.='>'.$nl.$nl.$code.$nl.'</rdf:RDF>'.$nl;
		return array("result"=>$result, "error"=>"");
	}

	function get_turtle_result($args=""){
		$ind=" ";
		$nl="\n";
		$code="";
		if(!isset($args["infos"]["template_triples"]["triples"])){
			return array("result"=>"", "error"=>"Could not construct graph.");
		}
		$tts=$args["infos"]["template_triples"]["triples"];
		$rs =& $args["rs"]; 
		$enc_vals=$this->config["encode_values"];
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$row_count=0;
		while($rs_row=mysql_fetch_array($rs)){
			$row_count++;
			$row=array();
			foreach($rs_row as $col=>$val){
				if(!is_numeric($col)){
					if(strpos($col, "__dt")!==false){
						$row[$col]=($conv_id) ? $this->api->get_val("CONV('".$val."', 16, 10)") : $this->api->get_val("'".$val."'");
					}
					elseif(isset($rs_row[$col."__type"]) && ($rs_row[$col."__type"]>1)){/* literal */
						$val=($enc_vals) ? rawurldecode($val) : $val;
						$val=$this->decode_mysql_utf8_bugs($val);
						$row[$col]=($fix_utf8) ? $this->api->adjust_utf8_string($val) : $val;
					}
					else{
						$row[$col]=($enc_vals) ? rawurldecode($val) : $val;
					}
					$cols[]=$col;
				}
			}
			foreach($tts as $tt){
				$cur_code=(strlen($code)) ? $nl : "";
				foreach(array("s", "p", "o") as $cur_part){
					$cur_term=$tt[$cur_part];
					$cur_type=$cur_term["type"];
					if($cur_type=="iri"){
						$cur_code.='<'.$cur_term["val"].'>';
					}
					elseif($cur_type=="var"){
						$cur_val=$row[$cur_term["val"]];
						$cur_val_type=isset($row[$cur_term["val"]."__type"]) ? $row[$cur_term["val"]."__type"] : "iri";
						if(in_array($cur_val_type, array("iri", "0"))){
							$cur_code.='<'.$cur_val.'>';
						}
						elseif($cur_val_type==1){/* bnode */
							$cur_code.=$cur_val;
						}
						else{/* literal */
							$cur_code.='"'.$this->api->escape_js_string($cur_val).'"';
							if(isset($row[$cur_term["val"]."__lang"]) && ($cur_lang=$row[$cur_term["val"]."__lang"])){
								$cur_code.='@'.rawurlencode($cur_lang);
							}
							elseif(isset($row[$cur_term["val"]."__dt"]) && ($cur_dt=$row[$cur_term["val"]."__dt"])){
								$cur_code.='^^<'.$cur_dt.'>';
							}
						}
					}
					elseif($cur_type=="bnode"){
						$cur_code.=$cur_term["val"]."_".$row_count;
					}
					elseif(in_array($cur_type, array("literal", "numeric"))){
						$cur_code.=$cur_term["delim_code"].$cur_term["val"].$cur_term["delim_code"];
					}
					else{
						$cur_code.='"unknown template part ('.$cur_type.')"';
					}
					$cur_code.=($cur_part=="o") ? " ." : " ";
				}
				$code.=$cur_code;
			}
			$code.=$nl;
		}
		return array("result"=>$code, "error"=>"");
	}

	function get_n3_result($args=""){
		return $this->get_turtle_result($args);
	}
	
	/*					*/

}

?>