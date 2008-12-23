<?php
/*
appmosphere RDF classes (ARC): http://arc.web-semantics.org/

Copyright © 2006 appmosphere web applications, Germany. All Rights Reserved.
This work is distributed under the W3C® Software License [1] in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231

ns doap                     http://usefulinc.com/ns/doap#
doap:name                   ARC embedded RDF Parser
doap:license                http://arc.web-semantics.org/license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
//release
doap:created                2006-05-30
doap:revision               0.1.3
//changelog
sk:releaseChanges           2006-05-28: release 0.1.0
                            2006-05-29: revision 0.1.1
                                        - added support for typing class values (e.g. class="-foaf-Person")
                                        - fixed some bugs (base handling, IRI calculation)
                            2006-05-30: revision 0.1.2
                                        - fix: schemas/prefixes can be defined after meta tags in the head section now
                            2006-05-30: revision 0.1.3
                                        - fix: nested literals work with PHP5 now
*/

class ARC_erdf_parser{

	var $version="0.1.3";

	function __construct($args=""){
		$this->init_args=is_array($args) ? $args : array();/* base, proxy_host, proxy_port, headers, save_data, encoding */
	}
	
	function ARC_erdf_parser($args=""){
		$this->__construct($args);
	}

	/*					*/
	
	function init($create_parser=true){
		$this->triples=array();
		$this->nodes=array();
		$this->node_count=0;
		$this->subjs=array();
		$this->subj_count=0;
		$this->base="";
		$this->result_headers=array();
		$this->data="";
		$this->level=0;
		$this->is_erdf=false;
		/* base */
		if(isset($this->init_args["base"]) && ($base=$this->init_args["base"])){
			$this->set_base($base);
		}
		/* subj */
		$this->cur_subj=$this->base;
		/* save_data */
		$this->save_data=(isset($this->init_args["save_data"])) ? $this->init_args["save_data"] : false;
		/* encoding */
		$this->encoding=(isset($this->init_args["encoding"]) && $this->init_args["encoding"]) ? $this->init_args["encoding"] : "UTF-8";
		/* parser */
		if($create_parser){
			$this->create_parser();
		}
	}

	/*					*/

	function set_base($base){
		$this->base=$base;
	}

	/*					*/
	
	function calc_abs_iri($path=""){
		$base=$this->base;
		$path=preg_replace("/^\.\//", "", $path);
		if(strpos($path, "/")===0){/* leading slash */
			if(preg_match("/([^\/]*[\/]{1,2}[^\/]+)\//", $base, $matches)){
				return $matches[1].$path;
			}
		}
		elseif($path==""){
			return $base;
		}
		elseif(strpos($path, "#")===0){
			return $base.$path;
		}
		elseif(preg_match("/^[a-z0-9]+\:/i", $path)){/* abs path */
			return $path;
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
	
	/*					*/
	
	function get_target_encoding(){
		return $this->target_encoding;
	}
	
	function get_result_headers(){
		return $this->result_headers;
	}
	
	function get_data(){
		return $this->data;
	}
	
	function get_parsed_url(){
		return (isset($this->parsed_url)) ? $this->parsed_url : "";
	}

	/*					*/

	function parse_web_file($url="", $redir_count=0){
		if(!isset($this->init_args["base"])){
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
			if((isset($url_parts["user"]) && strlen($url_parts["user"])) || (isset($this->init_args["proxy_host"]) && $this->init_args["proxy_host"] && isset($this->init_args["proxy_port"]) && $this->init_args["proxy_port"])){
				$http_code=$http_method.' '.$url.' HTTP/1.0'."\r\n";
			}
			else{
				$http_code=$http_method.' ';
				$http_code.=(isset($url_parts["path"])) ? $url_parts["path"] : '/';
				$http_code.=(isset($url_parts["query"]) && strlen($url_parts["query"])) ? "?".$url_parts["query"] : "";
				$http_code.=(isset($url_parts["fragment"]) && strlen($url_parts["fragment"])) ? "#".$url_parts["fragment"] : "";
				$http_code.=' HTTP/1.0'."\r\n";
			}
			/* custom headers */
			if(isset($this->init_args["headers"]) && ($headers=$this->init_args["headers"])){
				for($i=0,$i_max=count($headers);$i<$i_max;$i++){
					$http_code.=$headers[$i]."\r\n";
				}
			}
			if(strpos($http_code, "Host: ")===false){
				$http_code.='Host: '.$url_parts["host"]."\r\n";
			}
			if(strpos($http_code, "Accept: ")===false){
				$http_code.='Accept: text/html; q=0.9, */*; q=0.1'."\r\n";
			}
			if(strpos($http_code, "User-Agent: ")===false){
				$http_code.='User-Agent: ARC eRDF Parser v'.$this->version.' (http://arc.web-semantics.org/)'."\r\n";
			}
			$http_code.="\r\n";
			/* socket */
			if(isset($this->init_args["proxy_host"]) && $this->init_args["proxy_host"] && isset($this->init_args["proxy_port"]) && $this->init_args["proxy_port"]){
				$fp=@fsockopen($this->init_args["proxy_host"], $this->init_args["proxy_port"]);
				$server_str=$this->init_args["proxy_host"].":".$this->init_args["proxy_port"];
			}
			else{
				$fp=@fsockopen($url_parts["host"], $url_parts["port"]);
				$server_str=$url_parts["host"].":".$url_parts["port"];
			}
			if(!$fp){
				return array("error"=>"Socket error: could not connect to server '".$server_str."'", "result"=>"");
			}
			else{
				$redirect=false;
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
				while($data=$pre_data.fread($fp, 4096)){
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
						return array("error"=>"XML error: '".$error_str."' at line ".$line."\n", "result"=>"");
					}
				}
				$this->target_encoding=xml_parser_get_option($this->parser, XML_OPTION_TARGET_ENCODING);
				xml_parser_free($this->parser);
				fclose($fp);
			}
		}
		return $this->done();
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
		}
		return $this->done();
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
		return $this->done();
	}
		
	/*					*/

	function create_parser(){
		$parser=xml_parser_create($this->encoding);
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE,0);
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
		xml_set_element_handler($parser, "handle_open", "handle_close");
		xml_set_character_data_handler($parser, "handle_cdata");
		xml_set_object($parser, $this);
		$this->parser=$parser;
	}

	/*					*/

	function push_node($node){
		$node["id"]=$this->node_count;
		$this->nodes[$this->node_count] = $node;
		$this->node_count++;
	}
	
	function get_cur_node(){
		return $this->nodes[$this->node_count-1];
	}
	
	function update_node($node){
		$this->nodes[$node["id"]]=$node;
	}

	/*					*/

	function push_subj($subj){
		$this->subjs[$this->subj_count] = $subj;
		$this->subj_count++;
	}
	
	function pop_subj(){
		$new_subjs=array();
		$this->subj_count--;
		for($i=0,$i_max=$this->subj_count;$i<$i_max;$i++){
			$new_subjs[] = $this->subjs[$i];
		}
		$this->subjs = $new_subjs;
	}
	
	function get_cur_subj(){
		return $this->subjs[$this->subj_count-1];
	}
	
	/*					*/

	function handle_open($parser, $tag, $attrs){
		/* erdf check */
		if(isset($attrs["profile"]) && (strpos($attrs["profile"], "http://purl.org/NET/erdf/profile")!==false)){
			$this->is_erdf=true;
		}
		/* base check */
		if(($tag=="base") && isset($attrs["href"])){
			$this->set_base($attrs["href"]);
		}
		/* href, src, id */
		if(isset($attrs["href"])){
			$attrs["full_href"]=$this->calc_abs_iri($attrs["href"]);
		}
		if(isset($attrs["src"])){
			$attrs["full_src"]=$this->calc_abs_iri($attrs["src"]);
		}
		if(isset($attrs["id"])){
			$attrs["full_id"]=$this->calc_abs_iri("#".$attrs["id"]);
		}
		/* node */
		$node=array(
			"tag"=>$tag,
			"attrs"=>$attrs, 
			"subj"=>($this->subj_count) ? $this->get_cur_subj() : $this->base, 
			"level"=>$this->level, 
			"pos"=>0,
			"p_id"=>$this->node_count-1,
			"state"=>"open",
			"cdata"=>""
		);
		/* parent */
		if($this->node_count){
			$prev_node = $this->get_cur_node();
			if($prev_node["level"]==$this->level){
				$node["p_id"]=$prev_node["p_id"];
				$node["pos"]=$prev_node["pos"]+1;
			}
			elseif($prev_node["level"] > $this->level){
				while($prev_node["level"] > $this->level){
					$prev_node=$this->nodes[$prev_node["p_id"]];
				}
				$node["p_id"]=$prev_node["p_id"];
				$node["pos"]=$prev_node["pos"]+1;
			}
		}
		$this->push_node($node);
		$this->level++;
		/* subj */
		$subj=$node["subj"];
		if(isset($attrs["href"])){
			$subj=$attrs["full_href"];
		}
		elseif(isset($attrs["id"])){
			$subj=$this->calc_abs_iri("#".$attrs["id"]);
		}
		$this->push_subj($subj);
		/* cdata */
		$this->cur_cdata="";
	}
	
	/*					*/

	function handle_close($parser, $tag){
		$node = $this->get_cur_node();
		$node["state"]="closed";
		$this->update_node($node);
		$this->pop_subj();
		$this->level--;
	}

	/*					*/

	function handle_cdata($parser, $cdata){
		$node = $this->get_cur_node();
		if($cdata){
			if($node["state"]=="open"){
				$node["cdata"].=$cdata;
				$this->update_node($node);
			}
			else{/* cdata is sibling of node */
				$this->handle_open($parser, "cdata", array("val"=>$cdata));
				$this->handle_close($parser, "cdata");
			}
		}
	}

	/*					*/

	function done(){
		return array("error"=>"", "result"=>$this->is_erdf);
	}
	
	/*					*/
	
	function index_nodes_by_parent(){
		if(!isset($this->nodes_by_parent)){
			/* index by parent */
			$nodes_by_p=array();
			for($i=0,$i_max=count($this->nodes);$i<$i_max;$i++){
				$cur_node=$this->nodes[$i];
				$cur_node["id"]=$i;
				$cur_p_id=$cur_node["p_id"];
				if($cur_p_id!=-1){/* ignore root tag */
					if(!isset($nodes_by_p[$cur_p_id])){
						$nodes_by_p[$cur_p_id]=array();
					}
					$cur_pos=$cur_node["pos"];
					$nodes_by_p[$cur_p_id][$cur_pos]=$cur_node;
				}
			}
			$this->nodes_by_parent=$nodes_by_p;
		}
	}

	/*					*/

	function get_all_cdata($node){
		$result=$node["cdata"];
		if(isset($node["attrs"]["val"])){
			$result.=$node["attrs"]["val"];
		}
		/* child nodes */
		$id=$node["id"];
		if(isset($this->nodes_by_parent[$id])){
			foreach($this->nodes_by_parent[$id] as $cur_child_node){
				$result.=$this->get_all_cdata($cur_child_node);
			}
		}
		return $result;
	}

	/*					*/

	function get_triple_infos($args=""){
		if(!$this->is_erdf && (!is_array($args) || !isset($args["ignore_missing_profile"]) || !$args["ignore_missing_profile"])){
			return array("error"=>"could not extract triples", "result"=>array());
		}
		$triples=array();
		$prefixes=array();
		$same_ids=array();
		$this->index_nodes_by_parent();
		/* prefixes */
		if(isset($this->nodes_by_parent[1])){
			$head_nodes=$this->nodes_by_parent[1];
			foreach($head_nodes as $cur_node){
				$tag=$cur_node["tag"];
				$attrs=$cur_node["attrs"];
				foreach(array("rel", "href") as $cur_attr){
					$$cur_attr=(isset($attrs[$cur_attr])) ? trim($attrs[$cur_attr]) : "";
				}
				/* link (schema definitions) */
				if(($tag=="link") && preg_match("/^schema\.([0-9a-z_]+)$/i", $rel, $m)){
					$prefix=$m[1];
					$iri=$href;
					if(!isset($prefixes[$prefix])){
						$prefixes[$prefix]=$iri;
					}
				}
			}
		}
		/* triples */
		foreach($this->nodes_by_parent as $p_id=>$cur_p){
			foreach($cur_p as $cur_node){
				$tag=$cur_node["tag"];
				$subj=isset($same_ids[$cur_node["subj"]]) ? $same_ids[$cur_node["subj"]] : $cur_node["subj"];
				$attrs=$cur_node["attrs"];
				foreach(array("rel", "name", "href", "src", "content", "class", "id", "rev", "title") as $cur_attr){
					$$cur_attr=(isset($attrs["full_".$cur_attr])) ? trim($attrs["full_".$cur_attr]) : (isset($attrs[$cur_attr]) ? trim($attrs[$cur_attr]) : "");
				}
				/* meta name content */
				if(($tag=="meta") && preg_match_all("/([0-9a-z_]+)[\.\-]([^\s]+)/si", $name, $m)){
					for($i=0,$i_max=count($m[1]);$i<$i_max;$i++){
						$prefix=$m[1][$i];
						$local_name=$m[2][$i];
						if(isset($prefixes[$prefix])){
							$triples[]=array("s"=>$subj, "p"=>$prefixes[$prefix].$local_name, "p_qname"=>$prefix.":".$local_name, "p_type"=>"dt", "o"=>$content);
						}
					}
				}
				/* link|a rel href */
				if(in_array($tag, array("link", "a")) && preg_match_all("/([0-9a-z_]+)[\.\-]([^\s]+)/si", $rel, $m)){
					for($i=0,$i_max=count($m[1]);$i<$i_max;$i++){
						$prefix=$m[1][$i];
						$local_name=$m[2][$i];
						if(isset($prefixes[$prefix])){
							$p=$prefixes[$prefix].$local_name;
							if($p=="http://www.w3.org/2002/07/owl#sameAs"){
								$same_ids[$subj]=$href;
							}
							else{
								$triples[]=array("s"=>$subj, "p"=>$p, "p_qname"=>$prefix.":".$local_name, "p_type"=>"obj", "o"=>$href);
								if($title){
									$triples[]=array("s"=>$href, "p"=>"http://www.w3.org/2000/01/rdf-schema#label", "p_qname"=>"rdfs:label", "p_type"=>"dt", "o"=>$title);
								}
								elseif($label=$this->get_all_cdata($cur_node)){
									$triples[]=array("s"=>$href, "p"=>"http://www.w3.org/2000/01/rdf-schema#label", "p_qname"=>"rdfs:label", "p_type"=>"dt", "o"=>$label);
								}
							}
						}
					}
				}
				/* link|a rev href */
				if(in_array($tag, array("link", "a")) && preg_match_all("/([0-9a-z_]+)[\.\-]([^\s]+)/si", $rev, $m)){
					for($i=0,$i_max=count($m[1]);$i<$i_max;$i++){
						$prefix=$m[1][$i];
						$local_name=$m[2][$i];
						if(isset($prefixes[$prefix])){
							$triples[]=array("s"=>trim($href), "p"=>$prefixes[$prefix].$local_name, "p_qname"=>$prefix.":".$local_name, "p_type"=>"obj", "o"=>$subj);
							if($title){
								$triples[]=array("s"=>$href, "p"=>"http://www.w3.org/2000/01/rdf-schema#label", "p_qname"=>"rdfs:label", "p_type"=>"dt", "o"=>$title);
							}
							elseif($label=$this->get_all_cdata($cur_node)){
								$triples[]=array("s"=>$href, "p"=>"http://www.w3.org/2000/01/rdf-schema#label", "p_qname"=>"rdfs:label", "p_type"=>"dt", "o"=>$label);
							}
						}
					}
				}
				/* img class src */
				if(($tag=="img") && preg_match_all("/(\-?)([0-9a-z_]+)[\.\-]([^\s]+)/si", $class, $m)){
					for($i=0,$i_max=count($m[2]);$i<$i_max;$i++){
						$as_class=($m[1][$i]=="-");
						$prefix=$m[2][$i];
						$local_name=$m[3][$i];
						if(isset($prefixes[$prefix])){
							if($as_class){
								$triples[]=array("s"=>$src, "p"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#type", "p_qname"=>"rdf:type", "p_type"=>"obj", "o"=>$prefixes[$prefix].$local_name);
							}
							else{
								$triples[]=array("s"=>$subj, "p"=>$prefixes[$prefix].$local_name, "p_qname"=>$prefix.":".$local_name, "p_type"=>"obj", "o"=>$src);
							}
							if($title){
								$triples[]=array("s"=>$src, "p"=>"http://www.w3.org/2000/01/rdf-schema#label", "p_qname"=>"rdfs:label", "p_type"=>"dt", "o"=>$title);
							}
						}
					}
				}
				/* class */
				if(!in_array($tag, array("img", "a", "link", "meta", "object", "iframe")) && preg_match_all("/(\-?)([0-9a-z_]+)[\.\-]([^\s]+)/si", $class, $m)){
					for($i=0,$i_max=count($m[2]);$i<$i_max;$i++){
						$as_class=($m[1][$i]=="-");
						$prefix=$m[2][$i];
						$local_name=$m[3][$i];
						if(isset($prefixes[$prefix])){
							if($id){
								if($as_class){
									$triples[]=array("s"=>$id, "p"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#type", "p_qname"=>"rdf:type", "p_type"=>"obj", "o"=>$prefixes[$prefix].$local_name);
								}
								else{
									$triples[]=array("s"=>$subj, "p"=>$prefixes[$prefix].$local_name, "p_qname"=>$prefix.":".$local_name, "p_type"=>"obj", "o"=>$id);
								}
							}
							elseif(isset($attrs["title"])){
								$triples[]=array("s"=>$subj, "p"=>$prefixes[$prefix].$local_name, "p_qname"=>$prefix.":".$local_name, "p_type"=>"dt", "o"=>$title);
							}
							elseif($label=$this->get_all_cdata($cur_node)){
								$triples[]=array("s"=>$subj, "p"=>$prefixes[$prefix].$local_name, "p_qname"=>$prefix.":".$local_name, "p_type"=>"dt", "o"=>$label);
							}
						}
					}
				}
			}
		}
		return array("triples"=>$triples, "prefixes"=>$prefixes, "same_ids"=>$same_ids);
	}
	
	function get_triples(){
		$infos=$this->get_triple_infos();
		return (isset($infos["error"]) && $infos["error"]) ? array() : $infos["triples"];
	}
	
	/*					*/
	
	function get_rdfxml(){
		$infos=$this->get_triple_infos();
		$nl="\n";
		$ind="  ";
		$ni=$nl.$ind;
		$ni2=$ni.$ind;
		$result='<?xml version="1.0" encoding="UTF-8"?>';
		$result.=$nl.'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"';
		if(isset($infos["triples"])){
			$triples=$infos["triples"];
			$prefixes=$infos["prefixes"];
			if(!isset($prefixes["rdfs"])){
				$prefixes["rdfs"]="http://www.w3.org/2000/01/rdf-schema#";
			}
			if(!isset($prefixes["owl"])){
				$prefixes["owl"]="http://www.w3.org/2002/07/owl#";
			}
			/* ns declarations */
			foreach($prefixes as $prefix=>$iri){
				$result.=(!in_array($prefix, array("rdf"))) ? $ni.'xmlns:'.$prefix.'="'.htmlspecialchars($iri).'"' : '';
			}
			/* base */
			$result.=$ni.'xml:base="'.htmlspecialchars($this->base).'"';
			$result.='>';
			/* triples */
			$r_props=array();
			foreach($triples as $t){
				$cur_result="";
				/* s */
				$s=$t["s"];
				if(!isset($r_props[$s])){
					$r_props[$s]=array();
				}
				/* p, o */
				$p=$t["p"];
				$p_qname=$t["p_qname"];
				$p_type=$t["p_type"];
				$cur_result=$ni2.'<'.$p_qname;
				if($p_type=="obj"){
					$cur_result.=' rdf:resource="'.htmlspecialchars($t["o"]).'" />';
				}
				else{
					$cur_result.='>'.htmlspecialchars(trim($t["o"])).'</'.$p_qname.'>';
				}
				if(!in_array($cur_result, $r_props[$s])){
					$r_props[$s][]=$cur_result;
				}
			}
			foreach($r_props as $s=>$props){
				$result.=$nl.$ni.'<rdf:Description rdf:about="'.htmlspecialchars($s).'">';
				foreach($props as $cur_prop){
					$result.=$cur_prop;
				}
				$result.=$ni.'</rdf:Description>';
			}
		}
		else{
			$result.='>';
		}
		$result.=$nl.'</rdf:RDF>'.$nl;
		return $result;
	}

	/*					*/

}

?>