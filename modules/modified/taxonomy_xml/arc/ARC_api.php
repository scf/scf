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
sk:className                ARC_api
doap:name                   ARC API
doap:homepage               http://www.appmosphere.com/en-arc_api
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              ARC API
//release
doap:created                2006-10-01
doap:revision               0.1.1
//changelog
sk:releaseChanges           2006-03-28: release 0.1.0
                            2006-10-01: revision 0.1.1
                                        - minor tweaks
*/

class ARC_api{

	var $version = "0.1.1";
	var $db_con = false;
	var $db = false;
	var $errors=array();
	
	function __construct($args=""){/* config_path, inc_path, log_path */
		$this->args=is_array($args) ? $args : array();
		/* inc path */
		$this->inc_path=(isset($this->args["inc_path"])) ? $this->args["inc_path"] : "";
		$this->inc_path.=(strlen($this->inc_path) && substr($this->inc_path, -1)!="/") ? "/" : "";
		/* log path */
		$this->log_path=(isset($this->args["log_path"])) ? $this->args["log_path"] : "";
		$this->log_path.=(strlen($this->log_path) && substr($this->log_path, -1)!="/") ? "/" : "";
		/* config */
		if(isset($this->args["config_path"]) && ($this->args["config_path"]!==false)){
			if(!file_exists($this->args["config_path"])){
				$this->error("could not find configuration file at '".$this->args["config_path"]."'.");
				exit;
			}
			include_once($this->args["config_path"]);
			$this->config=arc_get_api_config();
		}
		elseif(isset($this->args["config"])){
			$this->config=$this->args["config"];
		}
		else{
			$this->error("missing parameter 'config_path' or 'config' in ARC_api.php.");
			exit;
		}
		/* defaults */
		$this->api_val2id_cache_size=isset($this->config["api_val2id_cache_size"]) ? $this->config["api_val2id_cache_size"] : 1000;
	}
	
	function custom_destruct($db_handle=""){
		if(!$db_handle){
			$this->db_connect();
		}
		$this->drop_merge_tables();
		if(!$db_handle){
			$this->db_disconnect();
		}
	}

	function ARC_api($args=""){
		$this->__construct($args);
	}

	/*					*/
	
	function get_config(){
		return $this->config;
	}
	
	function get_path($path="inc_path"){
		return (strpos($path, "path") && isset($this->$path)) ? $this->$path : false;
	}

	/*					*/
	
	function error($msg=""){
		$this->errors[]=$msg;
		return false;
	}
	
	function get_errors(){
		return $this->errors;
	}
	
	function print_errors($return_value=false, $separator="\n"){
		if(count($this->errors)){
			if($return_value){
				return join($separator, $this->errors);
			}
			else{
				echo join($separator, $this->errors);
			}
		}
		return "";
	}
	
	function get_default_error(){
		return array("error"=>"an error occurred, please see API error log for details.");
	}
	
	/*					*/
	
	function db_connect(){
		if(!$this->db_con = @mysql_connect($this->config["db_host"], $this->config["db_user"], $this->config["db_pwd"])){
			return $this->error("Could not connect to database server '".$this->config["db_host"]."'.");
		}
		elseif(!$this->db = @mysql_select_db($this->config["db_name"], $this->db_con)){
			return $this->error("Could not select database '".$this->config["db_name"]."'.");
		}
		@mysql_query('SET NAMES "utf8"');
		return true;
	}
	
	function db_disconnect(){
		if(isset($this->db_con)){
			@mysql_close($this->db_con);
			unset($this->db_con);
		}
		return true;
	}

	/*					*/
	
	function load_code($cls_or_fnc=""){
		$path=$this->inc_path.$cls_or_fnc.".php";
		if(class_exists($cls_or_fnc) || function_exists($cls_or_fnc)){
			return true;
		}
		if(!file_exists($path)){
			return $this->error("Could not find '".$path."'.");
		}
		include($path);
		return true;
	}
	
	/*					*/
	
	function get_component($name, $class_name){
		if(!isset($this->$name)){
			if(!$tmp=$this->load_code($class_name)){
				return $tmp;
			}
			$this->$name =& new $class_name($this);
		}
		return $this->$name;
	}

	function call_component($name, $mthd, $args=""){
		$get_comp_mthd="get_".$name;
		if(method_exists($this, $get_comp_mthd) && ($comp =& $this->$get_comp_mthd())){
			if(method_exists($comp, $mthd)){
				return $comp->$mthd($args);
			}
			$this->error("Method '".$mthd."' does not exist in component '".$name."'.");
		}
		return $this->get_default_error();
	}

	/*					*/
	
	function get_store_keeper(){
		return $this->get_component("store_keeper", "ARC_rdf_store_keeper");
	}

	function get_store(){
		return $this->get_component("store", "ARC_rdf_store");
	}
	
	function get_api_helper(){
		return $this->get_component("api_helper", "ARC_api_helper");
	}

	function get_merge_table_creator(){
		return $this->get_component("merge_table_creator", "ARC_rdf_store_merge_table_creator");
	}

	function get_sparql_parser(){
		return $this->get_component("sparql_parser", "ARC_sparql_parser");
	}
	
	function get_query_handler($query_type=""){
		if(in_array($query_type, array("select", "ask", "describe", "construct", "add", "delete", "update"))){
			return $this->get_component($query_type."_handler", "ARC_rdf_store_".$query_type."_handler");
		}
		return false;
	}

	function get_query_sub_handler($query_type=""){
		if(in_array($query_type, array("select_json"))){
			return $this->get_component($query_type."_sub_handler", "ARC_rdf_store_".$query_type."_sub_handler");
		}
		return false;
	}

	function get_sparql2sql_rewriter(){
		return $this->get_component("sparql2sql_rewriter", "ARC_sparql2sql_rewriter");
	}

	function get_rdfxml_parser(){
		return $this->get_component("rdfxml_parser", "ARC_rdfxml_parser");
	}
	
	function get_rdfxml_loader(){
		if($this->get_rdfxml_parser()){
			return $this->get_component("rdfxml_loader", "ARC_rdf_store_rdfxml_loader");
		}
		return false;
	}

	/*					*/
	
	function get_mtime(){
		return $this->call_component("api_helper", "get_mtime");
	}
	
	function get_init_mtime(){
		if(!isset($this->init_mtime)){
			$this->init_mtime=$this->get_mtime();
		}
		return $this->init_mtime;
	}

	/*					*/

	function store_exists(){
		$tbl_name=$this->config["prefix"]."_id2val";
		if(!mysql_query("SELECT 1 FROM ".$tbl_name." LIMIT 0")){/* table does not exist */
			return false;
		}
		return true;
	}
	
	function create_store(){
		return $this->call_component("store_keeper", "create_tables");
	}
	
	function delete_store(){
		return $this->call_component("store_keeper", "delete_tables");
	}
	
	function reset_store(){
		return $this->call_component("store_keeper", "reset_tables");
	}

	/*					*/
	
	function create_merge_tables($force_creation=false){
		return ($this->config["store_type"]=="split") ? $this->create_split_merge_tables($force_creation) : $this->create_basic_merge_tables($force_creation);
	}

	function create_split_merge_tables($force_creation=false){
		if($this->config["store_type"]!="split"){
			return true;
		}
		if($creator =& $this->get_merge_table_creator()){
			if($store =& $this->get_store()){
				$store->set_store_var("merge_info", "global", "merge_tbl_mtime", $this->get_init_mtime());
			}
			if($force_creation || !$creator->tables_created()){
				return $creator->create_merge_tables();
			}
			return true;
		}
		return $this->get_default_error();
	}
	
	function create_basic_merge_tables($force_creation=false){
		if($this->config["store_type"]!="basic+"){
			return true;
		}
		if($creator =& $this->get_merge_table_creator()){
			$this->init_mtime=$this->get_init_mtime();
			if($store =& $this->get_store()){
				$store->set_store_var("merge_info", "global", "merge_tbl_mtime", $this->get_init_mtime());
			}
			if($force_creation || !$creator->tables_created()){
				return $creator->create_merge_tables();
			}
			return true;
		}
		return $this->get_default_error();
	}
	
	function drop_merge_tables(){
		if(!in_array($this->config["store_type"], array("split", "basic+"))){
			return true;
		}
		if($creator =& $this->get_merge_table_creator()){
			if($store =& $this->get_store()){
				$my_m="".$this->get_init_mtime();
				$stored_m="".$store->get_store_var("merge_info", "global", "merge_tbl_mtime");
				$m_diff=$my_m-$stored_m;
				if(($my_m >= $stored_m) || ($m_diff > 36000)){/* no other users or old tables (10 hours) */
					return $creator->drop_merge_tables();
				}
			}
		}
		return $this->get_default_error();
	}
	
	/*					*/
	
	function get_prop_table_infos(){
		return $this->call_component("api_helper", "get_prop_table_infos");
	}
	
	/*					*/

	function get_base_tables(){
		return $this->call_component("api_helper", "get_base_tables");
	}
	
	function get_prop_tables(){
		return $this->call_component("api_helper", "get_prop_tables");
	}
	
	function get_triple_tables(){
		return $this->call_component("api_helper", "get_triple_tables");
	}
	
	function get_tables(){
		return $this->call_component("api_helper", "get_tables");
	}
	
	/*					*/

	function get_id($val="", $sql=false){
		if(!isset($this->id_cache) || ($this->id_cache_count > $this->api_val2id_cache_size)){
			$this->id_cache=array();
			$this->id_cache_count=0;
		}
		$hash=sha1($val.":".$sql);
		if(!isset($this->id_cache[$hash])){
			if(!$helper =& $this->get_api_helper()){
				return $this->get_default_error();
			}
			$id_type=$this->config["id_type"];
			if($id_type=="hash_int"){
				$this->id_cache[$hash]=$helper->get_hash_int_id($val, $sql);
			}
			elseif($id_type=="hash_md5"){
				$this->id_cache[$hash]=$helper->get_hash_md5_id($val, $sql);
			}
			elseif($id_type=="hash_sha1"){
				$this->id_cache[$hash]=$helper->get_hash_sha1_id($val, $sql);
			}
			elseif($id_type=="incr_int"){
				$this->id_cache[$hash]=$helper->get_incr_int_id($val);
			}
			$this->id_cache_count++;
		}
		return $this->id_cache[$hash];
	}
	
	function get_val($id_sql=""){
		if(!isset($this->val_cache) || ($this->val_cache_count > 1000)){
			$this->val_cache=array();
			$this->val_cache_count=0;
		}
		if(isset($this->val_cache[$id_sql])){
			return $this->val_cache[$id_sql];
		}
		/* db lookup */
		$val=$id_sql;
		if($rs=mysql_query("SELECT val FROM ".$this->config["prefix"]."_id2val WHERE id=".$id_sql)){
			$row=mysql_fetch_array($rs);
			$val=(isset($this->config["encode_values"]) && $this->config["encode_values"]) ? rawurldecode($row["val"]) : $row["val"];
			if(strlen($val) < 128){
				$this->val_cache[$id_sql]=$val;
			}
		}
		return $val;
	}
	
	/*					*/

	function adjust_utf8_string($args=""){
		return $this->call_component("api_helper", "adjust_utf8_string", $args);
	}

	function escape_js_string($args=""){
		return $this->call_component("api_helper", "escape_js_string", $args);
	}

	function encode_mysql_utf8_bugs($args=""){
		return $this->call_component("api_helper", "encode_mysql_utf8_bugs", $args);
	}

	function decode_mysql_utf8_bugs($args=""){
		return $this->call_component("api_helper", "decode_mysql_utf8_bugs", $args);
	}

	/*					*/

	function optimize_tables(){
		$prefix=$this->config["prefix"];
		$tmp=mysql_query("FLUSH TABLES");
		$tbls=$this->get_tables();
		$sql="";
		foreach($tbls as $cur_tbl){
			$sql.=(strlen($sql)) ? ", " : "";
			$sql.=" '".$prefix."_".$cur_tbl."'";
		}
		return mysql_query("ANALYZE TABLE".$sql);
	}

	function lock_tables($args=""){
		return $this->call_component("api_helper", "lock_tables", $args);
	}

	function unlock_tables(){
		return $this->call_component("api_helper", "unlock_tables");
	}

	/*					*/
	
	function move_duplicates(){
		return $this->call_component("store_keeper", "move_triple_duplicates");
	}
	
	function restore_duplicates(){
		return $this->call_component("store_keeper", "restore_triple_duplicates");
	}
	
	function consolidate_resources($args=""){/* ifp, ifps, fp, fps */
		return $this->call_component("store_keeper", "consolidate_resources", $args);
	}
	
	function undo_resource_consolidation($args=""){/* resource_id (iri|bnode_id) */
		return $this->call_component("store_keeper", "undo_resource_consolidation", $args);
	}
	
	function remove_unlinked_ids(){
		return $this->call_component("store_keeper", "clean_up_id2val");
	}

	/*					*/
	
	function query($args){
		return $this->call_component("store", "query", $args);
	}
	
	/*					*/

	function add_data($args=""){/* result_types(plain, json, array, xml), encoding */
		return $this->call_component("store", "add_data", $args);
	}

	/*					*/

	function delete_data($args=""){
		return $this->call_component("store", "delete_data", $args);
	}

	/*					*/

	function update_data($args=""){
		return $this->call_component("store", "update_data", $args);
	}

	/*					*/

}

?>