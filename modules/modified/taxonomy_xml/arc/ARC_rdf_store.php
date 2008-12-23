<?php
/*
appmosphere RDF classes (ARC): http://www.appmosphere.com/en-arc

Copyright (c) 2005 appmosphere web applications, Germany. All Rights Reserved.
This work is distributed under the W3C(r) Software License [1] in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231

ns sk                       http://www.appmosphere.com/ns/site_kit#
ns doap                     http://usefulinc.com/ns/doap#
sk:className                ARC_rdf_store
doap:name                   ARC RDF Store
doap:homepage               http://www.appmosphere.com/en-arc_rdf_store
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              A SPARQL-enabled RDF store written in PHP
//release
doap:created                2006-05-23
doap:revision               0.2.1
//changelog
sk:releaseChanges           2005-10-30: pre-release 0.0.1
                            2005-12-04: pre-release 0.0.2
                                        - simplified tables to better support unsmushing and graph queries
                            2006-01-27: release 0.1.0
                                        - rewrote most of the code
                                        - added support for custom prop_tables and mySQL's MERGE storage engine
                                        - added support for CONSTRUCT, DESCRIBE, and ASK
                                        - added add_data, update_data, delete_data
                            2006-02-17: revision 0.1.1
                                        - the loader's xml:base is set to the provided graph_iri now
                            2006-02-19: revision 0.1.2
                                        - added JSONP support to "add_data", "update_data", and "delete_data" methods
                            2006-03-08: revision 0.1.3
                                        - some bug-fixing and better error handling
                            2006-04-04: release 0.2.0
                                        - adjusted for ARC API integration
                            2006-05-23: revision 0.2.1
                                        - added "query_infos" result type
                            2006-10-01: revision 0.2.2.
                                        - minor tweak for now optional store vars
*/

class ARC_rdf_store{

	var $version="0.2.2";

	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store(&$api){
		$this->__construct($api);
	}

	/*					*/
	
	function query($args=""){/* query, result_type (query_infos|rows|json|xml|single|rows_n_count|row_count|sql), result_type_args, obj_props, dt_props, iri_alts, ifps, fps, fix_utf8 */
		if(!isset($args["query"]) || !strlen(trim($args["query"]))){
			return array("error"=>"missing or empty query");
		}
		$query=trim($args["query"]);
		$parser =& $this->api->get_sparql_parser();
		$parser->parse($query);
		/* error check */
		if($errors=$parser->get_errors()){
			return array("error"=>"Errors (or unsupported commands) in query :\n- ".implode("\n- ", $errors)."\n\n");
		}
		/* run query */
		$infos=$parser->get_infos();
		if(isset($args["result_type"]) && ($args["result_type"]=="query_infos")){
			return array("result"=>$infos, "error"=>"");
		}
		$q_type=$infos["query_type"];
		if(!$handler =& $this->api->get_query_handler($q_type)){
			return array("error"=>"Unsupported query type '".$q_type."'.");
		}
		$args["infos"]=$infos;
		return $handler->get_result($args);
	}

	function add_data($args=""){
		$handler =& $this->api->get_query_handler("add");
		return $handler ? $handler->get_result($args) : false;
	}
	
	function delete_data($args=""){
		$handler =& $this->api->get_query_handler("delete");
		return $handler ? $handler->get_result($args) : false;
	}
	
	function update_data($args=""){
		$handler =& $this->api->get_query_handler("update");
		return $handler ? $handler->get_result($args) : false;
	}
	
	/*					*/

	function get_store_vars($var_cat="", $var_cat_qlfr=""){
		$result=array();
    if(isset($this->config['enable_vars']) && $this->config['enable_vars']){
  		$tbl_name=$this->config["prefix"]."_store_var";
  		if($rs=mysql_query("SELECT var_name, var_val FROM ".$tbl_name." WHERE var_cat='".rawurlencode($var_cat)."' AND var_cat_qlfr='".md5($var_cat_qlfr)."'")){
  			while($cur_row=mysql_fetch_array($rs)){
  				$result[]=array("name"=>rawurldecode($cur_row["var_name"]), "value"=>rawurldecode($cur_row["var_val"]));
  			}
  		}
    }
		return $result;
	}
	
	function get_store_var($var_cat="", $var_cat_qlfr="", $var_name=""){
    if(isset($this->config['enable_vars']) && $this->config['enable_vars']){
  		$tbl_name=$this->config["prefix"]."_store_var";
  		if($rs=mysql_query("SELECT var_val FROM ".$tbl_name." WHERE var_cat='".rawurlencode($var_cat)."' AND var_cat_qlfr='".md5($var_cat_qlfr)."' AND var_name='".rawurlencode($var_name)."'")){
  			$cur_row=mysql_fetch_array($rs);
  			return rawurldecode($cur_row["var_val"]);
  		}
    }
		return false;
	}
	
	function set_store_var($var_cat="", $var_cat_qlfr="", $var_name="", $var_val=""){
    if(isset($this->config['enable_vars']) && $this->config['enable_vars']){
  		$tbl_name=$this->config["prefix"]."_store_var";
  		/* delete if exists */
  		$tmp=$this->delete_store_var($var_cat, $var_cat_qlfr, $var_name);
  		/* insert */
  		return mysql_query("INSERT INTO ".$tbl_name." (var_cat, var_cat_qlfr, var_name, var_val) VALUES ('".rawurlencode($var_cat)."', '".md5($var_cat_qlfr)."', '".rawurlencode($var_name)."', '".rawurlencode($var_val)."')");
    }
    return false;
	}
	
	function delete_store_var($var_cat="", $var_cat_qlfr="", $var_name=""){
    if(isset($this->config['enable_vars']) && $this->config['enable_vars']){
  		$tbl_name=$this->config["prefix"]."_store_var";
  		return mysql_query("DELETE FROM ".$tbl_name." WHERE var_cat='".rawurlencode($var_cat)."' AND var_cat_qlfr='".md5($var_cat_qlfr)."' AND var_name='".rawurlencode($var_name)."'");
    }
    return false;
	}

	function delete_store_vars($var_cat="", $var_cat_qlfr=false){
    if(isset($this->config['enable_vars']) && $this->config['enable_vars']){
  		$tbl_name=$this->config["prefix"]."_store_var";
  		if($var_cat_qlfr===false){
  			return mysql_query("DELETE FROM ".$tbl_name." WHERE var_cat='".rawurlencode($var_cat)."'");
  		}
  		else{
  			return mysql_query("DELETE FROM ".$tbl_name." WHERE var_cat='".rawurlencode($var_cat)."' AND var_cat_qlfr='".md5($var_cat_qlfr)."'");
  		}
    }
	}
	
	/*					*/
	
	function get_graph_vars($g_hash=""){
		$g_hash=(!$g_hash || (strpos($g_hash, ":")!==false)) ? md5($g_hash) : $g_hash;
		return $this->get_store_vars("g", $g_hash);
	}
	
	function get_graph_var($g_hash="", $var_name=""){
		$g_hash=(!$g_hash || (strpos($g_hash, ":")!==false)) ? md5($g_hash) : $g_hash;
		return $this->get_store_var("g", $g_hash, $var_name);
	}
	
	function set_graph_var($g_hash="", $var_name="", $var_val=""){
		$g_hash=(!$g_hash || (strpos($g_hash, ":")!==false)) ? md5($g_hash) : $g_hash;
		return $this->set_store_var("g", $g_hash, $var_name, $var_val);
	}
	
	function delete_graph_var($g_hash="", $var_name=""){
		$g_hash=(!$g_hash || (strpos($g_hash, ":")!==false)) ? md5($g_hash) : $g_hash;
		return $this->delete_store_var("g", $g_hash, $var_name);
	}

	function delete_graph_vars($g_hash=""){
		$g_hash=(!$g_hash || (strpos($g_hash, ":")!==false)) ? md5($g_hash) : $g_hash;
		return $this->delete_store_vars("g", $g_hash);
	}
	
	/*					*/
	
	
}

?>