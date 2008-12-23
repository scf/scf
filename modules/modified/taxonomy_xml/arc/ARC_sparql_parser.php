<?php
/*
appmosphere RDF classes (ARC): http://www.appmosphere.com/en-arc

Copyright © 2005 appmosphere web applications, Germany. All Rights Reserved.
This work is distributed under the W3C® Software License [1] in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231

ns sk                       http://www.appmosphere.com/ns/site_kit#
ns doap                     http://usefulinc.com/ns/doap#
sk:className                ARC_sparql_parser
doap:name                   ARC SPARQL Parser
doap:homepage               http://www.appmosphere.com/en-arc_sparql_parser
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              A PHP SPARQL parser
//release
doap:created                2005-06-06
doap:revision               0.2.5
//changelog
sk:releaseChanges           2005-04-25: release 0.1.0
                            2005-12-15: release 0.2.0
                                        - complete re-write
                                        - updated to SPARQL WD 2005-11-23
                            2006-01-04: revision 0.2.2
                                        - fixed group graph pattern bug
                                        - supports numbers in prefixes now
                                        - made where clause optional in describe queries
                                        - split associative "datasets" array in flat "datasets" and "named_datasets" arrays
                            2006-01-25: revision 0.2.3
                                        - bug fix and structure change in ValueLogical (returns unparsed_val now)
                            2006-02-07: revision 0.2.4
                                        - bug fix in turtle parser (numeric literal detection)
                            2006-06-06: revision 0.2.5
                                        - some regex fixes in qname and blank node parsing methods
*/

class ARC_sparql_parser{

	var $version="0.2.5";

	var $init_args=array();
	
	var $bnode_prefix="";
	var $bnode_count=0;
	
	var $base="";
	var $q="";
	
	function __construct($args=""){
		if(is_array($args)){
			$this->init_args=$args;
			foreach($args as $k=>$v){$this->$k=$v;}/* base, bnode_prefix */
		}
	}
	
	function ARC_sparql_parser($args=""){
		$this->__construct($args);
	}

	/*					*/

	function get_infos(){
		return $this->infos;
	}
	
	function get_query(){
		return $this->q;
	}
	
	/*					*/
	
	function set_bnode_prefix($prefix=""){
		$this->bnode_prefix=($prefix) ? $prefix : "arc".substr(md5(microtime()), 0, 4)."b";
	}

	/*					*/
	
	function get_warnings(){
		return $this->warnings;
	}
	
	function get_errors(){
		return $this->errors;
	}
	
	function get_logs(){
		return $this->logs;
	}
	
	function get_log(){
		return "- ".implode("<br />- ", $this->logs);
	}
	
	/*					*/
	
	function get_default_prefixes(){
		if(!isset($this->default_prefixes)){
			$this->set_default_prefixes();
		}
		return $this->default_prefixes;
	}
	
	function set_default_prefixes(){
		$this->default_prefixes=array(
			"rdf:"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#",
			"rdfs:"=>"http://www.w3.org/2000/01/rdf-schema#",
			"owl:"=>"http://www.w3.org/2002/07/owl#",
			"xsd:"=>"http://www.w3.org/2001/XMLSchema#",

			"dc:"=>"http://purl.org/dc/elements/1.1/",
			"dct:"=>"http://purl.org/dc/terms/",
			"dcterms:"=>"http://purl.org/dc/terms/",
			"rss:"=>"http://purl.org/rss/1.0/",
			"foaf:"=>"http://xmlns.com/foaf/0.1/",
			"doap:"=>"http://usefulinc.com/ns/doap#"
		);
	}

	/*					*/

	function get_next_bnode_id(){
		$this->bnode_count++;
		return "_:".$this->bnode_prefix.$this->bnode_count;
	}
	
	/*					*/
	
	function set_base($base=""){
		$this->base=$this->get_url_base($base);
		if(!$base){
			$this->warnings[]="empty base";
		}
	}

	function get_url_base($url=""){
		$base=$url;
		/* remove fragment */
		if(preg_match("/([^#]*)[#]?/", $url, $matches)){/* should always match, remove fragment */
			$base=$matches[1];
		}
		/* no path, no query, no trailing slash, e.g. http://www.example.com  -> add slash */
		if(preg_match("/\/\/[^\/]+$/", $base, $matches)){/* //+no more slashes */
			$base.="/";
		}
		return $base;
	}
	
	function calc_iri($path=""){
		$result="";
		/* abs uri */
		if(strpos($path, ":")!==false){
			if((strpos($path, "/")===false) || (strpos($path, "/") > strpos($path, ":"))){/* no slash after colon */
				return $path;
			}
		}
		if(strpos($path, "//")===0){/* net path */
			return "http:".$path;
		}
		/* rel uri */
		$cur_base=$this->get_url_base($this->base);
		if(strpos($path, "#")===0){
			return $cur_base.$path;
		}
		if(strpos($path, "/")===0){/* leading slash */
			if(preg_match("/([^\/]*[\/]{1,2}[^\/]+)\//", $cur_base, $matches)){
				return $matches[1].$path;
			}
		}
		if($path==""){
			return $cur_base;
		}
		/* rel path (../ or  path) */
		$cur_base=substr($cur_base, 0, strrpos($cur_base, "/"))."/";/* remove stuff after last slash */
		if(strpos($path, "../")===0){
			if(preg_match("/([^\/]*[\/]{1,2}[^\/]+\/)(.*)\//", $cur_base, $matches)){
				$server_part=$matches[1];
				$path_part=$matches[2];/* empty or with trailing / */
			}
			else{
				$server_part=$cur_base;
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
			return $cur_base.$path;
		}
		return $path;
	}
	
	/*					*/

	function extract_vars($val=""){
		$vars=array();
		if(preg_match_all("/[\?\$]{1}([0-9a-z_]+)/i", $val, $matches)){
			foreach($matches[1] as $cur_var){
				if(!in_array($cur_var, $vars)){
					$vars[]=$cur_var;
					$this->logs[]="adding var ".$cur_var;
				}
			}
		}
		return $vars;
	}
	
	/*					*/
	
	function expand_qname($val=""){
		$iri="";
		if(preg_match("/(.*\:)(.*)/", $val, $matches)){
			$prefix=$matches[1];
			$name=$matches[2];
			if(array_key_exists($prefix, $this->prefixes)){
				$iri=$this->prefixes[$prefix].$name;
			}
			elseif(array_key_exists($prefix, $this->default_prefixes)){
				$iri=$this->default_prefixes[$prefix].$name;
			}
			if(!in_array($iri, $this->iris)){
				$this->iris[]=$iri;
			}
			return $iri;
		}
		$this->errors[]="could not expand '".$val."' in expand_qname()";
		return $val;
	}
	
	function expand_to_iri($val=""){
		if(strpos($val, ":")!==false){/* qname */
			return $this->expand_qname($val);
		}
		elseif(preg_match("/\|(_iri_[0-9]+)\|/", $val, $matches)){/* |iri_x| */
			$iri=$this->calc_iri($this->iri_placeholders[trim($matches[1])]);
			if(!in_array($iri, $this->iris)){
				$this->iris[]=$iri;
			}
			return $iri;
		}
		$this->errors[]="could not expand '".$val."' in expand_to_iri()";
		return $val;
	}
	
	/*					*/
	
	function substitute_iri_refs($val=""){
		if(preg_match_all("/\<([^>\s]*)\>/sU", $val, $matches)){
			$iris=$matches[1];
			$prefix="_iri_";
			for($i=0,$i_max=count($iris);$i<$i_max;$i++){
				$cur_iri=$iris[$i];
				$val=str_replace("<".$cur_iri.">", "|".$prefix.$i."|", $val);
				$this->iri_placeholders[$prefix.$i]=$cur_iri;
				//$this->logs[]="replacing iri '".$cur_iri."' with ".$prefix.$i." in substitute_iri_refs()";
			}
		}
		return $val;
	}
	
	/*					*/

	function substitute_strings($val=""){
		$result="";
		$delims=array("d1"=>'"""', "d2"=>"'''", "d3"=>"'", "d4"=>'"', "d5"=>'`', "d6"=>"#");
		$brs=array("b1"=>"\r\n", "b2"=>"\r", "bn3"=>"\n");
		$cur_pos=0;
		$val_length=strlen($val);
		$prefix="_string_";
		$subs_count=0;
		while($cur_pos < $val_length){
			/* find next string or comment start */
			$next_delim_pos=$val_length;
			$next_delim_name="";
			foreach($delims as $cur_delim_name=>$cur_delim_code){
				$cur_next_delim_pos=strpos($val, $cur_delim_code, $cur_pos);
				if(($cur_next_delim_pos!==false) && ($cur_next_delim_pos < $next_delim_pos)){
					$next_delim_pos=$cur_next_delim_pos;
					$next_delim_name=$cur_delim_name;
				}
			}
			if($next_delim_name){
				if($next_delim_name==="d6"){/* comment */
					/* find next linebreak */
					$next_br_pos=$val_length;
					$next_br_name="";
					foreach($brs as $cur_br_name=>$cur_br_code){
						$cur_next_br_pos=strpos($val, $cur_br_code, $next_delim_pos);
						if(($cur_next_br_pos!==false) && ($cur_next_br_pos < $next_br_pos)){
							$next_br_pos=$cur_next_br_pos;
							$next_br_name=$cur_br_name;
						}
					}
					$result.=substr($val, $cur_pos, $next_delim_pos-$cur_pos);/* up to comment start */
					$cur_pos=$next_br_pos;
					$this->logs[]="removed comment '".substr($val, $next_delim_pos, $next_br_pos-$next_delim_pos)."' in substitute_strings()";
				}
				else{/* literal start */
					/* find literal end */
					$next_delim_code=$delims[$next_delim_name];
					$next_end_pos=strpos($val, $next_delim_code, $next_delim_pos+strlen($next_delim_code));
					while(($cur_prev_char=substr($val, $next_end_pos-1, 1)) && ($cur_prev_char=="\\")){
						$next_end_pos=strpos($val, $next_delim_code, $next_end_pos+1);
						if(!$next_end_pos || ($next_end_pos==$val_length-1)){
							$this->errors[]="unterminated literal in substitute_strings()";
							$next_end_pos=$val_length;
							break;
						}
					}
					if($next_end_pos){
						$result.=substr($val, $cur_pos, $next_delim_pos-$cur_pos);
						$str_val=substr($val, $next_delim_pos+strlen($next_delim_code), $next_end_pos-(strlen($next_delim_code)+$next_delim_pos));
						/* expand iris */
						if(preg_match_all("/\|(_iri_.+)\|/U", $str_val, $matches)){
							$iri_subs=$matches[1];
							foreach($iri_subs as $cur_subs){
								if($iri=$this->iri_placeholders[$cur_subs]){
									$str_val=str_replace("|".$cur_subs."|", "<".$iri.">", $str_val);
								}
							}
						}
						//$this->logs[]="substituted '".$str_val."' with ".$prefix.$subs_count;
						$this->str_placeholders[$prefix.$subs_count]=array(
							"delim_code"=>$next_delim_code,
							"val"=>$str_val
						);
						$result.=$prefix.$subs_count;
						$cur_pos=$next_end_pos+strlen($next_delim_code);
						$subs_count++;
					}
					else{
						$this->errors[]="unterminated literal in substitute_strings()";
						$result.=substr($val, $cur_pos);
						$cur_pos=$val_length;
					}
				}
			}
			else{
				$result.=substr($val, $cur_pos);
				$cur_pos=$val_length;
			}
		}
		return $result;
	}

	/*					*/

	function extract_bracket_data($val=""){
		$chars=array("("=>")", "{"=>"}", "["=>"]");
		if(($start_char=substr($val, 0, 1)) && ($end_char=$chars[$start_char])){
			$level=1;
			$val=substr($val, 1);
			$val_length=strlen($val);
			$cur_pos=0;
			while(($level!=0) && ($cur_pos<$val_length)){
				$next_end=strpos($val, $end_char, $cur_pos);
				if($next_end!==false){
					$next_start=strpos($val, $start_char, $cur_pos);
					if(($next_start!==false) && ($next_start<$next_end)){
						$cur_pos=$next_start+1;
						$level++;
					}
					else{
						$cur_pos=$next_end+1;
						$level--;
					}
				}
				else{
					$cur_pos=$val_length;
					$this->errors[]="could not extract data in extract_bracket_data()";
				}
			}
			return substr($val, 0, $cur_pos-1);
		}
		else{
			$this->errors[]="could not extract data in extract_bracket_data()";
			return false;
		}
	}
	
	function pop($ar=""){/* built-in array_pop() is buggy on certain php4 systems */
		$new_ar=array();
		if(is_array($ar)){
			for($i=0,$i_max=count($ar);$i<$i_max-1;$i++){
				$new_ar[]=$ar[$i];
			}
		}
		return $new_ar;
	}

	/*					*/

	function parse($q=""){
		$this->warnings=array();
		$this->errors=array();
		$this->logs=array();
		$this->infos=array("vars"=>array(), "result_vars"=>array());
		$this->iri_placeholders=array();
		$this->str_placeholders=array();
		$this->iris=array();

		$this->set_bnode_prefix($this->bnode_prefix);
		$this->set_default_prefixes();
		$this->prefixes=array();

		if(!$q){
			$this->errors[]="empty query";
			return true;
		}
		$this->q_init=$q;
		$this->q=$q;
		/* substitute iri refs */
		$this->q=$this->substitute_iri_refs($this->q);
		/* substitute strings, remove comments */
		$this->q=$this->substitute_strings($this->q);
		/* parse */
		$this->parse_Query();
	}
	
	/*	[1]				*/
	
	function parse_Query(){
		$this->q=trim($this->q);
		$this->parse_Prolog();
		if(preg_match("/^(SELECT|CONSTRUCT|DESCRIBE|ASK)/i", $this->q, $matches)){
			$this->infos["query_type"]=strtolower($matches[1]);
			$this->q=trim(substr($this->q, strlen($matches[0])));
			$mthd="parse_".ucfirst(strtolower($matches[1]))."Query";
			$this->$mthd();
		}
		else{
			$this->errors[]="missing or invalid query type in '".$this->q."'";
		}
	}
	
	/*	[2]				*/
	
	function parse_Prolog(){
		$this->parse_BaseDecl();
		$this->parse_PrefixDecl();
	}

	/*	[3]				*/

	function parse_BaseDecl(){
		if(preg_match("/^BASE\s*\|(.*)\|/isU", $this->q, $matches)){
			$base_iri=$this->iri_placeholders[$matches[1]];
			$this->q=trim(substr($this->q, strlen($matches[0])));
			$this->set_base($base_iri);
			$this->logs[]="setting base to ".$base_iri;
		}
	}
	
	/*	[4]				*/

	function parse_PrefixDecl(){
		$q=$this->q;
		while(preg_match("/^PREFIX\s*([^\s]*\:)\s+\|(.*)\|/isU", $q, $matches)){
			$qname_ns=trim($matches[1]);
			$q_iri_ref=$this->iri_placeholders[trim($matches[2])];
			$this->prefixes[$qname_ns]=$this->calc_iri($q_iri_ref);
			$this->logs[]="adding prefix '".$qname_ns."' -> '".$this->prefixes[$qname_ns]."'";
			$q=trim(substr($q, strlen($matches[0])));
		}
		$this->q=trim($q);
	}

	/*	[5]				*/
	
	function parse_SelectQuery(){
		/* distinct */
		$this->infos["distinct"]=false;
		if(preg_match("/^DISTINCT/i", $this->q, $matches)){
			$this->infos["distinct"]=true;
			$this->q=trim(substr($this->q, strlen($matches[0])));
		}
		/* vars */
		$vars=$this->extract_vars($this->q);
		$result_vars=array();
		/* result vars */
		if(preg_match("/^\*(.*)$/s", $this->q, $matches)){/* * */
			$result_vars=$vars;
			$this->q=trim($matches[1]);
		}
		else{/* explicit var list */
			$q=$this->q;
			while(preg_match("/^[\?\$]{1}([0-9a-z_]+)/i", $q, $matches)){
				$result_vars[]=$matches[1];
				$this->logs[]="adding result var ".$matches[1];
				$q=trim(substr($q, strlen($matches[0])));
			}
			$this->q=$q;
		}
		$this->infos["vars"]=$vars;
		$this->infos["result_vars"]=$result_vars;
		/* FROM */
		$this->parse_DatasetClause();
		/* WHERE */
		$this->parse_WhereClause();
		/* ORDER/LIMIT/OFFSET */
		$this->parse_SolutionModifier();
	}

	/*	[6]				*/

	function parse_ConstructQuery(){
		$this->infos["vars"]=$this->extract_vars($this->q);
		/* template */
		$this->parse_ConstructTemplate();
		/* FROM */
		$this->parse_DatasetClause();
		/* WHERE */
		$this->parse_WhereClause();
		/* ORDER/LIMIT/OFFSET */
		$this->parse_SolutionModifier();
	}

	/*	[7]				*/

	function parse_DescribeQuery(){
		/* vars */
		$vars=$this->extract_vars($this->q);
		$result_vars=array();
		$result_iris=array();
		$return_all=false;
		/* result vars/iris */
		if(preg_match("/^\*/", $this->q, $matches)){/* * */
			$result_vars=$vars;
			$return_all=true;
		}
		else{/* explicit var list */
			$q=$this->q;
			while($sub_result=$this->parse_VarOrIRIref($q)){
				if($sub_result["type"]=="var"){
					$result_vars[]=$sub_result["val"];
					$this->logs[]="adding result var ".$sub_result["val"];
				}
				elseif($sub_result["type"]=="iri"){
					$result_iris[]=$sub_result["val"];
					$this->logs[]="adding result iri ".$sub_result["val"];
				}
				$q=$sub_result["unparsed_val"];
			}
			$this->q=$q;
		}
		$this->infos["vars"]=$vars;
		$this->infos["result_vars"]=$result_vars;
		$this->infos["result_iris"]=$result_iris;
		/* FROM */
		$this->parse_DatasetClause();
		/* WHERE */
		$this->parse_WhereClause();
		/* ORDER/LIMIT/OFFSET */
		$this->parse_SolutionModifier();
		/* result_iris */
		if($return_all){
			$this->infos["result_iris"]=$this->iris;
		}
	}

	/*	[8]				*/

	function parse_AskQuery(){
		$this->infos["vars"]=$this->extract_vars($this->q);
		/* FROM */
		$this->parse_DatasetClause();
		/* WHERE */
		$this->parse_WhereClause();
	}

	/*	[9][10][11][12]	*/

	function parse_DatasetClause(){
		$q=$this->q;
		$this->infos["datasets"]=array();
		$this->infos["named_datasets"]=array();
		while(preg_match("/^FROM\s*(NAMED)?\s*([^\s]+)\s/is", $q, $matches)){
			$named=($matches[1]) ? true : false;
			$iri=$this->expand_to_iri($matches[2]);
			if($named){
				$this->infos["named_datasets"][]=$iri;
				$this->logs[]="adding named dataset: '".$iri."'";
			}
			else{
				$this->infos["datasets"][]=$iri;
				$this->logs[]="adding default dataset: '".$iri."'";
			}
			$q=trim(substr($q, strlen($matches[0])));
		}
		$this->q=trim($q);
	}
	
	/*	[13]			*/
	
	function parse_WhereClause(){
		if(preg_match("/^(WHERE)?\s*(\{.*)$/is", $this->q, $matches)){
			if($sub_result=$this->parse_GroupGraphPattern(trim($matches[2]))){
				$this->q=$sub_result["unparsed_val"];
				unset($sub_result["unparsed_val"]);
				$this->infos["patterns"]=$sub_result["entries"];
			}
			else{
				$this->errors[]="could not extract group graph pattern in parse_WhereClause()";
			}
		}
		else{
			if($this->infos["query_type"]!="describe"){
				$this->errors[]="empty where clause (or missing brackets) in parse_WhereClause()";
			}
		}
	}

	/*	[14]			*/
	
	function parse_SolutionModifier(){
		$this->parse_OrderClause();
		$this->parse_LimitClause();
		$this->parse_OffsetClause();
	}

	/*	[15]			*/
	
	function parse_OrderClause(){
		if(preg_match("/^ORDER\s*BY\s*(.*)/is", $this->q, $matches)){
			$this->q=trim($matches[1]);
			$this->parse_OrderCondition();
		}
	}
	
	/*	[16]			*/

	function parse_OrderCondition(){
		$q=$this->q;
		$conds=array();
		do{
			$cond=false;
			/* (ASC|DESC) (BrackettedExpression) */
			if(preg_match("/^(ASC|DESC)?(\s*)(\(.*)$/is", $q, $matches)){
				if(($bracket_data=$this->extract_bracket_data($matches[3])) && $sub_result=$this->parse_Expression(trim($bracket_data))){
					$cond=true;
					$conds[]=array("type"=>"expression", "direction"=>strtolower($matches[1]), "expression"=>$sub_result);
					$q=trim(substr($q, strlen($matches[1].$matches[2].$bracket_data)+2));
				}
				if(preg_match("/^(ASC|DESC)?(\s*)(\(.*)$/is", $q, $matches)){
					if(($bracket_data=$this->extract_bracket_data($matches[3])) && $sub_result=$this->parse_Expression(trim($bracket_data))){
						$cond=true;
						$conds[]=array("type"=>"expression", "direction"=>strtolower($matches[1]), "expression"=>$sub_result);
						$q=trim(substr($q, strlen($matches[1].$matches[2].$bracket_data)+2));
					}
				}
				if(preg_match("/^(ASC|DESC)?(\s*)(\(.*)$/is", $q, $matches)){
					if(($bracket_data=$this->extract_bracket_data($matches[3])) && $sub_result=$this->parse_Expression(trim($bracket_data))){
						$cond=true;
						$conds[]=array("type"=>"expression", "direction"=>strtolower($matches[1]), "expression"=>$sub_result);
						$q=trim(substr($q, strlen($matches[1].$matches[2].$bracket_data)+2));
					}
				}
				if(preg_match("/^(ASC|DESC)?(\s*)(\(.*)$/is", $q, $matches)){
					if(($bracket_data=$this->extract_bracket_data($matches[3])) && $sub_result=$this->parse_Expression(trim($bracket_data))){
						$cond=true;
						$conds[]=array("type"=>"expression", "direction"=>strtolower($matches[1]), "expression"=>$sub_result);
						$q=trim(substr($q, strlen($matches[1].$matches[2].$bracket_data)+2));
					}
				}
			}
			/* var */
			elseif($sub_result=$this->parse_Var($q)){
				$cond=true;
				$q=$sub_result["unparsed_val"];
				unset($sub_result["unparsed_val"]);
				$conds[]=$sub_result;
			}
			/* function call (iri|qname) */
			elseif($sub_result=$this->parse_FunctionCall($q)){
				$cond=true;
				$q=$sub_result["unparsed_val"];
				unset($sub_result["unparsed_val"]);
				$conds[]=$sub_result;
			}
		} while ($cond);
		$this->infos["order_conditions"]=$conds;
		$this->q=trim($q);
	}
	
	/*	[17]			*/
	
	function parse_LimitClause(){
		if(preg_match("/^LIMIT\s*([0-9]+)/is", $this->q, $matches)){
			$this->infos["limit"]=$matches[1];
			$this->q=trim(substr($this->q, strlen($matches[0])));
		}
	}

	/*	[18]			*/
	
	function parse_OffsetClause(){
		if(preg_match("/^OFFSET\s*([0-9]+)/is", $this->q, $matches)){
			$this->infos["offset"]=$matches[1];
			$this->q=trim(substr($this->q, strlen($matches[0])));
		}
	}

	/*	[19]			*/
	
	function parse_GroupGraphPattern($val=""){
		if(preg_match("/^(\{.*)$/s", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[1]);
			$unparsed_val_1=trim(substr($val, strlen($bracket_data)+2));
			$unparsed_val_1=(substr($unparsed_val_1, 0, 1)==".") ? trim(substr($unparsed_val_1, 1)) : $unparsed_val_1;
			$pattern=$this->parse_GraphPattern(trim($bracket_data));
			$unparsed_val_2=trim($pattern["unparsed_val"]);
			return array("type"=>"group", "entries"=>$pattern["entries"], "unparsed_val"=>$unparsed_val_1);
		}
		return false;
	}
	
	/*	[20]			*/
	
	function parse_GraphPattern($val=""){
		$entries=array();
		/* triples */
		if($val && ($sub_result=$this->parse_Triples($val)) && count($sub_result["triples"])){
			$entries[]=$sub_result;
			$val=$sub_result["unparsed_val"];
			$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
		}
		/* graph pattern, but not triples */
		if($val && ($sub_result=$this->parse_GraphPatternNotTriples($val)) && $sub_result["type"]){
			$val=$sub_result["unparsed_val"];
			unset($sub_result["unparsed_val"]);
			$entries[]=$sub_result;
			$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
			/* graph pattern */
			if($val && ($sub_result=$this->parse_GraphPattern($val)) && count($sub_result["entries"])){
				$entries[]=$sub_result["entries"];
				$val=$sub_result["unparsed_val"];
				$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
			}
		}
		return array("entries"=>$entries, "unparsed_val"=>trim($val));
	}
	
	/*	[21]			*/
	
	function parse_GraphPatternNotTriples($val=""){
		/* optional */
		if(preg_match("/^(OPTIONAL)(\s*)(\{.*)$/is", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[3]);
			return array(
				"type"=>"optional",
				"pattern"=>$this->parse_GroupGraphPattern("{".trim($bracket_data)."}"),
				"unparsed_val"=>trim(substr($val, strlen($matches[1].$matches[2].$bracket_data)+2))
			);
		}
		/* group or union */
		if(($sub_result=$this->parse_GroupGraphPattern($val)) && ($sub_result["type"])){
			$val=$sub_result["unparsed_val"];
			$result=$sub_result;
			/* union */
			if(preg_match("/^UNION/i", $val)){
				unset($sub_result["unparsed_val"]);
				$result=array("type"=>"union", "entries"=>array($sub_result));
				while(preg_match("/^UNION\s*(.*)$/s", $val, $matches)){
					$val=trim($matches[1]);
					if(($sub_result=$this->parse_GroupGraphPattern($val)) && ($sub_result["type"])){
						$val=$sub_result["unparsed_val"];
						unset($sub_result["unparsed_val"]);
						$result["entries"][]=$sub_result;
					}
				}
				$result["unparsed_val"]=$val;
			}
			return $result;
		}
		/* graph */
		if($sub_result=$this->parse_GraphGraphPattern($val)){
			return $sub_result;
		}
		/* constraint */
		if($sub_result=$this->parse_Constraint($val)){
			return $sub_result;
		}
		/* else */
		return false;
	}
	
	/*	[23]			*/
	
	function parse_GraphGraphPattern($val=""){
		if(preg_match("/^(GRAPH)(\s*)(.*)$/is", $val, $matches)){
			$val=trim($matches[3]);
			/* var */
			if($sub_result=$this->parse_Var($val)){
			}
			/* bnode */
			elseif($sub_result=$this->parse_BlankNode($val)){
			}
			/* iri */
			elseif($sub_result=$this->parse_IRIref($val)){
			}
			/* group */
			if($sub_result){
				$val=$sub_result["unparsed_val"];
				unset($sub_result["unparsed_val"]);
				if($sub_sub_result=$this->parse_GroupGraphPattern($val)){
					$val=$sub_sub_result["unparsed_val"];
					unset($sub_sub_result["unparsed_val"]);
					return array(
						"type"=>"graph",
						"graph"=>$sub_result,
						"pattern"=>$sub_sub_result,
						"unparsed_val"=>$val
					);
				}
			}
		}
		/* else */
		return false;
	}

	/*	[25]			*/

	function parse_Constraint($val=""){
		if(preg_match("/^(FILTER)(\s*)(.*)$/is", $val, $matches)){
			$val=trim($matches[3]);
			/* (...) */
			if($sub_result=$this->parse_BrackettedExpression($val)){
				return array(
					"type"=>"filter",
					"sub_type"=>"expression",
					"expression"=>$sub_result["expression"],
					"unparsed_val"=>$sub_result["unparsed_val"]
				);
			}
			/* built-in call */
			elseif($sub_result=$this->parse_BuiltInCall($val)){
				return array(
					"type"=>"filter",
					"sub_type"=>"built_in_call",
					"call"=>$sub_result,
					"unparsed_val"=>$sub_result["unparsed_val"]
				);
			}
			/* function call */
			elseif($sub_result=$this->parse_FunctionCall($val)){
				return array(
					"type"=>"filter",
					"sub_type"=>"function_call",
					"call"=>$sub_result,
					"unparsed_val"=>$sub_result["unparsed_val"]
				);
			}
		}
		/* else */
		return false;
	}

	/*	[26]			*/
	
	function parse_ConstructTemplate(){
		$q=$this->q;
		if(preg_match("/^(\{.*)$/s", $q, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[1]);
			$this->q=trim(substr($q, strlen($bracket_data)+2));
			if($sub_result=$this->parse_ConstructTriples(trim($bracket_data))){
				$this->infos["template_triples"]=$sub_result;
				/* set result vars */
				foreach($sub_result["triples"] as $cur_triple){
					foreach(array("s", "p", "o") as $cur_term_key){
						$cur_term=$cur_triple[$cur_term_key];
						$cur_term_type=$cur_term["type"];
						$cur_term_val=$cur_term["val"];
						if($cur_term_type=="var"){
							if(!in_array($cur_term_val, $this->infos["result_vars"])){
								$this->infos["result_vars"][]=$cur_term_val;
							}
						}
						if($cur_term_type=="bnode"){
							$cur_term_val=str_replace(":" , "_", $cur_term_val);
							if(!in_array($cur_term_val, $this->infos["result_vars"])){
								$this->infos["result_vars"][]=$cur_term_val;
							}
						}
					}
				}
			}
		}
		else{
			$this->errors[]="couldn't extract ConstructTriples in parse_ConstructTemplate";
		}
	}
	
	/*	[27]			*/
	
	function parse_ConstructTriples($val=""){
		$triples=array();
		/* triples1 */
		if($val && ($sub_result=$this->parse_Triples1($val)) && count($sub_result["triples"])){
			$triples=array_merge($triples, $sub_result["triples"]);
			$val=$sub_result["unparsed_val"];
			$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
			/* triples */
			if($val && ($sub_result=$this->parse_ConstructTriples($val)) && count($sub_result["triples"])){
				$triples=array_merge($triples, $sub_result["triples"]);
				$val=$sub_result["unparsed_val"];
				$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
			}
		}
		return array("type"=>"triples", "triples"=>$triples, "unparsed_val"=>trim($val));
	}
	
	/*	[28]			*/
	
	function parse_Triples($val=""){
		$triples=array();
		/* triples1 */
		if($val && ($sub_result=$this->parse_Triples1($val)) && count($sub_result["triples"])){
			$triples=array_merge($triples, $sub_result["triples"]);
			$val=$sub_result["unparsed_val"];
			$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
			/* triples */
			if($val && ($sub_result=$this->parse_Triples($val)) && count($sub_result["triples"])){
				$triples=array_merge($triples, $sub_result["triples"]);
				$val=$sub_result["unparsed_val"];
				$val=(substr($val, 0, 1)===".") ? trim(substr($val, 1)) : $val;
			}
		}
		return array("type"=>"triples", "triples"=>$triples, "unparsed_val"=>trim($val));
	}
	
	/*	[29][38][41][42]		*/
	
	function parse_Triples1($val=""){
		$nr=rand();
		$triples=array();
		$state=1;/* expecting subject */
		$s_stack=array();
		$p_stack=array();
		$state_stack=array();
		do{
			$proceed=false;
			$blank_node_prop_list_start_found=false;/* [ */
			
			$term=false;
			
			/* var */
			if($sub_result=$this->parse_Var($val)){
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* iri */
			elseif($sub_result=$this->parse_IRIref($val)){
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* rdf literal */
			elseif($sub_result=$this->parse_RDFLiteral($val)){
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* numeric */
			elseif(preg_match("/^(\+\-)?(.*)$/s", $val, $matches) && ($sub_result=$this->parse_NumericLiteral(trim($matches[2])))){
				$sub_result["negated"]=($matches[1]=="-") ? true : false;
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* boolean */
			elseif($sub_result=$this->parse_BooleanLiteral($val)){
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* bnode (_:foo or []) */
			elseif($sub_result=$this->parse_BlankNode($val)){
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* nil */
			elseif(preg_match("/^\(\s*\)(.*)$/s", $val, $matches)){
				$term=array("type"=>"iri", "val"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");
				$val=trim($matches[1]);
			}
			/* a */
			elseif(preg_match("/^a\s+(.*)$/s", $val, $matches)){
				$term=array("type"=>"iri", "val"=>"http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
				$val=trim($matches[1]);
			}
			/* = */
			elseif(preg_match("/^=\s*(.*)$/s", $val, $matches)){
				$term=array("type"=>"iri", "val"=>"http://www.w3.org/2002/07/owl#sameAs");
				$val=trim($matches[1]);
			}
			/* collection */
			elseif($sub_result=$this->parse_Collection($val)){
				$term=$sub_result;
				$val=$sub_result["unparsed_val"];
			}
			/* [ */
			elseif(preg_match("/^\[\s*(.*)$/s", $val, $matches)){
				$id=$this->get_next_bnode_id();
				if(!in_array(str_replace(":", "_", $id), $this->infos["vars"])){
					$this->infos["vars"][]=str_replace(":", "_", $id);
				}
				$term=array("type"=>"bnode", "val"=>$id);
				$val=trim($matches[1]);
				$state_stack[]=$state;
				$blank_node_prop_list_start_found=true;
			}
			/* ] */
			elseif(preg_match("/^\]\s*(.*)$/s", $val, $matches)){
				$val=trim($matches[1]);
				$proceed=true;
				/* pop s */
				$s_stack=$this->pop($s_stack);
				/* pop p */
				$p_stack=$this->pop($p_stack);
				/* state */
				$state=(count($state_stack)) ? $state_stack[count($state_stack)-1] : 1;
				$state_stack=$this->pop($state_stack);
			}
			/* ; */
			elseif(preg_match("/^\;\s*(.*)$/s", $val, $matches)){
				$val=trim($matches[1]);
				$proceed=true;
				/* state */
				$state=2;/* expecting predicate */
				/* pop p */
				$p_stack=$this->pop($p_stack);
			}
			/* , */
			elseif(preg_match("/^\,\s*(.*)$/s", $val, $matches)){
				$val=trim($matches[1]);
				$proceed=true;
				/* state */
				$state=3;/* expecting object */
			}
			if($term){
				//$this->logs[]="term ".$nr." : '".print_r($term, true)."'";
				unset($term["unparsed_val"]);
				$proceed=true;
				/* new s */
				if($state==1){
					$s_stack[]=$term;
					$state=2;
					$p_stack=$this->pop($p_stack);
				}
				/* new p */
				elseif($state==2){
					$p_stack[]=$term;
					$state=3;
				}
				/* new o */
				elseif($state==3){
					$triples[]=array(
						"s"=>$s_stack[count($s_stack)-1],
						"p"=>$p_stack[count($p_stack)-1],
						"o"=>$term
					);
					$this->logs[]="adding triple '".$s_stack[count($s_stack)-1]["val"]."' - '".$p_stack[count($p_stack)-1]["val"]."' - '".$term["val"]."'";
					$state=1;
					if($blank_node_prop_list_start_found){
						$state=2;
						$s_stack[]=$term;
					}
					elseif(count($state_stack)){/* still in [...] */
						$state=2;
					}
					elseif(substr($val, 0, 1)==="."){
						$val=trim(substr($val, 1));
					}
				}
			}
		} while ($proceed);
		return array("type"=>"triples", "triples"=>$triples, "unparsed_val"=>trim($val));
	}
	
	/*	[36]			*/

	function parse_Collection($val=""){
		if(preg_match("/^(\(.*)$/s", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[1]);
			/* just return the plain string for the moment */
			return array(
				"type"=>"collection",
				"val"=>trim($bracket_data),
				"unparsed_val"=>trim(substr($val, strlen($bracket_data)+2))
			);
		}
		/* else */
		return false;
	}
	
	/*	[39]			*/
	
	function parse_VarOrIRIref($val=""){
		if($sub_result=$this->parse_Var($val)){
			return $sub_result;
		}
		if($sub_result=$this->parse_IRIref($val)){
			return $sub_result;
		}
		return false;
	}

	/*	[41]			*/

	function parse_Var($val=""){
		if(preg_match("/^[\?\$]{1}([0-9a-z_]+)/i", $val, $matches)){
			return array(
				"type"=>"var",
				"val"=>$matches[1],
				"unparsed_val"=>trim(substr($val, strlen($matches[0])))
			);
		}
		return false;
	}

	/*	[43]			*/

	function parse_Expression($val=""){
		return $this->parse_ConditionalOrExpression($val);
	}

	/*	[44]			*/
	
	function parse_ConditionalOrExpression($val=""){
		if($sub_result=$this->parse_ConditionalAndExpression($val)){
			$val=$sub_result["unparsed_val"];
			$entries=array($sub_result);
			do{
				$proceed=false;
				if(preg_match("/^(\|\|)(.*)$/s", $val, $matches)){
					$operator=$matches[1];
					$val=trim($matches[2]);
					if($sub_sub_result=$this->parse_ConditionalAndExpression($val)){
						$proceed=true;
						$val=$sub_sub_result["unparsed_val"];
						unset($sub_sub_result["unparsed_val"]);
						$sub_sub_result["operator"]=$operator;
						$entries[]=$sub_sub_result;
					}
				}
			} while($proceed);
			if(count($entries)==1){
				return $sub_result;
			}
			else{
				unset($entries[0]["unparsed_val"]);
				return array(
					"type"=>"expression",
					"sub_type"=>"or",
					"entries"=>$entries,
					"unparsed_val"=>$val
				);
			}
		}
		return false;
	}
	
	/*	[45]			*/
	
	function parse_ConditionalAndExpression($val=""){
		if($sub_result=$this->parse_ValueLogical($val)){
			$val=$sub_result["unparsed_val"];
			$entries=array($sub_result);
			do{
				$proceed=false;
				if(preg_match("/^(\&\&)(.*)$/s", $val, $matches)){
					$operator=$matches[1];
					$val=trim($matches[2]);
					if($val && ($sub_sub_result=$this->parse_ValueLogical($val))){
						$proceed=true;
						$val=$sub_sub_result["unparsed_val"];
						unset($sub_sub_result["unparsed_val"]);
						$sub_sub_result["operator"]=$operator;
						$entries[]=$sub_sub_result;
					}
				}
			} while($proceed);
			if(count($entries)==1){
				return $sub_result;
			}
			else{
				//unset($entries[0]["unparsed_val"]);
				return array(
					"type"=>"expression",
					"sub_type"=>"and",
					"entries"=>$entries,
					"unparsed_val"=>$val
				);
			}
		}
		return false;
	}
	
	/*	[46]			*/
	
	function parse_ValueLogical($val=""){
		return $this->parse_RelationalExpression($val);
	}

	/*	[47]			*/

	function parse_RelationalExpression($val=""){
		if($sub_result=$this->parse_NumericExpression($val)){
			$val=$sub_result["unparsed_val"];
			if(preg_match("/^(\=|\!\=|\<|\>|\<\=|\>\=)(.*)$/s", $val, $matches)){
				$operator=$matches[1];
				$val=trim($matches[2]);
				if($sub_sub_result=$this->parse_NumericExpression($val)){
					$val=$sub_sub_result["unparsed_val"];
					unset($sub_sub_result["unparsed_val"]);
					$sub_sub_result["operator"]=$operator;
					unset($sub_result["unparsed_val"]);
					return array("expressions"=>array($sub_result, $sub_sub_result), "unparsed_val"=>$val);
				}
				else{
					$this->errors[]="expected NumericExpression in '".$val."' in parse_RelationalExpression()";
				}
			}
			return $sub_result;
		}
		return false;
	}
	
	/*	[48]			*/

	function parse_NumericExpression($val=""){
		return $this->parse_AdditiveExpression($val);
	}

	/*	[49]			*/

	function parse_AdditiveExpression($val=""){
		if($sub_result=$this->parse_MultiplicativeExpression($val)){
			$val=$sub_result["unparsed_val"];
			$entries=array($sub_result);
			do{
				$proceed=false;
				if(preg_match("/^(\+|\-)/", $val, $matches)){
					$operator=$matches[1];
					$val=trim(substr($val, 1));
					if($sub_sub_result=$this->parse_MultiplicativeExpression($val)){
						$proceed=true;
						$val=$sub_sub_result["unparsed_val"];
						unset($sub_sub_result["unparsed_val"]);
						$sub_sub_result["operator"]=$operator;
						$entries[]=$sub_sub_result;
					}
				}
			} while($proceed);
			if(count($entries)==1){
				return $sub_result;
			}
			else{
				unset($entries[0]["unparsed_val"]);
				return array(
					"type"=>"expression",
					"sub_type"=>"additive",
					"entries"=>$entries,
					"unparsed_val"=>$val
				);
			}
		}
		return false;
	}

	/*	[50]			*/

	function parse_MultiplicativeExpression($val=""){
		if($sub_result=$this->parse_UnaryExpression($val)){
			$val=$sub_result["unparsed_val"];
			$entries=array($sub_result);
			do{
				$proceed=false;
				if(preg_match("/^(\*|\/)/", $val, $matches)){
					$operator=$matches[1];
					$val=trim(substr($val, 1));
					if($sub_sub_result=$this->parse_UnaryExpression($val)){
						$proceed=true;
						$val=$sub_sub_result["unparsed_val"];
						unset($sub_sub_result["unparsed_val"]);
						$sub_sub_result["operator"]=$operator;
						$entries[]=$sub_sub_result;
					}
				}
			} while($proceed);
			if(count($entries)==1){
				return $sub_result;
			}
			else{
				unset($entries[0]["unparsed_val"]);
				return array(
					"type"=>"expression",
					"sub_type"=>"multiplicative",
					"entries"=>$entries,
					"unparsed_val"=>$val
				);
			}
		}
		return false;
	}

	/*	[51]			*/

	function parse_UnaryExpression($val=""){
		if(preg_match("/^(\!|\+|\-)?(.+)$/s", $val, $matches)){
			$result=$this->parse_PrimaryExpression(trim($matches[2]));
			$result["modifier"]=$matches[1];
			return $result;
		}
		return false;
	}
	
	/*	[52]			*/

	function parse_BuiltInCall($val=""){
		/* bound */
		if(preg_match("/^BOUND\s*\([\?\$]{1}([0-9a-z_]+)\)/is", $val, $matches)){
			return array(
				"type"=>"built_in_call",
				"call"=>"bound",
				"var"=>$matches[1],
				"unparsed_val"=>trim(substr($val, strlen($matches[0])))
			);
		}
		/* str, lang, etc (single-entry argument lists) */
		if(preg_match("/^(STR|LANG|DATATYPE|isIRI|isURI|isBlank|isLiteral)(\s*)(\(.*)$/is", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[3]);
			return array(
				"type"=>"built_in_call",
				"call"=>strtolower($matches[1]),
				"expression"=>$this->parse_Expression(trim($bracket_data)),
				"unparsed_val"=>trim(substr($val, strlen($matches[1].$matches[2].$bracket_data)+2))
			);
		}
		/* langmatches (2 arguments) */
		if(preg_match("/^(langMatches)(\s*)(\(.*)$/is", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[3]);
			$expr_1=$this->parse_Expression(trim($bracket_data));
			$rest=trim($expr_1["unparsed_val"]);
			$expr_2=(preg_match("/^,\s*(.*)$/s", $rest, $sub_matches)) ? $this->parse_Expression(trim($sub_matches[1])) : array();
			return array(
				"type"=>"built_in_call",
				"call"=>strtolower($matches[1]),
				"expressions"=>array($expr_1, $expr_2),
				"unparsed_val"=>trim(substr($val, strlen($matches[1].$matches[2].$bracket_data)+2))
			);
		}
		/* regex */
		if(preg_match("/^(REGEX)(\s*)(\(.*)$/is", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[3]);
			$expr_1=$this->parse_Expression(trim($bracket_data));
			$expr_2=(preg_match("/^,\s*(.*)$/s", trim($expr_1["unparsed_val"]), $sub_matches)) ? $this->parse_Expression(trim($sub_matches[1])) : array();
			$expr_3=(preg_match("/^,\s*(.*)$/s", trim($expr_2["unparsed_val"]), $sub_matches)) ? $this->parse_Expression(trim($sub_matches[1])) : array();
			return array(
				"type"=>"built_in_call",
				"call"=>strtolower($matches[1]),
				"expressions"=>array($expr_1, $expr_2, $expr_3),
				"unparsed_val"=>trim(substr($val, strlen($matches[1].$matches[2].$bracket_data)+2))
			);
		}
		return false;
	}
	
	/*	[54]			*/
	
	function parse_FunctionCall($val=""){
		if($sub_result=$this->parse_IRIref($val)){
			$val=$sub_result["unparsed_val"];
			if($sub_sub_result=$this->parse_ArgList($val)){
				$val=$sub_sub_result["unparsed_val"];
				unset($sub_sub_result["unparsed_val"]);
				return array(
					"type"=>"function_call",
					"iri"=>$sub_result["val"],
					"arg_list"=>$sub_sub_result,
					"unparsed_val"=>$val
				);
			}
		}
		/* else */
		return false;
	}
	
	/*	[55]			*/
	
	function parse_IRIrefOrFunction($val=""){
		if($sub_result=$this->parse_IRIref($val)){
			$val=$sub_result["unparsed_val"];
			if($sub_sub_result=$this->parse_ArgList($val)){
				$val=$sub_sub_result["unparsed_val"];
				unset($sub_sub_result["unparsed_val"]);
				return array(
					"type"=>"function_call",
					"iri"=>$sub_result["val"],
					"arg_list"=>$sub_sub_result,
					"unparsed_val"=>$val
				);
			}
			else{
				return $sub_result;
			}
		}
		return false;
	}
	
	/*	[56]			*/
	
	function parse_ArgList($val=""){
		/* () */
		if(preg_match("/^(\(\s*\))(.*)$/s", $val, $matches)){
			return array(
				"type"=>"arg_list",
				"entries"=>array(),
				"unparsed_val"=>trim($matches[2])
			);
		}
		/* list of expressions */
		if(preg_match("/^(\(.*)$/s", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[1]);
			$unparsed_val=trim(substr($val, strlen($bracket_data)+2));
			$val=$bracket_data;
			$entries=array();
			do{
				$proceed=false;
				$val=(substr($val, 0, 1)==",") ? trim(substr($val, 1)) : trim($val);
				if($val && ($sub_result=$this->parse_Expression($val))){
					$proceed=true;
					$val=$sub_result["unparsed_val"];
					unset($sub_result["unparsed_val"]);
					$entries[]=$sub_result;
				}
			} while($proceed);
			return array(
				"type"=>"arg_list",
				"entries"=>$entries,
				"unparsed_val"=>$unparsed_val
			);
		}
		return false;
	}
	
	/*	[57]			*/
	
	function parse_BrackettedExpression($val=""){
		if(preg_match("/^(\(.*)$/is", $val, $matches)){
			$bracket_data=$this->extract_bracket_data($matches[1]);
			return array(
				"type"=>"expression",
				"expression"=>$this->parse_Expression(trim($bracket_data)),
				"unparsed_val"=>trim(substr($val, strlen($bracket_data)+2))
			);
		}
		/* else */
		return false;
	}
	
	/*	[58]			*/
	
	function parse_PrimaryExpression($val=""){
		if(!$val){
			return false;
		}
		/* var */
		if(preg_match("/^[\?\$]{1}([0-9a-z_]+)/i", $val, $matches)){
			return array(
				"type"=>"var",
				/* "var"=>$matches[1], */
				"val"=>$matches[1],
				"unparsed_val"=>trim(substr($val, strlen($matches[0])))
			);
		}
		/* built-ins */
		if($sub_result=$this->parse_BuiltInCall($val)){
			return $sub_result;
		}
		/* RDFLiteral */
		if($sub_result=$this->parse_RDFLiteral($val)){
			return $sub_result;
		}
		/* NumericLiteral */
		if($sub_result=$this->parse_NumericLiteral($val)){
			return $sub_result;
		}
		/* BooleanLiteral */
		if($sub_result=$this->parse_BooleanLiteral($val)){
			return $sub_result;
		}
		/* bnode */
		if($sub_result=$this->parse_BlankNode($val)){
			return $sub_result;
		}
		/* (...) */
		if($sub_result=$this->parse_BrackettedExpression($val)){
			return $sub_result;
		}
		/* iri or function */
		if($sub_result=$this->parse_IRIrefOrFunction($val)){
			return $sub_result;
		}
		return false;
	}
	
	/*	[59][75]			*/
	
	function parse_NumericLiteral($val=""){
		/* double */
		if(preg_match("/^[0-9]*\.?[0-9]*[eE][+-]?[0-9]+/", $val, $matches)){
			return array("type"=>"numeric", "val"=>$matches[0], "sub_type"=>"double", "unparsed_val"=>trim(substr($val, strlen($matches[0]))));
		}
		/* decimal 1 */
		if(preg_match("/^[0-9]+\.[0-9]+/", $val, $matches)){
			return array("type"=>"numeric", "val"=>$matches[0], "sub_type"=>"decimal", "unparsed_val"=>trim(substr($val, strlen($matches[0]))));
		}
		/* decimal 2 */
		if(preg_match("/^[0-9]+\.[0-9]*/", $val, $matches)){
			return array("type"=>"numeric", "val"=>$matches[0], "sub_type"=>"decimal", "unparsed_val"=>trim(substr($val, strlen($matches[0]))));
		}
		/* decimal 3 */
		if(preg_match("/^[0-9]*\.[0-9]+/", $val, $matches)){
			return array("type"=>"numeric", "val"=>$matches[0], "sub_type"=>"decimal", "unparsed_val"=>trim(substr($val, strlen($matches[0]))));
		}
		/* integer */
		if(preg_match("/^[0-9]+/", $val, $matches)){
			return array("type"=>"numeric", "val"=>$matches[0], "sub_type"=>"integer", "unparsed_val"=>trim(substr($val, strlen($matches[0]))));
		}
		/* else */
		return false;
	}
	
	/*	[60][72]		*/
	
	function parse_RDFLiteral($val=""){
		if(preg_match("/^(_string_[0-9]+)/", $val, $matches)){
			$result=array(
				"type"=>"literal",
				"val"=>$this->str_placeholders[$matches[1]]["val"],
				"delim_code"=>$this->str_placeholders[$matches[1]]["delim_code"],
			);
			$unparsed_val=trim(substr($val, strlen($matches[0])));
			/* lang */
			if(preg_match("/^\@([a-z]+)(\-?)([a-z0-9]*)/i", $unparsed_val, $matches)){
				$result["lang"]=$matches[1].$matches[2].$matches[3];
				$unparsed_val=trim(substr($unparsed_val, strlen($matches[0])));
			}
			/* dt */
			elseif(preg_match("/^\^\^(.*)/s", $unparsed_val, $matches)){
				if($sub_result=$this->parse_IRIref($matches[1])){
					$result["dt"]=$sub_result["val"];
					$unparsed_val=trim($sub_result["unparsed_val"]);
				}
			}
			$result["unparsed_val"]=$unparsed_val;
			return $result;
		}
		return false;
	}

	/*	[61] */

	function parse_BooleanLiteral($val=""){
		if(preg_match("/^(true|false)/i", $val, $matches)){
			return array("type"=>"boolean", "val"=>$matches[1], "unparsed_val"=>trim(substr($val, strlen($matches[0]))));
		}
		/* else */
		return false;
	}
	
	/*	[63] */
	
	function parse_IRIref($val=""){
		/* iri */
		if(preg_match("/^\|(_iri_[0-9]+)\|(.*)$/s", $val, $matches)){
			$iri=$this->calc_iri($this->iri_placeholders[trim($matches[1])]);
			if(!in_array($iri, $this->iris)){
				$this->iris[]=$iri;
			}
			return array(
				"type"=>"iri",
				"val"=>$iri,
				"unparsed_val"=>trim($matches[2])
			);
		}
		/* qname */
		if(preg_match("/^([a-z0-9]*\:[a-z0-9.\-_]*)(.*)$/si", $val, $matches)){
			return array(
				"type"=>"iri",
				"val"=>$this->expand_qname($matches[1]),
				"unparsed_val"=>trim($matches[2])
			);
		}
		/* else */
		return false;
	}
	
	/*	[65]			*/
	
	function parse_BlankNode($val=""){
		/* _:foo */
		if(preg_match("/^_\:([a-z0-9\.\-\_]*)(.*)$/si", $val, $matches)){
			if(!in_array("__".$matches[1], $this->infos["vars"])){
				$this->infos["vars"][]="__".$matches[1];
			}
			return array(
				"type"=>"bnode",
				"val"=>"_:".$matches[1],
				"unparsed_val"=>trim($matches[2])
			);
		}
		/* [] */
		if(preg_match("/^\[\s*\](.*)$/s", $val, $matches)){
			$id=$this->get_next_bnode_id();
			if(!in_array(str_replace(":", "_", $id), $this->infos["vars"])){
				$this->infos["vars"][]=str_replace(":", "_", $id);
			}
			return array(
				"type"=>"bnode",
				"val"=>$id,
				"unparsed_val"=>trim($matches[1])
			);
		}
		/* else */
		return false;		
	}

	/*					*/

}

?>