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
sk:className                ARC_rdf_store_delete_handler
doap:name                   ARC RDF Store DELETE query handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles DELETE queries 
//release
doap:created                2006-04-12
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-04: release 0.1.0
*/

class ARC_rdf_store_delete_handler {

	var $version="0.1.0";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_delete_handler(&$api){
		$this->__construct($api);
	}

	/*					*/

	function get_result($args=""){/* graph_iri, del_rdfxml, del_s, del_p, del_o, del_o_lang, del_o_dt, result_type (sql|array|plain|xml|json), result_type_args (jsonp) */
		$args["result_type"]=(isset($args["result_type"])) ? $args["result_type"] : "array";
		/* del rdfxml */
		if(isset($args["del_rdfxml"]) && $args["del_rdfxml"]){
			return $this->get_del_rdfxml_result($args);
		}
		foreach(array("graph_iri", "del_s", "del_p", "del_o", "del_o_dt", "del_o_lang") as $cur_var){
			if(isset($args[$cur_var]) && $args[$cur_var]){/* at least one */
				return $this->get_del_pattern_result($args);
			}
		}
		return array("error"=>"Missing parameter 'del_rdfxml', 'graph_iri', or 'del_[s|p|o|o_dt|o_lang]'.", "result"=>"");
	}

	/*					*/

	function get_del_rdfxml_result($args=""){
		$sqls=array();/* if result_type=="sql" */
		$errors=array();
		$row_count=0;
		$parser =& $this->api->get_rdfxml_parser();
		$sub_result=$parser->parse_data(trim($args["del_rdfxml"]));
		$this->api->lock_tables();
		$t1=$this->api->get_mtime();
		$pattern_sql=(isset($args["graph_iri"]) && ($args["graph_iri"])) ? "g=".$this->api->get_id($args["graph_iri"], 1) : "";
		if(is_array($sub_result)){
			$t_tbls=$this->api->get_triple_tables();
			foreach($sub_result as $cur_triple){
				$t_sql="";
				/* s*/
				$s=$cur_triple["s"];
				if($s["type"]=="uri"){
					$t_sql.="s=".$this->get_id($s["uri"], 1);
				}
				/* p */
				if($t_sql){
					$t_sql.=" AND p=".$this->get_id($cur_triple["p"], 1);
				}
				/* o */
				if($t_sql){
					$o=$cur_triple["o"];
					if($o["type"]=="bnode"){
						$t_sql="";
					}
					if($t_sql){
						if($o["type"]=="uri"){
							$t_sql.=" AND o=".$this->get_id($o["uri"], 1);
						}
						else{/* literal */
							$t_sql.=" AND o=".$this->get_id($o["val"], 1);
							if(isset($o["dt"]) && $o["dt"]){
								$t_sql.=" AND o_dt=".$this->get_id($o["dt"], 1);
							}
							elseif(isset($o["lang"]) && $o["lang"]){
								$t_sql.=" AND o_lang='".rawurlencode($o["lang"])."'";
							}
						}
					}
				}
				if($t_sql){
					foreach($t_tbls as $cur_tbl){
						$cur_sql="DELETE FROM ".$this->config["prefix"]."_".$cur_tbl." WHERE ";
						$cur_sql.=($pattern_sql) ? $pattern_sql." AND ".$t_sql : $t_sql;
						if($args["result_type"]=="sql"){
							$sqls[]=$cur_sql;
						}
						elseif($tmp=mysql_query($cur_sql)){
							$row_count+=mysql_affected_rows();
						}
						else{
							$errors[]=mysql_error()." in '".mysql_real_escape_string($cur_sql)."'";
						}
					}
				}
			}
			if(isset($args["graph_iri"]) && ($args["graph_iri"])){
				$store=$this->api->get_store();
				$store->set_graph_var(md5($args["graph_iri"]), "modified_uts", time());
			}
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		/* result */
		$mthd="get_".$args["result_type"]."_result";
		if(method_exists($this, $mthd)){
			$sub_result=$this->$mthd(array("del_row_count"=>$row_count, "del_time"=>round($t2-$t1, 4), "errors"=>$errors, "sqls"=>$sqls, "result_type_args"=>$args["result_type_args"]));
			return array("result"=>$sub_result["result"], "error"=>$sub_result["error"], "del_row_count"=>$row_count, "del_time"=>round($t2-$t1, 4));
		}
		return array("result"=>"", "error"=>"Unsupported result type '".$args["result_type"]."'", "del_row_count"=>$row_count, "del_time"=>round($t2-$t1, 4));
	}
	
	function get_del_pattern_result($args=""){
		$sqls=array();/* if result_type=="sql" */
		$errors=array();
		$row_count=0;
		$this->api->lock_tables();
		$t1=$this->api->get_mtime();
		$pattern_sql="";
		$del_whole_graph=true;
		foreach(array("graph_iri"=>"g", "del_s"=>"s", "del_p"=>"p", "del_o"=>"o", "del_o_dt"=>"o_dt") as $cur_var=>$cur_col){
			if(isset($args[$cur_var]) && ($args[$cur_var])){
				$pattern_sql.=(strlen($pattern_sql)) ? " AND " : "";
				$pattern_sql.=$cur_col."=".$this->api->get_id($args[$cur_var], 1);
				if($cur_var!="graph_iri"){
					$del_whole_graph=false;
				}
			}
		}
		/* o_lang */
		if(isset($args["del_o_lang"]) && $args["del_o_lang"]){
			$pattern_sql.=(strlen($pattern_sql)) ? " AND " : "";
			$pattern_sql.="o_lang='".rawurlencode($args["del_o_lang"])."'";
			$del_whole_graph=false;
		}
		if($pattern_sql){
			$t_tbls=$this->api->get_triple_tables();
			foreach($t_tbls as $cur_tbl){
				$cur_sql="DELETE FROM ".$this->config["prefix"]."_".$cur_tbl." WHERE ".$pattern_sql;
				if($args["result_type"]=="sql"){
					$sqls[]=$cur_sql;
				}
				elseif($tmp=mysql_query($cur_sql)){
					$row_count+=mysql_affected_rows();
				}
				else{
					$errors[]=mysql_error()." in '".mysql_real_escape_string($cur_sql)."'";
				}
			}
			if($del_whole_graph){
				$store=$this->api->get_store();
				$store->delete_graph_vars(md5($args["graph_iri"]));
			}
			elseif(isset($args["graph_iri"]) && ($args["graph_iri"])){
				$store=$this->api->get_store();
				$store->set_graph_var(md5($args["graph_iri"]), "modified_uts", time());
			}
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		/* result */
		$mthd="get_".$args["result_type"]."_result";
		if(method_exists($this, $mthd)){
			$sub_result=$this->$mthd(array("del_row_count"=>$row_count, "del_time"=>round($t2-$t1, 4), "errors"=>$errors, "sqls"=>$sqls, "result_type_args"=>isset($args["result_type_args"]) ? $args["result_type_args"] : array()));
			return array("result"=>$sub_result["result"], "error"=>$sub_result["error"], "del_row_count"=>$row_count, "del_time"=>round($t2-$t1, 4));
		}
		return array("result"=>"", "error"=>"Unsupported result type '".$args["result_type"]."'", "del_row_count"=>$row_count, "del_time"=>round($t2-$t1, 4));
	
	}
	
	/*					*/

	function get_array_result($args=""){
		$error=(count($args["errors"])) ? ((count($args["errors"]) == 1) ? $args["errors"][0] : join(",\n", $args["errors"])) : "";
		$result=($error) ? "" : array("del_row_count"=>$args["del_row_count"], "del_time"=>$args["del_time"]); 
		return array("result"=>$result, "error"=>$error);
	}
	
	function get_sql_result($args=""){
		$error=(count($args["errors"])) ? ((count($args["errors"]) == 1) ? $args["errors"][0] : join(",\n", $args["errors"])) : "";
		$sql=(count($args["sqls"])) ? ((count($args["sqls"]) == 1) ? $args["sqls"][0] : join(";\n", $args["sqls"])) : "";
		return array("result"=>$sql, "error"=>$error);
	}

	function get_plain_result($args=""){
		$nl="\n";
		$result="";
		$error=(count($args["errors"])) ? ((count($args["errors"]) == 1) ? $args["errors"][0] : join(",\n", $args["errors"])) : "";
		if(!$error){
			$result=''.
				'del_row_count: '.$args["del_row_count"].
				$nl.'del_time: '.$args["del_time"].
			'';
		}
		return array("result"=>$result, "error"=>$error);
	}

	function get_xml_result($args=""){
		$nl="\n";
		$ind="  ";
		$result="";
		$error=(count($args["errors"])) ? ((count($args["errors"]) == 1) ? $args["errors"][0] : join(",\n", $args["errors"])) : "";
		$code='<?xml version="1.0" ?>';
		if(!$error){
			$code.=$nl.'<result>';
			$code.=$nl.$ind.'<del_row_count>'.$args["del_row_count"].'</del_row_count>';
			$code.=$nl.$ind.'<del_time>'.$args["del_time"].'</del_time>';
			$code.=$nl.'</result>';
		}
		else{
			$code.=$nl.'<errors>';
			foreach($args["errors"] as $cur_error){
				$code.=$nl.$ind.'<error>'.htmlspecialchars($cur_error).'</error>';
			}
			$code.=$nl.'</errors>';
		}
		return array("result"=>$code, "error"=>$error);
	}

	function get_json_result($args=""){
		$nl="\n";
		$ind="  ";
		$result="";
		$error=(count($args["errors"])) ? ((count($args["errors"]) == 1) ? $args["errors"][0] : join(",\n", $args["errors"])) : "";
		$jsonp=(isset($args["result_type_args"]) && isset($args["result_type_args"]["jsonp"])) ? $args["result_type_args"]["jsonp"] : "";
		$code='{';
		$code.=$nl.$ind.'del_row_count: '.$args["del_row_count"];
		$code.=",".$nl.$ind.'del_time: '.$args["del_time"];
		if($error){
			$code.=",".$nl.$ind.'errors: [ ';
			$added=false;
			foreach($args["errors"] as $cur_error){
				$code.=($added) ? "," : "";
				$code.=$nl.$ind.$ind.'"'.$this->api->escape_js_string($cur_error).'"';
				$added=true;
			}
			$code.=$nl.$ind.']';
		}
		$code.=$nl.'}';
		$code=($jsonp) ? $jsonp."(".$code.")" : $code;
		return array("result"=>$code, "error"=>$error);
	}

	/*					*/

}

?>