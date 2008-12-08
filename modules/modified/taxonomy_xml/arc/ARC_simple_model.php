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
sk:className                ARC_simple_model
doap:name                   ARC simple model
doap:homepage               http://www.appmosphere.com/en-arc_model
doap:license                http://www.appmosphere.com/en-arc_license
doap:programming-language   PHP
doap:maintainer             Benjamin Nowack
doap:shortdesc              A simple (i.e. array-based) PHP RDF model class for handling ARC triples
doap:description            features:
                            - get a model's resources: $resources=$model->get_resources(); $first_uri_or_bnode_id=$resources[0]["val"];
                            - get a model's typed resources, e.g.: $resources=$model->get_resources("http://xmlns.com/foaf/0.1/Person");
                            - get the properties of a model's resource: $props=$resources[0]["props"];$resource_name=$resources[0]["props"]["foaf:name"][0]["val"];
							- get resource properties by uri or bnode_id, e.g.: $props=$model->get_resource_props("http://www.example.com/res4711");
							- get a model's resource by uri or bnode_id, e.g.: $resource=$model->get_resource("http://www.example.com/res4711");
//release
doap:created                2004-12-03
doap:revision               0.1.0
//changelog
sk:releaseChanges           2004-12-03: release 0.1.0

*/

class ARC_simple_model{

	var $triples=array();
	var $ns_abbrs=array();/* uri2prefix mapping, URIs with empty prefixes will be stored without a ":"-connector */

	var $resources=array();
	var $resource_indices=array();/* resource_uri/bnode_id -> index in $resources array */
	var $typed_resources=array();/* class_uri -> resource_uri[] */
	
	function ARC_simple_model($args=""){
		if(is_array($args)){foreach($args as $k=>$v){$this->$k=$v;}}/* triples, ns_abbrs */
		$this->init();
	}

	/*					*/

	function set_triples($triples){
		$this->triples=$triples;
	}
		
	function get_triples(){
		return $this->triples;
	}

	/*					*/
	
	function get_abbr_val($val=""){
		/* split */
		if(preg_match("/(.+)#(.+)$/", $val, $matches)){/* fragId */
			$ns_uri=$matches[1]."#";
			$local_part=$matches[2];
		}
		elseif(preg_match("/(.+)\/([^\/]+)$/", $val, $matches)){/* last slash */
			$ns_uri=$matches[1]."/";
			$local_part=$matches[2];
		}
		if(isset($ns_uri) && isset($this->ns_abbrs[$ns_uri])){
			if($abbr=$this->ns_abbrs[$ns_uri]){
				return $abbr.":".$local_part;
			}
			else{/* empty abbr */
				return $local_part;
			}
		}
		return $val;
	}

	/*					*/

	function init(){
		$triples=&$this->triples;/* reference */
		
		$resources=array();/* resources */
		$resource_indices=array();/* resource_index */
		$typed_resources=array();/* key=class uri */
		for($i=0,$i_max=count($triples);$i<$i_max;$i++){
			$cur_t=$triples[$i];
			/* s,p,o vars */
			$s=$cur_t["s"];
			$s_type=$s["type"];
			$s_val=($s_type=="uri")? $s["uri"] : $s["bnode_id"];
			$s_val=($s_type=="uri")? $this->get_abbr_val($s_val) : $s_val;
			$p_full=$cur_t["p"];
			$p=$this->get_abbr_val($p_full);
			$o=$cur_t["o"];
			$o_type=$o["type"];
			$o_val=($o_type=="uri")? $o["uri"] : $o["bnode_id"];
			$o_val=($o_type=="literal")? $o["val"] : $o_val;
			$o_val=($o_type=="uri")? $this->get_abbr_val($o_val) : $o_val;
			$o_val=(strpos(utf8_decode(str_replace("?", "", $o_val)), "?")===false) ? utf8_decode($o_val) : $o_val;
			
			$o_dt=(($o_type=="literal") && ($o["dt"])) ? $o["dt"] : "";
			$o_lang=(($o_type=="literal") && ($o["lang"])) ? $o["lang"] : "";
			/* s */
			if(!isset($resource_indices[$s_val])){/* new entry */
				$cur_resource_index=count($resources);
				$resources[$cur_resource_index]=array("val"=>$s_val, "type"=>$s_type);
				$resource_indices[$s_val]=$cur_resource_index;
			}
			else{
				$cur_resource_index=$resource_indices[$s_val];
			}
			/* props */
			if(!isset($resources[$cur_resource_index]["props"])){
				$resources[$cur_resource_index]["props"]=array();
			}
			$props=&$resources[$cur_resource_index]["props"];
			/* props[$p] */
			if(!isset($props[$p])){
				$props[$p]=array();
			}
			$props[$p][]=array("val"=>$o_val, "type"=>$o_type, "dt"=>$o_dt, "lang"=>$o_lang);
			/* typed r */
			if($p_full=="http://www.w3.org/1999/02/22-rdf-syntax-ns#type"){
				if(!isset($typed_resources[$o_val])){
					$typed_resources[$o_val]=array();
				}
				$typed_resources[$o_val][]=&$resources[$cur_resource_index];
			}
		}
		$this->resources=$resources;
		$this->resource_indices=$resource_indices;
		$this->typed_resources=$typed_resources;
		return true;
	}

	/*					*/

	function get_resources($type=""){
		if($type){
			if(is_array($type)){
				$result=array();
				$added_resources=array();
				for($i=0,$i_max=count($type);$i<$i_max;$i++){
					$cur_type=$type[$i];
					$cur_resources=$this->typed_resources[$this->get_abbr_val($cur_type)];
					for($j=0,$j_max=count($cur_resources);$j<$j_max;$j++){
						$cur_resource=$cur_resources[$j];
						$cur_resource_identifier=$cur_resource["val"];
						if(!$added_resources[$cur_resource_identifier]){
							$result[]=$cur_resource;
							$added_resources[$cur_resource_identifier]=true;
						}
					}
				}
				return $result;
			}
			else{
				return $this->typed_resources[$this->get_abbr_val($type)];
			}
		}
		else{
			return $this->resources;
		}
	}

	function get_resource($identifier=""){
		return $this->resources[$this->resource_indices[$identifier]];
	}
	
	function resource_is_of_type($res, $type){
		$props=$res["props"][$this->get_abbr_val("http://www.w3.org/1999/02/22-rdf-syntax-ns#type")];
		if(is_array($props)){
			for($i=0,$i_max=count($props);$i<$i_max;$i++){
				if(is_array($type)){
					if(in_array($props[$i]["val"], $type)){
						return true;
					}
				}
				else{
					if($props[$i]["val"]==$this->get_abbr_val($type)){
						return true;
					}
				}
			}
		}
		return false;
	}
	
	/*					*/

	function get_resource_props($identifier="", $prop=""){
		if($prop){
			if(isset($this->resources[$this->resource_indices[$identifier]]["props"][$prop])){
				return $this->resources[$this->resource_indices[$identifier]]["props"][$prop];
			}
			return array();
		}
		else{
			return $this->resources[$this->resource_indices[$identifier]]["props"];
		}
	}
	
	function get_resource_prop_val($res, $prop){
		if($res && isset($res["props"]) && isset($res["props"][$prop])){
			return $res["props"][$prop][0]["val"];
		}
		return "";
	}
	
	function rpv($res, $prop){
		return $this->get_resource_prop_val($res, $prop);
	}
	
	/*					*/
	
	function get_list_entries($list_id){
		$result=array();
		if($res=$this->get_resource($list_id)){
			while($res){
				if($f_props=$res["props"][$this->get_abbr_val("http://www.w3.org/1999/02/22-rdf-syntax-ns#first")]){
					$first_id=$f_props[0]["val"];
					if($r_props=$res["props"][$this->get_abbr_val("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest")]){
						$result[]=$this->get_resource($first_id);
						$rest_id=$r_props[0]["val"];
						if($rest_id!="http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"){
							$res=$this->get_resource($rest_id);
						}
						else{
							$res=false;
						}
					}
				}
			}
		}
		return $result;
	}

	/*					*/

}
?>