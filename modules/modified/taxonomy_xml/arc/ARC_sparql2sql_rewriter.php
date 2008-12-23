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
sk:className                ARC_sparql2sql_rewriter
doap:name                   ARC SPARQL-to-SQL Rewriter
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              A PHP SPARQL to SQL rewriter for ARC RDF Store
//release
doap:created                2006-09-27
doap:revision               0.3.5
//changelog
sk:releaseChanges           2005-11-08: release 0.1.0
                            2006-01-26: release 0.2.0
                                        - complete re-write
                            2006-02-08: revision 0.2.1
                                        - tweaked DESCRIBE sql
                            2006-02-18: revision 0.2.2
                                        - regex fix
                                        - fixed bug in sub-pattern handling
                            2006-02-21: revision 0.2.3
                                        - security fix: auto-removing * from triple pattern comments 
                            2006-03-08: revision 0.2.4
                                        - checking if at least one result var exists in SELECT queries
                            2006-03-19: revision 0.2.5
                                        - added parentheses to FROM section for MySQL 5 compatibility
                            2006-03-19: release 0.3.0
                                        - adjusted for ARC API integration
                            2006-05-23: revision 0.3.1
                                        - added backticks to SQL
                                        - added another REGEX pattern: REGEX(?var1, ?var2)
                            2006-05-23: revision 0.3.2
                                        - minor tweak to avoid PHP notices
                            2006-05-23: revision 0.3.3
                                        - major fix to allow an unlimited number of non-triple pattern siblings
                            2006-05-23: revision 0.3.4
                                        - fix: unsupported filters led to invalid SQL
                                        - feature: added support for language queries (?s ?p "foo"@en // FILTER (lang(?o) = "en") // FILTER langMatches(lang(?o), "en") )
                                        - feature: added support for datatype queries (?s ?p "true"^^xsd:boolean)
                            2006-09-27: revision 0.3.5
                                        - fix: removed whitespace between CAST function and parentheses
*/

class ARC_sparql2sql_rewriter{

	var $version="0.3.5";

	var $logs=array();
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_sparql2sql_rewriter(&$api){
		$this->__construct($api);
	}

	/*					*/
	
	function init($args=""){/* infos, obj_props, dt_props, iri_alts */
		$this->infos=array();
		$this->obj_props= isset($this->config["obj_props"]) ? $this->config["obj_props"] : array();
		$this->dt_props= isset($this->config["dt_props"]) ? $this->config["dt_props"] : array();
		$this->iri_alts=array();
		$this->prop_table_infos=$this->api->get_prop_table_infos();
		foreach(array("infos", "objs_props", "dt_props", "iri_alts") as $cur_arg){
			if(isset($args[$cur_arg]) && is_array($args[$cur_arg])){
				$this->$cur_arg=$args[$cur_arg];
			}
		}
	}
	
	/*					*/
	
	function get_sql($args=""){
		$this->init($args);

		$this->term2alias=array();
		$this->optional_term2alias=array();
		$this->val_match_vars=array();
		$this->graphs=array();
		
		if(isset($this->infos["query_type"])){
			$mthd="get_".$this->infos["query_type"]."_sql";
			if(method_exists($this, $mthd)){
				return $this->$mthd();
			}
		}
		return $this->api->error("Undefined or unsupported query type '".$this->infos["query_type"]."'.");
	}

	/*					*/
	
	function get_select_sql(){
		$infos=$this->infos;
		if(!$infos["result_vars"]){
			return array("error"=>'no result variables specified');
		}
		if(isset($this->infos["patterns"])){
			$this->parse_patterns();/* creates where_code, optional_term2alias */
		}
		if(strpos($this->where_code, "__union_")){
			return $this->get_union_select_sql();
		}
		$left_join_code=$this->get_left_join_code();/* detects alias_alternatives */
		/* select */
		$result="SELECT";
		/* distinct */
		$result.=(isset($infos["distinct"]) && $infos["distinct"]) ? " DISTINCT" : "";
		/* count rows */
		if(isset($infos["count_rows"]) && $infos["count_rows"]){
			$result.=" SQL_CALC_FOUND_ROWS";
		}
		/* vars */
		$result.=$this->get_result_vars_code();
		/* from */
		$result.=$this->get_from_code();
		/* left joins */
		$result.=(strlen($left_join_code)) ? "\n /* left-joins */".$left_join_code : "";
		/* id2val joins */
		$id2val_join_code=$this->get_id2val_join_code();
		$result.=(strlen($id2val_join_code)) ? $id2val_join_code : "";
		/* where */
		$result.="\nWHERE \n ";
		$where_result="";
		/* dataset restrictions */
		$dataset_code=$this->get_dataset_code();
		$where_result.=(strlen($dataset_code)) ? "/* dataset restrictions */\n".$dataset_code."\n" : "";
		/* graph restrictions */
		$graph_code=$this->get_graph_code();
		if(strlen($graph_code)){
			$where_result.=(strlen($where_result)) ? " /* graph restrictions */\n AND \n ".$graph_code : " /* graph restrictions */\n ".$graph_code;
			$where_result.="\n";
		}
		/* where_code */
		if(strlen($this->where_code)){
			$where_result.=(strlen($where_result)) ? " /* triple patterns and filters */\n AND \n " : " /* triple patterns and filters */\n ";
			$where_result.=trim($this->where_code);
		}
		/* equi-joins */
		if($equi_join_code=$this->get_equi_join_code()){
			$where_result.=(strlen($where_result)) ? "\n /* equi-joins */\n AND ".$equi_join_code : "\n /* equi-joins */\n ".$equi_join_code;
		}
		$result.=(strlen($where_result)) ? 	$where_result : "1";
		/* order by */
		$result.=$this->get_order_by_code();
		/* limit/offset */
		$result.=$this->get_limit_offset_code();
		return $result;
	}
	
	function get_union_select_sql(){
		$infos=$this->infos;
		$result="";
		$where_code=trim($this->where_code);
		while(preg_match("/__union_([0-9]+)__/", $where_code, $matches)){
			$union_id=$matches[1];
			$branches=$this->union_branches[$union_id];
			for($i=0,$i_max=count($branches);$i<$i_max;$i++){
				$union_branch_id=$union_id."_".($i+1);
				$left_join_code=$this->get_left_join_code();/* detects alias_alternatives */
				/* select */
				$cur_result="SELECT";
				/* distinct, not on first branch */
				if(strlen($result)){
					$cur_result.=(isset($infos["distinct"]) && $infos["distinct"]) ? " DISTINCT" : " ALL";
				}
				/* count rows */
				if(isset($infos["count_rows"]) && $infos["count_rows"]){
					$result.=" SQL_CALC_FOUND_ROWS";
				}
				/* vars */
				$cur_result.=$this->get_result_vars_code(array("union_branch_id"=>$union_branch_id));
				/* from */
				$cur_result.=$this->get_from_code();
				/* left joins */
				$cur_result.=(strlen($left_join_code)) ? "\n /* left-joins */".$left_join_code : "";
				/* id2val joins */
				$id2val_join_code=$this->get_id2val_join_code();
				$cur_result.=(strlen($id2val_join_code)) ? $id2val_join_code : "";
				/* where */
				$cur_result.="\nWHERE \n ";
				/* dataset restrictions */
				$dataset_code=$this->get_dataset_code();
				$cur_result.=(strlen($dataset_code)) ? "/* dataset restrictions */\n".$dataset_code."\n" : "";
				/* graph restrictions */
				$graph_code=$this->get_graph_code();
				if(strlen($graph_code)){
					$cur_result.=" /* graph restrictions */";
					$cur_result.=(strlen($dataset_code)) ? "\n AND \n ".$graph_code : "\n ".$graph_code;
					$cur_result.="\n";
				}
				/* where_code */
				$cur_result.=" /* triple patterns and filters */\n ";
				$cur_result.=(strlen($graph_code.$dataset_code)) ? "AND \n " : "";
				$cur_result.=trim($where_code);
				/* equi-joins */
				$equi_join_code=$this->get_equi_join_code();
				$cur_result.=(strlen($equi_join_code)) ? "\n /* equi-joins */\n AND ".$equi_join_code : "";
				/* unions */
				$cur_branch=$branches[$i];
				$branch_result=str_replace("__union_".$union_id."__", $cur_branch, $cur_result);
				$result.=($i==0) ? "(".$branch_result."\n)" : "\nUNION \n(".$branch_result."\n)";
			}
			$where_code=str_replace("__union_".$union_id."__", "", $where_code);
		}
		/* order by */
		$result.=$this->get_order_by_code();
		/* limit/offset */
		$result.=$this->get_limit_offset_code();
		return $result;
	}

	/*					*/
	
	function get_construct_sql(){
		/* uses result_vars mentioned in construct template triples */
		return $this->get_select_sql();
	}
	
	/*					*/
	
	function get_describe_sql($infos="", $limit=100){/* called by ARC RDF Store once for all vars, then for each result_iri separately */
		if($infos){
			$this->infos=$infos;
		}
		if(isset($this->infos["result_vars"]) && count($this->infos["result_vars"])){
			return $this->get_select_sql();
		}
		elseif($this->infos["result_iris"]){
			$iri=$this->infos["result_iris"][0];
			//$q='SELECT ?ref_s ?ref_p ?p ?o WHERE { { ?ref_s  ?ref_p  <'.$iri.'> } UNION { <'.$iri.'> ?p ?o } }';
			$q='SELECT DISTINCT ?p ?o WHERE { <'.$iri.'> ?p ?o } ORDER BY ?p';
			$q.=($limit) ? ' LIMIT '.$limit : "";
			$parser=$this->api->get_sparql_parser();
			$parser->parse($q);
			$this->infos=$parser->get_infos();
			return $this->get_select_sql();
		}
		return "";
	}
	
	/*					*/

	function get_ask_sql(){
		$this->infos["result_vars"]=array("__ask_var__");
		$this->infos["limit"]=1;
		$sql=$this->get_select_sql();
		$sql=str_replace("SELECT", "SELECT 1 AS `success`", $sql);
		return $sql;
	}
	
	/*					*/
	
	function get_best_table_name($alias=""){
		$store_type=$this->config["store_type"];
		$prefix=$this->config["prefix"];
		$has_prop_tables=(isset($this->config["prop_tables"]) && count($this->config["prop_tables"])) ? true : false;
		if($store_type=="basic"){
			return $prefix."_triple";
		}
		if(isset($this->alias_prop_infos[$alias])){
			$a_info=$this->alias_prop_infos[$alias];
			/* graph check */
			if($a_info["in_graph"]){
				return $prefix."_triple_all_wdup";
			}
			if($store_type=="split"){
				/* try concrete p iri */
				if($a_info["p_term_type"]=="iri"){
					$p_iri=$a_info["p_term_val"];
					/* check prop_table_infos */
					if(isset($this->prop_table_infos[$p_iri])){
						return $this->prop_table_infos[$p_iri]["tbl"];
					}
					/* o is literal */
					if(($a_info["o_term_type"]=="literal") || in_array($p_iri, $this->dt_props)){
						return $prefix."_triple_dp";
					}
					/* o is iri */
					if(($a_info["o_term_type"]=="iri") || in_array($p_iri, $this->obj_props)){
						return $prefix."_triple_op";
					}
				}
				/* o is literal */
				if($a_info["o_term_type"]=="literal"){
					return $prefix."_triple_dp_all";
				}
				/* o is iri */
				if($a_info["o_term_type"]=="iri"){
					return $prefix."_triple_op_all";
				}
				/* o is obj */
				if($a_info["o_term_type"]=="var"){
					$o_var_val=$a_info["o_term_val"];
					if(($col_occurs=$this->var_col_occurs[$o_var_val]) && in_array("s", $col_occurs)){/* used to join => obj prop */
						return ($has_prop_tables) ? $prefix."_triple_op_all" : $prefix."_triple_op";
					}
				}
			}
		}
		return ($store_type=="split") ? $prefix."_triple_all" : $prefix."_triple";
	}
	
	/*					*/

	function get_result_vars_code($args=""){
		$result="";
		$union_branch_id="";
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}/* union_branch_id */
		$vars=$this->infos["result_vars"] ? $this->infos["result_vars"] : array();
		$added_aliases=array();/* each id2val join needs a separate alias */
		$conv_id=in_array($this->config["id_type"], array("hash_int", "incr_int"));
		foreach($vars as $cur_var){
			if(($alias_infos=@$this->term2alias[$cur_var]) || ($alias_infos=@$this->optional_term2alias[$cur_var])){
				$result.=strlen($result) ? ", \n " : "\n ";
				$null_var=false;/* whether var occurs in global pattern or current union_branch */
				$alias_info=$alias_infos[0];
				if($union_branch_id){
					$null_var=true;
					foreach($alias_infos as $cur_alias_info){
						/* todo: nested unions */
						if(!$cur_alias_info["union_branch_id"] || ($cur_alias_info["union_branch_id"]==$union_branch_id)){/* global var */
							$null_var=false;
							$alias_info=$cur_alias_info;
							break;
						}
					}
				}
				$alias=$alias_info["alias"];
				$col=$alias_info["col"];
				$term=$alias_info["term"];
				$tbl_alias="V".$alias;
				$alias_ext=2;
				while(in_array($tbl_alias, $added_aliases)){
					$tbl_alias="V".$alias."_".$alias_ext;
					$alias_ext++;
				}
				$full_tbl_alias=$tbl_alias.".val";
				/* alias alternatives (peer optionals handling) */
				if($alts=@$this->alias_alternatives["T".$alias.".".$col]){
					$sub_result="IFNULL(".$tbl_alias.".val, ";
					$sub_result_2="IFNULL(T".$alias.".__placeholder__, ";
					for($i=0,$i_max=count($alts);$i<$i_max;$i++){
						$cur_alt=$alts[$i];
						$alt_alias=$cur_alt["alias"];
						$alt_col=$cur_alt["col"];
						$alt_tbl_alias="V".$alt_alias;
						$alias_ext=2;
						while(in_array($alt_tbl_alias, $added_aliases)){
							$alt_tbl_alias=$cur_alt."_".$alias_ext;
							$alias_ext++;
						}
						$sub_result.=($i<$i_max-1) ? "IFNULL(".$alt_tbl_alias.".val" : $alt_tbl_alias.".val";
						$sub_result_2.=($i<$i_max-1) ? "IFNULL(T".$alt_alias.".__placeholder__" : "T".$alt_alias.".__placeholder__";
					}
					for($i=0;$i<$i_max;$i++){
						$sub_result.=")";
						$sub_result_2.=")";
					}
					$full_tbl_alias=$sub_result;
				}
				$result.=($null_var) ? "CONCAT('NULL ', ".$full_tbl_alias.") AS `".$cur_var."`" : $full_tbl_alias." AS `".$cur_var."`";
				$added_aliases[]=$tbl_alias;
				if($col=="s"){
					if(@$sub_result_2){
						$result.=",\n   ".str_replace("__placeholder__", "s_type", $sub_result_2)." AS `".$cur_var."__type`";
					}
					else{
						$result.=",\n   T".$alias.".s_type AS `".$cur_var."__type`";
					}
				}
				if($col=="o"){
					if(@$sub_result_2){
						$result.=",\n   ".str_replace("__placeholder__", "o_type", $sub_result_2)." AS `".$cur_var."__type`";
						$result.=",\n   ".str_replace("__placeholder__", "o_lang", $sub_result_2)." AS `".$cur_var."__lang`";
						if($conv_id){
							$result.=",\n   CONV(".str_replace("__placeholder__", "o_dt", $sub_result_2).", 10, 16) AS `".$cur_var."__dt`";
						}
						else{
							$result.=",\n   ".str_replace("__placeholder__", "o_dt", $sub_result_2)." AS `".$cur_var."__dt`";
						}
					}
					else{
						$result.=",\n   T".$alias.".o_type AS `".$cur_var."__type`";
						$result.=",\n   T".$alias.".o_lang AS `".$cur_var."__lang`";
						if($conv_id){
							$result.=",\n   CONV(T".$alias.".o_dt, 10, 16) AS `".$cur_var."__dt`";
						}
						else{
							$result.=",\n   T".$alias.".o_dt AS `".$cur_var."__dt`";
						}
					}
				}
			}
			elseif($alias_infos=@$this->graph_term2alias[$cur_var]){
				$alias=$alias_infos[0]["alias"];
				$tbl_alias="V".$alias."_g";
				$result.=strlen($result) ? ", \n " : "\n ";
				$result.=$tbl_alias.".val AS `".$cur_var."`";
			}
		}
		return $result;
	}

	/*					*/

	function get_from_code(){
		$result="";
		$added_aliases=array();
		/* t_count */
		for($i=1;$i<=$this->t_count;$i++){
			if(!in_array($i, $this->optional_t_counts)){
				$result.=strlen($result) ? ", \n " : "\nFROM (\n ";
				$tbl_alias="T".$i;
				$alias_ext=2;
				while(in_array($tbl_alias, $added_aliases)){
					$tbl_alias="T".$i."_".$alias_ext;
					$alias_ext++;
				}
				$cur_tbl_name=$this->get_best_table_name($i);
				$result.=$cur_tbl_name." ".$tbl_alias;
				$added_aliases[]=$tbl_alias;
			}
		}
		/* union_t_count */
		if($this->union_count){
			$min_union_t_count=$this->union_t_counts["base_t_count"]+1;
			$max_union_t_count=$min_union_t_count;
			foreach($this->union_t_counts as $union_id=>$cur_max_t_count){
				$max_union_t_count=max($cur_max_t_count, $max_union_t_count);
			}
			for($i=$min_union_t_count;$i<=$max_union_t_count;$i++){
				if(!in_array($i, $this->optional_t_counts)){
					$result.=strlen($result) ? ", \n " : "\nFROM (\n ";
					$tbl_alias="T".$i;
					$alias_ext=2;
					while(in_array($tbl_alias, $added_aliases)){
						$tbl_alias="T".$i."_".$alias_ext;
						$alias_ext++;
					}
					$cur_tbl_name=$this->get_best_table_name($i);
					$result.=$cur_tbl_name." ".$tbl_alias;
					$added_aliases[]=$tbl_alias;
				}
			}
		}
		$result.=(strlen($result)) ? "\n)" : "";
		return $result;
	}
	
	/*					*/
	
	function get_equi_join_code(){
		$result="";
		$added_joins=array();
		foreach($this->term2alias as $name=>$alias_infos){
			for($i=1,$i_max=count($alias_infos);$i<$i_max;$i++){
				$cur_alias_info_1=$alias_infos[$i-1];
				$cur_alias_1=$cur_alias_info_1["alias"];
				$cur_col_1=$cur_alias_info_1["col"];
				$cur_term_1=$cur_alias_info_1["term"];
				$tbl_alias_1="T".$cur_alias_1.".".$cur_col_1;

				$cur_alias_info_2=$alias_infos[$i];
				$cur_alias_2=$cur_alias_info_2["alias"];
				$cur_col_2=$cur_alias_info_2["col"];
				$cur_term_2=$cur_alias_info_2["term"];
				$tbl_alias_2="T".$cur_alias_2.".".$cur_col_2;
				
				if($tbl_alias_1!=$tbl_alias_2){
					if(!in_array($tbl_alias_1."=".$tbl_alias_2, $added_joins) && !in_array($tbl_alias_2."=".$tbl_alias_1, $added_joins)){
						$result.=(strlen($result)) ? "\n AND " : "";
						$result.=$tbl_alias_1."=".$tbl_alias_2;
						$added_joins[]=$tbl_alias_1."=".$tbl_alias_2;
					}
				}
			}
		}
		return $result;
	}
	
	/*					*/

	function get_left_join_code(){
		$result="";
		$added_aliases=array();
		$optional_sets=array();
		$alias2parent_optional=array();
		foreach($this->optional_term2alias as $name=>$alias_infos){
			//$result.="\n".$name;
			foreach($alias_infos as $alias_info){
				$joined_alias=$alias_info["alias"];
				//$result.="\n".$joined_alias;
				if(!in_array("T".$joined_alias, $added_aliases)){
					$joined_col=$alias_info["col"];
					$joined_term=$alias_info["term"];
					$joined_term_val=$joined_term["val"];
					if(($ref_alias_infos=@$this->term2alias[$joined_term_val]) || ($ref_alias_infos=@$this->optional_term2alias[$joined_term_val])){
						$ref_alias_info=$ref_alias_infos[0];
						$ref_alias=$ref_alias_info["alias"];
						$ref_col=$ref_alias_info["col"];
						$cur_tbl_name=$this->get_best_table_name($joined_alias);
						$result.="\n LEFT JOIN ".$cur_tbl_name." T".$joined_alias." ON ";
						$result.="\n  (";
						$result.="\n   T".$joined_alias.".".$joined_col."=T".$ref_alias.".".$ref_col;
						/* alias patterns */
						if($patterns=$this->optional_patterns["T".$joined_alias]){
							foreach($patterns as $cur_pattern){
								$result.="\n   AND\n   ".$cur_pattern;
							}
						}
						/* other terms in current alias pattern */
						$term_infos=$this->alias2term[$joined_alias];
						foreach($term_infos as $cur_term_info){
							$cur_term=$cur_term_info["term"];
							$cur_term_type=$cur_term["type"];
							$cur_term_val=$cur_term["val"];
							$cur_col=$cur_term_info["col"];
							if($cur_col!=$joined_col){
								//$result.="\nother: ".$cur_term_val." (".$cur_col.")";
								$other_alias_infos=false;
								/* check if term is used in non-optional patterns */
								if($other_alias_infos=@$this->term2alias[$cur_term_val]){
								}
								/* check if term is used in *earlier* optional patterns */
								elseif($pre_other_alias_infos=@$this->optional_term2alias[$cur_term_val]){
									$other_alias_infos=array();
									foreach($pre_other_alias_infos as $cur_other_alias_info){
										if($cur_other_alias_info["alias"]<$joined_alias){
											$other_alias_infos[]=$cur_other_alias_info;
										}
									}
								}
								if($other_alias_infos){
									foreach($other_alias_infos as $cur_other_alias_info){
										$other_alias=$cur_other_alias_info["alias"];
										$other_col=$cur_other_alias_info["col"];
										$other_tbl_alias="T".$other_alias.".".$other_col;
										$result.="\n   AND (";
										$result.="T".$joined_alias.".".$cur_col."=".$other_tbl_alias;
										/* find out if other_col is in different optional */
										if($cur_other_alias_info["optional_count"] && ($cur_other_alias_info["optional_count"]!=$alias_info["optional_count"])){
											$result.=" OR ".$other_tbl_alias." IS NULL";
											if(!isset($this->alias_alternatives[$other_tbl_alias])){
												$this->alias_alternatives[$other_tbl_alias]=array();
											}
											$this->alias_alternatives[$other_tbl_alias][]=array("alias"=>$joined_alias, "col"=>$cur_col);
										}
										$result.=")";
									}
								}
							}
						}
						/* dataset restrictions */
						if($dataset_code=$this->get_dataset_code(true, $joined_alias)){
							$result.="\n   AND\n   ".$dataset_code;
						}
						$result.="\n  )";
						$added_aliases[]="T".$joined_alias;
					}
					/* optional groups */
					if(!array_key_exists($alias_info["optional_count"], $optional_sets)){
						$optional_sets[$alias_info["optional_count"]]=array("T".$joined_alias.".".$joined_col);
						if($parent_optional_count=$alias_info["parent_optional_count"]){
							$alias2parent_optional["T".$joined_alias.".".$joined_col]=$parent_optional_count;
						}
					}
					else{
						$optional_sets[$alias_info["optional_count"]][]="T".$joined_alias.".".$joined_col;
					}
				}
			}
		}
		/* optional sets */
		$sub_result="";
		foreach($optional_sets as $k=>$cur_set){
			$null_set=$cur_set;
			$not_null_set=$cur_set;
			$set_entry=$cur_set[0];
			while($parent_set_id = @$alias2parent_optional[$set_entry]){
				/* nested optional, only NOT NULL if all parent patterns are NOT NULL as well */
				$parent_set=$optional_sets[$parent_set_id];
				$set_entry=$parent_set[0];
				foreach($parent_set as $cur_alias){
					if(!in_array($cur_alias, $not_null_set)){
						$not_null_set[]=$cur_alias;
					}
				}
			}
			if(count($not_null_set)>1){
				/* not null */
				$not_null_code="";
				foreach($not_null_set as $cur_alias){
					$not_null_code.=strlen($not_null_code) ? " AND " : "";
					$not_null_code.=$cur_alias." IS NOT NULL";
				}
				/* null */
				$null_code="";
				foreach($null_set as $cur_alias){
					$null_code.=strlen($null_code) ? " AND " : "";
					$null_code.=$cur_alias." IS NULL";
				}
				$sub_result.=strlen($sub_result) ? " AND " : "";
				$sub_result.="((".$not_null_code.") OR (".$null_code."))";
				$sub_result.="\n";
			}
		}
		if($sub_result){
			$this->where_code.=strlen($this->where_code) ? "\n AND " : "\n";
			$this->where_code.=$sub_result;
		}
		return $result;
	}
	
	/*					*/
	
	function get_id2val_join_code(){
		$result="\n /* id2val joins */";
		$vars=($this->infos["result_vars"]) ? $this->infos["result_vars"] : array();
		/* add regex'd vars */
		foreach($this->val_match_vars as $cur_var){
			if(!in_array($cur_var, $vars)){
				$vars[]=$cur_var;
			}
		}
		$added_aliases=array();/* each id2val join needs a separate alias */
		$tbl_name=$this->config["prefix"]."_id2val";
		$var2tbl_alias=array();
		foreach($vars as $cur_var){
			$tbl_alias="";
			if(($alias_infos=@$this->term2alias[$cur_var]) || ($alias_infos=@$this->optional_term2alias[$cur_var])){
				$alias_info=$alias_infos[0];
				$alias=$alias_info["alias"];
				$col=$alias_info["col"];
				$term=$alias_info["term"];
				$tbl_alias="V".$alias;
				$alias_ext=2;
				while(in_array($tbl_alias, $added_aliases)){
					$tbl_alias="V".$alias."_".$alias_ext;
					$alias_ext++;
				}
				$result.="\n LEFT JOIN ".$tbl_name." ".$tbl_alias." ON (";
				$result.="".$tbl_alias.".id=T".$alias.".".$col;
				$result.=")";
				$added_aliases[]=$tbl_alias;
				/* alias alternatives */
				if($alias_info["optional_count"]){
					foreach($alias_infos as $cur_alias_info){
						$cur_tbl_alias="T".$cur_alias_info["alias"].".".$cur_alias_info["col"];
						if($alts = @$this->alias_alternatives[$cur_tbl_alias]){
							foreach($alts as $cur_alt){
								$alt_alias=$cur_alt["alias"];
								$alt_col=$cur_alt["col"];
								$alt_tbl_alias="V".$alt_alias;
								$alias_ext=2;
								while(in_array($alt_tbl_alias, $added_aliases)){
									$alt_tbl_alias=$cur_alt."_".$alias_ext;
									$alias_ext++;
								}
								$result.="\n LEFT JOIN ".$tbl_name." ".$alt_tbl_alias." ON (";
								$result.="".$alt_tbl_alias.".id=T".$alt_alias.".".$alt_col;
								$result.=")";
							}
						}
					}
				}
			}
			elseif($alias_infos=@$this->graph_term2alias[$cur_var]){
				$alias=$alias_infos[0]["alias"];
				$tbl_alias="V".$alias."_g";
				$result.="\n LEFT JOIN ".$tbl_name." ".$tbl_alias." ON (";
				$result.="".$tbl_alias.".id=T".$alias.".g";
				$result.=")";
			}
			if($tbl_alias){
				$var2tbl_alias[$cur_var]=$tbl_alias;
			}
		}
		/* regex'd vars */
		foreach($this->val_match_vars as $cur_var){
			$this->where_code=str_replace("V__regex_match_".$cur_var."__", $var2tbl_alias[$cur_var], $this->where_code);
		}
		return $result;
	}
	
	/*					*/

	function get_order_by_code(){
		$result="";
		if($conds=@$this->infos["order_conditions"]){
			foreach($conds as $cur_cond){
				$cur_cond_type=$cur_cond["type"];
				$mthd="get_".$cur_cond_type."_order_by_code";
				if(method_exists($this, $mthd)){
					if($sub_result=$this->$mthd($cur_cond)){
						$result.=(strlen($result)) ? ",\n " : "\n ";
						$result.=$sub_result;
					}
				}
			}
		}
		return (trim($result)) ? "\nORDER BY ".$result : "";
	}
	
	function get_var_order_by_code($cond=""){
		$result="";
		$var=$cond["val"];
		if(($alias_infos=@$this->term2alias[$var]) || ($alias_infos=@$this->optional_term2alias[$var])){
			$alias_info=$alias_infos[0];
			$alias=$alias_info["alias"];
			$col=$alias_info["col"];
			$term=$alias_info["term"];
			
			$tbl_alias="T".$alias;
			if($col!="o" && in_array($var, $this->infos["result_vars"])){
				$result.=rawurlencode($var);
			}
			elseif($col=="o"){
				/* todo type casting if var is numeric */
				$result.=$tbl_alias.".o_comp";
			}
			else{
				/* todo: sort by val, not by id */
				$result.=$tbl_alias.".".$col;
			}
		}
		elseif($alias_infos=@$this->graph_term2alias[$var]){
			$alias=$alias_infos[0]["alias"];
			$tbl_alias="V".$alias."_g";
			//$result.=strlen($result) ? ", \n " : "\n ";
			$result.=$tbl_alias.".val";
		}
		return $result;
	}
	
	function get_expression_order_by_code($cond=""){
		$result="";
		$expr=$cond["expression"];
		$expr_type=$expr["type"];
		$mthd="get_".$expr_type."_order_by_code";
		if(method_exists($this, $mthd)){
			$result.=$this->$mthd($expr);
		}
		if($result && ($dir=$cond["direction"])){
			$result.=" ".rawurlencode(strtoupper($dir));
		}
		return $result;
	}
		
	/*					*/

	function get_limit_offset_code(){
		$result="";
		$offset=(@$this->infos["offset"]) ? $this->infos["offset"] : 0;
		$limit=(@$this->infos["limit"]) ? $this->infos["limit"] : 0;
		if($limit){
			$result="\nLIMIT ".rawurlencode($offset).",".rawurlencode($limit);
		}
		elseif($offset){
			$result="\nOFFSET ".rawurlencode($offset);
		}
		return $result;
	}
	
	/*					*/
	
	function get_graph_code(){
		$result="";
		$added_joins=array();
		foreach($this->graphs as $alias=>$graph){
			$graph_type=$graph["type"];
			$graph_val=$graph["val"];
			if($graph_type=="iri"){
				$result.=strlen($result) ? "\n AND " : "";
				$result.="T".$alias.".g=".$this->api->get_id($graph_val, 1);
				$result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($graph_val)))." */ ";
			}
			elseif($graph_type=="var"){
				/* graph joins */
				$alias_infos=$this->graph_term2alias[$graph_val];
				$alias_1=$alias_infos[0]["alias"];
				for($i=1,$i_max=count($alias_infos);$i<$i_max;$i++){
					$cur_alias_info=$alias_infos[$i];
					$cur_alias=$cur_alias_info["alias"];
					$cur_join="T".$alias_1.".g=T".$cur_alias.".g";
					if(($alias_1!=$cur_alias) && !in_array($cur_join, $added_joins)){
						$result.=strlen($result) ? "\n AND " : "";
						$result.=$cur_join;
						$added_joins[]=$cur_join;
					}
				}
				/* graph occurences in triple patterns */
				if($alias_infos=@$this->term2alias[$graph_val]){
					$alias_info=$alias_infos[0];
					$alias=$alias_info["alias"];
					$col=$alias_info["col"];
					$cur_join="T".$alias_1.".g=T".$alias.".".$col;
					if(($alias_1!=$alias) && !in_array($cur_join, $added_joins)){
						$result.=strlen($result) ? "\n AND " : "";
						$result.=$cur_join;
						$added_joins[]=$cur_join;
					}
				}
			}
		}
		return $result;
	}
	
	function get_dataset_code($for_optionals=false, $exact_alias=false){
		$result="";
		$added_aliases=array();
		$sets=$this->infos["datasets"];
		$n_sets=$this->infos["named_datasets"];
		if(!count($sets) && !count($n_sets)){
			return $result;
		}
		/* non-graph'd patterns */
		foreach($this->non_graph_aliases as $cur_alias){
			$sub_result="";
			if(!$exact_alias || ($exact_alias==$cur_alias)){
				if((!$for_optionals && !in_array($cur_alias, $this->optional_t_counts)) || ($for_optionals && in_array($cur_alias, $this->optional_t_counts))){
					/* datasets */
					foreach($sets as $cur_set){
						$sub_result.=strlen($sub_result) ? "\n  OR " : " ";
						$sub_result.="T".$cur_alias.".g=".$this->api->get_id($cur_set, 1);
						$sub_result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($cur_set)))." */ ";
					}
				}
			}
			if($sub_result){
				$result.=strlen($result) ? "\n AND " : " ";
				$result.="(".$sub_result.")";
			}
		}
		/* graph'd patterns */
		foreach($this->graphs as $cur_alias=>$graph){
			$graph_type=$graph["type"];
			$graph_val=$graph["val"];
			if(!$exact_alias || ($exact_alias==$cur_alias)){
				if(($graph_type=="var") && (!$for_optionals && !in_array($cur_alias, $this->optional_t_counts)) || ($for_optionals && in_array($cur_alias, $this->optional_t_counts))){
					$sub_result="";
					/* named datasets, if set */
					if(($my_sets=$n_sets) || ($my_sets=$sets)){
						foreach($my_sets as $cur_set){
							$sub_result.=strlen($sub_result) ? "\n  OR " : " ";
							$sub_result.="T".$cur_alias.".g=".$this->api->get_id($cur_set, 1);
							$sub_result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($cur_set)))." */ ";
						}
					}
					if($sub_result){
						$result.=strlen($result) ? "\n AND " : " ";
						$result.="(".$sub_result.")";
					}
				}
			}
		}
		return $result;
	}
	
	/*					*/

	function parse_patterns(){
		$infos=$this->infos;
		$where_code="";
		$this->t_count=0;
		$this->optional_count=0;
		$this->union_count=0;/* abs union count */
		$this->term2alias=array();
		$this->alias2term=array();
		$this->optional_term2alias=array();
		$this->optional_t_counts=array();
		$this->optional_patterns=array();
		$this->union_t_counts=array();/* individual t_counts for each union branch */
		$this->union_branches=array();
		$this->graphs=array();
		$this->graph_term2alias=array();
		$this->non_graph_aliases=array();
		$this->alias_alternatives=array();/* for peer optionals with common terms */
		$this->val_match_vars=array();/* for REGEX matching */
		$this->alias_prop_infos=array();
		$this->var_col_occurs=array();/* col positions (s,p,o) a var is used (needed to detect obj_props for table splitting) */
		$ind=" ";
		$cur_args=array(
			"ind"=>$ind,
			"in_optional"=>false,
			"optional_count"=>0,
			"parent_optional_count"=>0,
			"in_union"=>false,
			"union_count"=>0,
			"union_branch_id"=>"",
			"in_graph"=>false,
			"graph"=>array(),
			"pattern"=>""
		);
		$nl="\n";
		$ind=$cur_args["ind"];
		$ni=$nl.$ind;
		foreach($infos["patterns"] as $cur_pattern){
			$cur_args["pattern"]=$cur_pattern;
			$sub_r=$this->parse_pattern($cur_args);
			if(trim($sub_r)){
				$where_code.=(strlen($where_code)) ? $ni."AND".$ni."(".$nl." ".str_replace("\n", "\n ", $sub_r).$ni.")" : $sub_r;
			}
		}
		$this->where_code=$where_code;
	}
	
	/*					*/
	
	function parse_pattern($args=""){
		$r="";
		if(is_array($args) && isset($args["pattern"]) && ($pattern=$args["pattern"])){
			$nl="\n";
			$ind=$args["ind"];
			$ni=$nl.$ind;
			$cur_type=isset($pattern["type"]) ? $pattern["type"] : "";
			if($cur_type){
				$mthd="parse_".$cur_type."_pattern";
				if(method_exists($this, $mthd)){
					$r.=$this->$mthd($args);
				}
			}
			elseif(count($pattern)){/* has sub-patterns */
				foreach($pattern as $sub_pattern){
					$args["pattern"]=$sub_pattern;
					$sub_r=$this->parse_pattern($args);
					if(trim($sub_r)){
						$r.=(strlen($r)) ? $ni."AND".$ni."(".$nl." ".str_replace("\n", "\n ", $sub_r).$ni.")" : $sub_r;
					}
				}
			}
		}
		return $r;
	}
	
	/*					*/
	
	function parse_group_pattern($args=""){
		$r="";
		$pattern=$args["pattern"];
		if(isset($pattern["entries"]) && ($entries=$pattern["entries"])){
			$nl="\n";
			$ind=$args["ind"];
			$ni=$nl.$ind;
			$args["ind"].=" ";
			foreach($entries as $cur_pattern){
				$args["pattern"]=$cur_pattern;
				$sub_r=$this->parse_pattern($args);
				if(trim($sub_r)){
					$r.=(strlen($r)) ? $ni."AND".$ni."(".$nl." ".str_replace("\n", "\n ", $sub_r).$ni.")" : $sub_r;
				}
			}
		}
		return ($r) ? $ind."(\n".$r.$ni.")" : "";
	}

	/*					*/
	
	function parse_graph_pattern($args=""){
		$pattern=$args["pattern"];
		$args["in_graph"]=true;
		$args["graph"]=$pattern["graph"];
		$sub_pattern=$pattern["pattern"];
		$args["pattern"]=$sub_pattern;
		$args["ind"].=" ";
		return $this->parse_pattern($args);
	}

	/*					*/

	function parse_triples_pattern($args=""){
		$result="";
		$pattern=$args["pattern"];
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}
		if($triples=$pattern["triples"]){
			foreach($triples as $cur_t){
				if(!$in_union){
					$this->t_count++;
					$cur_t_count=$this->t_count;
				}
				else{
					$this->union_t_counts[$union_branch_id]++;
					$cur_t_count=$this->union_t_counts[$union_branch_id];
				}
				$tbl_alias="T".$cur_t_count;
				if($in_optional && !in_array($cur_t_count, $this->optional_t_counts)){
					$this->optional_t_counts[]=$cur_t_count;
					$this->logs[]="adding ".$cur_t_count." to optional_t_counts";
				}
				if($in_graph){
					$this->graphs[$cur_t_count]=$graph;
					if($graph["type"]=="var"){
						$graph_val=$graph["val"];
						if(!isset($this->graph_term2alias[$graph_val])){
							$this->graph_term2alias[$graph_val]=array();
						}
						$this->graph_term2alias[$graph_val][]=array("alias"=>$cur_t_count, "col"=>"g");
					}
				}
				elseif(!in_array($cur_t_count, $this->non_graph_aliases)){
					$this->non_graph_aliases[]=$cur_t_count;
				}
				$this->alias_prop_infos[$cur_t_count]=array("in_graph"=>$in_graph);
				foreach(array("s", "p", "o") as $cur_col){
					$cur_term=$cur_t[$cur_col];
					$cur_term_type=$cur_term["type"];
					$cur_term_val=$cur_term["val"];
					$cur_term_lang=isset($cur_term["lang"]) ? $cur_term["lang"] : "";
					$cur_term_dt=isset($cur_term["dt"]) ? $cur_term["dt"] : "";
					$sub_result="";
					if($cur_col=="p"){
						$this->alias_prop_infos[$cur_t_count]["p_term_type"]=$cur_term_type;
						$this->alias_prop_infos[$cur_t_count]["p_term_val"]=$cur_term_val;
					}
					if($cur_col=="o"){
						$this->alias_prop_infos[$cur_t_count]["o_term_type"]=$cur_term_type;
						$this->alias_prop_infos[$cur_t_count]["o_term_val"]=$cur_term_val;
					}
					if($cur_term_type=="iri"){
						$sub_result.="(\n".$ind;
						$sub_result.=" ".$tbl_alias.".".$cur_col."=".$this->api->get_id($cur_term_val, 1);
						$sub_result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($cur_term_val)))." */ ";
						/* iri alternatives expansion */
						if(isset($this->iri_alts) && isset($this->iri_alts[$cur_term_val]) && ($iri_alts=$this->iri_alts[$cur_term_val])){
							foreach($iri_alts as $cur_iri_alt){
								$sub_result.="\n".$ind."OR ";
								$sub_result.=$tbl_alias.".".$cur_col."=".$this->api->get_id($cur_iri_alt, 1);
								$sub_result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($cur_iri_alt)))." */ ";
							}
						}
						$sub_result.="\n".$ind.")";
					}
					elseif($cur_term_type=="literal"){
						$sub_result.="(\n".$ind;
						$sub_result.=" ".$tbl_alias.".".$cur_col."=".$this->api->get_id($cur_term_val, 1);
						$sub_result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($cur_term_val)))." */ ";
						if($cur_col=="o"){
							if($cur_term_lang){
								$sub_result.="\n".$ind." AND ";
								$sub_result.=$tbl_alias.".o_lang='".rawurlencode($cur_term_lang)."'";
							}
							elseif($cur_term_dt){
								$sub_result.="\n".$ind." AND ";
								$sub_result.=$tbl_alias.".o_dt=".$this->api->get_id($cur_term_dt, 1);
								$sub_result.=" /* ".str_replace("*", "", str_replace("#", "::", htmlspecialchars($cur_term_dt)))." */ ";
							}
						}
						$sub_result.="\n".$ind.")";
					}
					else{
						if($cur_term_type=="bnode"){
							//$cur_term_val=str_replace(":", "_", $cur_term_val);
						}
						/* term2alias/optional_term2alias */
						$term2alias_array=($in_optional) ? "optional_term2alias" : "term2alias";
						$term2alias_array_obj = $this->$term2alias_array;
						if(!isset($term2alias_array_obj[$cur_term_val])){
							eval('$this->'.$term2alias_array.'[$cur_term_val]=array();');
						}
						eval('$this->'.$term2alias_array.'[$cur_term_val][]=array("alias"=>$cur_t_count, "col"=>$cur_col, "term"=>$cur_term, "optional_count"=>$optional_count, "parent_optional_count"=>$parent_optional_count, "union_branch_id"=>$union_branch_id, "in_graph"=>$in_graph, "graph"=>$graph);');
						/* alias2term */
						if(!isset($this->alias2term[$cur_t_count])){
							$this->alias2term[$cur_t_count]=array();
						}
						$this->alias2term[$cur_t_count][]=array("term"=>$cur_term, "col"=>$cur_col);
						/* col occurrence */
						if(!isset($this->var_col_occurs[$cur_term_val])){
							$this->var_col_occurs[$cur_term_val]=array();
						}
						$this->var_col_occurs[$cur_term_val][]=$cur_col;
					}
					if($sub_result){
						if($in_optional){
							if(!isset($this->optional_patterns[$tbl_alias])){
								$this->optional_patterns[$tbl_alias]=array();
							}
							$this->optional_patterns[$tbl_alias][]=$sub_result;
							$this->logs[]="adding ".$sub_result." to optional_patterns[".$tbl_alias."]";
						}
						else{
							$result.=(strlen($result)) ? "\n".$ind."AND\n".$ind : "".$ind;
							$result.=$sub_result;
						}
					}
				}
			}
		}
		return $result;
	}
	
	/*					*/

	function parse_optional_pattern($args=""){
		$pattern=$args["pattern"];
		$args["in_optional"]=true;
		$args["parent_optional_count"]=$args["optional_count"];
		$this->optional_count++;
		$args["optional_count"]=$this->optional_count;
		$sub_pattern=$pattern["pattern"];
		$args["pattern"]=$sub_pattern;
		$args["ind"].=" ";
		return $this->parse_pattern($args);
	}

	/*					*/

	function parse_union_pattern($args=""){
		$result="";
		
		$union_count=0;
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}
		if($entries=$pattern["entries"]){
			$this->union_count++;
			$base_t_count=100+$this->t_count;
			if(!array_key_exists("base_t_count", $this->union_t_counts)){
				$this->union_t_counts["base_t_count"]=$base_t_count;
			}
			$pos=0;
			$this->union_branches[$this->union_count]=array();
			foreach($entries as $cur_pattern){
				
				$cur_type=$cur_pattern["type"];
				if(!$cur_type){
					$cur_pattern=$cur_pattern[0];
					$cur_type=$cur_pattern["type"];
				}
				$pos++;
				$union_branch_id=$this->union_count."_".$pos;
				$this->union_t_counts[$union_branch_id]=$base_t_count;
				
				$cur_args=array(
					"ind"=>$ind." ",
					"in_optional"=>$in_optional,
					"optional_count"=>$optional_count,
					"parent_optional_count"=>$parent_optional_count,
					"in_union"=>true,
					"union_count"=>$this->union_count,
					"union_branch_id"=>$union_branch_id,
					"pattern"=>$cur_pattern
				);
				$mthd="parse_".$cur_type."_pattern";
				if(method_exists($this, $mthd)){
					if($sub_code=$this->$mthd($cur_args)){
						$this->union_branches[$this->union_count][]=$sub_code;
						$result.=(strlen($result)) ? "" : "__union_".$this->union_count."__";
						//$result.=(strlen($result)) ? "\n".$ind."OR\n".$ind."(\n ".str_replace("\n", "\n ", $sub_code)."\n".$ind.")" : $sub_code;
					}
				}
			}
		}
		return (strlen($result)) ? $ind."(\n".$result."\n".$ind.")" : "";
	}
	
	/*					*/
	
	function parse_filter_pattern($args){
		$r="";
		$pattern=$args["pattern"];
		$sub_type=(isset($pattern["sub_type"])) ? $pattern["sub_type"] : "";
		if($sub_type){
			$args["ind"].=" ";
			$mthd="parse_".$sub_type."_filter_pattern";
			if(method_exists($this, $mthd)){
				$sub_r=$this->$mthd($args);
				if(trim($sub_r)){
					$r.=$sub_r;
				}
			}
		}
		return $r;
	}
	
	function parse_expression_filter_pattern($args){
		$r="";
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}
		/* for now: some hard-coded expressions only */
		if(isset($pattern["expression"])){
			$expr=$pattern["expression"];
			if(!isset($expr["type"])){
				if(isset($expr["expressions"])){
					$args["pattern"]=$expr;
					return $this->parse_expression_filter_pattern($args);
				}
			}
			else{
				/* built_in_call */
				if($expr["type"]=="built_in_call"){
					$sub_r=$this->parse_built_in_call_filter_pattern(array("pattern"=>$expr));
					if(trim($sub_r)){
						$r.=$sub_r;
					}
				}
			}
		}
		elseif(isset($pattern["expressions"]) && ($exprs=$pattern["expressions"])){
			/* ?var <|> numeric|literal|?var */
			if((count($exprs)==2) 
				&& (@$exprs[0]["type"]=="var")
				&& ($var_val=@$exprs[0]["val"])
				&& ($expr_2_type=$exprs[1]["type"])
				&& (in_array($expr_2_type, array("numeric", "literal", "var")))
				&& ($val_2=$exprs[1]["val"])
				&& ($operator=$exprs[1]["operator"])
				&& (in_array($operator, array("<", ">", "<=", ">=", "=", "!=")))
			){
				/* var */
				if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val]) || ($alias_infos=@$this->graph_term2alias[$var_val])){
					$alias_info=$alias_infos[0];
					$alias=$alias_info["alias"];
					$col=$alias_info["col"];
					$tbl_code=($col=="o" && !in_array($operator, array("=", "!="))) ? "T".$alias.".o_comp" : ($col=="g" ? "T".$alias.".g" : "V".$alias.".".$col);
					if($operator=="!="){
						$r.=" NOT (";
					}
					$r.=(in_array($expr_2_type, array("numeric", "literal")) && is_numeric($val_2)) ? "CAST(".$tbl_code." AS SIGNED) +0.00" : $tbl_code;
					/* operator */
					$r.=($operator!="!=") ? " ".$operator." " : " = ";
					/* literal */
					if(in_array($expr_2_type, array("numeric", "literal"))){
						if(is_numeric($val_2)){
							$r.=(($modifier=$exprs[1]["modifier"]) && ($modifier=="-")) ? $modifier : "";
							$r.=$val_2;
						}
						elseif(isset($exprs[1]["delim_code"]) && ($delim_code=$exprs[1]["delim_code"])){
							$delim_code_char=$delim_code{0};
							$r.=$delim_code_char.str_replace($delim_code_char, "", $val_2).$delim_code_char;
						}
					}
					if(in_array($expr_2_type, array("var"))){
						if(($ais=@$this->term2alias[$val_2]) || ($ais=@$this->optional_term2alias[$val_2]) || ($ais=@$this->graph_term2alias[$val_2])){
							$ai=$ais[0];
							$a=$ai["alias"];
							$col=$ai["col"];
							$r.=($col=="o" && !in_array($operator, array("=", "!="))) ? "T".$a.".o_comp" : ($col=="g" ? "T".$a.".g" : "V".$a.".".$col);
						}
						else{
							$r="";
						}
					}
					$r.=($r && ($operator=="!=")) ? ")" : "";
				}
			}
			/* LANG(?var)="en" */
			if((count($exprs)==2) && isset($exprs[0]["type"]) && isset($exprs[1]["type"])
				&& ($exprs[0]["type"]=="built_in_call")
				&& ($exprs[0]["call"]=="lang")
				&& isset($exprs[0]["expression"]) && isset($exprs[0]["expression"]["type"])
				&& ($exprs[0]["expression"]["type"]=="var")
				&& ($var_val=$exprs[0]["expression"]["val"])
				
				&& ($exprs[1]["type"]=="literal")
				&& ($lang_val=$exprs[1]["val"])

				&& ($operator=$exprs[1]["operator"])
				&& (in_array($operator, array("=", "!=")))
			){
				/* var */
				if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val]) || ($alias_infos=@$this->graph_term2alias[$var_val])){
					$alias_info=$alias_infos[0];
					$alias=$alias_info["alias"];
					$tbl_code="T".$alias.".o_lang";
					$r.=$tbl_code;
					/* operator */
					$r.=" ".$operator." ";
					/* lang */
					if($delim_code=$exprs[1]["delim_code"]){
						$delim_code_char=$delim_code{0};
						$r.=$delim_code_char.rawurlencode(str_replace($delim_code_char, "", $lang_val)).$delim_code_char;
					}
				}
			}
			if(trim($r)){
				if(!$in_optional){
					return $ind.$r;
				}
				else{
					/* add to optional patterns for left joins */
					if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val])){
						$alias_info=$alias_infos[0];
						$alias=$alias_info["alias"];
						$tbl_alias="T".$alias;
						if(!isset($this->optional_patterns[$tbl_alias])){
							$this->optional_patterns[$tbl_alias]=array();
						}
						$this->optional_patterns[$tbl_alias][]="(\n".$ind.$r."\n".substr($ind, 0, -1).")";
					}
				}
			}
		}
		/* result */
		return trim($r);
	}

	/*					*/

	function parse_built_in_call_filter_pattern($args=""){
		$result="";
		$call="";
		if(is_array($args)){foreach($args as $k=>$v){$$k=$v;}}
		if(is_array($pattern["call"])){
			$pattern=$pattern["call"];
		}
		if(is_array($pattern)){foreach($pattern as $k=>$v){$$k=$v;}}
		/* bound */
		if(($call=="bound") && $var){
			if($alias_infos=@$this->optional_term2alias[$var]){
				$alias_info=$alias_infos[0];
				$alias=$alias_info["alias"];
				$col=$alias_info["col"];
				$result.=($modifier=="!") ? "T".$alias.".".$col." IS NULL" : "T".$alias.".".$col." IS NOT NULL";
			}
		}
		/* isIRI/isURI */
		if(in_array($call, array("isiri", "isuri")) && ($expr=$expression) && ($expr["type"]=="var") && ($var_val=$expr["val"])){
			if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val])){
				$alias_info=$alias_infos[0];
				$alias=$alias_info["alias"];
				$col=$alias_info["col"];
				if($col=="p"){
					/* p is always an IRI */
					if($alias_info["optional_count"]){
						$result.=($expr["modifier"]=="!") ? "(T".$alias.".".$col." IS NULL OR 0)" : "";
					}
					else{
						$result.=($expr["modifier"]=="!") ? "0" : "";
					}
				}
				else{
					if($alias_info["optional_count"]){
						$result.=($expr["modifier"]=="!") ? "(T".$alias.".".$col." IS NULL OR NOT (T".$alias.".".$col."_type=0))" : "(T".$alias.".".$col." IS NULL OR T".$alias.".".$col."_type=0)";
					}
					else{
						$result.=($expr["modifier"]=="!") ? " NOT (T".$alias.".".$col."_type=0)" : " T".$alias.".".$col."_type=0";
					}
				}
			}
		}
		/* isBlank */
		if(in_array($call, array("isblank")) && ($expr=$expression) && ($expr["type"]=="var") && ($var_val=$expr["val"])){
			if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val])){
				$alias_info=$alias_infos[0];
				$alias=$alias_info["alias"];
				$col=$alias_info["col"];
				if($col=="p"){
					/* p is never a bnode */
					if($alias_info["optional_count"]){
						$result.=($expr["modifier"]=="!") ? "" : "(T".$alias.".".$col." IS NULL OR 0)";
					}
					else{
						$result.=($expr["modifier"]=="!") ? "" : "0";
					}
				}
				else{
					if($alias_info["optional_count"]){
						$result.=($expr["modifier"]=="!") ? "(T".$alias.".".$col." IS NULL OR NOT (T".$alias.".".$col."_type=1))" : "(T".$alias.".".$col." IS NULL OR T".$alias.".".$col."_type=1)";
					}
					else{
						$result.=($expr["modifier"]=="!") ? " NOT (T".$alias.".".$col."_type=1)" : " T".$alias.".".$col."_type=1";
					}
				}
			}
		}
		/* isLiteral */
		if(in_array($call, array("isliteral")) && ($expr=$expression) && ($expr["type"]=="var") && ($var_val=$expr["val"])){
			if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val])){
				$alias_info=$alias_infos[0];
				$alias=$alias_info["alias"];
				$col=$alias_info["col"];
				if($col=="p"){
					/* p is never a literal */
					if($alias_info["optional_count"]){
						$result.=($expr["modifier"]=="!") ? "" : "(T".$alias.".".$col." IS NULL OR 0)";
					}
					else{
						$result.=($expr["modifier"]=="!") ? "" : "0";
					}
				}
				else{
					if($alias_info["optional_count"]){
						$result.=($expr["modifier"]=="!") ? "(T".$alias.".".$col." IS NULL OR (T".$alias.".".$col."_type < 2))" : "(T".$alias.".".$col." IS NULL OR T".$alias.".".$col."_type > 1)";
					}
					else{
						$result.=($expr["modifier"]=="!") ? "(T".$alias.".".$col."_type < 2)" : " T".$alias.".".$col."_type > 1";
					}
				}
			}
		}
		/* regex */
		if(($call="regex") && ($exprs = @$expressions) && ((count($exprs)==3) || (count($exprs)==2))){
			$sub_result="";
			/* expr 1 */
			$expr_1=$exprs[0];
			if(isset($expr_1["call"]) && ($expr_1["call"]=="str")){
				$expr_1=$expr_1["expression"];
			}
			if(($expr_1["type"]=="var") 
				&& ($var_val=$expr_1["val"])
				&& ($expr_2=$exprs[1])
				&& (in_array($expr_2["type"], array("literal", "var")))
				&& ($match_val=$expr_2["val"])
			){
				$use_like=false;
				$prefix="";
				$suffix="";
				$sql_snippet="";
				if($expr_2["type"]=="literal"){
					if(preg_match("/^([\^]?)([a-z0-9_\-\@ \:\.,]+)([\$]?)$/i", $match_val, $matches)){/* simple string search */
						$use_like=true;
						$prefix=($matches[1]=='^') ? "" : "%";
						$suffix=($matches[3]=='$') ? "" : "%";
						$match_val=$matches[2];
						//$match_val=(substr($match_val, 0, 1)=='%') ? "\\".$match_val : $match_val;/* escape leading % */
						//$match_val=(substr($match_val, 0, 1)=='_') ? "\\".$match_val : $match_val;/* escape leading _ */
						$sql_snippet=($this->config["encode_values"]) ? "LIKE '".$prefix.rawurlencode($match_val).$suffix."'" : "LIKE '".$prefix.mysql_real_escape_string($match_val).$suffix."'";
					}
					else{
						$match_val=mysql_real_escape_string($match_val);
						$sql_snippet="REGEXP '".$match_val."'";/* won't work properly with "encode_values" */
					}
				}
				elseif($expr_2["type"]=="var"){
					if(($alias_infos=@$this->term2alias[$match_val]) || ($alias_infos=@$this->optional_term2alias[$match_val])){
						$alias_info=$alias_infos[0];
						$alias=$alias_info["alias"];
						$col=$alias_info["col"];
						$sql_snippet="REGEXP T".$alias.".".$col;
					}
				}
				if($sql_snippet && ($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val])){
					$alias_info=$alias_infos[0];
					$alias=$alias_info["alias"];
					$col=$alias_info["col"];
					$sub_result="V__regex_match_".$var_val."__.val ".$sql_snippet;
					$result.=(@$modifier && ($modifier=="!")) ? " NOT (".$sub_result.")" : " ".$sub_result;
					$this->val_match_vars[]=$var_val;
				}
			}
		}
		/* langMatches */
		if(($call="langmatches") && ($exprs = @$expressions) && (count($exprs)==2)){
			$sub_result="";
			/* expr 1 */
			$expr_1=$exprs[0];
			if(isset($expr_1["call"]) && ($expr_1["call"]=="lang")){
				$expr_1=$expr_1["expression"];
			}
			if(($expr_1["type"]=="var") 
				&& ($var_val=$expr_1["val"])
				&& ($expr_2=$exprs[1])
				&& (in_array($expr_2["type"], array("literal", "var")))
				&& ($lang_val=$expr_2["val"])
			){
				if(($alias_infos=@$this->term2alias[$var_val]) || ($alias_infos=@$this->optional_term2alias[$var_val])){
					$alias_info=$alias_infos[0];
					$alias=$alias_info["alias"];

					$prefix="";
					$suffix="";
					$sql_snippet="";
					if($expr_2["type"]=="literal"){
						if($lang_val=="*"){
							$sql_snippet.=(@$modifier && ($modifier=="!")) ? "(T".$alias.".o_lang='')" : " NOT (T".$alias.".o_lang='')";
						}
						else{
							$sql_snippet.=(@$modifier && ($modifier=="!")) ? " NOT (T".$alias.".o_lang LIKE '%".rawurlencode($lang_val)."%')" : " (T".$alias.".o_lang LIKE '%".rawurlencode($lang_val)."%')";
						}
					}
					elseif($expr_2["type"]=="var"){
						if(($alias_infos2=@$this->term2alias[$lang_val]) || ($alias_infos2=@$this->optional_term2alias[$lang_val])){
							$alias_info2=$alias_infos2[0];
							$alias2=$alias_info2["alias"];
							$col=$alias_info2["col"];
							$sql_snippet.=(@$modifier && ($modifier=="!")) ? " NOT (T".$alias.".o_lang LIKE T".$alias2.".o_lang)" : " (T".$alias.".o_lang LIKE T".$alias2.".o_lang)";
						}
					}
					if($sql_snippet){
						$result.=" ".$sql_snippet;
					}
				}
			}
		}
		return $result;
	}

	/*					*/

}

?>