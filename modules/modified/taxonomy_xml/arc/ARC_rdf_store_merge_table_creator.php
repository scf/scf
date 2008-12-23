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
sk:className                ARC_rdf_store_merge_table_creator
doap:name                   ARC RDF Store MERGE Table Creator
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              Creates temporary MERGE tables for a split ARC RDF Store 
//release
doap:created                2006-04-02
doap:revision               0.1.0
//changelog
sk:releaseChanges           2006-04-02: release 0.1.0
*/

class ARC_rdf_store_merge_table_creator {

	var $version="0.1.0";
	var $created=array();
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
		if(!isset($this->config["prop_tables"])){
			$this->config["prop_tables"]=array();
		}
	}
	
	function ARC_rdf_store_merge_table_creator(&$api){
		$this->__construct($api);
	}

	/*					*/
	
	function tables_created(){
		return ($this->created) ? true : false;
	}
	
	/*					*/

	function get_merge_tables(){
		if($this->config["store_type"]=="basic"){
			return array();
		}
		return ($this->config["store_type"]=="basic+") ? array("triple_all_wdup") : array("triple_all", "triple_all_wdup", "triple_dp_all", "triple_op_all");
	}

	/*					*/

	function create_merge_tables(){
		mysql_query("FLUSH TABLES");
		foreach($this->get_merge_tables() as $cur_tbl){
			$tbl_name=$this->config["prefix"]."_".$cur_tbl;
			if(!mysql_query("SELECT 1 FROM ".$tbl_name." LIMIT 0")){/* table does not exist */
				$cur_mthd="create_".$cur_tbl."_table";
				$tmp=$this->$cur_mthd($tbl_name, $this->config["prefix"], $this->config["prop_tables"]);
			}
		}
		$this->created=true;
		return true;
	}

	function drop_merge_tables(){
		mysql_query("FLUSH TABLES");
		foreach($this->get_merge_tables() as $cur_tbl){
			$tbl_name=$this->config["prefix"]."_".$cur_tbl;
			if(mysql_query("SELECT 1 FROM ".$tbl_name." LIMIT 0") || mysql_error()){/* table exists or is broken*/
				$tmp=mysql_query("DROP TABLE ".$tbl_name);
			}
		}
		$this->created=false;
		return true;
	}

	/*					*/
	
	function create_merge_table($tbl_name, $engine_info=""){
		/* id_type */
		$id_type=$this->config["id_type"];
		$id_code = strpos($id_type, "_int") ? "bigint(20)" : (($id_type=="hash_md5") ? "char(21) BINARY" : "char(26) BINARY");
		/* reversible_consolidation */
		$s_init_code=($this->config["reversible_consolidation"]) ? "s_init ".$id_code." NOT NULL," : "";
		$o_init_code=($this->config["reversible_consolidation"]) ? "o_init ".$id_code." NOT NULL," : "";
		/* index_type */
		$index_type=$this->config["index_type"];
		$index_graph_iris=$this->config["index_graph_iris"];
		if(($index_type=="basic") && !$index_graph_iris){
			$index_code="KEY spo (s,p,o), KEY so (s,o), KEY po (p,o), KEY o (o)";
		}
		elseif(($index_type=="basic") && $index_graph_iris){
			$index_code="KEY spog (s,p,o,g), KEY sog (s,o,g), KEY pog (p,o,g), KEY gsp (g,s,p),	KEY pg (p,g), KEY og (o,g)";
		}
		elseif(($index_type=="advanced") && !$index_graph_iris){
			$index_code="KEY spo (s,p,o), KEY so (s,o), KEY po (p,o), KEY sp (s,p), KEY s (s), KEY p (p), KEY o (o), KEY o_comp (o_comp)";
		}
		if(($index_type=="advanced") && $index_graph_iris){
			$index_code=" KEY spog (s,p,o,g), KEY sog (s,o,g), KEY pog (p,o,g), KEY gsp (g,s,p), KEY sg (s,g), KEY pg (p,g), KEY og (o,g), KEY o_comp (o_comp)";
		}
		/* charset */
		$charset_code=(isset($this->config["charset"]) && strlen($this->config["charset"])) ? " CHARACTER SET ".$this->config["charset"] : "";
		/* collation */
		$collation_code=(isset($this->config["charset_collation"]) && strlen($this->config["charset_collation"])) ? " COLLATE ".$this->config["charset_collation"] : "";
		/* sql */
		//$temporary=" TEMPORARY";
		$temporary="";
		$sql="
			CREATE".$temporary." TABLE ".$tbl_name." (
				s ".$id_code." NOT NULL,
				p ".$id_code." NOT NULL,
				o ".$id_code." NOT NULL,
				g ".$id_code." NOT NULL,
				s_type tinyint(1) NOT NULL default '0',
				".$s_init_code."
				o_type tinyint(1) NOT NULL default '0',
				".$o_init_code."
				o_lang char(8) NULL,
				o_dt ".$id_code." NOT NULL,
				o_comp char(35) NULL,
				misc tinyint(2) NOT NULL default '0',
				".$index_code."
			) ENGINE=".$engine_info.$charset_code.$collation_code.";
		";
		mysql_query($sql);
	}

	/*					*/	

	function create_triple_all_table($tbl_name, $prefix, $prop_tbl_infos){
		$engine_info="MERGE UNION=(".$prefix."_triple_op,".$prefix."_triple_dp";
		foreach($prop_tbl_infos as $cur_info){
			$engine_info.=($cur_info["prop_type"]=="obj") ? ",".$prefix."_triple_op_".$cur_info["name"] : ",".$prefix."_triple_dp_".$cur_info["name"];
		}
		$engine_info.=")";
		return $this->create_merge_table($tbl_name, $engine_info);
	}

	function create_triple_all_wdup_table($tbl_name, $prefix, $prop_tbl_infos){
		if($this->config["store_type"]=="basic+"){
			$engine_info="MERGE UNION=(".$prefix."_triple,".$prefix."_triple_dup)";
		}
		else{
			$engine_info="MERGE UNION=(".$prefix."_triple_op,".$prefix."_triple_dp";
			foreach($prop_tbl_infos as $cur_info){
				$engine_info.=($cur_info["prop_type"]=="obj") ? ",".$prefix."_triple_op_".$cur_info["name"] : ",".$prefix."_triple_dp_".$cur_info["name"];
			}
			$engine_info.=",".$prefix."_triple_dup)";
		}
		$this->create_merge_table($tbl_name, $engine_info);
	}

	function create_triple_dp_all_table($tbl_name, $prefix, $prop_tbl_infos){
		$engine_info="MERGE UNION=(".$prefix."_triple_dp";
		foreach($prop_tbl_infos as $cur_info){
			$engine_info.=($cur_info["prop_type"]=="dt") ? ",".$prefix."_triple_dp_".$cur_info["name"] : "";
		}
		$engine_info.=")";
		$this->create_merge_table($tbl_name, $engine_info);
	}

	function create_triple_op_all_table($tbl_name, $prefix, $prop_tbl_infos){
		$engine_info="MERGE UNION=(".$prefix."_triple_op";
		foreach($prop_tbl_infos as $cur_info){
			$engine_info.=($cur_info["prop_type"]=="obj") ? ",".$prefix."_triple_op_".$cur_info["name"] : "";
		}
		$engine_info.=")";
		$this->create_merge_table($tbl_name, $engine_info);
	}

	/*					*/

}

?>