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
sk:className                ARC_rdf_store_rdfxml_loader
doap:name                   ARC RDF Store RDF/XML Loader
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              Streaming RDF/XML loader for ARC RDF Store
//release
doap:created                2006-10-05
doap:revision               0.2.2
//changelog
sk:releaseChanges           2006-02-14: release 0.1.0
                            2006-02-18: revision 0.1.1
                                        - fixed bug that caused invalid bnode_id generation
                            2006-04-11: release 0.2.0
                                        - adjusted for ARC API integration
                            2006-05-23: revision 0.2.1
                                        - small bugfix (check for existence of dt and lang)
                            2006-10-05: revision 0.2.2
                                        - minor tweaks
*/

class ARC_rdf_store_rdfxml_loader extends ARC_rdfxml_parser{

	var $version="0.2.2";

	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_rdfxml_loader(&$api){
		$this->__construct($api);
	}
	
	/*					*/
	
	function prepare($args=""){/* graph_iri, encoding, bnode_prefix, proxy_host, proxy_port, user_agent, headers, insert_start, insert_end */
		/* parser args */
		$parser_args=array(
			"base"=>$args["graph_iri"],
			"save_data"=>false,
			"max_lines"=>0,
			"encoding"=>(isset($args["encoding"]) && $args["encoding"]) ? $args["encoding"] : "auto"
		);
		$cur_args=array(
			"bnode_prefix",
			"proxy_host",
			"proxy_port",
			"user_agent",
			"headers",
		);
		foreach($cur_args as $cur_arg){
			$parser_args[$cur_arg]=isset($args[$cur_arg]) ? $args[$cur_arg] : (isset($this->config[$cur_arg]) ? $this->config[$cur_arg] : false);
		}
		parent::__construct($parser_args);
		/* loader args */
		$cur_args=array(
			"graph_iri"=>"",
			"log_inserts"=>false,
			"write_insert_log"=>false,
			"insert_timeout"=>300,
			"insert_start"=>0,
			"insert_end"=>0,
			"preserve_node_ids"=>false,
			"loader_id2val_sql_buffer_size"=>50000,
			"loader_triple_sql_buffer_size"=>50000,
			"loader_id_cache_size"=>2500
		);
		foreach($cur_args as $cur_arg=>$default_val){
			$this->$cur_arg=isset($args[$cur_arg]) ? $args[$cur_arg] : (isset($this->config[$cur_arg]) ? $this->config[$cur_arg] : $default_val);
		}
		/* misc */
		$this->prev_t_count=0;
		$this->triple_sql=array();
		$this->insert_logs=array();
		$this->prop_table_infos=$this->api->get_prop_table_infos();
		foreach($this->prop_table_infos as $cur_p=>$infos){
			$this->triple_sql[$infos["tbl"]]="";
		}
		$this->id_cache=array();
		$this->id_cache_count=0;
		$this->id2val_sql="";
		$this->g_sql=$this->api->get_id($this->graph_iri, 1);
		$this->timer=$this->api->get_mtime();
		$this->prev_timer=$this->timer;
		@set_time_limit($this->insert_timeout);
	}
	
	/*					*/
	
	function encode_mysql_utf8_bugs($val=""){
		return $this->api->encode_mysql_utf8_bugs($val);
	}

	/*					*/
	
	function set_graph_iri($iri=""){
		$this->graph_iri=$iri;
		$this->g_id_sql=$this->api->get_id($this->graph_iri, 1);
	}
	
	function activate_insert_log(){
		$this->log_inserts=true;
	}
	
	function reset_timer(){
		$this->timer=$this->api->get_mtime();
		$this->prev_timer=$this->timer;
	}
	
	function get_insert_logs(){
		return $this->insert_logs;
	}
	
	/*					*/
	
	function create_o_comp($val="", $p="", $o_dt=""){
		/* try date */
		if(preg_match("/^[0-9]{1,2}\s+[a-z]+\s+[0-9]{4}/i", $val)){/* e.g. 12 May 2004 */
			if(($uts=strtotime($val)) && ($uts!==-1)){
				return date("Y-m-d\TH:i:s", $uts);
			}
		}
		/* numeric */
		if(is_numeric($val)){
			$val=sprintf("%f", $val);
			if(preg_match("/([\-\+])([0-9]*)\.([0-9]*)/", $val, $matches)){
				return $matches[1].sprintf("%018s", $matches[2]).".".sprintf("%-015s", $matches[3]);
			}
			if(preg_match("/([0-9]*)\.([0-9]*)/", $val, $matches)){
				return "+".sprintf("%018s", $matches[1]).".".sprintf("%-015s", $matches[2]);
			}
			return $val;
		}
		/* any other string: remove tags, linebreaks etc.  */
		$tmp=rawurlencode(strip_tags($val));
		foreach(array('%0D', '%0A', '%22', '%27', '%60', '%3C', '%3E') as $cur_char){
			$tmp=str_replace($cur_char, '', $tmp);
		}
		return substr(trim(rawurldecode($tmp)), 0, 35);
	}

	/*					*/
	
	function add_triple($s, $p, $o){
		$g_val=$this->graph_iri;
		$g_id_sql=$this->g_sql;
		/* s */
		if($s["type"]==="uri"){
			$s_val=$s["uri"];
			$s_type_val=0;
		}
		else{/* bnode */
			$node_id=substr($s["bnode_id"], 2);
			$s_val=($this->preserve_node_ids) ? '_:'.$node_id : '_:b'.md5($g_val.$node_id).'_'.substr($node_id, -10);
			$s_type_val=1;
		}
		$s_id_sql=$this->api->get_id($s_val, 1);
		$s_init_id_sql=$s_id_sql;
		/* p */
		$p_val=$p;
		$p_id_sql=$this->api->get_id($p_val, 1);
		/* o */
		$o_dt_val="";
		$o_lang_val="";
		if($o["type"]==="uri"){
			$o_val=$o["uri"];
			$o_type_val=0;
		}
		elseif($o["type"]==="bnode"){
			$node_id=substr($o["bnode_id"], 2);
			$o_val=($this->preserve_node_ids) ? '_:'.$node_id : '_:b'.md5($g_val.$node_id).'_'.substr($node_id, -10);
			$o_type_val=1;
		}
		else{/* literal */
			$o_val=$o["val"];
			$o_val=$this->encode_mysql_utf8_bugs($o_val);
			$o_type_val=(strlen(rawurlencode($o_val))>255) ? 3 : 2;
			$o_dt_val=(isset($o["dt"]) && $o["dt"]) ? $o["dt"] : $o_dt_val;
			$o_lang_val=(!$o_dt_val && isset($o["lang"]) && $o["lang"]) ? $o["lang"] : $o_lang_val;
		}
		$o_id_sql=$this->api->get_id($o_val, 1);
		$o_init_id_sql=$o_id_sql;
		$o_dt_id_sql=$this->api->get_id($o_dt_val, 1);
		$o_comp_val=$this->create_o_comp($o_val, $p_val, $o_dt_val);
		/* id2val sql */
		$id_type=$this->config["id_type"];
		$enc_vals = (isset($this->config["encode_values"])) ? $this->config["encode_values"] : false;
		$tbl=$this->config["prefix"]."_id2val";
		foreach(array('s', 'p', 'o', 'o_dt', 'g') as $cur_col){
			$id_var=$cur_col."_id_sql";
			$id_sql=$$id_var;
			if($id_sql && ($id_type!="incr_int") && (!array_key_exists($id_sql, $this->id_cache))){
				$this->id_cache[$id_sql]=1;
				$this->id_cache_count++;
				if($this->id_cache_count > $this->loader_id_cache_size){
					$this->id_cache=array();
					$this->id_cache_count=0;
				}
				$val_var=$cur_col."_val";
				$this->id2val_sql.=(strlen($this->id2val_sql)) ? ", " : "INSERT IGNORE INTO ".$tbl." (id, val) VALUES ";
				$this->id2val_sql.="(".$id_sql.", ";
				$this->id2val_sql.=($enc_vals) ? "'".rawurlencode($$val_var)."')" : "'".mysql_real_escape_string($$val_var)."')";
				if(strlen($this->id2val_sql) > ($this->loader_id2val_sql_buffer_size)){
					@set_time_limit($this->insert_timeout);
					$sub_result=mysql_query($this->id2val_sql);
					if($this->log_inserts){
						$this->add_insert_log("id2val");
					}
					$this->id2val_sql="";
				}
			}
		}
		/* triple sql */
		$store_type=$this->config["store_type"];
		if(in_array($store_type, array("basic", "basic+"))){
			$tbl=$this->config["prefix"]."_triple";
		}
		elseif(isset($this->prop_table_infos[$p_val])){
			$tbl=$this->prop_table_infos[$p_val]["tbl"];
		}
		else{
			$tbl=($o_type_val < 2) ? $this->config["prefix"]."_triple_op" : $this->config["prefix"]."_triple_dp";
		}
		$cols = (isset($this->config["reversible_consolidation"]) && $this->config["reversible_consolidation"]) ? array('s', 'p', 'o', 'g', 's_type', 's_init', 'o_type', 'o_init', 'o_lang', 'o_dt', 'o_comp') : array('s', 'p', 'o', 'g', 's_type', 'o_type', 'o_lang', 'o_dt', 'o_comp');
		if(!isset($this->triple_sql[$tbl])){
			$this->triple_sql[$tbl]="";
		}
		$this->triple_sql[$tbl].= (isset($this->triple_sql[$tbl]) && strlen($this->triple_sql[$tbl])) ? ", " : "INSERT INTO ".$tbl." (".join(", ", $cols).") VALUES ";
		$this->triple_sql[$tbl].="(";
		foreach($cols as $cur_col){
			$this->triple_sql[$tbl].=($cur_col=="s") ? "" : ", ";
			$cur_id_sql_var=$cur_col."_id_sql";
			$cur_val_var=$cur_col."_val";/* o_lang, o_comp */
			$this->triple_sql[$tbl].=(isset($$cur_id_sql_var)) ? $$cur_id_sql_var : "'".$$cur_val_var."'";
		}
		$this->triple_sql[$tbl].=")";
		if(strlen($this->triple_sql[$tbl]) > ($this->loader_triple_sql_buffer_size)){
			@set_time_limit($this->insert_timeout);
			$sub_result=mysql_query($this->triple_sql[$tbl]);
			$this->triple_sql[$tbl]="";
			if($this->log_inserts){
				$this->add_insert_log("triple", $tbl);
			}
		}
		$this->t_count++;
	}
	
	/*					*/
	
	function add_insert_log($tbl="triple", $triple_tbl=""){
		$t1_all=$this->timer;
		$t1_prev=$this->prev_timer;
		$t2=$this->api->get_mtime();
		$dur_all=$t2-$t1_all;
		$dur_prev=$t2-$t1_prev;
		if($tbl=="triple"){
			$t_count_all=$this->t_count;
			$t_count_prev=$t_count_all-$this->prev_t_count;
			$log_code="inserting ".$t_count_prev." t in '".$triple_tbl."' at ".round($t_count_prev/$dur_prev)." t/sec -- (".ceil($t_count_all/$dur_all)." t/sec / ".$t_count_all." triples in ".round($dur_all, 4)." sec overall)";
			$this->prev_timer=$t2;
			$this->prev_t_count=$t_count_all;
		}
		elseif($tbl=="id2val"){
			$log_code="inserting id2val after ".round($dur_all, 4)." sec";
			$log_code.=" (".mysql_affected_rows()." rows)";
		}
		$this->insert_logs[]=$log_code;
		if($this->write_insert_log && ($this->api->get_path("log_path")!==false)){
			$fp=@fopen($this->api->get_path("log_path")."arc_insert_log.txt", "a");
			@fwrite($fp, $log_code."\r\n");
			@fclose($fp);
		}
	}

	/*					*/
	
	function done(){
		/* execute remaining triple/id2val sql code */
		if($this->log_inserts){
			$this->insert_logs[]="--- done, clearing buffers ---";
		}
		if($this->id2val_sql){
			//echo "<br /><br />".$this->id2val_sql;
			$sub_result=mysql_query($this->id2val_sql);
			$this->id2val_sql="";
			if($this->log_inserts){
				$this->add_insert_log("id2val");
			}
		}
		foreach($this->triple_sql as $tbl=>$code){
			if($code){
				$sub_result=mysql_query($code);
				$this->triple_sql[$tbl]="";
				if($this->log_inserts){
					$this->add_insert_log("triple", $tbl);
				}
			}
		}
		$this->triples=array("result"=>$this->t_count, "insert_logs"=>$this->insert_logs);
	}
	
	/*					*/

}

?>