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
doap:shortdesc              This class handles SPARQL ASK queries 
//release
doap:created                2006-04-19
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-19: release 0.1.0
*/

class ARC_rdf_store_ask_handler {

	var $version="0.1.0";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_ask_handler(&$api){
		$this->__construct($api);
	}

	/*					*/

	function get_result($args=""){/* infos, result_type (xml|json|sql|plain|bool), result_type_args() */
		if(!isset($args["infos"])){
			return array("result"=>"", "error"=>"missing parameter 'infos'");
		}
		$args["result_type"]=(isset($args["result_type"])) ? $args["result_type"] : "xml";
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

	function get_plain_result($args=""){
		$rs = (isset($args["rs"])) ? $args["rs"] : false;
		$result=($rs && ($row=mysql_fetch_array($rs)) && $row["success"]) ? "1" : "0";
		return array("result"=>$result, "error"=>"");
	}

	function get_bool_result($args=""){
		$rs = (isset($args["rs"])) ? $args["rs"] : false;
		$result=($rs && ($row=mysql_fetch_array($rs)) && $row["success"]) ? true : false;
		return array("result"=>$result, "error"=>"");
	}

	function get_xml_result($args=""){
		$nl="\n";
		$ind=" ";
		$rs = (isset($args["rs"])) ? $args["rs"] : false;
		$code=''.
			'<?xml version="1.0" ?>'.
			$nl.'<sparql xmlns="http://www.w3.org/2005/sparql-results#">'.
			$nl.$ind.'<head>'.
			$nl.$ind.'</head>'.
			$nl.$ind.'<boolean>'.
		'';
		$code.=($rs && ($row=mysql_fetch_array($rs)) && $row["success"]) ? "true" : "false";
		$code.=''.
			'</boolean>'.
			$nl.'</sparql>'.
		'';
		return array("result"=>$code, "error"=>"");
	}

	function get_json_result($args=""){
		$nl="\n";
		$ind=" ";
		$rs = (isset($args["rs"])) ? $args["rs"] : false;
		$jsonp=(isset($args["result_type_args"]["jsonp"])) ? $args["result_type_args"]["jsonp"] : false;
		$code=''.
			'{'.
			$nl.$ind.'head: {}'.
			','.$nl.$ind.'boolean: '.
		'';
		$code.=($rs && ($row=mysql_fetch_array($rs)) && $row["success"]) ? "true" : "false";
		$code.=$nl.'}';
		$result=($jsonp) ? $jsonp.'('.$code.')' : $code;
		return array("result"=>$code, "error"=>"");
	}

	/*					*/

}

?>