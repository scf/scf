<?php

require_once drupal_get_path('module', 'rdfnodeproxy') . '/RdfNodeProxyModule.php';

class ScienceCommonsGeneProxyModule extends RdfNodeProxyModule {
  
  private $defaultGraph = "http://purl.org/commons/hcls/gene";

  private $propKey = array(
    'dc:identifier' => array('egid', 'first'),
    'dc:title' => array('names', 'implode'), // default, but will be replaced by parse of body
    'rdfs:label' => array('title', 'first'), // default, but will be replaced by parse of body
    'sc:from_species_described_by' => array('species_tid', 'first', 'translateSpeciesToTid'),
    'rdfs:comment' => array('body', 'first')
  );

  public function __construct () {
    parent::__construct("sciencecommons_geneproxy");
    $this->definePrefix("sc", "http://purl.org/science/owl/sciencecommons/");
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['admin/content/sciencecommons_geneproxy'] = array(
      'title' => 'ScienceCommons Gene Proxy',
      'description' => 'Configure ScienceCommons Gene Proxy',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('sciencecommons_geneproxy_admin_settings_form'),
      'access arguments' => array('administer node proxies')
    );
    // NOTE: don't use the actual hook_search for this, but still
    // include it in the search local tasks (I think)
    /** TODO: FINISH
    $items['search/sciencecommons_geneproxy'] = array(
      'title' => t('Entrez Gene (via ScienceCommons RDF)'),
      'description' => 'Search Entrez Gene records from the ScienceCommons SPARQL endpoint',
      'access callback' => 'user_access',
      // use the 'create ...' permission because what's the point of
      // searching if you can't create proxies from the results
      'access arguments' => array('create proxied sciencecommons gene records'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('sciencecommons_geneproxy_search_form'),
      'access arguments' => array('administer node proxies')
    );

    $items['search//%menu_tail'] = array(
      'title callback' => 'module_invoke',
      'title arguments' => array($name, 'search', 'name', TRUE),
      'page callback' => 'search_view',
      'page arguments' => array($name),
      'access callback' => '_search_menu',
      'access arguments' => array($name),
      'type' => MENU_LOCAL_TASK,
      'parent' => 'search',
      'file' => 'search.pages.inc',
    */
    return $items;
  }

  /****************************************************************************
   * @see hook_perm()
   ****************************************************************************/
  public function perm () {
    return array('create proxied sciencecommons gene records');
  }

  /****************************************************************************
   * @see hook_search()
   ****************************************************************************/
  public function search ($op = 'search', $keys = NULL) {
    if ($op == 'name') {
      if (user_access('create proxied sciencecommons gene records')) {
        return t('Entrez Gene (via ScienceCommons RDF)');
      }
    } else if ($op == 'search') {
      // no point in allowing search if you can't actually create the
      // records, so use same perm here
      if (user_access('create proxied sciencecommons gene records')) {
        $found = array();
        if ($keys == '!scftest') {
          $rows = array(
            array(
              'uri' => 'http://purl.org/commons/record/ncbi_gene/27756',
              'species' => 'http://purl.org/commons/record/taxon/10090',
              'comment' => 'Entrez gene record for mouse Lsm2 id: 27756.LSM2 homolog, U6 small nuclear RNA associated (S. cerevisiae) Synonyms: D17H6S56E-2, D17H6S56E2, Dmapl, Dmpkap, G7b, MGC13889, Sm-X5, SmX5, snRNP.'
            ),
            array(
              'uri' => 'http://purl.org/commons/record/ncbi_gene/1',
              'species' => 'http://purl.org/commons/record/taxon/9606',
              'comment' => 'Entrez gene record for human A1BG id: 1.alpha-1-B glycoprotein Synonyms: A1B, ABG, DKFZp686F0970, GAB, HYST2477.'
            ),
            array(
              'uri' => 'http://purl.org/commons/record/ncbi_gene/362845',
              'species' => 'http://purl.org/commons/record/taxon/10116',
              'comment' => 'Entrez gene record for rat LOC362845 id: 362845.similar to zinc finger protein 709 Synonyms: .'
            )
          );
        } else {
          $store = $this->getStore();
          $qs = $this->usePrefixes(array("sc", "dc", "rdfs"));
          $keys = check_plain($keys);
          $filter = '';
          $distinct = '';
          if (!empty($keys)) {
            $filter = "  ?uri dc:title ?title . FILTER regex(?title, '(?i)$keys') .\n";
            $distinct = 'DISTINCT';
          }
          $qs .= "SELECT $distinct ?uri ?species ?comment\n";
          $qs .= "FROM <" . $this->defaultGraph . ">\n";
          $qs .= "WHERE {\n";
          $qs .= "  ?uri a sc:gene_record .\n";
          $qs .= "  ?uri sc:from_species_described_by ?species .\n";
          $qs .= "  ?uri rdfs:comment ?comment .\n";
          $qs .= $filter;
          $qs .= "}\n";
          $qs .= "ORDER BY ASC(?comment)\n";
          $qs .= "LIMIT 100";
          
          drupal_set_message("SPARQL: <br/>\n<textarea>" . $qs . "</textarea>", 'info');
          
          $rows = $store->query($qs, 'rows');
        }
        if (!empty($rows)) {
          $species_found = array();
          foreach ($rows as $row) {
            if (isset($row['species'])) {
              $species_found[$row['species']] = 1;
            }
          }          
          $species_key = array();
          $vid = $this->getSpeciesVocabId();
          if ($vid) {
            $species_key = taxonomy_bulk_get_synonym_map($vid, array_keys($species_found));
            // dvm($species_key);
          }
          foreach ($rows as $row) {
            $node = (object) array();
            if ($vid) {
              if ($term = $this->translateUri($row['species'], $species_key, NULL)) {
                $node->species_link = l($term['name'], 'taxonomy/term/' . $term['tid']);
              }
            }
            $this->parseCommentIntoNode($node, $row['comment']);
            if (isset($node->title)) {
              $found[] = array(
                'title' => $node->title,
                'type' => 'gene',
                'species_link' => @$node->species_link,
                'link' => url('nodeproxy/get/gene', array('query' => array('sc_uri' => $row['uri']))),
                'extra' => array(
                  'symbol' => $node->symbol,
                  'names' => $node->names
                )
              );
            }
          }
        }
        // dvm($found);
        return $found;
      }
    }
  }

  // --------------------------------------- template methods

  protected function translateSpeciesToTid ($uri) {
    if ($vid = $this->getSpeciesVocabId()) {
      $key = taxonomy_bulk_get_synonym_map($vid, $uri);
      $term = $this->translateUri($uri, $key, NULL);
      if (isset($term) && is_array($term)) 
        return $term['tid'];
    }
    // else
    return NULL;
  }

  protected function parseCommentIntoNode (&$node, $comment) {
    $matches = array();
    if (preg_match('/^Entrez gene record for (mouse|human|rat) (.+) id: \d+\.(.+) Synonyms: (.*)\.$/', $comment, $matches)) {
      $node->symbol = $matches[2];
      $node->title = $matches[3];
      if (isset($node->species)) {
        $node->title .= ' [' . $node->species . ']';
      }
      $node->names = $matches[4];
    }
  }

  /**
   * @returns TRUE IFF fields were actually updated (or at least
   * confirmed to have not changed from previous update).  In other
   * words, return a diagnostic string if you e.g. can't contact
   * remote source
   */
  protected function updateNodeFields (&$node, $extid, $info) {
    if ($info->idtype == 'sc_uri') {
      $store = $this->getStore();
      $res = $this->construct1($store, $extid, $this->defaultGraph);
      if (isset($res['result']) && $res['result']) {
        //dvm($res['result']);
        $this->mapToNode($node, $extid, $res['result'], $this->propKey);
        $this->parseCommentIntoNode($node, $node->body);
        if (($vid = $this->getSpeciesVocabId()) && isset($node->species_tid)) {
          /*
          $term = (object) array(
            'tid' => $node->species_tid,
            'vid' => $vid
          );
          $node->taxonomy[$vid] = $term;
          */
          $node->taxonomy[$vid] = $node->species_tid;
        }
        return TRUE;
      } else {
        return "could not reach server";
      }
    } 
    // else
    return "required remote ID type 'sc_uri' not supplied";
  }

  // --------------------------------------- menu callbacks

  // --------------------------------------- utility

  protected function getSpeciesVocabId () {
    return variable_get('species_core_vocabulary', 0);
  }

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
function sciencecommons_geneproxy () {
  return ScienceCommonsGeneProxyModule::getInstance();
}

