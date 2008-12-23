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
sk:className                ARC_rdf_store_describe_handler
doap:name                   ARC RDF Store DESCRIBE query handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles SPARQL DESCRIBE queries 
//release
doap:created                2006-04-18
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-18: release 0.1.0
*/

class ARC_rdf_store_describe_handler {

	var $version="0.1.0";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
		
		$this->ns_prefixes=array();
		$this->ns_prefix_count=0;
	}
	
	function ARC_rdf_store_describe_handler(&$api){
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

	function get_default_ifp_iris(){
		if($api_helper =& $this->api->get_api_helper()){
			return $api_helper->get_default_ifp_iris();
		}
		return array();
	}

	/*					*/

	function decode_mysql_utf8_bugs($val=""){
		return $this->api->decode_mysql_utf8_bugs($val);
	}

	/*					*/
	
	function get_result($args=""){/* infos, result_type (rdfxml|n3|turtle|json|sql), result_type_args(jsonp, create_qnames) */
		$args["result_type"]=(isset($args["result_type"])) ? $args["result_type"] : "rdfxml";
		if(!isset($args["infos"])){
			return array("result"=>"", "error"=>"missing parameter 'infos'");
		}
		$result_type=$args["result_type"];
		/* result iris */
		$ids=array();
		if(isset($args["infos"]["result_iris"]) && is_array($args["infos"]["result_iris"])){
			foreach($args["infos"]["result_iris"] as $cur_iri){
				$ids[$cur_iri]=(strpos($cur_iri, "_:")===0) ? "bnode" : "iri";
			}
		}
		/* rewrite to sql */
		if(!$rewriter =& $this->api->get_sparql2sql_rewriter()){
			return $this->api->get_default_error();
		}
		/* retrieve var resource ids */
		$enc_vals=$this->config["encode_values"];
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		if(isset($args["infos"]["result_vars"]) && $args["infos"]["result_vars"]){
			$sql=$rewriter->get_sql($args);
			if(is_array($sql) && ($error=$sql["error"])){
				return $sql;
			}
			if($this->config["store_type"]!="basic"){
				$this->api->create_merge_tables();
			}
			if(!$rs=mysql_query($sql)){
				return array("result"=>"", "error"=>mysql_error());
			}
			/* retrieve ids */
			while($rs_row=mysql_fetch_array($rs)){
				foreach($rs_row as $col=>$val){
					if(isset($rs_row[$col."__type"]) && ($rs_row[$col."__type"] < 2)){/* bnode or iri */
						$val=($enc_vals) ? rawurldecode($val) : $val;
						$ids[$val]=($rs_row[$col."__type"]==1) ? "bnode" : "iri";
					}
				}
			}
			@mysql_free_result($rs);
		}
		/* sql to retrieve descriptions */
		$sql="";
		$id_infos=array();
		foreach($ids as $id=>$type){
			$cur_sql=$rewriter->get_describe_sql(array("result_iris"=>array($id)), 150);
			$id_infos[]=array("id"=>$id, "type"=>$type, "sql"=>$cur_sql);
			if($result_type=="sql"){
				$sql.=strlen($sql) ? ";\n\n" : "";
				$sql.=$cur_sql;
			}
		}
		/* sql */
		if($result_type=="sql"){
			return array("result"=>$sql, "error"=>"");
		}
		if($this->config["store_type"]!="basic"){
			$this->api->create_merge_tables();
		}
		/* code */
		$nl="\n";
		$code="";
		$bnodes=array();
		$described_bnodes=array();
		/* main descriptions */
		foreach($id_infos as $info){
			if($info["type"]=="bnode"){
				$described_bnodes[]=$info["id"];
				$bnodes[]=$info["id"];
			}
			if($rs=mysql_query($info["sql"])){
				$args["id_info"]=$info;
				$args["rs"] =& $rs;
				$sub_result=$this->get_resource_description($args);
				if($sub_code=$sub_result["result"]){
					$code.=(($result_type=="json") && strlen($code)) ? ",".$nl : ((strlen($code)) ? $nl : "");
					$code.=$sub_code;
				}
				foreach($sub_result["bnodes"] as $cur_bnode){
					if(!in_array($cur_bnode, $bnodes) && !in_array($cur_bnode, $described_bnodes)){
						$bnodes[]=$cur_bnode;
					}
				}
				@mysql_free_result($rs);
			}
		}
		/* bnodes linked from main descriptions */
		$args["ifp_iris"]=isset($args["ifp_iris"]) ? $args["ifp_iris"] : ((isset($this->config["ifp_iris"])) ? $this->config["ifp_iris"] : $this->get_default_ifp_iris());
		$args["compact_description"]=true;
		$level=0;
		$max_level=isset($args["max_describe_depth"]) ? $args["max_describe_depth"] : ((isset($this->config["max_describe_depth"])) ? $this->config["max_describe_depth"] : 3);
		while(($level < $max_level) && (count($bnodes) > count($described_bnodes))){
			foreach($bnodes as $cur_bnode){
				if(!in_array($cur_bnode, $described_bnodes)){
					$described_bnodes[]=$cur_bnode;
					if($rs=mysql_query($rewriter->get_describe_sql(array("result_iris"=>array($cur_bnode)), 50))){
						$args["id_info"]=array("id"=>$cur_bnode, "type"=>"bnode");
						$args["rs"] =& $rs;
						$sub_result=$this->get_resource_description($args);
						if($sub_code=$sub_result["result"]){
							$code.=(($result_type=="json") && strlen($code)) ? ",".$nl : ((strlen($code)) ? $nl : "");
							$code.=$sub_code;
						}
						foreach($sub_result["bnodes"] as $cur_bnode){
							if(!in_array($cur_bnode, $bnodes) && !in_array($cur_bnode, $described_bnodes)){
								$bnodes[]=$cur_bnode;
							}
						}
						@mysql_free_result($rs);
					}
				}
			}
			$level++;
		}
		/* result */
		$mthd="get_".$result_type."_result";
		if(method_exists($this, $mthd)){
			$args["code"]=$code;
			$sub_result=$this->$mthd($args);
			return array("result"=>$sub_result["result"], "error"=>$sub_result["error"]);
		}
		/* unsupported result type */
		return array(
			"result"=>false,
			"error"=>"Unsupported result type '".rawurlencode($result_type)."'",
			"headers"=>array()
		);
	}

	/*					*/
	
	function get_resource_description($args=""){/* id_info, rs, ifp_iris, result_type, result_type_args(jsonp, create_qnames), compact_description, fix_utf8 */
		$result_type=$args["result_type"];
		if(!in_array($result_type, array("rdfxml", "n3", "turtle", "json"))){
			return array("result"=>"", "error"=>"Unsupported result type '".rawurlencode($result_type)."'");
		}
		$ind=" ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";
		$enc_vals=$this->config["encode_values"];
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$compact_description=isset($args["compact_description"]) ? $args["compact_description"] : false;
		$ifp_iris=isset($args["ifp_iris"]) ? $args["ifp_iris"] : ((isset($this->config["ifp_iris"])) ? $this->config["ifp_iris"] : $this->get_default_ifp_iris());
		$bnodes=array();
		$id_info=$args["id_info"];
		$rs =& $args["rs"];
		/* s */
		$s_val=$id_info["id"];
		if($result_type=="rdfxml"){
			$result=$nl.$ind.'<rdf:Description';
			$result.=($id_info["type"]=="iri") ? ' rdf:about="'.htmlspecialchars($s_val).'"' : ' rdf:nodeID="'.substr($s_val, 2).'"';
			$result.='>';
		}
		elseif(in_array($result_type, array("n3", "turtle"))){
			$result=($id_info["type"]=="iri") ? $nl.'<'.htmlspecialchars($s_val).'>' : $nl.$s_val;
		}
		elseif($result_type=="json"){
			$result=$nl.$ind2.'{';
			$result.=$nl.$ind3.'id: ';
			$result.=($id_info["type"]=="iri") ? '"'.$s_val.'"' : '"'.substr($s_val, 2).'"';
			$result.=",".$nl.$ind3.'id_type: "'.$id_info["type"].'"';
			$result.=",".$nl.$ind3.'properties: {';
			$create_qnames=(isset($args["result_type_args"]["create_qnames"])) ? $args["result_type_args"]["create_qnames"] : false;
		}
		$po_code="";
		$prev_p_qname="";
		while($rs_row=mysql_fetch_array($rs)){
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
				}
			}
			/* p, o */
			$p_val=$row["p"];
			$p_qname=$this->get_qname($p_val);
			$o_val=$row["o"];
			$o_type=$row["o__type"];
			$o_val_lang=$row["o__lang"];
			$o_val_dt=($o_val_lang) ? "" : $row["o__dt"];
			/* rdfxml */
			if($result_type=="rdfxml"){
				$cur_po_code=$nl.$ind2.'<'.$p_qname;
				if($o_type==0){
					$cur_po_code.=' rdf:resource="'.htmlspecialchars($o_val).'"/>';
				}
				elseif($o_type=="1"){
					$cur_po_code.=' rdf:nodeID="'.substr($o_val, 2).'"/>';
					if(!in_array($o_val, $bnodes)){
						$bnodes[]=$o_val;
					}
				}
				else{
					$cur_po_code.=($o_val_lang) ? ' xml:lang="'.rawurlencode($o_val_lang).'"' : "";
					$cur_po_code.=($o_val_dt && ($o_val_dt!="http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral")) ? ' rdf:datatype="'.htmlspecialchars($o_val_dt).'"' : "";
					if((strpos(trim($o_val), "<")===0) || ($o_val_dt=="http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral")){
						$o_val=str_replace("&", "&amp;", $o_val);
						$cur_po_code.=' rdf:parseType="Literal">'.$o_val;
					}
					elseif(htmlspecialchars(str_replace('"', "", $o_val)) != str_replace('"', "", $o_val)){/* <, >, & */
						$cur_po_code.='><![CDATA['.$o_val.']]>';
					}
					else{
						$cur_po_code.='>'.$o_val;
					}
					$cur_po_code.='</'.$p_qname.'>';
				}
				if($compact_description && in_array($p_val, $ifp_iris)){
					$po_code=$cur_po_code;
					break;
				}
				else{
					$po_code.=$cur_po_code;
				}
			}
			/* turtle, n3 */
			elseif(in_array($result_type, array("n3", "turtle"))){
				$cur_p_code="";
				$cur_o_code="";
				if($prev_p_qname!=$p_qname){
					if($prev_p_qname){/* close triple */
						$cur_p_code.=' ;';
					}
					$cur_p_code.=$nl.$ind.$p_qname;
				}
				else{
					$cur_o_code.=$nl.$ind;
					for($i=0;$i<strlen($p_qname);$i++){
						$cur_o_code.=" ";
					}
					$cur_o_code.=" ";
				}
				$cur_p_code.=($prev_p_qname==$p_qname) ? "," : "";
				if($o_type==0){
					$cur_o_code.='<'.$o_val.'>';
				}
				elseif($o_type=="1"){
					$cur_o_code.=$o_val;
					if(!in_array($o_val, $bnodes)){
						$bnodes[]=$o_val;
					}
				}
				else{
					$o_val=$this->api->escape_js_string($o_val);
					$cur_o_code.='"'.$o_val.'"';
					if($o_val_lang){
						$cur_o_code.='@'.rawurlencode($cur_lang);
					}
					elseif($o_val_dt){
						$cur_o_code.='^^<'.$o_val_dt.'>';
					}
				}
				if($compact_description && in_array($p_val, $ifp_iris)){
					$cur_po_code=" ".$p_qname." ".$cur_o_code;
					$po_code=$cur_po_code;
					break;
				}
				else{
					$po_code.=$cur_p_code." ".$cur_o_code;
				}
				$prev_p_qname=$p_qname;
			}
			elseif($result_type=="json"){
				$cur_p_code="";
				$cur_o_code="";
				$p_qname=($create_qnames) ? $this->get_qname($p_val) : $p_val;
				if($prev_p_qname!=$p_qname){
					if($prev_p_qname){/* close triple */
						$cur_p_code.=$nl.$ind4.'],';
					}
					$cur_p_code.=$nl.$ind4.'"'.$p_qname.'": [';
				}
				$cur_o_code.=($prev_p_qname==$p_qname) ? "," : "";
				$cur_o_code.=$nl.$ind4.$ind.'{';
				if($o_type==0){
					$cur_o_code.=$nl.$ind4.$ind2.'type: "iri"';
					$cur_o_code.=",".$nl.$ind4.$ind2.'value: "'.$o_val.'"';
				}
				elseif($o_type=="1"){
					$cur_o_code.=$nl.$ind4.$ind2.'type: "bnode"';
					$cur_o_code.=",".$nl.$ind4.$ind2.'value: "'.substr($o_val, 2).'"';
					if(!in_array($o_val, $bnodes)){
						$bnodes[]=$o_val;
					}
				}
				else{
					$cur_o_code.=($o_val_dt) ? $nl.$ind4.$ind2.'type: "typed-literal"' : $nl.$ind4.$ind2.'type: "literal"';
					$cur_o_code.=",".$nl.$ind4.$ind2.'value: "'.$this->api->escape_js_string($o_val).'"';
					if($o_val_lang){
						$cur_o_code.=",".$nl.$ind4.$ind2.'lang: "'.rawurlencode($o_val_lang).'"';
					}
					elseif($o_val_dt){
						$cur_o_code.=",".$nl.$ind4.$ind2.'datatype: "'.$o_val_dt.'"';
					}
				}
				$cur_o_code.=$nl.$ind4.$ind.'}';
				if($compact_description && in_array($p_val, $ifp_iris)){
					$cur_po_code=$nl.$ind4.$ind.'"'.$p_qname.'": [';
					$cur_po_code.=$cur_o_code;
					$po_code=$cur_po_code;
					break;
				}
				else{
					$po_code.=$cur_p_code.$cur_o_code;
				}
				$prev_p_qname=$p_qname;
			}
		}
		/* close description */
		if($result_type=="rdfxml"){
			$result=($po_code) ? $result.$po_code.$nl.$ind.'</rdf:Description>' : "";
		}
		elseif(in_array($result_type, array("n3", "turtle"))){
			$result=($po_code) ? $result.$po_code.' .' : "";
		}
		elseif($result_type=="json"){
			if($po_code){
				$result.=$po_code;
				$result.=$nl.$ind3.$ind.']';
			}
			$result.=$nl.$ind3.'}';
			$result.=$nl.$ind2.'}';
		}
		return array("result"=>$result, "bnodes"=>$bnodes, "error"=>"");
	}
	
	/*					*/

	function get_rdfxml_result($args=""){
		$nl="\n";
		$ind=" ";
		$result=''.
			'<?xml version="1.0" encoding="UTF-8"?>'.
			$nl.'<rdf:RDF'.
			$nl.$ind.'xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"';
		'';
		foreach($this->ns_prefixes as $iri=>$prefix){
			$result.=$nl.$ind.'xmlns:'.$prefix.'="'.$iri.'"';
		}
		$result.='>'.$nl.$args["code"].$nl.'</rdf:RDF>'.$nl;
		return array("result"=>$result, "error"=>"");
	}

	function get_turtle_result($args=""){
		$nl="\n";
		$result=''.
			$nl.'@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .';
		'';
		foreach($this->ns_prefixes as $iri=>$prefix){
			$result.=$nl.'@prefix '.$prefix.': <'.$iri.'> .';
		}
		$result.=$nl.$args["code"].$nl;
		return array("result"=>$result, "error"=>"");
	}
	
	function get_n3_result($args=""){
		return $this->get_turtle_result($args);
	}
	
	function get_json_result($args=""){
		$nl="\n";
		$ind=" ";
		$result=''.
			'{'.
			$nl.$ind.'prefixes: {'.
			$nl.$ind.$ind.'rdf: "http://www.w3.org/1999/02/22-rdf-syntax-ns#"';
		'';
		foreach($this->ns_prefixes as $iri=>$prefix){
			$result.=",".$nl.$ind.$ind.$prefix.': "'.$iri.'"';
		}
		$result.=''.
			$nl.$ind.'}'.
			','.$nl.$ind.'resources: ['.
			$nl.$args["code"].
			$nl.$ind.']'.
			$nl.'}'.
		'';
		$result=(isset($args["result_type_args"]["jsonp"]) && ($jsonp=$args["result_type_args"]["jsonp"])) ? $jsonp.'('.$result.')' : $result;
		return array("result"=>$result, "error"=>"");
	}

	/*					*/

}

?>