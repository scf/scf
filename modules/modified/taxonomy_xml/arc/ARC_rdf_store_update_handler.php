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
sk:className                ARC_rdf_store_update_handler
doap:name                   ARC RDF Store UPDATE query handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles UPDATE queries 
//release
doap:created                2006-04-14
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-14: release 0.1.0
*/

class ARC_rdf_store_update_handler {

	var $version="0.1.0";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_update_handler(&$api){
		$this->__construct($api);
	}

	/*					*/

	function get_result($args=""){/* graph_iri, add_rdfxml, add_triple, preserve_node_ids, proxy_host, proxy_port, insert_timeout, log_inserts, result_type (plain|array|json|xml), result_type_args (jsonp) */
		$result_type=(isset($args["result_type"])) ? $args["result_type"] : "array";
		$args["result_type"]="array";
		$del_result=$this->api->delete_data($args);
		$add_result=$this->api->add_data($args);
		/* result */
		$args["result_type"]=$result_type;
		$mthd="get_".$args["result_type"]."_result";
		if(method_exists($this, $mthd)){
			$sub_result=$this->$mthd(array("del_result"=>$del_result, "add_result"=>$add_result, "result_type_args"=>isset($args["result_type_args"]) ? $args["result_type_args"] : array()));
			return array("result"=>$sub_result["result"], "error"=>$sub_result["error"]);
		}
		return array("result"=>"", "error"=>"Unsupported result type '".$args["result_type"]."'", "add_result"=>$add_result, "del_result"=>$del_result);
	}
		
	/*					*/

	function get_array_result($args=""){
		/* del */
		$del_args=$args["del_result"];
		$del_error=$del_args["error"];/* string */
		$del_result=$del_args["result"];/* empty string or array */
		/* add */
		$add_args=$args["add_result"];
		$add_error=$add_args["error"];/* string */
		$add_result=$add_args["result"];/* empty string or array */
		/* error & result */
		$error=$del_error;
		$error.=$add_error ? ",\n".$add_error : "";
		$result=$del_result ? $del_result : array();
		$result=$add_result ? array_merge($result, $add_result) : $result;
		return array("result"=>$result, "error"=>$error);
	}
	
	function get_plain_result($args=""){
		/* del */
		$del_args=$args["del_result"];
		$del_error=$del_args["error"];/* string */
		$del_result=$del_args["result"];/* empty string or array */
		/* add */
		$add_args=$args["add_result"];
		$add_error=$add_args["error"];/* string */
		$add_result=$add_args["result"];/* empty string or array */
		/* error & result */
		$nl="\n";
		$error=$del_error;
		$error.=$add_error ? ",\n".$add_error : "";
		$result=$del_result;
		$result.=($add_result) ? $nl.$add_result : "";
		return array("result"=>$result, "error"=>$error);
	}
	
	function get_xml_result($args=""){
		/* del */
		$del_args=$args["del_result"];
		$del_error=$del_args["error"];/* string */
		$del_result=$del_args["result"];/* empty string or array */
		/* add */
		$add_args=$args["add_result"];
		$add_error=$add_args["error"];/* string */
		$add_result=$add_args["result"];/* empty string or array */
		/* error & result */
		$nl="\n";
		$ind="  ";
		$error=$del_error;
		$error.=$add_error ? ",\n".$add_error : "";
		$result="";
		$code='<?xml version="1.0" ?>';
		if(!$error){
			$code.=$nl.'<result>';
			if(!$del_error){
				$code.=$nl.$ind.'<del_row_count>'.$del_result["del_row_count"].'</del_row_count>';
				$code.=$nl.$ind.'<del_time>'.$del_result["del_time"].'</del_time>';
			}
			if(!$add_error){
				$code.=$nl.$ind.'<add_triple_count>'.$add_result["add_triple_count"].'</add_triple_count>';
				$code.=$nl.$ind.'<add_load_time>'.$add_result["add_load_time"].'</add_load_time>';
				if($logs=$add_result["insert_logs"]){
					$code.=$nl.$ind.'<insert_logs>';
					foreach($logs as $cur_log){
						$code.=$nl.$ind.$ind.'<log>'.htmlspecialchars($cur_log).'</log>';
					}
					$code.=$nl.$ind.'</insert_logs>';
				}
			}
			$code.=$nl.'</result>';
		}
		else{
			$code.=$nl.'<error>'.htmlspecialchars($error).'</error>';
		}
		return array("result"=>$code, "error"=>$error);
	}

	function get_json_result($args=""){
		/* del */
		$del_args=$args["del_result"];
		$del_error=$del_args["error"];/* string */
		$del_result=$del_args["result"];/* empty string or array */
		/* add */
		$add_args=$args["add_result"];
		$add_error=$add_args["error"];/* string */
		$add_result=$add_args["result"];/* empty string or array */
		/* error & result */
		$nl="\n";
		$ind="  ";
		$error=$del_error;
		$error.=$add_error ? ",\n".$add_error : "";
		$result="";
		$jsonp=(isset($args["result_type_args"]) && isset($args["result_type_args"]["jsonp"])) ? $args["result_type_args"]["jsonp"] : "";
		$code='{';
		if(!$del_error){
			$code.=$nl.$ind.'del_row_count: '.$del_result["del_row_count"];
			$code.=",".$nl.$ind.'del_time: '.$del_result["del_time"];
		}
		if(!$add_error){
			$code.=strlen($code) ? "," : "";
			$code.=$nl.$ind.'add_triple_count: '.$add_result["add_triple_count"];
			$code.=",".$nl.$ind.'add_load_time: '.$add_result["add_load_time"];
			if($logs=$add_result["insert_logs"]){
				$code.=",".$nl.$ind.'insert_logs: [';
				$log_code="";
				foreach($logs as $cur_log){
					$log_code.=(strlen($log_code)) ? "," : "";
					$log_code.=$nl.$ind.$ind.'"'.$this->api->escape_js_string($cur_log).'"';
				}
				$code.=$log_code;
				$code.=$nl.$ind.']';
			}
		}
		if($error){
			$code.=strlen($code) ? "," : "";
			$code.=$nl.$ind.'error: "'.$this->api->escape_js_string($error).'"';
		}
		$code.=$nl.'}';
		$code=($jsonp) ? $jsonp."(".$code.")" : $code;
		return array("result"=>$code, "error"=>$error);
	}

	/*					*/

}

?>