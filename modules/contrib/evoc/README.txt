// $Id: README.txt,v 1.1.2.2 2009/02/08 15:02:05 scor Exp $

RDF external vocabulary importer
================================

The RDF external vocabulary importer (evoc) provides a user interface and an API to fetch external vocabularies and make them available to other modules for mapping Drupal object. This helper module is required by other modules like RDF CCK and Neologism.

Evoc requires the modules:
* RDF - http://drupal.org/project/rdf
* SPARQL - http://drupal.org/project/sparql

To install evoc, place the entire evoc folder into your modules directory.
Go to Administer -> Site building -> Modules and enable the evoc module.

Browse to the form available at evoc/import. Enter the URI of the vocabulary you want to import, and the prefix that you want to use in the system to refer to this vocabulary. Example of prefixes are dc for Dublin Core, foaf for Friend of a Friend etc. 

Upon installation, the following vocabularies are imported:
dc : http://purl.org/dc/elements/1.1/
foaf : http://xmlns.com/foaf/0.1/
sioc : http://rdfs.org/sioc/ns#

Other common vocabularies include:
skos : http://www.w3.org/2008/05/skos#
doap : http://usefulinc.com/ns/doap#
dcterms : http://purl.org/dc/terms/
dcmitype : http://purl.org/dc/dcmitype/



AUTHOR/MAINTAINER
======================
scor (Stéphane Corlosquet) http://drupal.org/user/52142
