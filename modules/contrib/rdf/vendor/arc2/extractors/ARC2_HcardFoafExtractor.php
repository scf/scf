<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 hCard Extractor
author:   Benjamin Nowack
version:  2007-10-06
*/

ARC2::inc('MicroformatsExtractor');

class ARC2_HcardFoafExtractor extends ARC2_MicroformatsExtractor {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_HcardFoafExtractor($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->terms = array(
      /* root  */
      'vcard',
      /* skipped */
      '#agent', '#label', '#class', '#mailer', '#key', '#rev', '#sort-string',
      /* props */
      'bday', 'category', 'email', 'fn', 'logo', 'n', 'nickname', 'note', 'org', 'photo', 'role', 
      'sound', 'tel', 'title', 'tz', 'uid', 'url',
    );
    $this->a['ns']['vcard'] = 'http://www.w3.org/2001/vcard-rdf/3.0#';
    $this->a['ns']['dct'] = 'http://purl.org/dc/terms/';
    $this->a['ns']['dc'] = 'http://purl.org/dc/elements/1.1/';
  }

  /*  */
  
  function extractRDF() {
    foreach ($this->nodes as $n) {
      if (!$vals = $this->v('class m', array(), $n['a'])) continue;
      if (!in_array('vcard', $vals)) continue;
      /* hcard  */
      $t_vals = array(
        's' => $this->getResID($n, 'vcard') . '_agent',
        's_type' => $this->getResType($n),
      );
      $t = ' ?s a ?s_type . ';
      /* properties */
      foreach ($this->terms as $term) {
        $m = 'extract' . $this->camelCase($term);
        if (method_exists($this, $m)) {
          list ($t_vals, $t) = $this->$m($n, $t_vals, $t);
        }
      }
      /* result */
      $doc = $this->getFilledTemplate($t, $t_vals, $n['doc_base']);
      $this->addTs(ARC2::getTriplesFromIndex($doc));
    }
  }
  
  /*  */
  
  function getResType($n) {
    if (($sub_n_1 = $this->getSubNodeByClass($n, 'org')) && ($sub_n_2 = $this->getSubNodeByClass($n, 'fn'))) {
      if ($sub_n_1['id'] == $sub_n_2['id']) {
        return $this->a['ns']['foaf'] . 'Organization';
      }
    }
    return $this->a['ns']['foaf'] . 'Agent';
  }
  
  /*  */

  function extractSimple($n, $t_vals, $t, $cls, $prop = '') {
    if ($sub_ns = $this->getSubNodesByClass($n, $cls)) {
      $tc = 0;
      $prop = $prop ? $prop : 'vcard:' . strtoupper($cls);
      foreach ($sub_ns as $sub_n) {
        $var = $this->normalize($cls) . '_'. $tc;
        if ($t_vals[$var] = $this->getNodeContent($sub_n)) {
          $t .= '?s ' . $prop . ' ?' . $var . ' . ';
          $tc++;
        }
      }
    }
    return array($t_vals, $t);
  }

  /*  */

  function extractBday($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'bday');
  }
  
  /*  */

  function extractCategory($n, $t_vals, $t) {
    if ($sub_ns = $this->getSubNodesByClass($n, 'category')) {
      $tc = 0;
      foreach ($sub_ns as $sub_n) {
        list ($sub_t_vals, $sub_t) = $this->extractRelTagCategory($sub_n, $t_vals, $t, $tc);
        if ($sub_t != $t) {
          list ($t_vals, $t) = array($sub_t_vals, $sub_t);
        }
        else {
          list ($t_vals, $t) = $this->extractPlainCategory($sub_n, $t_vals, $t, $tc);
        }
        $tc++;
      }
    }
    return array($t_vals, $t);
  }

  function extractRelTagCategory($n, $t_vals, $t, $tc) {
    $href = $this->v('href iri', '', $n['a']);
    $rels = $this->v('rel m', array(), $n['a']);
    if ($href && in_array('tag', $rels)) {
      $parts = preg_match('/^(.*\/)([^\/]+)\/?$/', $href, $m) ? array('space' => $m[1], 'tag' => rawurldecode($m[2])) : array('space' => '', 'tag' => '');
      if ($tag = $parts['tag']) {
        $t_vals['cat_' . $tc] = $tag;
        $t .= '?s dc:subject ?cat_' . $tc . ' . ';
        //$t .= '?s vcard:CATEGORIES ?tag_' . $tc . ' . ';
      }
    }
    return array($t_vals, $t);
  }
  
  function extractPlainCategory($n, $t_vals, $t, $tc) {
    if ($tag = $this->getNodeContent($n)) {
      $t_vals['cat_' . $tc] = $tag;
      $t .= '?s dc:subject ?cat_' . $tc . ' . ';
      //$t .= '?s vcard:CATEGORIES ?tag_' . $tc . ' . ';
    }
    return array($t_vals, $t);
  }
  
  /*  */
  
  function extractEmail($n, $t_vals, $t) {
    $tc = 0;
    foreach ($sub_ns = $this->getSubNodesByClass($n, 'email') as $sub_n) {
      $val = '';
      if ($sub_sub_n = $this->getSubNodeByClass($sub_n, 'value')) {
        $val = $this->getNodeContent($sub_sub_n);
      }
      else {
        $full_val = $this->getContent($sub_n, 1);
        if (preg_match('/([a-z0-9\-\.\_]+\@[a-z0-9\.\-\_]+)/i', $full_val, $m)) {
          $val = $m[1];
        }
      }
      if ($val) {
        $val = !preg_match('/^mailto/', $val) ? 'mailto:' . $val : $val;
        $t_vals['email_' . $tc] = $val;
        $t .= '?s vcard:EMAIL ?email_' . $tc . ' . ';
        if ($sub_sub_n = $this->getSubNodeByClass($sub_n, 'type')) {
          $type = preg_match('/^([^\s]+)/', $this->getNodeContent($sub_sub_n), $m) ? $m[1] : '';
          $t .= $type ? '?email_' . $tc . ' a vcard:' . $type . ' . ' : '';
        }
        $tc++;
      }
    }
    return array($t_vals, $t);
  }

  /*  */

  function extractFn($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'fn', 'foaf:name');
  }
  
  /*  */

  function extractLogo($n, $t_vals, $t) {
    if ($sub_ns = $this->getSubNodesByClass($n, 'logo')) {
      $tc = 0;
      foreach ($sub_ns as $sub_n) {
        if ($sub_n['tag'] == 'img') {
          if ($t_vals['logo_url_' . $tc] = $this->v('src iri', '', $sub_n['a'])) {
            $t .= '?s vcard:LOGO ?logo_url_' . $tc . ' . ';
            $tc++;
          }
        }
      }
    }
    return array($t_vals, $t);
  }

  /*  */
  
  function extractN($n, $t_vals, $t) {
    if ($sub_n = $this->getSubNodeByClass($n, 'n')) {
      $tc = 0;
      $sub_terms = array(
        'honorific-prefix' => 'foaf:title',
        'given-name' => 'foaf:givenname',
        'additional-name' => '',
        'family-name' => 'foaf:family_name',
        'honorific-suffix' => 'foaf:title'
      );
      foreach ($sub_terms as $cls => $term) {
        if ($term && ($sub_sub_n = $this->getSubNodeByClass($sub_n, $cls))) {
          $t_vals['n_' . $tc] = $this->getNodeContent($sub_sub_n);
          $t .= '?s ' . $term . ' ?n_' . $tc . ' . ';
          $tc++;
        }
      }
    }
    return array($t_vals, $t);
  }

  /*  */

  function extractNickname($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'nickname', 'foaf:nick');
  }
  
  /*  */

  function extractNote($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'note', 'rdfs:comment');
  }

  /*  */

  function extractOrg($n, $t_vals, $t) {
    if ($sub_n = $this->getSubNodeByClass($n, 'org')) {
      $t_vals['org'] = $this->getResID($sub_n, 'org');
      if ($sub_sub_n = $this->getSubNodeByClass($sub_n, 'organization-name')) {
        if ($t_vals['org_name'] = $this->getNodeContent($sub_sub_n)) {
          $t .= '?s vcard:ORG ?org . ?org a foaf:Organization ; foaf:name ?org_name . ';
        }
      }
      elseif ($t_vals['org_name'] = $this->getNodeContent($sub_n)) {
        $t .= '?s vcard:ORG ?org . ?org a foaf:Organization ; foaf:name ?org_name . ';
      }
      if ($sub_sub_n = $this->getSubNodeByClass($sub_n, 'organization-unit')) {
        if ($t_vals['org_unit'] = $this->getNodeContent($sub_sub_n)) {
          $t .= '?s vcard:ORG ?org . ?org a foaf:Organization ; vcard:Orgunit ?org_unit . ';
        }
      }
    }
    return array($t_vals, $t);
  }

  /*  */

  function extractPhoto($n, $t_vals, $t) {
    if ($sub_ns = $this->getSubNodesByClass($n, 'photo')) {
      $tc = 0;
      foreach ($sub_ns as $sub_n) {
        if ($sub_n['tag'] == 'img') {
          if ($t_vals['photo_url_' . $tc] = $this->v('src iri', '', $sub_n['a'])) {
            $t .= '?s foaf:img ?photo_url_' . $tc . ' . ';
            $tc++;
          }
        }
      }
    }
    return array($t_vals, $t);
  }

  /*  */

  function extractRole($n, $t_vals, $t) {/* role / business category, e.g. Web Developer */
    return $this->extractSimple($n, $t_vals, $t, 'role');
  }

  /*  */

  function extractSound($n, $t_vals, $t) {
    if ($sub_n = $this->getSubNodeByClass($n, 'sound')) {
      if ($t_vals['sound_src'] = $this->v('src iri', '', $sub_n['a'])) {
        $t .= '?s vcard:SOUND ?sound_src . ';
      }
      if ($t_vals['sound_data'] = $this->v('data', '', $sub_n['a'])) {
        $t .= '?s vcard:SOUND ?sound_data . ';
      }
    }
    return array($t_vals, $t);
  }
  
  /*  */

  function extractTel($n, $t_vals, $t) {
    $tc = 0;
    foreach ($sub_ns = $this->getSubNodesByClass($n, 'tel') as $sub_n) {
      $val = '';
      if ($sub_sub_n = $this->getSubNodeByClass($sub_n, 'value')) {
        $val = $this->getNodeContent($sub_sub_n);
      }
      else {
        $full_val = $this->getContent($sub_n, 1);
        if (preg_match('/([0-9\-\.\+]+)/', $full_val, $m)) {
          $val = $m[1];
        }
      }
      if ($val) {
        $val = !preg_match('/^tel/', $val) ? 'tel:' . $val : $val;
        $t_vals['tel_' . $tc] = $val;
        $t .= '?s vcard:TEL ?tel_' . $tc . ' . ';
        if ($sub_sub_n = $this->getSubNodeByClass($sub_n, 'type')) {
          $type = preg_match('/^([^\s]+)/', $this->getNodeContent($sub_sub_n), $m) ? $m[1] : '';
          $t .= $type ? '?tel_' . $tc . ' rdf:type vcard:' . $type . ' . ' : '';
        }
        $tc++;
      }
    }
    return array($t_vals, $t);
  }

  /*  */

  function extractTitle($n, $t_vals, $t) {/* job title, e.g. CTO */
    return $this->extractSimple($n, $t_vals, $t, 'title');
  }

  /*  */

  function extractTz($n, $t_vals, $t) {/* e.g. -05:00 */
    return $this->extractSimple($n, $t_vals, $t, 'tz');
  }

  /*  */

  function extractUid($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'uid');
  }

  /*  */

  function extractUrl($n, $t_vals, $t) {
    if ($sub_n = $this->getSubNodeByClass($n, 'url')) {
      if ($t_vals['url'] = $this->v('href iri', '', $sub_n['a'])) {
        $t .= '?s foaf:homepage ?url . ';
      }
    }
    return array($t_vals, $t);
  }
  
  /*  */
  /*  */

  function extract_hcard_info($n) {
    if (($vals = array_intersect($this->mf_hcard_terms, $n['class_vals'])) && ($hcard_subj = $this->get_mf_subject($n['node'], array('vcard')))) {
      $node = $n['node'];
      /* props */
      foreach ($vals as $val) {
        $o = $this->get_mf_node_value($node, $val);
        /* org */
        if ($val == 'org') {
          /* in hresume.experience? */
          if (($exp_node = $this->get_mf_parent_node($node, array('experience'))) && $this->get_mf_subject($exp_node, array('hresume'))) {
            if ($history_event = $this->get_mf_child_node($exp_node, array('vevent'))) {
              $history_event_id = $this->get_mf_node_iri($history_event, array('vevent'));
              $this->add_triple(array('s' => $history_event_id, 'p' => $n['ns']['cv'].'employedIn', 'p_type' => 'obj', 'o' => $org_id));
            }
          }
        }
        /* other vals */
        else {
          /* basic vals */
          if ($o) {
            $this->add_triple(array('s' => $hcard_subj, 'p' => $n['ns']['vcard'].$val, 'p_type' => 'dt', 'o' => $o));
            /* title in hresume.experience? */
            if (($exp_node = $this->get_mf_parent_node($node, array('experience'))) && $this->get_mf_subject($exp_node, array('hresume'))) {
              if ($history_event = $this->get_mf_child_node($exp_node, array('vevent'))) {
                $history_event_id = $this->get_mf_node_iri($history_event, array('vevent'));
                $this->add_triple(array('s' => $history_event_id, 'p' => $n['ns']['cv'].'jobTitle', 'p_type' => 'dt', 'o' => $o));
              }
            }
            /* fn: add foaf:name */
            if ($val == 'fn') {
              $agent_id = $this->get_mf_hcard_agent($hcard_subj);
              $this->add_triple(array('s' => $hcard_subj, 'p' => $n['ns']['foaf'].'topic', 'p_type' => 'obj', 'o' => $agent_id));
              $this->add_triple(array('s' => $agent_id, 'p' => $n['ns']['foaf'].'name', 'p_type' => 'dt', 'o' => $o));
              $this->set_mf_agent_fn($agent_id, $o);
            }
          }
        }
      }
    }
  }
  
  
}
