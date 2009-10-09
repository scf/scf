// $Id: README.txt,v 1.1.2.1 2009/02/08 02:04:57 scor Exp $

RDF CCK
=======

The RDF CCK module allows site administrators to map each content type, node title, node body and CCK field to an RDF term (class or property).

RDF CCK requires the modules:
* RDF - http://drupal.org/project/rdf
* CCK 2.x - http://drupal.org/project/cck
* RDF external vocabulary importer (evoc) - http://drupal.org/project/evoc

To install RDF CCK, place the entire rdfcck folder into your modules directory.
Go to Administer -> Site building -> Modules and enable the RDF CCK module.

By default, RDF CCK will create local classes and properties for all your content types and fields which will be exported at node/*/rdf.


Mappings to external vocabularies
=================================

This version 2.x of RDF CCK uses the RDF external vocabulary importer module (evoc) to map your local Drupal data model to external RDF terms. First make sure to import at least one vocabulary with the form available at evoc/import. See the evoc module documentation for more details. Common vocabularies are:

dc : http://purl.org/dc/elements/1.1/
foaf : http://xmlns.com/foaf/0.1/
sioc: http://rdfs.org/sioc/ns#

Go to Administer -> Content management -> Content types. Choose an existing content type and click on the tab "Manage RDF mappings". This page will give you an overview of the current mappings on your site. Assign the mappings to your content type and fields. Finally browse to node/{nid}/rdf where {nid} corresponds to a node of the type you just edited.


AUTHOR/MAINTAINER
======================
scor (Stéphane Corlosquet) http://drupal.org/user/52142
