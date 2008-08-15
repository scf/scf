<?php
class cssparser {
  var $css;
  var $html;
  
  function cssparser() {
    // Register "destructor"
    //register_shutdown_function(array(&$this, "finalize"));
    //$this->Clear();
    $this->css = array();
  }
  
  function Clear() {
    //unset($this->css);
    $this->css = array();
  }
  
  function Add($key, $codestr) {
  	if (!$codestr) return;
  
    $key = strtolower($key);
    $codestr = strtolower($codestr);
    if(!isset($this->css[$key])) {
      $this->css[$key] = array();
    }
    $codes = explode(";",$codestr);
    if(count($codes) > 0 and $codes[0]) {
      foreach($codes as $code) {
        //$code = trim($code);
        $codepieces = explode(":",$code);
        if(strlen($codepieces[0]) > 0) {
          $this->css[$key][trim($codepieces[0])] = trim($codepieces[1]);
        }
      }
    }
  }
  
  function Get($key, $property) {
    $key = strtolower($key);
    $property = strtolower($property);
    
    if (isset($this->css[$key][$property]))
    	return $this->css[$key][$property];
    	    
    preg_match('/^([0-9A-Za-z_-]*)(:[0-9A-Za-z_-]*)?(\.[0-9A-Za-z_-]*)?(#[0-9A-Za-z_-]*)?$/', $key, $matches);
    $tag = $matches[1];
    $subtag = $matches[2];
    $class = $matches[3];
    if (isset($matches[4])) $id = $matches[4];
    $result = "";
    foreach($this->css as $_tag => $value) {
        preg_match('/^([0-9A-Za-z_-]*)(:[0-9A-Za-z_-]*)?(\.[0-9A-Za-z_-]*)?(#[0-9A-Za-z_-]*)?$/', $_tag, $matches);
	    $_tag = $matches[1];
	    $_subtag = $matches[2];
	    $_class = $matches[3];
	    if (isset($matches[4])) $_id = $matches[4];  
      
      $tagmatch = $subtagmatch = $classmatch = $idmatch = false;
      
      if (!$_tag or $tag == $_tag) $tagmatch = true;
      if (!$_subtag or $subtag == $_subtag) $subtagmatch = true;
      if (!$_class or $class == $_class) $classmatch = true;
      if (!isset($id) or !isset($_id) or $id == $_id) $idmatch = true;
      
      if($tagmatch and $subtagmatch and $classmatch and $idmatch) {
      	if (isset($value[$property]))
      		$result = $value[$property];
      }
    }
    return $result;
  }
  
  function GetSection($key) {
    $key = strtolower($key);
    
    $matches = preg_match($key, '/^(\w*)(:\w*)?(\.\w*)?(#\w*)?$/');
    $tag = $matches[0];
    $subtag = $matches[1];
    $class = $matches[2];
    $id = $matches[3];
    
    $result = array();
    foreach($this->css as $_tag => $value) {
      list($_tag, $_subtag) = explode(":",$_tag);
      list($_tag, $_class) = explode(".",$_tag);
      list($_tag, $_id) = explode("#",$_tag);
      
      $tagmatch = (strcmp($tag, $_tag) == 0) | (strlen($_tag) == 0);
      $subtagmatch = (strcmp($subtag, $_subtag) == 0) | (strlen($_subtag) == 0);
      $classmatch = (strcmp($class, $_class) == 0) | (strlen($_class) == 0);
      $idmatch = (strcmp($id, $_id) == 0);
      
      if($tagmatch & $subtagmatch & $classmatch & $idmatch) {
        $temp = $_tag;
        if((strlen($temp) > 0) & (strlen($_class) > 0)) {
          $temp .= ".".$_class;
        } elseif(strlen($temp) == 0) {
          $temp = ".".$_class;
        }
        if((strlen($temp) > 0) & (strlen($_subtag) > 0)) {
          $temp .= ":".$_subtag;
        } elseif(strlen($temp) == 0) {
          $temp = ":".$_subtag;
        }
        foreach($this->css[$temp] as $property => $value) {
          $result[$property] = $value;
        }
      }
    }
    return $result;
  }
  
  function ParseStr($str) {
    //$this->Clear();
    // Remove comments
    $str = preg_replace("/\/\*(.*)?\*\//Usi", "", $str);
    // Parse this damn csscode
    $parts = explode("}",$str);
    if(count($parts) > 0) {
      foreach($parts as $part) {
      	if (strpos($part, '{') === false) continue;      	
      	
        list($keystr,$codestr) = explode("{",$part);
        $keys = explode(",",trim($keystr));
        if(count($keys) > 0) {
          foreach($keys as $key) {
            if(strlen($key) > 0) {
              $key = str_replace("\n", "", $key);
              $key = str_replace("\\", "", $key);
              $this->Add($key, trim($codestr));
            }
          }
        }
      }
    }
    //
    return (count($this->css) > 0);
  }
  
  function Parse($filename) {
    //$this->Clear();
    if(file_exists($filename)) {
      return $this->ParseStr(file_get_contents($filename));
    } else {
      return false;
    }
  }
  
  function GetCSS() {
    $result = "";
    foreach($this->css as $key => $values) {
      $result .= $key." {\n";
      foreach($values as $key => $value) {
        $result .= "  $key: $value;\n";
      }
      $result .= "}\n\n";
    }
    return $result;
  }
}
?>
