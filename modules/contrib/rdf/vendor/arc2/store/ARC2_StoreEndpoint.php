<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 SPARQL Endpoint
author:   Benjamin Nowack
version:  2008-02-01 (Addition: check store_allow_extension_functions options)
*/

ARC2::inc('Store');

class ARC2_StoreEndpoint extends ARC2_Store {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_StoreEndpoint($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->headers = array('http' => 'HTTP/1.1 200 OK');
    $this->read_key = $this->v('endpoint_read_key', '', $this->a);
    $this->write_key = $this->v('endpoint_write_key', '', $this->a);
    $this->a['store_allow_extension_functions'] = $this->v('store_allow_extension_functions', 0, $this->a);    
    $this->result = '';
  }

  /*  */
  
  function getQueryString($mthd = '') {
    $r = '';
    if (!$mthd || ($mthd == 'post')) {
      $r = @file_get_contents('php://input');
    }
    $r = !$r ?$this->v1('QUERY_STRING', '', $_SERVER) : $r;
    return $r;
  }

  function p($name='', $mthd = '', $multi = '', $default = '') {
    $mthd = strtolower($mthd);
    if($multi){
      $qs = $this->getQueryString($mthd);
      if (preg_match_all('/\&' . $name . '=([^\&]*)/', $qs, $m)){
        foreach ($m[1] as $i => $val) {
          $m[1][$i] = stripslashes($val);
        }
        return $m[1];
      }
      return $default ? $default : array();
    }
    $args = array_merge($_GET, $_POST);
    $r = isset($args[$name]) ? $args[$name] : $default;
    return is_array($r) ? $r : stripslashes($r);
  }
  
  /*  */

  function getFeatures() {
    return $this->v1('endpoint_features', array(), $this->a);
  }

  function setHeader($k, $v) {
    $this->headers[$k] = $v;
  }
  
  function sendHeaders() {
    foreach ($this->headers as $k => $v) {
      header($v);
    }
  }
  
  function getResult() {
    return $this->result;
  }
  
  /*  */
  
  function handleRequest($auto_setup = 0) {
    if (!$this->isSetUp()) {
      if ($auto_setup) {
        $this->setUp();
        return $this->handleRequest();
      }
      else {
        $this->setHeader('http', 'HTTP/1.1 400 Bad Request');
        $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
        $this->result = 'Missing configuration or the endpoint store was not set up yet.';
      }
    }
    elseif ($img = $this->p('img')) {
      $this->handleImgRequest($img);
    }
    elseif ($q = $this->p('query')) {
      $this->checkProcesses();
      $this->handleQueryRequest($q);
    }
    else {
      $this->handleEmptyRequest();
    }
  }
  
  function go($auto_setup = 0) {
    $this->handleRequest($auto_setup);
    $this->sendHeaders();
    echo $this->getResult();
  }
  
  /*  */
  
  function handleImgRequest($img) {
    $this->setHeader('content-type', 'Content-type: image/gif');
    $imgs = array(
      'bg_body' => base64_decode('R0lGODlhAQBkAMQAAPf39/Hx8erq6vPz8/Ly8u/v7+np6fT09Ovr6/b29u3t7ejo6Pz8/Pv7+/39/fr6+vj4+P7+/vn5+f///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAAAAAAALAAAAAABAGQAAAUp4GIIiFIExHAkAAC9cAxJdG3TT67vTe//jKBQ6Cgaj5GkcpmcOJ/QZwgAOw=='),
    );
    $this->result = isset($imgs[$img]) ? $imgs[$img] : '';
  }
  
  /*  */
  
  function handleEmptyRequest() {
    $this->setHeader('content-type', 'Content-type: text/html; charset=utf-8');
    $this->result = $this->getHTMLFormDoc();
  }

  /*  */
  
  function checkProcesses() {
    
  }
  
  /*  */

  function handleQueryRequest($q) {
    ARC2::inc('SPARQLPlusParser');
    $p = & new ARC2_SPARQLPlusParser($this->a, $this);
    $p->parse($q);
    $infos = $p->getQueryInfos();
    /* errors? */
    if ($errors = $this->getErrors()) {
      $this->setHeader('http', 'HTTP/1.1 400 Bad Request');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = join("\n", $errors);
      return true;
    }
    $qt = $infos['query']['type'];
    /* wrong read key? */
    if ($this->read_key && ($this->p('key') != $this->read_key) && preg_match('/^(select|ask|construct|describe)$/', $qt)) {
      $this->setHeader('http', 'HTTP/1.1 401 Access denied');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Access denied. Missing or wrong "key" parameter.';
      return true;
    }
    /* wrong write key? */
    if ($this->write_key && ($this->p('key') != $this->write_key) && preg_match('/^(load|insert|delete|update)$/', $qt)) {
      $this->setHeader('http', 'HTTP/1.1 401 Access denied');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Access denied. Missing or wrong "key" parameter.';
      return true;
    }
    /* non-allowed query type? */
    if (!in_array($qt, $this->getFeatures())) {
      $this->setHeader('http', 'HTTP/1.1 401 Access denied');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Access denied for "' .$qt. '" query';
      return true;
    }
    /* load/insert/delete via GET */
    if (in_array($qt, array('load', 'insert', 'delete')) && isset($_GET['query'])) {
      $this->setHeader('http', 'HTTP/1.1 501 Not Implemented');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Query type "' .$qt. '" not supported via GET';
      return true;
    }
    /* unsupported query type */
    if (!in_array($qt, array('select', 'ask', 'describe', 'construct', 'load', 'insert', 'delete'))) {
      $this->setHeader('http', 'HTTP/1.1 501 Not Implemented');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Unsupported query type "' .$qt. '"';
      return true;
    }
    /* adjust infos */
    $infos = $this->adjustQueryInfos($infos);
    $t1 = ARC2::mtime();
    $r = array('result' => $this->runQuery($infos, $qt));
    $t2 = ARC2::mtime();
    $r['query_time'] = $t2 - $t1;
    /* query errors? */
    if ($errors = $this->getErrors()) {
      $this->setHeader('http', 'HTTP/1.1 400 Bad Request');
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Error: ' . join("\n", $errors);
      return true;
    }
    /* result */
    $m = 'get' . ucfirst($qt) . 'ResultDoc';
    if (method_exists($this, $m)) {
      $this->result = $this->$m($r);
    }
    else {
      $this->setHeader('content-type', 'Content-type: text/plain; charset=utf-8');
      $this->result = 'Result serializer not available, dumping raw data:' . "\n" . print_r($r, 1);
    }
  }
  
  /*  */

  function adjustQueryInfos($infos) {
    /* limit */
    if ($max_l = $this->v('endpoint_max_limit', 0, $this->a)) {
      if ($this->v('limit', $max_l + 1, $infos['query']) > $max_l) {
        $infos['query']['limit'] = $max_l;
      }
    }
    /* default-graph-uri / named-graph-uri */
    $dgs = $this->p('default-graph-uri', '', 1);
    $ngs = $this->p('named-graph-uri', '', 1);
    if (count(array_merge($dgs, $ngs))) {
      $ds = array();
      foreach ($dgs as $g) {
        $ds[] = array('graph' => $this->calcURI($g), 'named' => 0);
      }
      foreach ($ngs as $g) {
        $ds[] = array('graph' => $this->calcURI($g), 'named' => 1);
      }
      $infos['query']['dataset'] = $ds;
    }
    /* sql result format */
    if (($this->p('format') == 'sql') || ($this->p('output') == 'sql')) {
      $infos['result_format'] = 'sql';
    }
    return $infos;
  }
  
  /*  */

  function getResultFormat($formats, $default) {
    $prefs = array();
    /* arg */
    if (($v = $this->p('format')) || ($v = $this->p('output'))) {
      $prefs[] = $v;
    }
    /* accept header */
    if ($vals = explode(',', $_SERVER['HTTP_ACCEPT'])) {
      $o_vals = array();
      foreach ($vals as $val) {
        if (preg_match('/(rdf\+n3|x\-turtle|rdf\+xml|sparql\-results\+xml|sparql\-results\+json|json)/', $val, $m)) {
          $o_vals[$m[1]] = 1;
          if (preg_match('/\;q\=([0-9\.]+)/', $val, $sub_m)) {
            $o_vals[$m[1]] = 1 * $sub_m[1];
          }
        }
      }
      arsort($o_vals);
      foreach ($o_vals as $val => $prio) {
        $prefs[] = $val;
      }
    }
    /* default */
    $prefs[] = $default;
    foreach ($prefs as $pref) {
      if (isset($formats[$pref])) {
        return $formats[$pref];
      }
    }
  }

  /*  */

  function getSelectResultDoc($r) {
    $formats = array(
      'xml' => 'SPARQLXML', 'sparql-results+xml' => 'SPARQLXML', 
      'json' => 'SPARQLJSON', 'sparql-results+json' => 'SPARQLJSON',
      'php_ser' => 'PHPSER', 'sql' => 'xxSQL', 'plain' => 'Plain', 'htmltab' => 'HTMLTable',
    );
    if ($f = $this->getResultFormat($formats, 'xml')) {
      $m = 'get' . $f . 'SelectResultDoc';
      return method_exists($this, $m) ? $this->$m($r) : 'not implemented';
    }
    return '';
  }
  
  function getSPARQLXMLSelectResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/sparql-results+xml');
    $vars = $r['result']['variables'];
    $rows = $r['result']['rows'];
    $dur = $r['query_time'];
    $nl = "\n";
    /* doc */
    $r = '' .
      '<?xml version="1.0"?>' . 
      $nl . '<sparql xmlns="http://www.w3.org/2005/sparql-results#">' .
    '';
    /* head */
    $r .= $nl . '  <head>';
    $r .= $nl . '    <!-- query time: '. round($dur, 4) .' sec -->';
    if (is_array($vars)) {
      foreach ($vars as $var) {
        $r .= $nl . '    <variable name="' .$var. '"/>';
      }
    }
    $r .= $nl . '  </head>';
    /* results */
    $r .= $nl . '  <results>';
    if (is_array($rows)) {
      foreach ($rows as $row) {
        $r .= $nl . '    <result>';
        foreach ($vars as $var) {
          if (isset($row[$var])) {
            $r .= $nl . '      <binding name="' .$var. '">';
            if ($row[$var . ' type'] == 'iri') {
              $r .= $nl . '        <uri>' .htmlspecialchars($row[$var]). '</uri>';
            }
            elseif ($row[$var . ' type'] == 'bnode') {
              $r .= $nl . '        <bnode>' .substr($row[$var], 2). '</bnode>';
            }
            else {
              $dt = isset($row[$var . ' dt']) ? ' datatype="' .htmlspecialchars($row[$var . ' dt']). '"' : '';
              $lang = isset($row[$var . ' lang']) ? ' xml:lang="' .htmlspecialchars($row[$var . ' lang']). '"' : '';
              $r .= $nl . '        <literal' . $dt . $lang . '>' .htmlspecialchars($row[$var]). '</literal>';
            }
            $r .= $nl . '      </binding>';
          }
        }
        $r .= $nl . '    </result>';
      }
    }
    $r .= $nl . '  </results>';
    /* /doc */
    $r .= $nl . '</sparql>';
    return $r;
  }
  
  /*  */
  
  function getSPARQLJSONSelectResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/sparql-results+json');
    $vars = $r['result']['variables'];
    $rows = $r['result']['rows'];
    $dur = $r['query_time'];
    $nl = "\n";
    /* doc */
    $r = '{';
    /* head */
    $r .= $nl . '  "head": {';
    $r .= $nl . '    "vars": [';
    $first_var = 1;
    foreach ($vars as $var) {
      $r .= $first_var ? $nl : ',' . $nl;
      $r .= '      "' .$var. '"';
      $first_var = 0;
    }
    $r .= $nl . '    ]';
    $r .= $nl . '  },';
    /* results */
    $r .= $nl . '  "results": {';
    $r .= $nl . '    "bindings": [';
    $first_row = 1;
    foreach ($rows as $row) {
      $r .= $first_row ? $nl : ',' . $nl;
      $r .= '      {';
      $first_var = 1;
      foreach ($vars as $var) {
        if (isset($row[$var])) {
          $r .= $first_var ? $nl : ',' . $nl . $nl;
          $r .= '        "' .$var. '": {';
          if ($row[$var . ' type'] == 'iri') {
            $r .= $nl . '          "type": "uri",';
            $r .= $nl . '          "value": "' .mysql_real_escape_string($row[$var]). '"';
          }
          elseif ($row[$var . ' type'] == 'bnode') {
            $r .= $nl . '          "type": "bnode",';
            $r .= $nl . '          "value": "' . substr($row[$var], 2) . '"';
          }
          else {
            $dt = isset($row[$var . ' dt']) ? ',' . $nl .'          "datatype": "' .mysql_real_escape_string($row[$var . ' dt']). '"' : '';
            $lang = isset($row[$var . ' lang']) ? ',' . $nl .'          "xml:lang": "' .mysql_real_escape_string($row[$var . ' lang']). '"' : '';
            $type = $dt ? 'typed-literal' : 'literal';
            $val = str_replace(array("\r\n", "\r", "\n", '"', '\\\"'), array('\r\n', '\r', '\n', '\"', '\\\\\"'), $row[$var]);
            $r .= $nl . '          "type": "' . $type . '",';
            $r .= $nl . '          "value": "' . $val . '"';
            $r .= $dt . $lang;
          }
          $r .= $nl . '        }';
          $first_var = 0;
        }
      }
      $r .= $nl . '      }';
      $first_row = 0;
    }
    $r .= $nl . '    ]';
    $r .= $nl . '  }';
    /* /doc */
    $r .= $nl . '}';
    if (($v = $this->p('jsonp')) || ($v = $this->p('callback'))) {
      $r = $v . '(' . $r . ')';
    }
    return $r;
  }
  
  function getPHPSERSelectResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return serialize($r);
  }

  function getSQLSelectResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return $r['result'];
  }

  function getPlainSelectResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return print_r($r['result'], 1);
  }

  function getHTMLTableSelectResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/html');
    $vars = $r['result']['variables'];
    $rows = $r['result']['rows'];
    $dur = $r['query_time'];
    $r = $this->getHTMLDocHead();
    $r .= '
      <body>
        <table>
          ' . $this->getHTMLTableRows($rows, $vars) . '
        </table>
      </body>
      </html>
    ';
    return $r;
  }
  
  function getHTMLTableRows($rows, $vars) {
    $r = '';
    foreach ($rows as $row) {
      $hr = '';
      $rr = '';
      foreach ($vars as $var) {
        $hr .= $r ? '' : '<th>' . htmlspecialchars($var) . '</th>';
        $rr .= '<td>' . htmlspecialchars($row[$var]) . '</td>';
      }
      $r .= $hr ? '<tr>' . $hr . '</tr>' : '';
      $r .= '<tr>' . $rr . '</tr>';
    }
    return $r ? $r : '<em>No results found</em>';
  }

  /*  */
  
  function getAskResultDoc($r) {
    $formats = array(
      'xml' => 'SPARQLXML', 'sparql-results+xml' => 'SPARQLXML', 
      'json' => 'SPARQLJSON', 'sparql-results+json' => 'SPARQLJSON',
      'plain' => 'Plain',
      'php_ser' => 'PHPSER'
    );
    if ($f = $this->getResultFormat($formats, 'xml')) {
      $m = 'get' . $f . 'AskResultDoc';
      return method_exists($this, $m) ? $this->$m($r) : 'not implemented';
    }
    return '';
  }

  function getSPARQLXMLAskResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/sparql-results+xml');
    $r_val = $r['result'] ? 'true' : 'false';
    $dur = $r['query_time'];
    $nl = "\n";
    return '' .
      '<?xml version="1.0"?>' .
      $nl . '<sparql xmlns="http://www.w3.org/2005/sparql-results#">' .
      $nl . '  <head>' .
      $nl . '    <!-- query time: '. round($dur, 4) .' sec -->' .
      $nl . '  </head>' .
      $nl . '  <boolean>' .$r_val. '</boolean>' .
      $nl . '</sparql>' .
    '';
  }
  
  function getSPARQLJSONAskResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/sparql-results+json');
    $r_val = $r['result'] ? 'true' : 'false';
    $dur = $r['query_time'];
    $nl = "\n";
    $r = '' .
      $nl . '{' .
      $nl . '  "head": {' .
      $nl . '  },' .
      $nl . '  "boolean" : ' . $r_val . 
      $nl . '}' . 
    '';
    if (($v = $this->p('jsonp')) || ($v = $this->p('callback'))) {
      $r = $v . '(' . $r . ')';
    }
    return $r;
  }    
  
  function getPHPSERAskResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return serialize($r);
  }

  function getPlainAskResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return $r['result'] ? 'true' : 'false';
  }

  /*  */

  function getConstructResultDoc($r) {
    $formats = array(
      'rdfxml' => 'RDFXML', 'rdf+xml' => 'RDFXML', 
      'json' => 'RDFJSON', 'rdf+json' => 'RDFJSON',
      'turtle' => 'Turtle', 'x-turtle' => 'Turtle', 'rdf+n3' => 'Turtle',
      'php_ser' => 'PHPSER'
    );
    if ($f = $this->getResultFormat($formats, 'rdfxml')) {
      $m = 'get' . $f . 'ConstructResultDoc';
      return method_exists($this, $m) ? $this->$m($r) : 'not implemented';
    }
    return '';
  }
  
  /*  */
  
  function getRDFXMLConstructResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/rdf+xml');
    $index = $r['result'];
    $ser = ARC2::getRDFXMLSerializer($this->a);
    $dur = $r['query_time'];
    return $ser->getSerializedIndex($index) . "\n" . '<!-- query time: ' . $dur . ' -->';
  }
  
  function getTurtleConstructResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/x-turtle');
    $index = $r['result'];
    $ser = ARC2::getTurtleSerializer($this->a);
    $dur = $r['query_time'];
    return '# query time: ' . $dur . "\n" . $ser->getSerializedIndex($index);
  }
  
  function getRDFJSONConstructResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/json');
    $index = $r['result'];
    $ser = ARC2::getRDFJSONSerializer($this->a);
    $dur = $r['query_time'];
    $r = $ser->getSerializedIndex($index);
    if (($v = $this->p('jsonp')) || ($v = $this->p('callback'))) {
      $r = $v . '(' . $r . ')';
    }
    return $r;
  }

  function getPHPSERConstructResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return serialize($r);
  }
  
  /*  */
  
  function getDescribeResultDoc($r) {
    $formats = array(
      'rdfxml' => 'RDFXML', 'rdf+xml' => 'RDFXML', 
      'json' => 'RDFJSON', 'rdf+json' => 'RDFJSON',
      'turtle' => 'Turtle', 'x-turtle' => 'Turtle', 'rdf+n3' => 'Turtle',
      'php_ser' => 'PHPSER'
    );
    if ($f = $this->getResultFormat($formats, 'rdfxml')) {
      $m = 'get' . $f . 'DescribeResultDoc';
      return method_exists($this, $m) ? $this->$m($r) : 'not implemented';
    }
    return '';
  }
  
  /*  */
  
  function getRDFXMLDescribeResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/rdf+xml');
    $index = $r['result'];
    $ser = ARC2::getRDFXMLSerializer($this->a);
    $dur = $r['query_time'];
    return $ser->getSerializedIndex($index) . "\n" . '<!-- query time: ' . $dur . ' -->';
  }
  
  function getTurtleDescribeResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/x-turtle');
    $index = $r['result'];
    $ser = ARC2::getTurtleSerializer($this->a);
    $dur = $r['query_time'];
    return '# query time: ' . $dur . "\n" . $ser->getSerializedIndex($index);
  }
  
  function getRDFJSONDescribeResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: application/json');
    $index = $r['result'];
    $ser = ARC2::getRDFJSONSerializer($this->a);
    $dur = $r['query_time'];
    $r = $ser->getSerializedIndex($index);
    if (($v = $this->p('jsonp')) || ($v = $this->p('callback'))) {
      $r = $v . '(' . $r . ')';
    }
    return $r;
  }

  function getPHPSERDescribeResultDoc($r) {
    $this->setHeader('content-type', 'Content-Type: text/plain');
    return serialize($r);
  }
  
  /*  */
  
  function getHTMLFormDoc() {
    $r = '' . 
        $this->getHTMLDocHead() . '
      	<body>
          <h1>ARC SPARQL+ Endpoint (v' . ARC2::getVersion() . ')</h1>
          <div class="intro">
            <p>
              This interface implements 
              <a href="http://www.w3.org/TR/rdf-sparql-query/">SPARQL</a> and
              <a href="http://arc.semsol.org/docs/v2/sparql+">SPARQL+</a> via <a href="http://www.w3.org/TR/rdf-sparql-protocol/#query-bindings-http">HTTP Bindings</a>. 
            </p>
            <p>
              Enabled operations: ' . join(', ', $this->getFeatures()) . '
            </p>
            <p>
              Max. number of results : ' . $this->v('endpoint_max_limit', '<em>unrestricted</em>', $this->a) . '
            </p>
          </div>
          <form id="sparql-form" action="" enctype="application/x-www-form-urlencoded" method="post">
            <textarea id="query" name="query" rows="20" cols="80">
SELECT ?s ?p ?o WHERE {
  ?s ?p ?o .
}
LIMIT 10
            </textarea>
            Output format (if supported by query type): 
            <select id="output" name="output">
              <option value="">default</option>
              <option value="xml">XML</option>
              <option value="json">JSON</option>
              <option value="plain">Plain</option>
              <option value="php_ser">Serialized PHP</option>
              <option value="turtle">Turtle</option>
              <option value="rdfxml">RDF/XML</option>
              <!-- <option value="sql">SQL</option> -->
              <option value="htmltab">HTML Table</option>
            </select><br /><br />
            
            jsonp/callback (for JSON results):
            <input type="text" id="jsonp" name="jsonp" /><br /><br />
            
            API key (if required):
            <input type="text" id="key" name="key" /><br /><br />
            
            Set form method: 
            <a href="javascript:;" onclick="javascript:document.getElementById(\'sparql-form\').method=\'get\'">GET</a> 
            <a href="javascript:;" onclick="javascript:document.getElementById(\'sparql-form\').method=\'post\'">POST</a> 
            <br /><br />
            
            <input type="submit" value="Send Query" />
          </form>
      	</body>
    	</html>
    ';
    return $r;
  }
  
  function getHTMLDocHead() {
    return '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
      <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
      	<head>
      		<title>ARC SPARQL+ Endpoint</title>
      		<style type="text/css">
            body {
              font-size: 14px;
            	font-family: Trebuchet MS, Verdana, Geneva, sans-serif;
              background: #fff url(?img=bg_body) top center repeat-x;
              padding: 5px 20px 20px 20px;
              color: #666;
            }
            h1 { font-size: 1.6em; font-weight: normal; }
            a { color: #c00000; }
            th, td {
              border: 1px dotted #eee;
              padding: 2px 4px; 
            }
            
            #query { display: block; width: 80%; height: 300px; margin-bottom: 10px;}
            
      		</style>
      	</head>
    ';
  }
  
  /*  */
  
}
