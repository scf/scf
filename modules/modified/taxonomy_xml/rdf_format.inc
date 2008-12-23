<?php
/* double-commented to avoid conflict with svn
// $Id: rdf_format.inc,v 1.1.4.9 2008/10/30 04:44:59 dman Exp $ 
*/

/**
 * @file Include routines for RDF parsing and taxonomy/term creation.
 */

define('TAXONOMY_XML_RDF_NS', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
define('TAXONOMY_XML_TYPE', TAXONOMY_XML_RDF_NS .'type');
define('TAXONOMY_XML_UNTYPED', 'UNTYPED');

define('TAXONOMY_XML_RDFS_NS', 'http://www.w3.org/2000/01/rdf-schema#');

// See  http://www.w3.org/2004/12/q/doc/rdf-labels.html
define('TAXONOMY_XML_CONTENTLABEL_NS', 'http://www.w3.org/2004/12/q/contentlabel#');
define('TAXONOMY_XML_CATEGORY', TAXONOMY_XML_CONTENTLABEL_NS .'Category');

// OWL - Web Ontology Language - Formalized Meaning and Logic
define('TAXONOMY_XML_OWL_NS', 'http://www.w3.org/2002/07/owl#');

define('TAXONOMY_XML_W3C_WN', 'http://www.w3.org/2006/03/wn/wn20/');
define('TAXONOMY_XML_W3C_WN_SCHEMA', TAXONOMY_XML_W3C_WN .'schema/');

// Dublin Core - Metadata standards
define('TAXONOMY_XML_DC_NS', 'http://purl.org/dc/elements/1.1/');
// Simple Knowledge Organization System - Structural information management 
define('TAXONOMY_XML_SKOS_NS', 'http://www.w3.org/2004/02/skos/core#');
// Taxonomic Database Working Group - Biodiversity Information Standards (LSIDs etc)
define('TAXONOMY_XML_TDWG_NS', 'http://rs.tdwg.org/ontology/voc/Collection#');

/**
 * Read in RDF taxonomies and vocabularies. Create vocabs and terms as needed.
 * 
 * See formats.html readme for information about the RDF input supported.
 * 
 * Targets include :
 *   ICRA      Content Rating  http://www.icra.org/vocabulary/ 
 *   WordNet   Lexicon http: //wordnet.princeton.edu/ 
 *   SUMO   http://www. ontologyportal.org/
 * 
 * ... and the ontologies found at http://www.schemaweb.info/ that implement
 * appropriate parts of the RDF Schema "rdfs" (eg Classes with subclassOf)
 * 
 * @param $data the string containing XML/RDF
 * @param $vid int Vocab ID. May be modified by ref if this process creates a
 * new vocab to use.
 * @param $url optional source URL this RDF came from if needed to resolve GUIDs
 * etc. Cannot work for uploads.
 */
function taxonomy_xml_rdf_parse(&$data, &$vid, $url = NULL) {
  drupal_set_message(t("Parsing RDF"));

  // Use ARC parser
  include_once("arc/ARC_rdfxml_parser.php"); 
  $parser_args=array(
    "bnode_prefix" => "genid",
    "base" => "",
  );
  $parser = new ARC_rdfxml_parser($parser_args);
  $triples = $parser->parse_data($data);
  if (! is_array($triples)) {
    drupal_set_message(t("Problem parsing input %message", array('%message' => $triples)), 'error');
    return;
  }

  drupal_set_message(t("%count data triples (atomic statements) found in the source RDF doc", array('%count' => count($triples))));
  # dpm($triples);

  // The RDF input may come in several flavours, 
  // Resources of the following 'types' may be cast into taxonomy terms for our purposes.
  // That is, an rdf:Class is a Drupal:term
  //
  // Add to this list as needed
  //
  $term_types = array(
   TAXONOMY_XML_RDF_NS .'Property',
   TAXONOMY_XML_DC_NS .'subject',
   TAXONOMY_XML_RDFS_NS .'Class',
   TAXONOMY_XML_W3C_WN_SCHEMA .'Word',
   TAXONOMY_XML_W3C_WN_SCHEMA .'NounWordSense',
   TAXONOMY_XML_W3C_WN_SCHEMA .'NounSynset',
   TAXONOMY_XML_CONTENTLABEL_NS .'Category',
   TAXONOMY_XML_SKOS_NS .'Concept',
   'urn:lsid:ubio.org:classificationbank',
  );

  // A Drupal 'vocabulary' is represented by an owl:Ontology
  // or other similar shaped constructs
  $vocabulary_types = array(
    TAXONOMY_XML_OWL_NS .'Ontology',
    TAXONOMY_XML_RDF_NS .'Description',
    'http://www.w3.org/2001/12/Glossary',
    TAXONOMY_XML_TDWG_NS .'Collection',
  );

  $resources_by_type = taxonomy_xml_convert_triples_to_sorted_objects($triples);

  # dpm($resources_by_type);

  // The resources are all initialized as data objects.
  // Resource types we expect to be dealing with are just vocabs and terms. 
  drupal_set_message(t("Found %count different <strong>kinds</strong> of resources in the input : %types", array('%count' => count($resources_by_type), '%types' => join(', ', array_keys($resources_by_type)))));
  #dpm($resources_by_type);
  
  if ($vid == 0) {
    // We've been asked to use the vocab described in the source file.
    // If the vid has already been set, we ignore vocab definitions found in the file
    
    // Scan the sorted objects for vocabulary definitions
    // Hopefully there's only one vocab per file, but loop anyway
    $vocabularies = array();
    foreach ($vocabulary_types as $vocabulary_type) {
      if (isset($resources_by_type[$vocabulary_type]) && is_array($resources_by_type[$vocabulary_type])) {
        foreach ($resources_by_type[$vocabulary_type] as $uri => &$vocabulary_handle) {
          $vocabularies[$uri] = &$vocabulary_handle;
        }
      }
    }
    drupal_set_message(t("Found %count resources to be used as vocabulary definitions", array('%count' => count($vocabularies))));

    if (! $vocabularies) {
      // Create a placeholder.
      $vocabularies[] = array('name' => 'Imported Vocabulary');
    }
    $vid = taxonomy_xml_absorb_vocabulary_definitions($vocabularies);
    // $vocabularies now contains a keyed array of target vocabularies the terms may be put into
    // $vid is the default one (most common is one vocab per input file) to be used unless otherwise defined per-term.
  }
  else {
  // Else using a form-selected vocob.
    $vocabularies[$vid] = taxonomy_vocabulary_load($vid);
  }

  #dpm(array('vocabs are' => $vocabularies));  

  // Gather the resources that will become terms.
  // Slightly long way (not using array_merge), as I need to merge indexed and by reference
  $terms = array();
  foreach ($term_types as $term_type) {
    if (isset($resources_by_type[$term_type]) && is_array($resources_by_type[$term_type])) {
      foreach ($resources_by_type[$term_type] as $uri => &$term_handle) {
        // Grab name/label early for debugging and indexing
        $predicates = $term_handle->predicates;
        if (isset($predicates['label'])) {
          $term_handle->name = $predicates['label'][0];
        }
        $terms[$uri] = &$term_handle;
      }
    }
  }

  // Some of the RDF documents I've been fed DO NOT DEFINE A TYPE for their primary subject.
  // Neither 
  // http://www.ubio.org/authority/metadata.php nor 
  // http://biocol.org/ nor 
  // http://lsid.tdwg.org/
  // return RDF that says WHAT the data is. Those that use LSIDs have a type encoded in the Identifier itself :-/
  
  // I end up with a collection of data but no idea what it's really talking about.
  // But IF an entity is rdf:about="THIS URL" then we will take a leap and assume that is our target lump of data.
  // ... this worked for biocol input
  foreach ( (array)$resources_by_type[TAXONOMY_XML_UNTYPED] as $identifier => $untyped_lump) {
    if ($identifier == $url) {
      // Looks like this was the specific thing we were looking for
      $terms[$identifier] = $untyped_lump;
      # dpm(array("The default 'HERE' entity is " => $untyped_lump));
    }
  } 
  
  drupal_set_message(t("Found %count resources to be imported as terms into vocabulary %vid", array('%count' => count($terms), '%vid' => $vid)));

  // $predicate_synonyms is a translation array to match rdf-speak with Drupal concepts
  $predicate_synonyms = taxonomy_xml_relationship_synonyms();

  //
  // START MAKING TERMS
  //
  foreach ($terms as $identifier => &$term) {
    drupal_set_message(t("Reviewing term %identifier '%name' and analyzing its properties", array('%identifier' => $identifier, '%name' => $term->name)));

    if (!isset($term->vid)) { 
      // This is just a default fallback. Imported terms should really have already chosen their vid.
      $term->vid = $vid; 
    }
    // When running in batch, children will have a hard time finding their 
    // parents if they only know them by source-localized ID (probably a URI) 
    // and the destination-taxonomy (here) HASN'T REMEMBERED THAT INFO. 
    // Because taxonomy.module just doesn't. 
    // We require some other module (taxonomy_enhancer is good) to save that 
    // metadata for us so the child can find its target later.
    // This is our 'identifier' - the REMOTE identifier not the local one.
    if (!isset($term->uri)) { 
      $term->uri = $identifier; 
    }

    #dpm($term);

    // Build term from data
    
    // Convert all input predicates into attributes on the object 
    // the taxonomy.module will understand
    taxonomy_xml_canonicize_predicates($term) ;

    // Ensure name is valid    
    if (! $term->name) {
      
      // Look, if we don't even have a name, creating a term is a waste of time.
      // RDF feeds commonly consist of a bunch of pointers, we can't invent placeholders until we know a little more.
      // Let's not do this.
      drupal_set_message(t("Not enough information yet to create a term referred to as %identifier. Not creating it yet.", array('%identifier' => $identifier)));
      continue;
      /*
      // Fallback to a name, identifier derived (roughly) from the URI identifier - not always meaningful, but all we have in some contexts.
      $term->name = basename($identifier);
      drupal_set_message(t("Problem, we were unable to find a specific label for the term referred to as %identifier. Guessing that %name will be good enough.", array('%identifier' => $identifier, '%name' => $term->name)));
      */
    }

    // See if a definition already exists in the DB. Build on that.
    $existing_term = _taxonomy_xml_get_term_placeholder($term->name, $vid);
    // Merge the old term objects properties into this one. Really just want its tid, but there may be more info I should not lose.
    // New input takes precedence over older data
    foreach ((array)$existing_term as $key => $value) { 
      if (! isset($term->$key)) {
        $term->$key = $value; 
      }
    }
    // The term object is now as tidy as it can be as a self-contained entity.
    # dpm($term);    

    if (variable_get('taxonomy_xml_reuseids', FALSE)) {
      // MAINTAIN IDS
      // Because this is likely to be used with a site-cloning set-up, it would help if we tried to match IDs
      // OTOH, doing so could be very messy for other situations.
      // So, 
      //  iff there is no pre-existing term with this id, 
      //  create this one as a clone with the old ID. 
      // This requires a little DB sneakiness.
      if ($term->internal_id && ! taxonomy_get_term($term->internal_id)) {
        $term->tid = $term->internal_id;
        drupal_set_message(t("Doing sneaky import of %term_name re-using the internal id = %term_id", array('%term_name' => $term->name, '%term_id' => $term->internal_id)));
        db_query("INSERT INTO {term_data} (tid, name, description, vid, weight) VALUES (%d, '%s', '%s', %d, %d)", $term->tid, $term->name, $term->description, $term->vid, $term->weight);
  
        # sequences is gone in D6. Will inserting beyond the auto-increment self-correct?
        $current_id = db_last_insert_id('term_data', 'tid');
        if ($current_id < $term->tid) {
          // This is probably now MYSQL specific.
          db_query("ALTER TABLE {term_data} AUTO_INCREMENT = %d;", $term->tid);
        }
        
      }
    }

    # Here's where last-minute data storage done by other modules gets set up
    module_invoke_all('taxonomy_term_presave', $term);

    #dpm(array("ready to save" => $term));
    $save_term = (array)$term;
    $status = taxonomy_save_term($save_term);

    // Re-retrieve the new term definition, just in case anything extra happened to it during processing
    $new_term = taxonomy_xml_get_term_by_name_from_vocab($term->name, $term->vid);
    if (! $new_term) {
      drupal_set_message(t("It seems like we failed to create and retrieve a term called %term_name", array('%term_name' => $term->name)), 'error');
    }
    // Merge retrieved values back over our main definition so the handles are up-to-date
    foreach ((array)$new_term as $key => $value) { 
      $term->$key = $value; 
    }

    if ( $status == SAVED_NEW ) {
      // Just remember this is fresh - for useful feedback messages.
      $term->taxonomy_xml_new_term = TRUE;
    }

    // It's possible that not all the referenced items were available in the current document/loop
    // Add referred items to the import queue for later processing
    taxonomy_xml_add_all_children_to_queue($term);
    $term->taxonomy_xml_presaved = TRUE; // A flag to avoid double-processing
  } // end term-construction loop;

  #dpm(array("Saved all, now linking!" => $terms));
  
  // Now the terms are all happily created, create their relationships
  // Couldn't do so until they had all been given tids.
  taxonomy_xml_set_term_relations($terms);

  #dpm(array('After re-linking, we now have all terms set' => $terms));

  return $terms;
}

/**
 * Compile triple statements into information objects again.
 * 
 * Returns a nested array, Indexed on their URI/id, and grouped by type
 * (references so we can change them).
 * 
 * Not all RDF data objects declare exactly what they are, some just announce
 * that they exist.
 * Some guesswork is done if their identifier is an LSID - we can deduce
 * what type of object it refers to. An explicit RDF:type will take priority
 * over this assumption.
 */
function taxonomy_xml_convert_triples_to_sorted_objects(&$triples) {  
  // Triples are boringly granular bits of information.
  // Merge them.
  $resources = array();
  $resources_by_type = array();
  foreach ($triples as $triplenum => $statement) {
    $subject_uri = $statement['s']['uri'];
    if (! isset($resources[$subject_uri]) ) {
      // Create placeholder if this is the first occurance
      $resources[$subject_uri] = (object)array(); 
    }
    $subject = &$resources[$subject_uri];

    # dpm(array("Processing a statement about $subject_uri" => $statement));
    
    switch ($statement['o']['type']) {
      case 'uri' :
        $object_uri = $statement['o']['uri'];

        // Also make a placeholder for the object, for convenience
        // It's not much fun referring to something that doesn't exist.
        if (! isset($resources[$object_uri]) ) {
          $resources[$object_uri] = (object)array(); 
        }

        $object_val = $object_uri;
      break;
      default :
        $object_val = trim($statement['o']['val']);
    }
    
    // Placeholders ready, now add this statements info
    // Namespaces are boring, Simplify the predicates
    // TODO - revisit if namespaces are needed
    $predicate = taxonomy_xml_rdf_shortname($statement['p']);

    if (! isset($subject->predicates[$predicate]) ) {
      $subject->predicates[$predicate] = array();
    }

    // Some properties can be collated, listed 
    // Some need to be merged or selected (languages)
    // In this stage of pre-processing, we cannot select which string we need, so gather all values
    if ($statement['o']['type'] == 'literal' && ($lang = $statement['o']['lang'])) {
      $subject->predicates[$predicate][$lang] = $object_val;
    }
    else {
      // Only add uniques, Keeps clutter down
      if (! in_array($object_val, $subject->predicates[$predicate])) {
        $subject->predicates[$predicate][] = $object_val;
      }
    }
    
    if ($predicate == 'type') {
      // Very important info!
      $subject->type = $object_val;
      // Sort it! (by reference)
      $resources_by_type[$subject->type][$subject_uri] = &$subject;
    }
    if ($predicate == TAXONOMY_XML_NAME) {
      $subject->name = $object_val;
    }
    
    // This is very memory-intensive for big vocabs. Try to clean up :(
    unset($triples[$triplenum]);
  }

  // A special work-around for irregular data.
  // Scan the full array for any lost (untyped) data, 
  // Make some guesses if we can, and collect the rest into a catch-all 'untyped' list.
  $unknown_resources = array();
  foreach ($resources as $uri => &$subject) {

    // While we are looping, 
    // Make a guess at its original, internal ID
    // grabbing the last numeric bit from the id in the document
    // eg from '#vocab/1' or '#vocabulary:1' or #term33
    // Be very generic and forgiving in the format we look for
    $parts = preg_split('|[^\d]|', $uri);
    $last_num = array_pop($parts);
    if (is_numeric($last_num)) {
      $subject->internal_id = $last_num;
    }
    // Not really used much yet.

    // Anyway, check the type. If not known, 
    // This could confuse us later, make a note for analysis.
    if (! isset($subject->type)) {

      $url_parts = @parse_url($uri);
      if (isset($url_parts['host'])) {
        // looks (roughly) like a valid URI - No need to complain about legal external references.
        // It's only unresolvable ones that could be a problem.
        continue;
      }
            
      // If the identifier of this resource is an 'LSID'
      // then the type is sort of embedded in the string as the 'namespace'.
      // See if we can extract it.
      if ($lsid = taxonomy_xml_parse_lsid($uri)) {
        $resources_by_type[$lsid['type']][$uri] = &$subject;
        continue;
      }

      // Nope, it's a total UFO
      $unknown_resources[$uri] = &$subject;
    }

  }
  if ($unknown_resources) {
    // Just FYI, make a note about the quality of data found.
    // Do not complain about URLs - this is quite normal.
    drupal_set_message(t("Found %count Unsorted (untyped) resources. They are entities that are the subject of a statement, but I don't know what <em>type</em> of thing they are. Not sure what I'll do with these. They are things that have had statements made about them .. that I don't recognise. Probably just extra data found in the input and ignored. %unknown", array('%count' => count($unknown_resources), '%unknown' => join(', ', array_keys($unknown_resources))) ));
    $resources_by_type[TAXONOMY_XML_UNTYPED] = $unknown_resources;
  }
  
  return $resources_by_type;
}

/**
 * Choose a string from an array of language-tagged possibilities
 * 
 * Util func to help read complex RDF statements.
 */
function taxonomy_xml_get_literal_string($values) {
  if (! is_array($values)) return trim($values);
  // May need to choose language
  if (count($values) == 1) {
    $out = array_pop($values);
  }
  else {
    // TODO add language selector
    if ($label = $values['en']) {
      $out = $label;
    }
    else { // fine, whatever
      $out = array_pop($values);
    }
  }
  return trim($out);
}

/**
 * Return the shorthand label of a potentially long RDF URI
 * 
 * EG, for http://www.w3.org/1999/02/22-rdf-syntax-ns#Property
 * return 'Property'
 * ... for sanity
 * 
 * Also flatten LSIDs - which are used like URIs but just are NOT as useful
 * 
 */
function taxonomy_xml_rdf_shortname($uri) {

  // For LSID simplification, flatten assorted RDF-LSID-Predicates (from any authority) into their simple name
  if (($lsid = taxonomy_xml_parse_lsid($uri)) && ($lsid['namespace'] == 'predicates') ) {
    return $lsid['identifier'];
  }
    
  $parts = parse_url($uri);
  $shortname =$parts['fragment'] ? $parts['fragment'] : (
    $parts['query'] ? $parts['query'] : (
      basename($parts['path']) 
      ));
  // The proper method for guessing simple names is probably documented elsewhere.
  // ... this does the trick for now.
  return $shortname;
}

/**
 * Return an XML/RDF document representing this vocab
 * 
 * I'd like to use ARC libraries, but it doesn't appear to include an RDF
 * serializer output method, only an input parser...
 * 
 * Uses PHP DOM to create DOM document and nodes.
 * 
 * We use namespaces carefully here, although it may create wordy output if the
 * DOM is not optimizing the declarations for us. Still, best to be explicit, it
 * would seem.
 * 
 * The URI used to refer to other resources is based on the source document
 * location, eg
 * http://this.server/taxonomy_xml/{vid}/rdf#{tid}
 * 
 * Preamble should look something like:
 * 
 * <rdf:RDF xmlns:rdf ="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
 *   xmlns: rdfs="http://www.w3.org/2000/01/rdf-schema#"
 *   xmlns: owl="http://www.w3.org/2002/07/owl#"
 *
 */
function taxonomy_xml_rdf_create($vid, $parent = 0, $depth = -1, $max_depth = NULL) {
  $vocabulary = taxonomy_vocabulary_load($vid);

  $domcontainer = taxonomy_xml_rdf_document();
  $dom = $domcontainer->ownerDocument;
  #dpm(array(domcontainer => $domcontainer, dom => $dom));

  // define the vocab
  taxonomy_xml_add_vocab_as_rdf($domcontainer, $vocabulary);

  // and more details?

  // Now start adding terms. 
  // They are listed as siblings, not children of the ontology
  $tree = module_invoke('taxonomy', 'get_tree', $vid, $parent, $depth, $max_depth);
  taxonomy_xml_add_terms_as_rdf($domcontainer, $tree);

  $result = $dom->savexml();
  
  // Minor layout tweak for readability
  $result = preg_replace('|(<[^<]*/[^>]*>)|', "$1\n", $result);
  $result = preg_replace('|><|', ">\n<", $result);
  # dpm($result);
  return $result;
}

/**
 * Set up an RDF document preamble.
 * Returns a document, also sets the passed handle to the RDF node that content
 * should land in
 * 
 */
function taxonomy_xml_rdf_document() {
  $dom = new domdocument('1.0', 'UTF-8');

  $dom->appendchild($dom->createcomment(xmlentities("
    This file was created by Drupal taxonomy_xml import/export tool. 
    http://drupal.org/project/taxonomy_xml
    /* $Id: rdf_format.inc,v 1.1.4.9 2008/10/30 04:44:59 dman Exp $ */

    The RDF schema in this file is intended to follow the Working Draft
    described at http://www.w3.org/TR/wordnet-rdf/ for the notation of
    thesauri and taxonomies.
    ")
  ));
  $dom->appendchild($dom->createprocessinginstruction('xml-stylesheet', 'href="render-taxonomy-rdf.xsl" type="text/xsl"' ));

  $domcontainer = $dom->createelementns(TAXONOMY_XML_RDF_NS, 'rdf:RDF');
  #  $rdfnode->setattribute('xmlns', TAXONOMY_XML_RDFS_NS);
  $dom->appendchild($domcontainer);

  // Why can't I set more namespaces?
  // By appending a namespaced att, the extra namespaces appear at the top.
  // Otherwise the appear everywhere. There must be a better way
  $domcontainer->setattributens(TAXONOMY_XML_RDFS_NS, 'rdfs:title', "Initializing namespace in PHP is hard" );
  $domcontainer->setattributens(TAXONOMY_XML_OWL_NS, 'owl:hack', "Initializing namespace in PHP is hard" );

  // Note that one way to get namespaces to work right is by adding new 
  // elements to their context asap, not by waiting until after further bits are added.
  return $domcontainer;
}

/**
 * Create a vocabulary definition (just the def, not its terms) and insert it
 * into the given document element.
 * 
 * @param $domcontainer an XML dom document, modified by ref.
 * @param $vocabulary a vocab object
 */
function taxonomy_xml_add_vocab_as_rdf(&$domcontainer, $vocabulary) {
  $dom = $domcontainer->ownerDocument;

  // Describe the vocabulary itself
  $vocabnode = $dom->createelementns(TAXONOMY_XML_OWL_NS, 'owl:Ontology');
  $domcontainer->appendchild($vocabnode);
  
  // If this was a cannonic vocab, we would use a full URI as identifiers
  $vocabnode->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:ID', 'vocabulary-'. $vocabulary->vid );
  $vocabnode->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:about', url('taxonomy_xml/'. $vocabulary->vid .'/rdf', array( 'absolute' => TRUE) ) );

  $vocabnode->appendchild(
    $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:label', xmlentities($vocabulary->name))
  );

  if ($vocabulary->description) {
    $vocabnode->appendchild(
      $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:comment', xmlentities($vocabulary->description))
    );
  }

  $vocabnode->appendchild(
    $dom->createelementns(TAXONOMY_XML_OWL_NS, 'owl:versionInfo', xmlentities(format_date(time(), 'large')))
  );
}

/**
 * Given a list of terms, append definitions of them to the passed DOM container
 * 
 * Following w3c, SUMO and Wordnet examples (tho not any explicit instructions,
 * taxonomy terms are modelled as rdfs:Class objects structured using rdfs:
 * subClassOf statements.
 * 
 * Sample from Wordnet:
 * 
 * <Class rdf:about="http://xmlns.com/wordnet/1.6/Cat">
 *   <label>Cat  [ 1 ]</label>
 *   <comment>feline mammal usually having thick soft fur and being unable
 * to roar; domestic cats; wildcats</comment>
 *   <subClassOf>
 *     <Class rdf:about="http://xmlns.com/wordnet/1.6/Feline" />
 *   </subClassOf>
 * </Class>
 * 
 * I'm copying that syntax.
 * 
 * @param $termlist a FLAT array of all terms, internally cross-referenced to
 * each other defining the tree stucture
 */
function taxonomy_xml_add_terms_as_rdf(&$domcontainer, $termlist) {
  if (! $termlist) return;
  $dom = $domcontainer->ownerDocument;
  
  foreach ($termlist as $term) {
    module_invoke_all('taxonomy_term_load', $term);
    #dpm($term);
    $termnode = $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:Class');
    $termnode->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:ID', 'term-'. $term->tid );
    $domcontainer->appendchild($termnode);

    if ($term->uri) {
      $termnode->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:about', $term->uri );
    }
    else if ($term->field_uri) {
      $termnode->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:about', $term->field_uri[0]['#value'] );
    }

    $termnode->appendchild(
      $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:label', xmlentities($term->name))
    );
dpm(array($term->name => xmlentities($term->name)));    
    if ($term->description) {
      $termnode->appendchild(
        $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:comment', xmlentities($term->description) )
      );
    }

    $vocab_ref = $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:isDefinedBy');
    $vocab_ref->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:resource', '#vocabulary-'. $term->vid );
    $termnode->appendchild($vocab_ref);

    foreach ((array) taxonomy_get_related($term->tid) as $relatedid => $relatedterm) {
      $related_node = $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:seeAlso', xmlentities($relatedterm->name) );
      $related_node->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:resource', '#term-'. $relatedid );
      $termnode->appendchild($related_node);
    }
    
    $synonyms = taxonomy_get_synonyms($term->tid);
    // TODO - figure out the right syntax for synonym
    // I'm using 'equivalentClass' ... although that's really intended for merging different vocabs.
    foreach ((array) $synonyms as $synonymname) {
      $synonymnode = $parent_node = $dom->createelementns(TAXONOMY_XML_OWL_NS, 'owl:equivalentClass', xmlentities($synonymname) );
      $termnode->appendchild($synonymnode);
    }

    foreach ((array) $term->parents as $parentid) {
      $parentlist = array();
      if ($parentid) {
        $parentlist[$parentid] = $parent =  taxonomy_get_term($parentid);
        $parent_node = $dom->createelementns(TAXONOMY_XML_RDFS_NS, 'rdfs:subClassOf', xmlentities($parent->name));
        $parent_node->setattributens(TAXONOMY_XML_RDF_NS, 'rdf:resource', '#term-'. $parentid );
        $termnode->appendchild($parent_node);
      }
    }
    # dpm(array('adding term to rdf' => $term));
    #$termnode->appendchild($dom->createcomment(print_r($term, 1)));

    // workaround for large vocabs - extend runtime indefinately
    set_time_limit(10);
  }
  // Done all terms in list
}

