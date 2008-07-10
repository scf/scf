<?php

require_once drupal_get_path('module', 'nodeproxy') . '/AbstractNodeProxyModule.php';

define('RDFNODEPROXY_TYPES_VAR', 'rdfnodeproxy_types');

abstract class RdfNodeProxyModule extends AbstractNodeProxyModule {
  
  private $prefixes = array(
    'rdf' => "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
    'rdfs' => "http://www.w3.org/2000/01/rdf-schema#",
    'xsd' => "http://www.w3.org/2001/XMLSchema#", 
    'owl' => "http://www.w3.org/2002/07/owl#",
    'dc' => "http://purl.org/dc/elements/1.1/"
  );
  
  public function __construct ($moduleName) {
    parent::__construct($moduleName);
  }

  protected function definePrefix ($pref, $uri) {
    if (!empty($uri)) {
      $this->prefixes[$pref] = $uri;
    }
  }

  protected function getPrefixUri ($pref) {
    return $this->prefixes[$pref];
  }

  protected function uri ($prefixOrQName, $name = NULL) {
    if (isset($name)) {
      return $this->getPrefixUri($prefixOrQName) . $name;
    } 
    list($prefix, $name) = explode(':', $prefixOrQName, 2);
    return $this->getPrefixUri($prefix) . $name;
  }

  protected function translateUri ($uri, $map, $deflt = '<unknown>') {
    if (isset($map[$uri])) 
      return $map[$uri];
    else
      return $deflt;
  }

  /**
   * Get SPARQL PREFIX statements for the given array of prefixes
   */
  protected function usePrefixes ($prefixes) {
    $str = "";
    foreach ($prefixes as $pref) {
      $uri = $this->getPrefixUri($pref);
      if (!empty($uri)) {
        $str .= "PREFIX $pref: <" . $uri . ">\n";
      }
    }
    return $str;
  }

  /**
   * helper method for CONSTRUCT'ing the { s, p, o } graph where
   * subject is the given $uri.
   */
  protected function construct1 ($store, $uri, $graphs = array(), $prefixes = NULL) {
    if (!isset($store)) {
      return array();
    }
    $qs = '';
    if (isset($prefixes)) {
      $qs = $this->usePrefixes($prefixes);
    }
    if (!empty($graphs)) {
      if (!is_array($graphs)) {
        $graphs = array($graphs);
      }
    }
    $qs .= "CONSTRUCT { <" . $uri . "> ?p ?o }\n";
    foreach ($graphs as $graph) {
      $qs .= "FROM <" . $graph . ">\n";
    }
    $qs .= "WHERE { <" . $uri . "> ?p ?o }\n";
    // drupal_set_message("SPARQL: \n" . drupal_urlencode($qs));
    return $store->query($qs);
  }

  /**
   * helper method for SELECT'ing the two columns [prop, obj] for a
   * given subj uri.
   */
  protected function select2 ($store, $uri, $graphs = array(), $prefixes = NULL) {
    if (!isset($store)) {
      return array();
    }
    $qs = '';
    if (isset($prefixes)) {
      $qs = $this->usePrefixes($prefixes);
    }
    if (!empty($graphs)) {
      if (!is_array($graphs)) {
        $graphs = array($graphs);
      }
    }
    $qs .= "SELECT ?p ?o\n";
    foreach ($graphs as $graph) {
      $qs .= "FROM <" . $graph . ">\n";
    }
    $qs .= "WHERE { <" .$uri . "> ?p ?o }\n";
    return $store->query($qs);
  }

  /**
   * set node fields based on the ARC2 Resource Index $ridx returned
   * from a query.
   */
  protected function mapToNode (&$node, $uri, $ridx, $pkey) {
    $preds = $ridx[$uri];
    if (isset($preds)) {
      foreach ($pkey as $prop => $fieldspec) {
        $propUri = $this->uri($prop);
        $field = $fieldspec[0];
        $fieldJoin = $fieldspec[1];
        $fieldTrans = isset($fieldspec[2]) ? $fieldspec[2] : NULL;
        if (isset($preds[$propUri])) {
          $objs = $preds[$propUri];
          $val = array();
          foreach ($objs as $objval) {
            if (isset($fieldTrans)) {
              $val[] = $this->$fieldTrans($objval['val']);
            } else {
              $val[] = $objval['val'];
            }
          }
          switch ($fieldJoin) {
            case 'first':
              $node->$field = $val[0];
              break;
            case 'implode':
              $node->$field = implode(', ', $val);
              break;
            default:
              $node->$field = $val;
          }
        }
      }
    }
  }

  protected function getSparqlUrl () {
    return variable_get($this->moduleName . '_sparql_url', NULL);
  }

  protected function getStore () {
    $url = $this->getSparqlUrl();
    if (!empty($url)) {
      $config = array('endpoint_url' => $url);
      return ARC2::getComponent('RemoteEndpointPlugin', $config);
    }
    drupal_set_message("Configuration error: remote endpoint url is empty for module " . $this->moduleName, 'error');
    return NULL;
  }

  /****************************************************************************
   * @see hook_nodeapi() where $op = 'view'
   ****************************************************************************/
  public function oldCrapForTesting (&$node) {
    $node->content['rdfnodeproxy_uri'] = array(
      '#value' => "<h3>Node URI = '" . $node->rdfnodeproxy_uri . "'</h3>",
      '#weight' => -2
    );
    
    // $client = new SimpleSparqlClient("http://localhost:8080/openrdf-sesame/repositories/scfnat/");
    $client = new SimpleSparqlClient("http://sparql.neurocommons.org:8890/sparql/");
    /*
      $qs = "PREFIX sc: <http://purl.org/science/owl/sciencecommons/>\n";
      $qs .= "SELECT ?s ?p ?o\n";
      $qs .= "FROM <http://purl.org/commons/hcls/gene>\n";
      $qs .= "WHERE { ?s rdf:type sc:gene_record . ?s ?p ?o }\n";
      $qs .= "LIMIT 10";
    */
    $qs = "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n";
    $qs .= "PREFIX sc: <http://purl.org/science/owl/sciencecommons/>\n";
    $qs .= "PREFIX dc: <http://purl.org/dc/elements/1.1/>\n";
    $qs .= "CONSTRUCT {\n";
    $qs .= "  ?s ?pl ?lit .\n";
    // $qs .= "  ?s ?pr ?res .\n";
    $qs .= "}\n";
    $qs .= "FROM <http://purl.org/commons/hcls/gene>\n";
    $qs .= "WHERE {\n";
    $qs .= "  ?s ?pl ?lit .\n";
    $qs .= "  ?s a sc:gene_record .\n";
    $qs .= "  ?s dc:identifier ?id .\n";
    $qs .= "  filter(isLiteral(?lit)) .\n";
    $qs .= "  filter(xsd:integer(?id) > 11286 && xsd:integer(?id) < 11299) .\n";
    /*
      $qs .= "  optional {\n";
      $qs .= "    ?s ?pr ?res .\n";
      $qs .= "    ?res ?p2 ?o2 .\n";
      $qs .= "   }\n";
    */
    $qs .= "}\n";
    $qs .= "LIMIT 1";
    
    $result = $client->query($qs);
    
    dvm($result);
    // arc2_include();
    // $parser = arc2_parse_sparql_results($result);
    $parser = ARC2::getTurtleParser();
    $parser->parse('', $result);
    
    $triples = $parser->getTriples();
    /* 
      $structure = $parser->getStructure();
    */
    
    $node->content['rdfnodeproxy_results'] = array(
      '#value' => dpr($triples, TRUE),
      '#weight' => -1
    );
    
    /*
      $cols = $client->cols($result);
      $rows = $client->rows($result, TRUE);
      // $tree = $client->tree($result);
      $node->content['rdfnodeproxy_results'] = array(
        '#value' => theme('table', $cols, $rows),
        '#weight' => -1
      );
    */
  }

  // --------------------------------------- utility

  // --------------------------------------- BOILERPLATE SINGLETON CODE

  private static $INSTANCE = NULL;

  // boilerplate: could move to superclass but would then 
  // need a map of instances
  public static function getInstance () {
    if (self::$INSTANCE === NULL) {
      self::$INSTANCE = new self;
    }
    return self::$INSTANCE;
  }


}

/**
 * Handy method to return the singleton instance.
 */
function rdfnodeproxy () {
  return RdfNodeProxyModule::getInstance();
}

