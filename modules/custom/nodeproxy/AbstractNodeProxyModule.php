<?php

/**
 * Helper base class for node proxy modules
 */
abstract class AbstractNodeProxyModule {
  
  protected $moduleName;

  protected function __construct ($moduleName) {
    $this->moduleName = $moduleName;
  }

  // ------------------------------------------------------- drupal hooks

  // ------------------------------------------------------- nodeproxy hooks

  /****************************************************************************
   * nodeproxy's nodeproxy_lookup() hook
   *
   * given a node stub object &$node, do the following:
   * 
   * (1) inspect query params to see if relevant param(s) supplied.
   *
   * (2) If not, return FALSE.
   *
   * (3) If Query string contained the idtype param this module is
   * looking for, then augment the $node object accordingly if
   * necessary (to prevent having to do this check again).  Do this by
   * adding to the $node->nodeproxies[][][] array.  At this point we
   * already know that the return value will be TRUE.
   *
   * (4) perform appropriate db lookup to see if there is an existing
   * proxy record for this module/idtype/extid combo.  If YES, then
   * fill in node->nid and return TRUE
   *
   * NOTE: this method should NOT attempt to actually fill or update
   * the object's proxied field values from the remote source yet.
   *
   * @param &$node a node stub may be empty except for the type field.
   *
   * @returns TRUE iff the current request (e.g. query params)
   * contains a recognized extid which the current module could use to
   * proxy the node (assuming remote lookups are successful).  In
   * other words, does this look like something that I'll be able to
   * handle?
   *
   * TODO: figure out a way to officially support the fact that, with
   * careful ordering of proxy modules you can:
   *
   * (1) use two or more "supplemental"s to fill in enough fields to be a "primary"
   *
   * (2) use the results of an early lookup (which fills in node
   * fields) as the extid id for a later proxying (e.g. an Entrez Gene
   * ID that comes from an initial third-party RDF lookup can be used
   * to get further info from Entrez Gene itself).
   *
   *
   ****************************************************************************/
  public function proxyLookup (&$node, $info) {
    $idtype = $info->idtype;
    $extid = $this->getExtIdFromRequest($info);
    if (isset($extid)) {
      $extid = urldecode($extid);
      if (!isset($node->nodeproxies))
        $node->nodeproxies = array();
      $node->nodeproxies[$this->moduleName][$idtype] = (object) array(
        'module' => $this->moduleName,
        'idtype' => $idtype,
        'extid' => $extid,
        );
      // if the node has no nid yet, then try to determine it
      if (!isset($node->nid)) {
        $nid = $this->findProxyNodeId($extid, $info);
        if (isset($nid) && $nid) {
          $node->nid = $nid;
        }
      }
      return TRUE;
    }
    // else: this request doesn't provide enough info to use for proxying
    return FALSE;
  }

  protected function getExtIdFromRequest ($info) {
    if (isset($_GET[$info->idparam])) {
      return urldecode($_GET[$info->idparam]);
    }
    // else
    return NULL;
  }

  /**
   * return a (non-zero) node id or FALSE
   */
  protected function findProxyNodeId ($extid, $info) {
    $sql = "SELECT nid FROM {nodeproxy} WHERE module = '%s' AND idtype = '%s' AND extid = '%s'";
    // should return 0 or 1
    return db_result(db_query($sql, $this->moduleName, $info->idtype, $extid));
  }

  /****************************************************************************
   * nodeproxy's nodeproxy_sync() hook
   *
   * update field values based on retrieval of data from proxy sources
   * 
   * @returns TRUE iff some external data was sync'ed EVEN if it was
   * NOT different from previous data (this tells nodeproxy that it must
   * update the proxy records to at least reflect new updated time)
   ****************************************************************************/
  public function proxySync (&$node, $info) {
    $idtype = $info->idtype;
    $mod = $this->moduleName;
    $np = isset($node->nodeproxies[$mod][$idtype]) ? $node->nodeproxies[$mod][$idtype] : (object) array();
    $stale = TRUE;
    // can't do expire any sooner than ten sec.  Any less and you
    // start risking an infinite loop after redirect if this is a
    // blind nodeproxy/get request.
    $expires_in = max(10, $this->expiresSec()); // sec
    $now = time();
    if (isset($np->expires)) {
      if ($np->expires >= $now) {
        $stale = FALSE;
      }
    } else if (isset($np->updated) && (($np->updated + $expires_in) >= $now)) {
      $stale = FALSE;
    }
    if (isset($np->extid)) {
      $extid = $np->extid;
    } else {
      $extid = $this->getExtIdFromRequest($info);
    }
    if (empty($extid)) {
      $extid = $this->discoverExternalId($node, $info);
    }
    if ($stale && !empty($extid)) {
      // use the extid to actually get the data
      $result = $this->updateNodeFields($node, $extid, $info);
      $np->status = ($result === TRUE);
      if (!$np->status && is_string($result)) {
        $np->message = $result;
      } else {
        $now = time();
        $np->updated = $now;
        $np->expires = $now + $expires_in;
      }
      $np->extid = $extid;
      if (isset($np->nid)) {
        $np->_dirty = TRUE;
      } else {
        $np->_new = TRUE;
      }
      $node->nodeproxies[$mod][$idtype] = $np;
      // update node->nodeproxy_coverage (should end up being either
      // 'primary' or 'supplemental')
      $cov = $info->coverage;
      if (($cov == 'primary')
        || !isset($node->nodeproxy_coverage)
        || ($node->nodeproxy_coverage != 'primary')) {
        $node->nodeproxy_coverage = $cov;
      }
      return $np->status;
    }
    // else nothing to do (no proxying in this case)
    return FALSE;
  }

  /****************************************************************************
   * nodeproxy's nodeproxy_affect_node_form() hook
   ****************************************************************************/
  public function proxyAffectNodeForm (&$form, $info) {
    $form['title'] = $this->fakeFormField($form['title']);
    $form['body_field']['body'] = $this->fakeFormField($form['body_field']['body']);
    //$form['title']['#attributes']['disabled'] = 'disabled';
    //$form['title']['#disabled'] = TRUE;
    return $form;
  }

  /**
   * only really works for textfields
   */
  protected function fakeFormField ($f) {
    $f['#attributes']['style'] = 'display: none;';
    $fakefield = '<span class="fakeinput">' . $f['#default_value'] . '</span>';
    $prefix = '#prefix';
    if ($f['#type'] == "textfield") {
      $prefix = '#field_prefix';
    } 
    if (isset($f[$prefix])) {
      $f[$prefix] .= $fakefield;
    } else {
      $f[$prefix] = $fakefield;
    }
    return $f;
  }

  // ------------------------------------------------------- template methods

  /**
   * template method: use to cull an external ID from normal node fields
   *
   * Used for instance if the results of a previous proxy yield a
   * suitable external ID
   *
   * NOTE: if one is found AND if it is deemed to be fully stable with
   * respect to the node's identity, then this method should set the
   * appropriate nodeproxy_X field on the node and save a nodeproxy
   * record for this ID, so this method doesn't need to be used the
   * next time.
   *
   * @param $info is used only by modules that proxy for more than one
   * node type or id type
   */
  protected function discoverExternalId (&$node, $info) {
    return NULL;
  }
    
  /**
   * @returns TRUE IFF fields were actually updated (or at least
   * confirmed to have not changed from previous update).  In other
   * words, return FALSE if you e.g. can't contact remote source
   */
  protected function updateNodeFields (&$node, $extid, $info) {
    $node->title = "Proxy " . $node->type . " node [" . $extid . "]";
    $node->body = "If you are seeing this, then your node proxying module forgot to implement the updateNodeFields().";
    return TRUE;
  }
  
  protected function expiresSec () {
    // default to 24 hrs
    $default_expires = 60 * 60 * 24;
    return variable_get($this->moduleName . '_expires_sec', $default_expires);
  }

  // ------------------------------------------------------- utility
  
}
