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
sk:className                ARC_rdf_store_select_handler
doap:name                   ARC RDF Store SELECT query handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles SPARQL SELECT queries 
//release
doap:created                2006-10-24
doap:revision               0.1.2
//changelog
sk:releaseChanges           2006-04-04: release 0.1.0
                            2006-05-23: revsion 0.1.1
                                        - added "xml" as default result_type
                            2006-10-24: revision 0.1.2
                                        - minor tweaks (getting rid of php notices)
*/

class ARC_rdf_store_select_handler {

	var $version="0.1.2";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_select_handler(&$api){
		$this->__construct($api);
	}

	/*					*/

	function decode_mysql_utf8_bugs($val=""){
		return $this->api->decode_mysql_utf8_bugs($val);
	}

	/*					*/
	
	function get_result($args=""){/* infos, result_type (rows|json|xml|single|rows_n_count|row_count|sql), result_type_args(jsonp, count_rows), obj_props, dt_props, iri_alts, fix_utf8 */
		if(!isset($args["infos"])){
			return array("result"=>"", "error"=>"missing parameter 'infos'");
		}
		if(!isset($args["result_type"]) || !$args["result_type"]){
			$args["result_type"]="xml";
			//return array("result"=>"", "error"=>"missing parameter 'result_type'");
		}
		$result_type=$args["result_type"];
		/* adjust infos */
		if($result_type=="rows_count"){
			$args["infos"]["limit"]=1;
		}
		if(in_array($result_type, array("rows_n_count", "row_count"))){
			$args["infos"]["count_rows"]=true;
		}
		/* rewrite to sql */
		if(!$rewriter=$this->api->get_sparql2sql_rewriter()){
			return $this->api->get_default_error();
		}
		$sql=$rewriter->get_sql($args);
		if($result_type=="row_count"){
			/* remove aliases */
			$sql=preg_replace("/(\s+)(AS\s+[a-z0-9_]+)([^a-z0-9_])/is", "\\1\\3", $sql);
			/* remove id2val joins */
			while(($new_sql=preg_replace("/V([0-9_]+)\.val(.*FROM.*)(LEFT JOIN [^\(]+\(V\\1\.id=)(T[0-9]+\.[a-z]+)\s*\)\s*/s", "\\4\\2", $sql)) && ($new_sql!=$sql)){
			 	$sql=$new_sql;
			}
			/* replace CONV expressions in result vars block */
			$sql=preg_replace("/CONV\(([^,]+),[^\)]+\)(.*FROM)/s", "\\1\\2", $sql);
		}
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
		if(!$rs=@mysql_query($sql)){
			return array("result"=>"", "error"=>mysql_error());
		}
		$t2=$this->api->get_mtime();
		$dur=$t2-$t1;
		/* row_count */
		$row_count=mysql_num_rows($rs);
		if(isset($args["infos"]["count_rows"]) && $args["infos"]["count_rows"]){
			$sub_row=mysql_fetch_array(mysql_query("SELECT FOUND_ROWS() AS row_count"));
			$row_count=$sub_row["row_count"];
		}
		if($result_type=="row_count"){
			return array("result"=>$row_count, "error"=>"", "query_time"=>$dur);
		}
		/* first row */
		$enc_vals = isset($this->config["encode_values"]) ? $this->config["encode_values"] : false;
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$cols=array();
		$row=array();
		if($rs_row=mysql_fetch_array($rs)){
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
		}
		/* single */
		if($result_type=="single"){
			return array("result"=>$row, "row_count"=>$row_count, "error"=>"", "query_time"=>$dur);
		}
		/* json */
		if($result_type=="json"){
			$handler =& $this->api->get_query_sub_handler("select_json");
			$args["row"]=$row;
			$args["rs"] =& $rs;
			$args["cols"]=$cols;
			$sub_result=$handler->get_json_result($args);
			@mysql_free_result($rs);
			return array(
				"result"=>$sub_result["result"],
				"error"=>$sub_result["error"],
				"headers"=>isset($sub_result["headers"]) ? $sub_result["headers"] : array(),
				"row_count"=>$row_count,
				"query_time"=>$dur
			);
		}
		/* any other result type */
		$mthd="get_".$result_type."_result";
		if(method_exists($this, $mthd)){
			$args["row"]=$row;
			$args["rs"] =& $rs;
			$args["cols"]=$cols;
			$sub_result=$this->$mthd($args);
			@mysql_free_result($rs);
			return array(
				"result"=>$sub_result["result"],
				"error"=>$sub_result["error"],
				"headers"=>isset($sub_result["headers"]) ? $sub_result["headers"] : array(),
				"row_count"=>$row_count,
				"query_time"=>$dur
			);
		}
		@mysql_free_result($rs);
		/* unsupported result type */
		return array(
			"result"=>false,
			"error"=>"unknown or unsupported result type '".rawurlencode($result_type)."'",
			"headers"=>array(),
			"row_count"=>$row_count,
			"query_time"=>$dur
		);
	}

	/*					*/

	function get_rows_result($args=""){/* row, rs, cols, infos, fix_utf8 */
		if(!count($args["row"])){
			return array("result"=>array(), "error"=>"");
		}
		$enc_vals = isset($this->config["encode_values"]) ? $this->config["encode_values"] : false;
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$api =& $this->api;
		$api_helper =& $this->api->get_api_helper();
		$rows=array($args["row"]);
		$cols=$args["cols"];
		$rs =& $args["rs"];
		while($rs_row=mysql_fetch_array($rs)){
			$row=array();
			foreach($cols as $col){
				if(isset($rs_row[$col])){
					if(strpos($col, "__dt")!==false){
						$row[$col]=($conv_id) ? $api->get_val("CONV('".$rs_row[$col]."', 16, 10)") : $api->get_val("'".$rs_row[$col]."'");
					}
					elseif(isset($rs_row[$col."__type"]) && ($rs_row[$col."__type"]>1)){/* literal */
						$val=($enc_vals) ? rawurldecode($rs_row[$col]) : $rs_row[$col];
						$val=$this->decode_mysql_utf8_bugs($val);
						$row[$col]= ($fix_utf8) ? $api_helper->adjust_utf8_string($val) : $val;
					}
					else{
						$row[$col]=($enc_vals) ? rawurldecode($rs_row[$col]) : $rs_row[$col];
					}
				}
			}
			$rows[]=$row;
		}
		return array("result"=>$rows, "error"=>"");
	}

	/*					*/

	function get_rows_n_count_result($args=""){
		return $this->get_rows_result($args);
	}

	/*					*/

	function get_xml_result($args=""){/* row, rs, cols, infos */
		$ind="  ";
		$ind2=$ind.$ind;
		$ind3=$ind2.$ind;
		$ind4=$ind3.$ind;
		$nl="\n";
		$enc_vals = (isset($this->config["encode_values"]) && $this->config["encode_values"]) ? true : false;
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		$fix_utf8=(isset($args["fix_utf8"])) ? $args["fix_utf8"] : false;
		$api =& $this->api;
		$api_helper =& $this->api->get_api_helper();
		$code='<?xml version="1.0" ?>'.$nl;
		$code.='<sparql xmlns="http://www.w3.org/2005/sparql-results#">'.$nl;
		/* head */
		$vars=$args["infos"]["result_vars"];
		$code.=$ind.'<head>'.$nl;
		foreach($vars as $cur_var){
			$code.=$ind.$ind.'<variable name="'.$cur_var.'"/>'.$nl;
		}
		$code.=$ind.'</head>'.$nl;
		/* results */
		$code.=$ind.'<results';
		$code.=(isset($args["infos"]["distinct"]) && $args["infos"]["distinct"]) ? ' distinct="true"' : ' distinct="false"';
		$code.=(isset($args["infos"]["order_conditions"]) && $args["infos"]["order_conditions"]) ? ' ordered="true"' : ' ordered="false"';
		$code.='>'.$nl;
		if(isset($args["row"]) && ($row=$args["row"])){
			$rs =& $args["rs"];
			do {
				$code.=$ind2.'<result>'.$nl;
				foreach($vars as $cur_var){
					if(isset($row[$cur_var]) && $row[$cur_var]){
						$val=($enc_vals) ? rawurldecode($row[$cur_var]) : $row[$cur_var];
						$code.=$ind3.'<binding name="'.$cur_var.'">'.$nl;
						if(!isset($row[$cur_var."__type"]) || ($row[$cur_var."__type"]==0)){/* iri */
							$code.=$ind4.'<uri>'.htmlspecialchars($val).'</uri>'.$nl;
						}
						elseif($row[$cur_var."__type"]==1){
							$code.=$ind4.'<bnode>'.substr($val, 2).'</bnode>'.$nl;
						}
						else{
							$code.=$ind4.'<literal';
							if(isset($row[$cur_var."__lang"]) && ($lang=$row[$cur_var."__lang"])){
								$code.=' xml:lang="'.$lang.'"';
							}
							elseif(isset($row[$cur_var."__dt"]) && ($dt=$row[$cur_var."__dt"])){
								$dt_val=($conv_id) ? $api->get_val("CONV('".$dt."', 16, 10)") : $api->get_val("'".$dt."'");
								$code.=($dt_val) ? ' datatype="'.htmlspecialchars($dt_val).'"' : "";
							}
							$val=$this->decode_mysql_utf8_bugs($val);
							$val=($fix_utf8) ? $api_helper->adjust_utf8_string($val) : $val;
							$code.='>'.htmlspecialchars($val).'</literal>'.$nl;
						}
						$code.=$ind3.'</binding>'.$nl;
					}
				}
				$code.=$ind2.'</result>'.$nl;
				$row=mysql_fetch_array($rs);
			} while ($row);
		}
		$code.=$ind.'</results>'.$nl;
		$code.='</sparql>'.$nl;
		return array("result"=>$code, "error"=>"");
	}
	
	/*					*/

}
?>