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
sk:className                ARC_rdf_store_keeper
doap:name                   ARC RDF Store Keeper
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              ARC RDF Store maintenance functions
//release
doap:created                2006-10-01
doap:revision               0.1.3
//changelog
sk:releaseChanges           2006-02-08: release 0.1.0
                            2006-02-19: revision 0.1.1
                                        - added "delete_tables" method
                            2006-03-28: revision 0.1.2
                                        - adjusted for ARC API integration
                            2006-10-01: revision 0.1.3
                                        - some minor tweaks
*/

class ARC_rdf_store_keeper {

	var $version="0.1.3";
	
	function __construct(&$api){
		$this->api =& $api;
		$this->config=$this->api->get_config();
	}
	
	function ARC_rdf_store_keeper(&$api){
		$this->__construct($api);
	}

	/*					*/

	function create_tables(){
		if(!isset($this->config["store_type"]) || !in_array($this->config["store_type"], array("basic", "basic+", "split"))){
			return $this->api->error("Undefined or invalid store type.");
		}
		/* basic tables */
		$tbls=$this->api->get_base_tables();
		foreach($tbls as $cur_tbl){
			$tbl_name=$this->config["prefix"]."_".$cur_tbl;
			if(!mysql_query("SELECT 1 FROM ".$tbl_name." LIMIT 0")){/* table does not exist */
				$cur_mthd="create_".$cur_tbl."_table";
				$tmp=$this->$cur_mthd($tbl_name);
			}
		}
		/* prop_tables */
		if(($this->config["store_type"]=="split") && isset($this->config["prop_tables"])){
			$tbls=$this->api->get_prop_tables();
			foreach($tbls as $cur_tbl){
				$tbl_name=$this->config["prefix"]."_".$cur_tbl;
				if(!mysql_query("SELECT 1 FROM ".$tbl_name." LIMIT 0")){/* table does not exist */
					$tmp=$this->create_triple_table($tbl_name);
				}
			}
		}
		return true;
	}
	
	function delete_tables(){
		$tmp=mysql_query("FLUSH TABLES");
		$tbls=$this->api->get_tables();
		foreach($tbls as $cur_tbl){
			$tmp=mysql_query("DROP TABLE IF EXISTS ".$this->config["prefix"]."_".$cur_tbl);
		}
	}

	function reset_tables(){
		$prefix=$this->config["prefix"];
		$tmp=mysql_query("FLUSH TABLES");
		$tbls=$this->api->get_tables();
		foreach($tbls as $cur_tbl){
			$tmp=mysql_query("TRUNCATE ".$prefix."_".$cur_tbl);
		}
		/* add empty val to id2val */
		$id_type=$this->config["id_type"];
		if($id_type=="incr_int"){
			$id=$this->api->get_id("");/* auto-adds row to id2val table */
		}
		else{
			mysql_query("INSERT IGNORE INTO ".$prefix."_id2val (id, val) VALUES(".$this->api->get_id("", 1).", '')");
		}
	}
	
	/*					*/
	
	function create_triple_table($tbl_name){
		/* id_type */
		$id_type=$this->config["id_type"];
		$id_code = strpos($id_type, "_int") ? "bigint(20)" : (($id_type=="hash_md5") ? "char(21) BINARY" : "char(26) BINARY");
		/* reversible_consolidation */
		$s_init_code=(isset($this->config["reversible_consolidation"]) && $this->config["reversible_consolidation"]) ? "s_init ".$id_code." NOT NULL," : "";
		$o_init_code=(isset($this->config["reversible_consolidation"]) && $this->config["reversible_consolidation"]) ? "o_init ".$id_code." NOT NULL," : "";
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
		$sql="
			CREATE TABLE ".$tbl_name." (
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
			) ENGINE=MyISAM".$charset_code.$collation_code.";
		";
		mysql_query($sql);
	}

	function create_triple_dp_table($tbl_name){
		$this->create_triple_table($tbl_name);
	}

	function create_triple_op_table($tbl_name){
		$this->create_triple_table($tbl_name);
	}

	function create_triple_dup_table($tbl_name){
		$this->create_triple_table($tbl_name);
	}

	function create_id2val_table($tbl_name){
		/* id_type */
		$id_type=$this->config["id_type"];
		if($id_type=="hash_int"){
			$id_code="bigint(20) NOT NULL";
		}
		elseif($id_type=="hash_md5"){
			$id_code="char(21) BINARY NOT NULL";
		}
		elseif($id_type=="hash_sha1"){
			$id_code="char(26) BINARY NOT NULL";
		}
		elseif($id_type=="incr_int"){
			$id_code="bigint(20) NOT NULL AUTO_INCREMENT";
		}
		/* index_words */
		$index_words=(isset($this->config["index_words"])) ? $this->config["index_words"] : false;
		$index_addition=($index_words) ? ",\n FULLTEXT KEY val (val) " : "";
		/* charset */
		$charset_code=(isset($this->config["charset"]) && strlen($this->config["charset"])) ? " CHARACTER SET ".$this->config["charset"] : "";
		/* collation */
		$collation_code=(isset($this->config["charset_collation"]) && strlen($this->config["charset_collation"])) ? " COLLATE ".$this->config["charset_collation"] : "";
		/* sql */
		$sql="
			CREATE TABLE ".$tbl_name." (
				id ".$id_code.", 
				misc tinyint(1) NOT NULL default '0',
				val text NULL,  
				PRIMARY KEY (id)".$index_addition."
			) ENGINE=MyISAM ".$charset_code.$collation_code.";
		";
		$tmp=mysql_query($sql);
		/* add empty val to id2val */
		if($id_type=="incr_int"){
			$id=$this->api->get_id("");/* auto-adds row to id2val table */
		}
		else{
			$tmp=mysql_query("INSERT IGNORE INTO ".$tbl_name." (id, val) VALUES(".$this->api->get_id("", 1).", '')");
		}
	}
	
	function create_store_var_table($tbl_name){
		/* charset */
		$charset_code=(isset($this->config["charset"]) && strlen($this->config["charset"])) ? " CHARACTER SET ".$this->config["charset"] : "";
		/* collation */
		$collation_code=(isset($this->config["charset_collation"]) && strlen($this->config["charset_collation"])) ? " COLLATE ".$this->config["charset_collation"] : "";
		$sql="
			CREATE TABLE ".$tbl_name." (
				var_cat varchar(20) NOT NULL,
				var_cat_qlfr varchar(80) NULL,
				var_name varchar(40) NOT NULL,
				var_val text NULL,
				KEY ccqn (var_cat,var_cat_qlfr,var_name)
			) ENGINE=MyISAM ".$charset_code.$collation_code.";
		";
		mysql_query($sql);
	}

	/*					*/

	function clean_up_id2val(){
		$tbl_1=$this->config["prefix"]."_id2val";
		$store_type=$this->config["store_type"];
		$tmp=$this->api->create_split_merge_tables();
		$this->api->lock_tables(array("I2V", "T"));
		/* flag values */
		$tmp=mysql_query("UPDATE ".$tbl_1." SET misc=1");
		$tmp=mysql_query("UPDATE ".$tbl_1." SET misc=0 WHERE val=''");
		/* un-flag ref'd hashes */
		$cols=array('s', 'p', 'o', 'g', 'o_dt');
		if($this->config["reversible_consolidation"]){
			$cols=array_merge($cols, array('s_init', 'o_init'));
		}
		foreach($cols as $cur_col){
			if($store_type=="split"){
				$sql="
					UPDATE ".$tbl_1." I2V, ".$this->config["prefix"]."_triple_all_wdup T
					SET I2V.misc=0
					WHERE I2V.misc=1 AND I2V.id=T.".$cur_col."
				";
				$tmp=mysql_query($sql);
			}
			elseif(in_array($store_type, array("basic", "basic+"))){
				$sql="
					UPDATE ".$tbl_1." I2V, ".$this->config["prefix"]."_triple T
					SET I2V.misc=0
					WHERE I2V.misc=1 AND I2V.id=T.".$cur_col."
				";
				$tmp=mysql_query($sql);
				$sql="
					UPDATE ".$tbl_1." I2V, ".$this->config["prefix"]."_triple_dup T
					SET I2V.misc=0
					WHERE I2V.misc=1 AND I2V.id=T.".$cur_col."
				";
				$tmp=mysql_query($sql);
			}
		}
		$result=mysql_query("DELETE FROM ".$tbl_1." WHERE misc=1");
		$del_count=mysql_affected_rows();
		$this->api->unlock_tables();
		return array("del_count"=>$del_count);
	}

	/*					*/

	function move_triple_duplicates(){
		$tbls=$this->api->get_triple_tables();
		$prefix=$this->config["prefix"];
		$id_type=$this->config["id_type"];
		$conv_id=in_array($id_type, array("hash_int", "incr_int"));
		
		$dup_count=0;
		$t1=$this->api->get_mtime();
		$this->api->lock_tables();
		$dup_tbl=$prefix."_triple_dup";
		$diff_cols=array("g");
		if(isset($this->config["reversible_consolidation"]) && $this->config["reversible_consolidation"]){
			$diff_cols[]="s_init";
			$diff_cols[]="o_init";
		}
		foreach($tbls as $cur_tbl){
			$main_tbl=$prefix."_".$cur_tbl;
			/* remove triple dupes with different g/s_init/o_init */
			foreach($diff_cols as $cur_diff_col){
				$offset=0;
				do{
					$proceed=false;
					/* retrieve spo dupes (w/o un-indexed lang/dt) */
					$cur_col_code=($conv_id) ? "CONV(T1.s, 10, 16) AS s,CONV(T1.p, 10, 16) AS p,CONV(T1.o, 10, 16) AS o,CONV(T1.".$cur_diff_col.", 10, 16) AS ".$cur_diff_col : "T1.s,T1.p,T1.o,T1.".$cur_diff_col;
					$sql="SELECT DISTINCT ".$cur_col_code." FROM ".$main_tbl." T1, ".$main_tbl." T2 WHERE T1.s=T2.s AND NOT (T1.".$cur_dist_col."=T2.".$cur_dist_col.") AND T1.p=T2.p AND T1.o=T2.o LIMIT ".$offset.",100";
					$offset+=100;
					if($rs=mysql_query($sql)){
						$row_count=mysql_num_rows($rs);
						$proceed=($row_count>1) ? true : false;
						while($row=mysql_fetch_array($rs)){
							$s_id=$row["s"];
							$p_id=$row["p"];
							$o_id=$row["o"];
							$cur_diff_col_id=$row[$cur_diff_col];
							$s_sql=($conv_id) ? "CONV('".$s_id."', 16, 10)" : "'".$s_id."'";
							$p_sql=($conv_id) ? "CONV('".$p_id."', 16, 10)" : "'".$p_id."'";
							$o_sql=($conv_id) ? "CONV('".$o_id."', 16, 10)" : "'".$o_id."'";
							$cur_diff_col_sql=($conv_id) ? "CONV('".$cur_diff_col_id."', 16, 10)" : "'".$cur_diff_col_id."'";
							/* check for complete dupes */
							$sub_col_code=($conv_id) ? "T1.o_lang,CONV(T1.o_dt, 10, 16) AS o_dt" : "T1.o_lang,T1.o_dt";
							$sql="SELECT DISTINCT ".$sub_col_code." FROM ".$main_tbl." T1, ".$main_tbl." T2 WHERE T1.s=".$s_sql." AND T2.s=".$s_sql." AND T1.".$cur_dist_col."=".$cur_dist_col_sql." AND NOT (T2.".$cur_dist_col."=".$cur_dist_col_sql.") AND T1.p=".$p_sql." AND T2.p=".$p_sql." AND T1.o=".$o_sql." AND T2.o=".$o_sql." AND T1.o_lang=T2.o_lang AND T1.o_dt=T2.o_dt LIMIT 2";
							if(($rs2=mysql_query($sql)) && (mysql_num_rows($rs2))){
								$rs2_row=mysql_fetch_array($rs2);
								$o_lang=$rs2_row["o_lang"];
								$o_dt_id=$rs2_row["o_dt"];
								$o_lang_sql="'".$o_lang."'";
								$o_dt_sql=($conv_id) ? "CONV('".$o_dt_id."', 16, 10)" : "'".$o_dt_id."'";
								/* copy dupes */
								$ins_sql="INSERT INTO ".$dup_tbl." SELECT * FROM ".$main_tbl." WHERE s=".$s_sql." AND p=".$p_sql." AND o=".$o_sql." AND o_lang=".$o_lang_sql." AND o_dt=".$o_dt_sql." AND NOT (".$cur_dist_col."=".$cur_dist_col_sql.")";
								$tmp=mysql_query($ins_sql);
								$cur_dup_count=mysql_affected_rows();
								$dup_count+=$cur_dup_count;
								/* del dupes */
								$del_sql="DELETE FROM ".$main_tbl." WHERE s=".$s_sql." AND p=".$p_sql." AND o=".$o_sql." AND o_lang=".$o_lang_sql." AND o_dt=".$o_dt_sql." AND NOT (".$cur_dist_col."=".$cur_dist_col_sql.")";
								$tmp=mysql_query($del_sql);
								@mysql_free_result($rs2);
								/* adjust offset (could perhaps be optimized) */
								$offset=0;
							}
						}
					}
				} while ($proceed);
			}
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		$t3=$this->api->get_mtime();
		return array("duplicate_count"=>$dup_count, "processing_time"=>round($t2-$t1, 4), "index_update_time"=>round($t3-$t2, 4));
	}

	function restore_triple_duplicates(){
		$prefix=$this->config["prefix"];
		$id_type=$this->config["id_type"];
		$conv_id=in_array($id_type, array("hash_int", "incr_int"));

		$dup_tbl=$prefix."_triple_dup";
		$t_count=0;
		$t1=$this->api->get_mtime();
		$this->api->lock_tables();
		if(in_array($this->config["store_type"], array("basic", "basic+"))){
			$main_tbl=$prefix."_triple";
			$tmp=mysql_query("INSERT INTO ".$main_tbl." SELECT * FROM ".$dup_tbl);
			$t_count+=mysql_affected_rows();
			$tmp=mysql_query("DELETE FROM ".$dup_tbl."");
		}
		elseif($this->config["store_type"]=="split"){
			/* prop tables */
			$prop_tbl_infos=$this->api->get_prop_table_infos();
			foreach($prop_tbl_infos as $p=>$infos){
				$main_tbl=$infos["tbl"];
				$p_id_code=$this->api->get_id($p, 1);
				$tmp=mysql_query("INSERT INTO ".$main_tbl." SELECT * FROM ".$dup_tbl." WHERE p=".$p_id_code);
				$t_count+=mysql_affected_rows();
				$tmp=mysql_query("DELETE FROM ".$dup_tbl." WHERE p=".$p_id_code);
			}
			/* dp */
			$main_tbl=$prefix."_triple_dp";
			$tmp=mysql_query("INSERT INTO ".$main_tbl." SELECT * FROM ".$dup_tbl." WHERE (o_type=2 OR o_type=3)");
			$t_count+=mysql_affected_rows();
			$tmp=mysql_query("DELETE FROM ".$dup_tbl." WHERE (o_type=2 OR o_type=3)");
			/* op */
			$main_tbl=$prefix."_triple_op";
			$tmp=mysql_query("INSERT INTO ".$main_tbl." SELECT * FROM ".$dup_tbl);
			$t_count+=mysql_affected_rows();
			$tmp=mysql_query("DELETE FROM ".$dup_tbl."");
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		$t3=$this->api->get_mtime();
		return array("triple_count"=>$t_count, "processing_time"=>round($t2-$t1, 4), "index_update_time"=>round($t3-$t2, 4));
	}

	/*					*/
	
	function consolidate_resources($args=""){
		$tmp=$this->api->create_split_merge_tables();
		$tmp1=$tmp2=$tmp3=$tmp4=false;
		if(isset($args["ifp"]) && ($ifp=$args["ifp"])){
			$tmp1=$this->consolidate_resources_on_ifp($ifp);
		}
		if(isset($args["ifps"]) && ($ifps=$args["ifps"])){
			$tmp2=$this->consolidate_resources_on_ifp($ifps);
		}
		if(isset($args["fp"]) && ($fp=$args["fp"])){
			$tmp3=$this->consolidate_resources_on_fp($fp);
		}
		if(isset($args["fps"]) && ($fps=$args["fps"])){
			$tmp4=$this->consolidate_resources_on_fp($fps);
		}
		if(!is_array($tmp1) && !is_array($tmp2) && !is_array($tmp3) && !is_array($tmp4)){
			return array("error"=>"missing_parameter (ifp, ifps, fp, or fps)");
		}
		$result=array("error"=>"", "subject_count"=>0, "object_count"=>0, "processing_time"=>0, "index_update_time"=>0);
		for($i=1;$i<=4;$i++){
			$cur_tmp=${"tmp".$i};
			if(is_array($cur_tmp)){
				foreach($cur_tmp as $k=>$v){
					if(!is_numeric($k) && isset($result[$k])){
						if(is_numeric($v)){
							$result[$k]+=$v;
						}
						else{
							$result[$k].=$v;
						}
					}
				}
			}
		}
		return $result;
	}

	function consolidate_resources_on_ifp($p=""){
		if(!$p){
			return array("error"=>"empty ifp");
		}
		$ps=is_array($p) ? $p : array($p);
		$tbls=$this->api->get_triple_tables();
		$prefix=$this->config["prefix"];
		$id_type=$this->config["id_type"];
		$conv_p=($id_type=="hash_int") ? true : false;
		$conv_id=in_array($id_type, array("hash_int", "incr_int"));
		
		$o_count=0;
		$s_count=0;
		$t1=$this->api->get_mtime();
		$this->api->lock_tables();

		$main_tbl=($this->config["store_type"]=="split") ? $prefix."_triple_all" : $prefix."_triple";
		$empty_id_sql=$this->api->get_id("", 1);
		foreach($ps as $p){
			$p_sql=$this->api->get_id($p, 1);
			do{
				$proceed=false;
				$cur_col_code=($conv_id) ? "CONV(T1.s, 10, 16) AS s, T1.s_type, CONV(T1.o, 10, 16) AS o" : "T1.s, T1.s_type, T1.o";
				$sql="SELECT DISTINCT ".$cur_col_code." FROM ".$main_tbl." T1, ".$main_tbl." T2 WHERE T1.p=".$p_sql." AND T2.p=".$p_sql." AND NOT (T1.o=".$empty_id_sql.") AND T1.o=T2.o AND NOT (T1.s=T2.s) ORDER BY T1.s_type ASC LIMIT 2";
				if(($rs=mysql_query($sql)) && ($row_count=mysql_num_rows($rs)) && ($row_count > 1)){
					$proceed=true;
					$row=mysql_fetch_array($rs);
					$o_id=$row["o"];
					$s_id=$row["s"];/* use as canonical s */
					$o_sql=($conv_id) ? "CONV('".$o_id."', 16, 10)" : "'".$o_id."'";
					$s_sql=($conv_id) ? "CONV('".$s_id."', 16, 10)" : "'".$s_id."'";
					$s_type=$row["s_type"];
					@mysql_free_result($rs);
					foreach($tbls as $cur_tbl){
						$t_tbl=$prefix."_".$cur_tbl;
						/* consolidate objects */
						$sql="UPDATE ".$t_tbl." T2 LEFT JOIN ".$t_tbl." T1 ON (T2.o=T1.s) SET T2.o=".$s_sql.", T2.o_type=".$s_type." WHERE T1.p=".$p_sql." AND T1.o=".$o_sql;
						$tmp=mysql_query($sql);
						$o_count+=mysql_affected_rows();
						/* consolidate subjects */
						$sql="UPDATE ".$t_tbl." SET s=".$s_sql.", s_type=".$s_type." WHERE p=".$p_sql." AND o=".$o_sql;
						$tmp=mysql_query($sql);
						$s_count+=mysql_affected_rows();
					}
				}
			} while ($proceed);
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		$t3=$this->api->get_mtime();
		return array("subject_count"=>$s_count, "object_count"=>$o_count, "processing_time"=>round($t2-$t1, 4), "index_update_time"=>round($t3-$t2, 4));
	}
	
	function consolidate_resources_on_fp($p=""){
		if(!$p){
			return array("error"=>"empty fp");
		}
		$ps=is_array($p) ? $p : array($p);
		$tbls=$this->api->get_triple_tables();
		$prefix=$this->config["prefix"];
		$id_type=$this->config["id_type"];
		$conv_p=($id_type=="hash_int") ? true : false;
		$conv_id=in_array($id_type, array("hash_int", "incr_int"));

		$o_count=0;
		$s_count=0;
		$t1=$this->api->get_mtime();
		$this->api->lock_tables();

		$main_tbl=($this->config["store_type"]=="split") ? $prefix."_triple_all" : $prefix."_triple";
		$empty_id_sql=$this->api->get_id("", 1);
		foreach($ps as $p){
			$p_sql=$this->api->get_id($p, 1);
			do{
				$proceed=false;
				$cur_col_code=($conv_id) ? "CONV(T1.o, 10, 16) AS o, T1.o_type, CONV(T1.s, 10, 16) AS s" : "T1.o, T1.o_type, T1.s";
				$sql="SELECT DISTINCT ".$cur_col_code." FROM ".$main_tbl." T1, ".$main_tbl." T2 WHERE T1.p=".$p_sql." AND T2.p=".$p_sql." AND NOT (T1.s=".$empty_id_sql.") AND T1.s=T2.s AND NOT (T1.o=T2.o) ORDER BY T1.o_type ASC LIMIT 2";
				if(($rs=mysql_query($sql)) && ($row_count=mysql_num_rows($rs)) && ($row_count > 1)){
					$proceed=true;
					$row=mysql_fetch_array($rs);
					$s_id=$row["s"];
					$o_id=$row["o"];/* use as canonical o */
					$s_sql=($conv_id) ? "CONV('".$s_id."', 16, 10)" : "'".$s_id."'";
					$o_sql=($conv_id) ? "CONV('".$o_id."', 16, 10)" : "'".$o_id."'";
					$o_type=$row["o_type"];
					@mysql_free_result($rs);
					foreach($tbls as $cur_tbl){
						$t_tbl=$prefix."_".$cur_tbl;
						/* consolidate subject */
						$sql="UPDATE ".$t_tbl." T2 LEFT JOIN ".$t_tbl." T1 ON (T2.s=T1.o) SET T2.s=".$o_sql.", T2.s_type=".$o_type." WHERE T1.p=".$p_sql." AND T1.s=".$s_sql;
						$tmp=mysql_query($sql);
						$s_count+=mysql_affected_rows();
						/* consolidate objects */
						$sql="UPDATE ".$t_tbl." SET o=".$o_sql.", o_type=".$o_type." WHERE p=".$p_sql." AND s=".$s_sql;
						$tmp=mysql_query($sql);
						$o_count+=mysql_affected_rows();
					}
				}
			} while ($proceed);
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		$t3=$this->api->get_mtime();
		return array("subject_count"=>$s_count, "object_count"=>$o_count, "processing_time"=>round($t2-$t1, 4), "index_update_time"=>round($t3-$t2, 4));
	}
	
	function undo_resource_consolidation($args=""){
		if(!isset($this->config["reversible_consolidation"]) || !$this->config["reversible_consolidation"]){
			return array("error"=>"not supported");
		}

		$tbls=$this->api->get_triple_tables();
		$tbls[]="triple_dup";/* reset dupes as well */
		$prefix=$this->config["prefix"];

		$s_count=0;
		$o_count=0;
		$t1=$this->api->get_mtime();
		$this->api->lock_tables();
		
		$resource_id=(isset($args["resource_id"])) ? $args["resource_id"] : false;
		$resource_id_type=($resource_id && (strpos($resource_id, "_:")===0)) ? "bnode" : "iri";
		$resource_id_sql=($resource_id) ? $this->api->get_id($resource_id, 1) : "";

		foreach($tbls as $cur_tbl){
			$t_tbl=$prefix."_".$cur_tbl;
			/* s */
			$sql="UPDATE ".$t_tbl." SET s=s_init";
			if($resource_id && ($resource_id_type=="iri")){
				$sql.=" WHERE s=".$resource_id_sql." AND s_type=0";
			}
			elseif($resource_id && ($resource_id_type=="bnode")){
				$sql.=" WHERE s=".$resource_id_sql." AND s_type=1";
			}
			$tmp=mysql_query($sql);
			$s_count+=mysql_affected_rows();
			/* o */
			$sql="UPDATE ".$t_tbl." SET o=o_init";
			if($resource_id && ($resource_id_type=="iri")){
				$sql.=" WHERE o=".$resource_id_sql." AND o_type=0";
			}
			elseif($resource_id && ($resource_id_type=="bnode")){
				$sql.=" WHERE o=".$resource_id_sql." AND o_type=1";
			}
			$tmp=mysql_query($sql);
			$o_count+=mysql_affected_rows();
		}
		$t2=$this->api->get_mtime();
		$this->api->unlock_tables();
		$t3=$this->api->get_mtime();
		return array("subject_count"=>$s_count, "object_count"=>$o_count, "processing_time"=>round($t2-$t1, 4), "index_update_time"=>round($t3-$t2, 4));
	}
	
	/*					*/

}

?>