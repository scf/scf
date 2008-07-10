<?php

function species_taxonomy_init ($type) {
  switch ($type) {
    case 'core':
      return species_taxonomy_init_core();
    case 'host':
      return species_taxonomy_init_host();
    case 'reactivity':
      return species_taxonomy_init_reactivity();
  }
}
  
function species_taxonomy_init_core () {
  return array(
    array(
      'name' => 'Mammalia',
      'synonyms' => array('mammals'),
      'description' => 'Mammals'
    ),
    array(
      'name' => 'Bos taurus',
      'description' => 'Bovine',
      'synonyms' => array('bovine', 'cattle', 'cow'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Canis familiaris',
      'description' => 'Dog',
      'synonyms' => array('dog'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Homo sapiens',
      'description' => 'Human',
      'synonyms' => array('human', 'man'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Mus musculus',
      'description' => 'Mouse',
      'synonyms' => array('mouse'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Pan troglodytes',
      'description' => 'Chimpanzee',
      'synonyms' => array('chimpanzee'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Rattus norvegicus',
      'description' => 'Rat',
      'synonyms' => array('rat'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Sus scrofa',
      'description' => 'Pig',
      'synonyms' => array('pig'),
      'parents' => array('Mammalia')
    ),
    array(
      'name' => 'Non-mammalian vertebrates',
      'description' => 'Non-mammalian vertebrates',
    ),
    array(
      'name' => 'Danio rerio',
      'description' => 'Danio rerio or zebrafish',
      'synonyms' => array('zebrafish'),
      'parents' => array('Non-mammalian vertebrates')
    ),
    array(
      'name' => 'Invertebrates',
      'description' => 'Invertebrates',
    ),
    array(
      'name' => 'Caenorhabditis elegans',
      'description' => 'C. elegans or nematode',
      'synonyms' => array('nematode'),
      'parents' => array('Invertebrates')
    ),
    array(
      'name' => 'Drosophila melanogaster',
      'description' => 'Drosophila melanogaster or fruit fly',
      'synonyms' => array('fruit fly'),
      'parents' => array('Invertebrates')
    ),
  );
}

function species_taxonomy_init_host () {
  return array(
    array(
      'name' => 'mouse',
      'description' => 'Mouse',
      'synonyms' => array('Mus musculus')
    ),
    array(
      'name' => 'chicken',
      'description' => 'Chicken',
      'synonyms' => array('Gallus gallus'),
    ),
    array(
      'name' => 'goat',
      'description' => 'Goat',
      'synonyms' => array('Capra hircus'),
    ),
    array(
      'name' => 'rabbit',
      'description' => 'Rabbit',
      'synonyms' => array('Oryctolagus cuniculus'),
    ),
  );
}

function species_taxonomy_init_reactivity () {
  return array(
    array(
      'name' => 'human',
      'description' => 'Human',
      'synonyms' => array('Homo sapiens')
    ),
    array(
      'name' => 'mouse',
      'description' => 'Mouse',
      'synonyms' => array('Mus musculus')
    ),
    array(
      'name' => 'rat',
      'description' => 'Rat',
      'synonyms' => array('Rattus norvegicus'),
    ),
  );
}