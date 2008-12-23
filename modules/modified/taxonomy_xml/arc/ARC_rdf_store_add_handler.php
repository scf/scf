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
sk:className                ARC_rdf_store_add_handler
doap:name                   ARC RDF Store add_data query handler
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              This class handles add_data() queries 
//release
doap:created                2006-10-24
doap:revision               0.1.1
//changelog
sk:releaseChanges           2006-04-12: release 0.1.0
                            2006-10-24: revision 0.1.1
                                        - fixed bug in turtle converter (XML Literals weren't detected properly)
*/

class ARC_rdf_store_add_handler {

	var $version="0.1.1";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_add_handler(&$api){
		$this->__construct($api);
	}

	/*					*/

	function get_result($args=""){/* graph_iri, add_rdfxml, add_triple, preserve_node_ids, proxy_host, proxy_port, insert_timeout, log_inserts, write_insert_log, result_type (plain|array|json|xml), result_type_args (jsonp) */
		$args["result_type"]=(isset($args["result_type"])) ? $args["result_type"] : "array";
		/* add triple */
		if(isset($args["add_triple"]) && $args["add_triple"]){
			return $this->get_add_triple_result($args);
		}
		/* add rdfxml */
		if(isset($args["add_rdfxml"]) && $args["add_rdfxml"]){
			return $this->get_add_rdfxml_result($args);
		}
		/* add local data */
		if(isset($args["graph_path"]) && $args["graph_path"]){
			return $this->get_add_local_data_result($args);
		}
		/* add remote data */
		if(isset($args["graph_iri"]) && $args["graph_iri"]){
			return $this->get_add_remote_data_result($args);
		}
		$this->api->optimize_tables();
		return array("error"=>"Missing parameter 'add_triple', 'add_rdfxml', 'graph_path', or 'graph_iri'.", "result"=>"");
	}

	/*					*/
	
	function get_add_triple_result($args=""){/* graph_iri, add_triple */
		if(!isset($args["graph_iri"])){
			return array("error"=>"Missing parameter 'graph_iri'", "result"=>"");
		}
		$rest=trim($args["add_triple"]);
		$add_rdfxml='<?xml version="1.0" encoding="utf-8"?>';
		$add_rdfxml.='<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">';
		$prefix_count=0;
		/* s */
		if(preg_match("/^\s*\<([^\>]+)\>(.*)$/s", $rest, $matches)){/* iri */
			$add_rdfxml.='<rdf:Description rdf:about="'.$matches[1].'">';
		}
		elseif(preg_match("/^\_\:([^\s]+)\s+(.*)$/s", $rest, $matches)){/* bnode */
			$add_rdfxml.='<rdf:Description rdf:nodeID="'.$matches[1].'">';
		}
		else{
			return array("error"=>"could not parse 'add_triple' subject.", "result"=>"");
		}
		$rest=trim($matches[2]);
		/* p */
		if(!preg_match("/^\s*\<([^\>]+)\>(.*)$/s", $rest, $matches)){
			return array("error"=>"could not parse 'add_triple' predicate.", "result"=>"");
		}
		$rest=trim($matches[2]);
		if(!preg_match("/\s*(.*[\/|#|\:])([^\/|#|\:]+)/s", $matches[1], $sub_matches)){
			return array("error"=>"could not parse 'add_triple' predicate.", "result"=>"");
		}
		$p_ns_iri=$sub_matches[1];
		$p_local_name=$sub_matches[2];
		$p_ns_prefix='ns'.$prefix_count;
		$add_rdfxml.='<'.$p_ns_prefix.":".$p_local_name.' xmlns:'.$p_ns_prefix.'="'.$p_ns_iri.'"';
		$prefix_count++;
		/* o */
		if(preg_match("/^\<([^\>]+)\>(.*)$/", $rest, $matches)){/* iri */
			$add_rdfxml.=' rdf:resource="'.$matches[1].'"/>';
		}
		elseif(preg_match("/^\_\:([^\s]+)\s+(.*)$/s", $rest, $matches)){/* bnode */
			$add_rdfxml.=' rdf:nodeID="'.$matches[1].'"/>';
		}
		elseif(preg_match("/^[\"\']+(.*)[\"\']+\^\^\<([^\>]+)\>\s*\.\s*$/s", $rest, $matches)){/* quoted literal with dt */
      if ($matches[2] == 'http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral') {
  			$add_rdfxml .= ' rdf:parseType="Literal"';
      }
      else {
        $add_rdfxml .= ' rdf:datatype="'.$matches[2].'"';
      }
      $add_rdfxml .= '>'.$matches[1].'</'.$p_ns_prefix.":".$p_local_name.'>';
		}
		elseif(preg_match("/^[\"\']+(.*)[\"\']+\@([^\s]+)\s*\.\s*$/s", $rest, $matches)){/* quoted literal with lang */
			$add_rdfxml.=' xml:lang="'.$matches[2].'">'.$matches[1].'</'.$p_ns_prefix.":".$p_local_name.'>';
		}
		elseif(preg_match("/^[\"\']+(.*)[\"\']+\s*\.?\s*$/s", $rest, $matches)){/* quoted untyped literal */
			$add_rdfxml.='>'.htmlspecialchars($matches[1]).'</'.$p_ns_prefix.":".$p_local_name.'>';
		}
		elseif(preg_match("/^(true|false)\s*\.\s*$/s", $rest, $matches)){/* boolean literal */
			$add_rdfxml.=' rdf:datatype="http://www.w3.org/2001/XMLSchema#boolean">'.$matches[1].'</'.$p_ns_prefix.":".$p_local_name.'>';
		}
		elseif(preg_match("/^([\-\+0-9\.]+)\s*\.\s*$/s", $rest, $matches)){/* numeric literal */
			$dt=(strpos($matches[1], ".")!==false) ? "float" : "integer";
			$add_rdfxml.=' rdf:datatype="http://www.w3.org/2001/XMLSchema#'.$dt.'">'.$matches[1].'</'.$p_ns_prefix.":".$p_local_name.'>';
		}
		else{
			return array("error"=>"could not parse 'add_triple' object.", "result"=>"");
		}
		$add_rdfxml.='</rdf:Description></rdf:RDF>';
		$args["add_rdfxml"]=$add_rdfxml;
		return $this->get_add_rdfxml_result($args);
	}

	function get_add_rdfxml_result($args=""){
		if(!isset($args["graph_iri"])){
			return array("error"=>"Missing parameter 'graph_iri'", "result"=>"");
		}
		if(!isset($args["add_rdfxml"])){
			return array("error"=>"Missing parameter 'add_rdfxml'", "result"=>"");
		}
		$args["loader_method"]="parse_data";
		return $this->get_loader_result($args);
	}
	
	function get_add_local_data_result($args=""){
		if(!isset($args["graph_iri"])){
			return array("error"=>"Missing parameter 'graph_iri'", "result"=>"");
		}
		$args["loader_method"]="parse_file";
		return $this->get_loader_result($args);
	}

	function get_add_remote_data_result($args=""){
		if(!isset($args["graph_iri"])){
			return array("error"=>"Missing parameter 'graph_iri'", "result"=>"");
		}
		$args["loader_method"]="parse_web_file";
		return $this->get_loader_result($args);
	}

	/*					*/
	
	function get_loader_result($args){
		$loader =& $this->api->get_rdfxml_loader();
		$loader->prepare($args);
		$this->api->lock_tables();
		$t1=$this->api->get_mtime();
		$loader->reset_timer();
		if(isset($args["log_inserts"]) && $args["log_inserts"]){
			$loader->activate_insert_log();
		}
		if($args["loader_method"]=="parse_data"){
			$sub_result= (trim($args["add_rdfxml"])) ? $loader->parse_data(trim($args["add_rdfxml"])) : array("result"=>0, "add_load_time"=>0, "insert_logs"=>array());
			if(!is_array($sub_result) && (strpos($sub_result, "XML error")!==false)){/* parse error */
				$args["encoding"]="ISO-8859-1";
				$loader->prepare($args);
				$sub_result=$loader->parse_data(trim($args["add_rdfxml"]));
			}
		}
		elseif($args["loader_method"]=="parse_file"){
			$sub_result= (trim($args["graph_path"])) ? $loader->parse_file(trim($args["graph_path"])) : array("result"=>0, "add_load_time"=>0, "insert_logs"=>array());
			if(!is_array($sub_result) && (strpos($sub_result, "XML error")!==false)){/* parse error */
				$args["encoding"]="ISO-8859-1";
				$loader->prepare($args);
				$sub_result=$loader->parse_file(trim($args["graph_path"]));
			}
		}
		elseif($args["loader_method"]=="parse_web_file"){
			$sub_result= (trim($args["graph_iri"]) && (strpos(trim($args["graph_iri"]), "http://")===0)) ? $loader->parse_web_file(trim($args["graph_iri"])) : array("result"=>0, "add_load_time"=>0, "insert_logs"=>array());
			if(!is_array($sub_result) && (strpos($sub_result, "XML error")!==false)){/* parse error */
				$args["encoding"]="ISO-8859-1";
				$loader->prepare($args);
				$sub_result=$loader->parse_web_file(trim($args["graph_iri"]));
			}
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		$t3=$this->api->get_mtime();
		$triple_count=0;
		if(is_array($sub_result)){
			$triple_count=$loader->get_triple_count();
			$store=$this->api->get_store();
			$store->set_graph_var(md5($args["graph_iri"]), "modified_uts", time());
		}
		/* result */
		$mthd="get_".$args["result_type"]."_result";
		if(method_exists($this, $mthd)){
			$sub_result=$this->$mthd(array("add_triple_count"=>$triple_count, "add_load_time"=>round($t2-$t1, 4), "loader_result"=>$sub_result, "result_type_args"=>isset($args["result_type_args"]) ? $args["result_type_args"] : array()));
			return array("result"=>$sub_result["result"], "error"=>$sub_result["error"], "add_triple_count"=>$triple_count, "add_load_time"=>round($t2-$t1, 4));
		}
		return array("result"=>"", "error"=>"Unsupported result type '".$args["result_type"]."'", "add_triple_count"=>$triple_count, "add_load_time"=>round($t2-$t1, 4));
	}

	/*					*/
	
	function get_array_result($args=""){
		$loader_result=$args["loader_result"];
		$result="";
		$error="";
		if(is_array($loader_result)){
			$result=array(
				'add_triple_count'=>$loader_result["result"],
				'add_load_time'=>$args["add_load_time"],
				'insert_logs'=>$loader_result["insert_logs"]
			);
		}
		else{/* error */
			$error=$loader_result;
		}
		return array("result"=>$result, "error"=>$error);
	}

	function get_plain_result($args=""){
		$loader_result=$args["loader_result"];
		$result="";
		$error="";
		if(is_array($loader_result)){
			$result=''.
				'add_triple_count:'.$loader_result["result"]."\n".
				'load_time:'.$args["add_load_time"]."\n".
				'logs:'.join(" | ", $loader_result["insert_logs"]).
			'';
		}
		else{/* error */
			$error=$loader_result;
		}
		return array("result"=>$result, "error"=>$error);
	}
	
	function get_xml_result($args=""){
		$loader_result=$args["loader_result"];
		$nl="\n";
		$ind="  ";
		$error="";
		$code='<?xml version="1.0" ?>';
		$code.=$nl.'<result>';
		if(is_array($loader_result)){
			$code.=$nl.$ind.'<add_triple_count>'.$loader_result["result"].'</add_triple_count>';
			$code.=$nl.$ind.'<add_load_time>'.$args["add_load_time"].'</add_load_time>';
			if($logs=$loader_result["insert_logs"]){
				$code.=$nl.$ind.'<insert_logs>';
				foreach($logs as $cur_log){
					$code.=$nl.$ind.$ind.'<log>'.htmlspecialchars($cur_log).'</log>';
				}
				$code.=$nl.$ind.'</insert_logs>';
			}
		}
		else{
			$error=$loader_result;
			$code.=$ind.'<error>'.$error.'</error>'.$nl;
		}
		$code.=$nl.'</result>'.$nl;
		return array("result"=>$code, "error"=>$error);
	}

	function get_json_result($args=""){
		$loader_result=$args["loader_result"];
		$nl="\n";
		$ind="  ";
		$jsonp=(isset($args["result_type_args"]) && isset($args["result_type_args"]["jsonp"])) ? $args["result_type_args"]["jsonp"] : "";
		$error="";
		$code='{';
		if(is_array($loader_result)){
			$code.=$nl.$ind.'add_triple_count: '.$loader_result["result"];
			$code.=",".$nl.$ind.'add_load_time: '.$args["add_load_time"];
			if($logs=$loader_result["insert_logs"]){
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
		else{
			$error=$loader_result;
			$code.=$nl.$ind.'error: "'.$this->api->escape_js_string($error).'"';
		}
		$code.=$nl.'}';
		$code=($jsonp)? $jsonp."(".$code.")" : $code;
		return array("result"=>$code, "error"=>$error);
	}
		
	/*					*/

}

?>