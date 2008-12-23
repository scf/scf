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
sk:className                ARC_rdfxml_parser
doap:name                   ARC RDF/XML Parser
doap:homepage               http://www.appmosphere.com/en-arc_rdfxml_parser
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              A lightweight, non-validating PHP RDF/XML parser that returns an array of triples or an error string
//release
doap:created                2006-10-06
doap:revision               0.2.8
//changelog
sk:releaseChanges           2004-07-21: release 0.1.0
                            2004-08-10: revision 0.1.1
                                        - datatype handling bug fixed
                            2004-09-03: revision 0.1.2
                                        - fixed fsockopen bug (now sets the default port to '80')
                            2004-10-16: revision 0.1.3
                                        - fixed http bug in get_web_file
                            2004-11-16: revision 0.1.4
                                        - added support for MGET
                            2004-12-03: revision 0.1.5
                                        - replaced array_pop which didn't work properly with some PHP versions
                            2004-12-15: revision 0.1.6
                                        - added support for rdf:type on empty property elements
                            2005-03-17: revision 0.1.7
                                        - added basic support for "max_lines" for input streams
                            2005-03-29: revision 0.1.8
                                        - fixed xml:base bug (didn't correctly remove trailing "#"s)
                            2005-04-06: revision 0.1.9
                                        - presetting the parser encoding to UTF-8 (related to PHP bug #32001)
                            2005-04-10: release 0.2.0
                                        - allow the setting of an empty encoding via the init_args array
                                        - fixed xml literal parsing bug
                                        - added support for default namespaces in xml literals (2 of 14 unpassed positive parser tests, 12 remaining)
                                        - added support for xml:base on other nodes than the root nodes (12 of 12 unpassed positive parser tests, 0 remaining)
                            2005-04-21: revision 0.2.1
                                        - fixed bug in calc_base
                            2005-04-29: revision 0.2.2
                                        - added auto-detection for source encoding via http headers and/or xml prolog
                            2005-07-18: revision 0.2.3
                                        - renamed "calc_base" method to "calc_uri"
                                        - added "calc_base" method to support relative xml:base URIs
                                        - fixed bug in cdata handling
                            2005-07-31: revision 0.2.4
                                        - added trim()s to literal data
                            2006-02-08: revision 0.2.5
                                        - added __construct to ease inheritence
                                        - added method done() (used by ARC RDF Store RDF/XML Loader)
                                        - added 304/4xx/5xx and redirect (30x) detection
                            2006-03-11: revision 0.2.6
                                        - several minor tweaks
                            2006-06-11: revision 0.2.7
                                        - minor tweaks to eliminate php notices
                            2006-10-08: revision 0.2.8
                                        - removed trim()s for non-xml cdata (o_cdata)
*/

class ARC_rdfxml_parser{

	var $version="0.2.8";

	var $triples;
	var $subjs;
	var $nsps;
	var $s_count=0;
	var $t_count=0;
	var $bnode_id=0;
	var $xml_lang="";
	var $xml_base="";
	var $state=1;
	var $max_lines=0;
	var $save_data=false;

	function __construct($args=""){
		$this->init_args=$args;/* base, bnode_prefix, proxy_host, proxy_port, user_agent, headers, save_data, max_lines, encoding */
		$this->skip_terms=array(
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# RDF",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# Description",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# ID",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# about",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# parseType",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# resource",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# nodeID",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# datatype",
			"http://www.w3.org/1999/02/22-rdf-syntax-ns# type"
		);
	}
	
	function ARC_rdfxml_parser($args=""){
		$this->__construct($args);
	}

	function init($create_parser=true){
		$this->triples=array();
		$this->subjs=array();
		$this->nsps=array();/* namespace prefixes */
		$this->bnode_id=0;
		$this->s_count=0;
		$this->t_count=0;
		$this->xml_lang="";
		$this->state=1;
		$this->xml_base="";
		$this->result_headers=array();
		$this->encoding="UTF-8";
		/* base */
		if($base=$this->init_args["base"]){
			$this->set_base($base);
		}
		/* bnode_prefix */
		if($bnode_prefix=$this->init_args["bnode_prefix"]){
			$this->bnode_prefix=$bnode_prefix;
		}
		else{
			$this->bnode_prefix="arc".substr(md5(uniqid(rand())), 0, 4)."b";
		}
		/* save_data */
		if(isset($this->init_args["save_data"])){
			$this->save_data=$this->init_args["save_data"];
			$this->data="";
		}
		/* max_lines */
		if(isset($this->init_args["max_lines"])){
			$this->max_lines=$this->init_args["max_lines"];
		}
		/* encoding */
		if(isset($this->init_args["encoding"]) && $this->init_args["encoding"]){
			$this->encoding=$this->init_args["encoding"];
		}
		/* parser */
		if($create_parser){
			$this->create_parser();
		}
	}
	
	function create_parser(){
		$parser=xml_parser_create_ns($this->encoding, " ");
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE,0);
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
		xml_set_element_handler($parser, "handle_open", "handle_close");
		xml_set_character_data_handler($parser, "handle_cdata");
		xml_set_start_namespace_decl_handler($parser, "handle_ns_decl");
		xml_set_object($parser, $this);
		$this->parser=$parser;
	}
	
	function get_target_encoding(){
		return $this->target_encoding;
	}
	
	function get_data(){
		return $this->data;
	}
	
	function get_parsed_url(){
		return (isset($this->parsed_url)) ? $this->parsed_url : "";
	}
	
	function done(){
	}
	
	function get_triple_count(){
		return $this->t_count;
	}

	/*					*/
	
	function set_base($base){
		if(strlen($this->xml_base===0) || (strpos($base, ":")!==false)){
			$this->xml_base=$base;
		}
		else{
			$this->xml_base=$this->calc_base($base);
		}
	}

	function get_cur_xml_base($s=""){
		if($s){
			if(isset($s["p_xml_base"]) && ($base=$s["p_xml_base"])){
				return $base;
			}
			elseif(isset($s["xml_base"]) && ($base=$s["xml_base"])){
				return $base;
			}
		}
		return $this->xml_base;
	}
	
	function get_clean_base($base=""){
		/* remove fragment */
		if(preg_match("/([^#]*)[#]?/", $base, $matches)){/* should always match, remove fragment */
			$base=$matches[1];
		}
		/* no path, no query, no trailing slash, e.g. http://www.example.com  -> add slash */
		if(preg_match("/\/\/(.*)/", $base, $matches)){/* //+something */
			if(strpos($matches[1], "/")===false){/* no more slashes */
				$base.="/";
			}
		}
		return $base;
	}
	
	/*					*/
	
	function calc_abs_path($path="", $base=""){
		if(strpos($path, "/")===0){/* leading slash */
			if(preg_match("/([^\/]*[\/]{1,2}[^\/]+)\//", $base, $matches)){
				return $matches[1].$path;
			}
		}
		elseif($path==""){
			return $base;
		}
		else{/* rel path (../ or  path) */
			/* remove stuff after last slash */
			$base=substr($base, 0, strrpos($base, "/"))."/";
			if(strpos($path, "../")===0){
				if(preg_match("/([^\/]*[\/]{1,2}[^\/]+\/)(.*)\//", $base, $matches)){
					$server_part=$matches[1];
					$path_part=$matches[2];
				}
				else{
					$server_part=$base;
					$path_part="";
				}
				while(strpos($path, "../")===0){
					$path=substr($path, 3);
					$path_part=strlen($path_part) ? substr($path_part, 0, -1) : "";/* remove / */
					if(strpos($path_part, "/")){
						$path_part=substr($path_part, 0, strrpos($path_part, "/"))."/";/* remove stuff after (new) last slash */
					}
					else{
						$path_part="";
					}
				}
				return $server_part.$path_part.$path;
			}
			else{
				return $base.$path;
			}
		}
		return $path;
	}
	
	function calc_base($path=""){
		if(strpos($path, ":")!==false){/* is abs uri */
			return $path;
		}
		elseif(strpos($path, "//")===0){/* net path */
			return "http:".$path;
		}
		/* relative base */
		$s=($this->s_count) ? $this->subjs[$this->s_count-1] : false;
		$cur_base=$this->get_cur_xml_base($s); 
		$cur_base=$this->get_clean_base($cur_base);
		return $this->calc_abs_path($path, $cur_base);
	}

	function calc_uri($s="", $path="", $term=""){
		$result="";
		if(strpos($path, ":")!==false){/* is abs uri */
			return $path;
		}
		$cur_base=$this->get_cur_xml_base($s);
		$cur_base=$this->get_clean_base($cur_base);
		if($term=="ID"){
			return $cur_base."#".$path;
		}
		elseif(strpos($path, "#")===0){
			return $cur_base.$path;
		}
		elseif(strpos($path, "//")===0){/* net path */
			return "http:".$path;
		}
		return $this->calc_abs_path($path, $cur_base);
	}
	
	/*					*/
	
	function add_triple($s, $p, $o){
		/* echo "adding triple: ".$s["bnode_id"].$s["uri"]."	".$p."	".$o["uri"].$o["bnode_id"].$o["val"]."\n"; */
		$this->triples[$this->t_count]=array("s"=>$s, "p"=>$p, "o"=>$o);
		$this->t_count++;
	}
	
	/*					*/

	function reify($statement, &$s, $p, $o){
		$this->add_triple($statement, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", array("type"=>"uri", "uri"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#Statement"));
		$this->add_triple($statement, "http://www.w3.org/1999/02/22-rdf-syntax-ns#subject", $s);
		$this->add_triple($statement, "http://www.w3.org/1999/02/22-rdf-syntax-ns#predicate", $p);
		$this->add_triple($statement, "http://www.w3.org/1999/02/22-rdf-syntax-ns#object", $o);
	}

	/*					*/
	
	function create_bnode_id(){
		$this->bnode_id++;
		return "_:".$this->bnode_prefix.$this->bnode_id;
	}
	
	/*					*/

	function push_s(&$s){
		$this->subjs[$this->s_count]=$s;
		$this->s_count++;
	}
	
	function pop_s(){
		$new_subjs=array();
		$this->s_count--;
		for($i=0,$i_max=$this->s_count;$i<$i_max;$i++){
			$new_subjs[]=$this->subjs[$i];
		}
		$this->subjs=$new_subjs;
		return true;
	}
	
	function get_cur_lang($s=""){
		if($s){
			if(isset($s["p_xml_lang"]) && ($lang=$s["p_xml_lang"])){
				return $lang;
			}
			elseif(isset($s["xml_lang"]) && ($lang=$s["xml_lang"])){
				return $lang;
			}
		}
		return $this->xml_lang;
	}

	/*					*/

	function handle_open($parser, $tag, $attrs){
		/* echo "at state ".$this->state." opening ".$tag."\n"; */
		switch($this->state){
			case 2:/* expecting p open */
				$this->handle_open_2($tag, $attrs);
				break;
			case 4:/* expecting sub_node */
				$this->handle_open_4($tag, $attrs);
				break;
			case 1:/* expecting s open */
				$this->handle_open_1($tag, $attrs);
				break;
			case 6:/* expecting xml data */
				$this->handle_open_6($tag, $attrs);
				break;
			default:
				echo "unexpected handle_open call (at state ".$this->state.") (".$tag.") \n";
		}
	}


	function handle_open_1($tag, $attrs){
		$xml="http://www.w3.org/XML/1998/namespace";
		$rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#";
		/* rdf:RDF */
		if($tag===$rdf." RDF"){
			/* lang */
			$this->xml_lang=(isset($attrs[$xml." lang"]) && ($xml_lang=$attrs[$xml." lang"])) ? $xml_lang : $this->xml_lang;
			/* base */
			if(isset($attrs[$xml." base"]) && ($xml_base=$attrs[$xml." base"])){
				$this->set_base($xml_base);
			}
			return true;
		}
		$cur_s=array();
		/* base */
		if(isset($attrs[$xml." base"]) && ($xml_base=$attrs[$xml." base"])){
			$cur_s["xml_base"]=$this->calc_base($xml_base);
		}
		elseif($prev_s=&$this->subjs[$this->s_count-1]){/* s is an o, too */
			if($p_xml_base=$prev_s["p_xml_base"]){
				$cur_s["xml_base"]=$p_xml_base;
			}
			elseif($xml_base=$prev_s["xml_base"]){
				$cur_s["xml_base"]=$xml_base;
			}
		}
		else{/* top level node */
			$cur_s["xml_base"]=$this->xml_base;
		}
		/* lang */
		if(isset($attrs[$xml." lang"]) && ($xml_lang=$attrs[$xml." lang"])){
			$cur_s["xml_lang"]=$xml_lang;
		}
		elseif($prev_s=&$this->subjs[$this->s_count-1]){/* s is an o, too */
			if($p_xml_lang=$prev_s["p_xml_lang"]){
				$cur_s["xml_lang"]=$p_xml_lang;
			}
			elseif($xml_lang=$prev_s["xml_lang"]){
				$cur_s["xml_lang"]=$xml_lang;
			}
		}
		else{/* top level node */
			$cur_s["xml_lang"]=$this->xml_lang;
		}
		/* rdf:ID */
		if(isset($attrs[$rdf." ID"]) && ($rdf_id=$attrs[$rdf." ID"])){
			$cur_s["type"]="uri";
			$cur_s["uri"]=$this->calc_uri($cur_s, $rdf_id, "ID");
		}
		/* rdf:about */
		elseif(isset($attrs[$rdf." about"])){
			$cur_s["type"]="uri";
			$uri=$attrs[$rdf." about"];
			$cur_s["uri"]=$this->calc_uri($cur_s, $uri, "about");
		}
		/* bnode */
		else{
			$cur_s["type"]="bnode";
			/* rdf:nodeID */
			if(isset($attrs[$rdf." nodeID"]) && ($rdf_nodeID=$attrs[$rdf." nodeID"])){
				$cur_s["bnode_id"]="_:".$rdf_nodeID;
			}
			else{/* create bnode_id */
				$cur_s["bnode_id"]=$this->create_bnode_id();
			}
		}
		/* typed node */
		if($tag!=$rdf." Description"){
			$this->add_triple($cur_s, $rdf."type", array("type"=>"uri", "uri"=>str_replace(" ", "", $tag)));
		}
		/* (additional) typing attr */
		if(isset($attrs[$rdf." type"]) && ($rdf_type=$attrs[$rdf." type"])){
			$this->add_triple($cur_s, $rdf."type", array("type"=>"uri", "uri"=>$rdf_type));
		}
		/* Seq|Bag|Alt */
		$cur_s["li_count"]=0;/* rdf:li elements can exist in any description element */
		if(($tag===$rdf." Seq") || ($tag===$rdf." Bag") || ($tag===$rdf." Alt")){
			$cur_s["sba"]=true;
		}
		/* any other attrs (qualified, but not from rdf skip_terms or xml namespace) */
		$cur_lang=$this->get_cur_lang($cur_s);
		foreach($attrs as $k=>$v){
			if(strpos($k, $xml)===false && strpos($k, " ")!==false){
				if(strpos($k, $rdf)===false){
					$this->add_triple($cur_s, str_replace(" ", "", $k), array("type"=>"literal", "val"=>$v, "lang"=>$cur_lang));
				}
				elseif(!in_array($k, $this->skip_terms)){/* add, but may warn */
					$this->add_triple($cur_s, str_replace(" ", "", $k), array("type"=>"literal", "val"=>$v, "lang"=>$cur_lang));
				}
			}
		}
		$this->push_s($cur_s);
		$this->state=2;
	}
	
	function handle_open_2($tag, $attrs){
		$xml="http://www.w3.org/XML/1998/namespace";
		$rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#";
		$cur_p=$tag;
		$cur_s =& $this->subjs[$this->s_count-1];
		unset($cur_s["p_xml_base"]);
		unset($cur_s["p_xml_lang"]);
		unset($cur_s["p_rdf_ID"]);
		unset($cur_s["coll"]);
		/* base */
		if($xml_base=@$attrs[$xml." base"]){
			$cur_s["p_xml_base"]=$this->calc_base($xml_base);
		}
		/* lang */
		if($xml_lang=@$attrs[$xml." lang"]){
			$cur_s["p_xml_lang"]=$xml_lang;
		}
		/* adjust li */
		if($cur_p===$rdf." li"){
			$li_count=@$cur_s["li_count"]+1;
			$cur_s["li_count"]=$li_count;
			$cur_p=$rdf."_".$li_count;
		}
		$cur_s["cur_p"]=str_replace(" ", "", $cur_p);
		/* rdf:ID => reification */
		if($rdf_ID=@$attrs[$rdf." ID"]){
			$cur_s["p_rdf_ID"]=$rdf_ID;
		}
		/* rdf:resource */
		if(isset($attrs[$rdf." resource"])){
			$rdf_resource=$attrs[$rdf." resource"];
			$rdf_resource=$this->calc_uri($cur_s, $rdf_resource, "resource");
			$this->add_triple($cur_s, $cur_s["cur_p"], array("type"=>"uri", "uri"=>$rdf_resource));
			/* typing */
			if(isset($attrs[$rdf." type"])){
				$this->add_triple(array("type"=>"uri", "uri"=>$rdf_resource), $rdf."type", array("type"=>"uri", "uri"=>$attrs[$rdf." type"]));
			}
			/* reification */
			if($rdf_ID){/* reify, p is an empty element */
				$this->reify(array("type"=>"uri", "uri"=>$this->calc_uri($cur_s, $rdf_ID, "ID")), $cur_s, array("type"=>"uri", "uri"=>$cur_s["cur_p"]), array("type"=>"uri", "uri"=>$rdf_resource));
				unset($cur_s["p_rdf_ID"]);
			}
			$this->state=3;
		}
		/* named bnode */
		elseif($rdf_nodeID=@$attrs[$rdf." nodeID"]){
			$this->add_triple($cur_s, $cur_s["cur_p"], array("type"=>"bnode", "bnode_id"=>"_:".$rdf_nodeID));
			$this->state=3;
			if($rdf_ID){/* reify */
				$this->reify(array("type"=>"uri", "uri"=>$this->calc_uri($cur_s, $rdf_ID, "ID")), $cur_s, array("type"=>"uri", "uri"=>$cur_s["cur_p"]), array("type"=>"bnode", "bnode_id"=>"_:".$rdf_nodeID));
			}
		}
		/* rdf:parseType */
		elseif($rdf_parseType=@$attrs[$rdf." parseType"]){
			if($rdf_parseType==="Literal"){
				$cur_s["o_xml_level"]=0;
				$cur_s["o_xml_data"]="";
				$cur_s["p_xml_literal_level"]=0;
				$cur_s["declared_namespaces"]=array();
				$this->state=6;
			}
			elseif($rdf_parseType==="Resource"){
				$sub_s=array("type"=>"bnode", "bnode_id"=>$this->create_bnode_id());
				$this->add_triple($cur_s, str_replace(" ", "", $cur_p), $sub_s);
				$this->push_s($sub_s);
				if(isset($cur_s["p_rdf_ID"]) && ($p_rdf_ID=$cur_s["p_rdf_ID"])){/* reify, p is an empty element */
					$this->reify(array("type"=>"uri", "uri"=>$this->calc_uri($cur_s, $p_rdf_ID, "ID")), $cur_s, array("type"=>"uri", "uri"=>$cur_s["cur_p"]), $sub_s);
					unset($cur_s["p_rdf_ID"]);
				}
				$this->state=2;
			}
			elseif($rdf_parseType==="Collection"){
				$cur_s["coll"]=true;
				$this->state=4;
			}
		}
		else{/* o is sub_node or literal */
			/* typed literal */
			if($rdf_datatype=@$attrs[$rdf." datatype"]){
				$cur_s["o_rdf_datatype"]=$rdf_datatype;
			}
			$this->state=4;
		}
		/* any other attrs (qualified, but not from rdf or xml namespace, except rdf:type) */
		unset($tmp_node);
		foreach($attrs as $k=>$v){
			if((strpos($k, $rdf)===false) && (strpos($k, $xml)===false) && (strpos($k, " ")!==false)){
				if(!isset($tmp_node) || !$tmp_node){
					$cur_lang=$this->get_cur_lang($cur_s);
					if($rdf_resource){
						$tmp_node=array("type"=>"uri", "uri"=>$rdf_resource);
					}
					else{
						$tmp_node=array("type"=>"bnode", "bnode_id"=>$this->create_bnode_id());
						$this->add_triple($cur_s, str_replace(" ", "", $cur_p), $tmp_node);
					}
				}
				if(isset($cur_s["p_rdf_ID"]) && ($p_rdf_ID=$cur_s["p_rdf_ID"])){/* reify, but only once, p is an empty element */
					$this->reify(array("type"=>"uri", "uri"=>$this->calc_uri($cur_s, $p_rdf_ID, "ID")), $cur_s, array("type"=>"uri", "uri"=>$cur_s["cur_p"]), $tmp_node);
					unset($cur_s["p_rdf_ID"]);
				}
				$this->add_triple($tmp_node, str_replace(" ", "", $k), array("type"=>"literal", "val"=>$v, "lang"=>$cur_lang));
				$this->state=3;
			}
		}
	}
	
	function handle_open_4($tag, $attrs){
		$cur_s=array();
		$prev_s=&$this->subjs[$this->s_count-1];
		/* base */
		if($xml_base=@$attrs["http://www.w3.org/XML/1998/namespace base"]){
			$cur_s["xml_base"]=$this->calc_base($xml_base);
		}
		elseif($p_xml_base=@$prev_s["p_xml_base"]){
			$cur_s["xml_base"]=$p_xml_base;
		}
		elseif($xml_base=@$prev_s["xml_base"]){
			$cur_s["xml_base"]=$xml_base;
		}
		else{/* top level node */
			$cur_s["xml_base"]=$this->xml_base;
		}
		/* lang */
		if($xml_lang=@$attrs["http://www.w3.org/XML/1998/namespace lang"]){
			$cur_s["xml_lang"]=$xml_lang;
		}
		elseif($p_xml_lang=@$prev_s["p_xml_lang"]){
			$cur_s["xml_lang"]=$p_xml_lang;
		}
		elseif($xml_lang=@$prev_s["xml_lang"]){
			$cur_s["xml_lang"]=$xml_lang;
		}
		else{/* top level node */
			$cur_s["xml_lang"]=$this->xml_lang;
		}
		/* rdf:ID */
		if($rdf_id=@$attrs["http://www.w3.org/1999/02/22-rdf-syntax-ns# ID"]){
			$cur_s["type"]="uri";
			//$cur_s["uri"]=$this->full_base."#".$rdf_id;
			$cur_s["uri"]=$this->calc_uri($cur_s, $rdf_id, "ID");
		}
		/* rdf:about */
		elseif(isset($attrs["http://www.w3.org/1999/02/22-rdf-syntax-ns# about"])){
			$cur_s["type"]="uri";
			$uri=$attrs["http://www.w3.org/1999/02/22-rdf-syntax-ns# about"];
			$cur_s["uri"]=$this->calc_uri($cur_s, $uri, "about");
		}
		/* bnode */
		else{
			$cur_s["type"]="bnode";
			/* rdf:nodeID */
			if($rdf_nodeID=@$attrs["http://www.w3.org/1999/02/22-rdf-syntax-ns# nodeID"]){
				$cur_s["bnode_id"]="_:".$rdf_nodeID;
			}
			else{/* create bnode_id */
				$cur_s["bnode_id"]=$this->create_bnode_id();
			}
		}
		/* Collection */
		if(@$prev_s["coll"] || @$prev_s["is_list"]){/* collection is not empty || cur_s is next entry in collection */
			$list_bnode_id=$this->create_bnode_id();
			$list=array("type"=>"bnode", "bnode_id"=>$list_bnode_id);
			if($prev_p=@$prev_s["cur_p"]){
				$this->add_triple($prev_s, $prev_s["cur_p"], $list);
			}
			else{
				$this->add_triple($prev_s, "http://www.w3.org/1999/02/22-rdf-syntax-ns#rest", $list);
			}
			$list["is_list"]=true;
			$this->push_s($list);
			/* cur_s is first */
			$this->add_triple($list, "http://www.w3.org/1999/02/22-rdf-syntax-ns#first", $cur_s);
			$cur_s["in_list"]=true;
			$this->push_s($cur_s);
			$this->state=2;
		}
		else{
			$this->add_triple($prev_s, $prev_s["cur_p"], $cur_s);
			$this->push_s($cur_s);
			$this->state=2;
		}
		/* typed node */
		if($tag!="http://www.w3.org/1999/02/22-rdf-syntax-ns# Description"){
			$this->add_triple($cur_s, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", array("type"=>"uri", "uri"=>str_replace(" ", "", $tag)));
		}
		/* (additional) typing attr */
		if($rdf_type=@$attrs["http://www.w3.org/1999/02/22-rdf-syntax-ns# type"]){
			$this->add_triple($cur_s, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", array("type"=>"uri", "uri"=>$rdf_type));
		}
		/* Seq|Bag|Alt */
		$cur_s["li_count"]=0;/* rdf:li elements can exist in any description element */
		if(($tag==="http://www.w3.org/1999/02/22-rdf-syntax-ns# Seq") || ($tag==="http://www.w3.org/1999/02/22-rdf-syntax-ns# Bag") || ($tag==="http://www.w3.org/1999/02/22-rdf-syntax-ns# Alt")){
			$cur_s["sba"]=true;
		}
		/* any other attrs (qualified, but not from rdf skip_terms or xml namespace) */
		$cur_lang=$this->get_cur_lang($cur_s);
		foreach($attrs as $k=>$v){
			if((strpos($k, "http://www.w3.org/XML/1998/namespace")===false) && (strpos($k, " ")!==false)){
				if(strpos($k, "http://www.w3.org/1999/02/22-rdf-syntax-ns#")===false){
					$this->add_triple($cur_s, str_replace(" ", "", $k), array("type"=>"literal", "val"=>$v, "lang"=>$cur_lang));
				}
				elseif(!in_array($k, $this->skip_terms)){/* add, but may warn */
					$this->add_triple($cur_s, str_replace(" ", "", $k), array("type"=>"literal", "val"=>$v, "lang"=>$cur_lang));
				}
			}
		}
	}

	function handle_open_6($tag, $attrs){
		$cur_s=&$this->subjs[$this->s_count-1];
		$data=$cur_s["o_xml_data"];
		$xml_level=$cur_s["o_xml_level"];
		$decl_nss=$cur_s["declared_namespaces"];
		$tag_parts=explode(" ", $tag);
		if(count($tag_parts)==1){/* no qname */
			$data.='<'.$tag;
		}
		else{
			$ns_uri=$tag_parts[0];
			$local_name=$tag_parts[1];
			$nsp=$this->nsps[$ns_uri];
			$data.=(strlen($nsp)) ? '<'.$nsp.":".$local_name : '<'.$local_name;
			/* declare ns */
			if(!@$decl_nss[$nsp."=".$ns_uri]){
				$data.=(strlen($nsp)) ? ' xmlns:'.$nsp.'="'.$ns_uri.'"' : ' xmlns="'.$ns_uri.'"';
				$decl_nss[$nsp."=".$ns_uri]=true;
				$cur_s["declared_namespaces"]=$decl_nss;
			}
		}
		foreach($attrs as $k=>$v){
			if(strpos($k, " ")){/* qualified attr */
				$attr_parts=explode(" ", $k);
				$a_ns_uri=$attr_parts[0];
				$a_local_name=$attr_parts[1];
				$a_nsp=$this->nsps[$a_ns_uri];
				$data.= (strlen($a_nsp)) ? ' '.$a_nsp.':'.$a_local_name.'="'.$v.'"' : ' '.$a_local_name.'="'.$v.'"';
			}
			else{/* unqualified attr */
				$data.=' '.$k.'="'.$v.'"';
			}
		}
		$data.='>';
		$cur_s["o_xml_data"]=$data;
		$cur_s["o_xml_level"]=$xml_level+1;

		if(str_replace(" ", "", $tag)==$cur_s["cur_p"]){/* container prop in XML */
			$cur_s["p_xml_literal_level"]=$cur_s["p_xml_literal_level"]+1;
		}
	}
	
	/*					*/

	function handle_close($parser, $tag){
		/* echo "at state ".$this->state." closing ".$tag."\n"; */
		switch($this->state){
			case 3:/* p _close_ */
				$this->state=2;
				break;
			case 2:/* no (more) props */
				if($cur_s=$this->subjs[$this->s_count-1]){
					$cur_p=isset($cur_s["cur_p"]) ? $cur_s["cur_p"] : "";
					if($cur_p===str_replace(" ", "", $tag) || ($tag==="http://www.w3.org/1999/02/22-rdf-syntax-ns# li" && $cur_p==="http://www.w3.org/1999/02/22-rdf-syntax-ns#_".$cur_s["li_count"])){
						/* closing p */
					}
					else{
						$this->pop_s();
						$this->state = (@$this->subjs[$this->s_count-1]) ? 2 : 1;/* s was o of upper triple | back at root, expecting siblings */
					}
					if(@$cur_s["in_list"]){
						$this->state=4;
					}
				}
				
				break;
			case 4:/* empty p or p_close after cdata reading or p_close after collection */
				$cur_s=&$this->subjs[$this->s_count-1];
				if(@$cur_s["is_list"]){
					$this->add_triple($cur_s, "http://www.w3.org/1999/02/22-rdf-syntax-ns#rest", array("type"=>"uri", "uri"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
					/* back to list start */
					$coll_p=str_replace(" ", "", $tag);
					while($cur_s["cur_p"]!=$coll_p){
						$next_s=$cur_s;
						$this->pop_s();
						$cur_s=&$this->subjs[$this->s_count-1];
					}
					if($p_rdf_ID=$cur_s["p_rdf_ID"]){/* reify */
						$this->reify(array("type"=>"uri", "uri"=>$this->calc_uri($cur_s, $p_rdf_ID, "ID")), $cur_s, array("type"=>"uri", "uri"=>$cur_s["cur_p"]), $next_s);
					}
					$this->state=2;
				}
				else{
					$this->add_triple($cur_s, $cur_s["cur_p"], array("type"=>"literal", "val"=>@$cur_s["o_cdata"], "dt"=>@$cur_s["o_rdf_datatype"], "lang"=>$this->get_cur_lang($cur_s)));
					if($p_rdf_ID=@$cur_s["p_rdf_ID"]){/* reify */
						$this->reify(array("type"=>"uri", "uri"=>$this->calc_uri($cur_s, $p_rdf_ID, "ID")), $cur_s, array("type"=>"uri", "uri"=>$cur_s["cur_p"]), array("type"=>"literal", "val"=>$cur_s["o_cdata"], "dt"=>$cur_s["o_rdf_datatype"], "lang"=>$this->get_cur_lang($cur_s)));
					}
					unset($cur_s["o_cdata"]);
					unset($cur_s["o_rdf_datatype"]);
					$this->state=2;
				}
				break;
			case 6:/* expecting xml data */
				$cur_s=&$this->subjs[$this->s_count-1];
				$data=$cur_s["o_xml_data"];
				$xml_level=$cur_s["o_xml_level"];
				if($xml_level===0){/* p close after xml reading */
					$this->add_triple($cur_s, $cur_s["cur_p"], array("type"=>"literal", "val"=>trim($data), "dt"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral", "lang"=>$this->get_cur_lang($cur_s)));
					unset($cur_s["o_xml_data"]);
					$this->state=2;
				}
				else{
					$tag_parts=explode(" ", $tag);
					if(count($tag_parts)==1){/* no qname */
						$data.='</'.$tag.'>';
					}
					else{
						$ns_uri=$tag_parts[0];
						$local_name=$tag_parts[1];
						$nsp=$this->nsps[$ns_uri];
						$data.=(strlen($nsp)) ? '</'.$nsp.":".$local_name.'>' : '</'.$local_name.'>';
					}
					$cur_s["o_xml_data"]=$data;
					$cur_s["o_xml_level"]=$xml_level-1;
					if(str_replace(" ", "", $tag)===$cur_s["cur_p"]){/* container prop in XML */
						$cur_s["p_xml_literal_level"]--;
					}
				}
				break;
		}
	}

	/*					*/

	function handle_cdata($parser, $cdata){
		switch($this->state){
			case 6:
				$cur_s=&$this->subjs[$this->s_count-1];
				if(isset($cur_s["o_xml_data"])){
					$cur_s["o_xml_data"].=$cdata;
				}
				elseif(($cdata=="\n") || ($cdata=="\r\n")){
					$cur_s["o_xml_data"]=$cdata;
				}
				elseif(trim($cdata)){
					$cur_s["o_xml_data"]=$cdata;
				}
				break;
			case 4:
				$cur_s=&$this->subjs[$this->s_count-1];
				if(isset($cur_s["o_cdata"])){
					$cur_s["o_cdata"].=$cdata;
				}
				else{
					$cur_s["o_cdata"]=$cdata;
				}
				break;
		}
	}

	/*					*/
	
	function handle_ns_decl($parser, $nsp, $ns_uri){
		$this->nsps[$ns_uri]=$nsp;
	}

	/*					*/

	function get_triples(){
		return $this->triples;
	}

	function get_result_headers(){
		return $this->result_headers;
	}
	
	/*					*/

	function parse_web_file($url="", $redir_count=0){
		if(!isset($this->init_args["base"]) || !$this->init_args["base"]){
			$this->init_args["base"]=$url;
		}
		$this->init(false);
		if(!$url){$url=$this->full_base;}
		if($url){
			if($redir_count){$this->parsed_url=$url;}
			/* http method */
			$http_method=isset($this->init_args["http_method"]) ? $this->init_args["http_method"] : "GET";
			$url_parts=parse_url($url);
			if(!isset($url_parts["port"])){
				$url_parts["port"]=80;
			}
			if((isset($url_parts["user"]) && strlen($url_parts["user"])) || ($this->init_args["proxy_host"] && $this->init_args["proxy_port"])){
				$http_code=$http_method.' '.$url.' HTTP/1.0'."\r\n";
			}
			else{
				$http_code=$http_method.' '.$url_parts["path"];
				$http_code.=(isset($url_parts["query"]) && strlen($url_parts["query"])) ? "?".$url_parts["query"] : "";
				$http_code.=(isset($url_parts["fragment"]) && strlen($url_parts["fragment"])) ? "#".$url_parts["fragment"] : "";
				$http_code.=' HTTP/1.0'."\r\n";
			}
			/* custom headers */
			if($headers=$this->init_args["headers"]){
				for($i=0,$i_max=count($headers);$i<$i_max;$i++){
					$http_code.=$headers[$i]."\r\n";
				}
			}
			if(strpos($http_code, "Host: ")===false){
				$http_code.='Host: '.$url_parts["host"]."\r\n";
			}
			if(strpos($http_code, "Accept: ")===false){
				$http_code.='Accept: application/rdf+xml; q=0.9, */*; q=0.1'."\r\n";
			}
			if(strpos($http_code, "User-Agent: ")===false){
				$ua_string=($this->init_args["user_agent"]) ? $this->init_args["user_agent"] : "ARC RDF/XML Parser v".$this->version." (http://www.appmosphere.com/en-arc_rdfxml_parser)";
				$http_code.='User-Agent: '.$ua_string."\r\n";
			}
			$http_code.="\r\n";
			/* socket */
			if($this->init_args["proxy_host"] && $this->init_args["proxy_port"]){
				$fp=@fsockopen($this->init_args["proxy_host"], $this->init_args["proxy_port"]);
				$server_str=$this->init_args["proxy_host"].":".$this->init_args["proxy_port"];
			}
			else{
				$fp=@fsockopen($url_parts["host"], $url_parts["port"]);
				$server_str=$url_parts["host"].":".$url_parts["port"];
			}
			if(!$fp){
				return "Socket error: could not connect to server '".$server_str."'";
			}
			else{
				fputs($fp, $http_code);
				/* http-headers */
				$cur_line=fgets($fp, 256);
				/* 304/4xx/5xx handling */
				if(preg_match("/^HTTP[^\s]+\s+([0-9]{1})([0-9]{2})(.*)$/i", trim($cur_line), $matches)){
					$code_1=$matches[1];
					$code_2=$matches[2];
					$msg=trim($matches[3]);
					if(in_array($code_1, array("4", "5"))){
						return $code_1.$code_2." ".$msg;
					}
					if($code_1.$code_2=="304"){
						return $code_1.$code_2." ".$msg;
					}
					$redirect=($code_1=="3") ? true : false;
				}
				while(!feof($fp) && trim($cur_line)){
					$this->result_headers[]=$cur_line;
					if(($this->encoding=="auto") && (strpos(strtolower($cur_line), "content-type")!==false)){
						if(strpos(strtolower($cur_line), "utf-8")){$this->encoding="UTF-8";}
						elseif(strpos(strtolower($cur_line), "iso-8859-1")){$this->encoding="ISO-8859-1";}
						elseif(strpos(strtolower($cur_line), "us-ascii")){$this->encoding="US-ASCII";}
					}
					/* 3xx handling */
					if($redirect && preg_match("/^Location:\s*(http.*)$/i", $cur_line, $matches)){
						fclose($fp);
						unset($this->encoding);
						unset($this->init_args["base"]);
						return ($redir_count>3) ? $cur_line : $this->parse_web_file(trim($matches[1]), $redir_count+1);
					}
					$cur_line=fgets($fp, 256);
				}
				/* first lines of body to detect encoding */
				$pre_data=fread($fp, 512);
				if(($this->encoding=="auto") && (preg_match("/\<\?xml .* encoding(.+).*\?\>/", $pre_data, $matches))){
					$cur_match=$matches[1];
					if(strpos(strtolower($cur_match), "utf-8")){$this->encoding="UTF-8";}
					elseif(strpos(strtolower($cur_match), "iso-8859-1")){$this->encoding="ISO-8859-1";}
					elseif(strpos(strtolower($cur_match), "us-ascii")){$this->encoding="US-ASCII";}
				}
				if($this->encoding=="auto"){
					$this->encoding="UTF-8";
				}
				$this->create_parser();
				/* body */
				$max_lns=$this->max_lines;
				while(($data=$pre_data.fread($fp, 4096)) && (($max_lns===0) || xml_get_current_line_number($this->parser)<=$max_lns)){
					$started=true;
					$pre_data="";
					if($this->save_data){
						$this->data.=$data;
					}
					if(!$success=xml_parse($this->parser, $data, feof($fp))){
						$error_str = xml_error_string(xml_get_error_code($this->parser));
						$line = xml_get_current_line_number($this->parser);
						fclose($fp);
						xml_parser_free($this->parser);
						return "XML error: '".$error_str."' at line ".$line."\n";
					}
				}
				$this->target_encoding=xml_parser_get_option($this->parser, XML_OPTION_TARGET_ENCODING);
				xml_parser_free($this->parser);
				fclose($fp);
				$this->done();
			}
		}
		return $this->triples;
	}


	function parse_file($path){
		if($fp=fopen($path, "r")){
			if(!$this->init_args["base"]){
				$this->init_args["base"]=$path;
			}
			$this->init(false);
			$this->encoding=($this->encoding=="auto") ? "UTF-8" : $this->encoding;
			$this->create_parser();
			while($data=fread($fp, 4096)){
				if($this->save_data){
					$this->data.=$data;
				}
				if(!$success=xml_parse($this->parser, $data, feof($fp))){
					$error_str = xml_error_string(xml_get_error_code($this->parser));
					$line = xml_get_current_line_number($this->parser);
					fclose($fp);
					xml_parser_free($this->parser);
					return "XML error: '".$error_str."' at line ".$line."\n";
				}
			}
			$this->target_encoding=xml_parser_get_option($this->parser, XML_OPTION_TARGET_ENCODING);
			xml_parser_free($this->parser);
			fclose($fp);
			$this->done();
		}
		return $this->triples;
	}
	
	function parse_data($data){
		$this->init(false);
		$this->encoding=($this->encoding=="auto") ? "UTF-8" : $this->encoding;
		$this->create_parser();
		if($this->save_data){
			$this->data=$data;
		}
		if(!$success=xml_parse($this->parser, $data, true)){
			$error_str = xml_error_string(xml_get_error_code($this->parser));
			$line = xml_get_current_line_number($this->parser);
			xml_parser_free($this->parser);
			return "XML error: '".$error_str."' at line ".$line."\n";
		}
		$this->target_encoding=xml_parser_get_option($this->parser, XML_OPTION_TARGET_ENCODING);
		xml_parser_free($this->parser);
		$this->done();
		return $this->triples;
	}
		
	/*					*/
}

?>