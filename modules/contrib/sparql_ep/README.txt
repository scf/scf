README.txt
==========

The RDF SPARQL Endpoint module indexes the RDF data publicly available on a Drupal site into an ARC2 RDF store. It also provides a SPARQL endpoint via the ARC2 SPARQL interface.

Because it relies on the ARC2 library, this module requires MySQL and will not work on other databases. The SPARQL module at http://drupal.org/project/sparql should be used instead.

RDF SPARQL Endpoint requires the modules:
* RDF - http://drupal.org/project/rdf (with the ARC2 library installed)
* (recommended) RDF CCK - http://drupal.org/project/rdfcck

To install RDF SPARQL Endpoint:
1. place the entire sparql_ep folder into your modules directory.
2. Go to Administer -> Site building -> Modules and enable the RDF SPARQL Endpoint module.
3. Make sure the RDF data is publicly available at node/*/rdf (check the permission 'access RDF data' for anonymous user).
4. Click on the "Build RDF index" link (this can take some time depending on how many nodes and fields you have on your site).
5. Click on the "SPARQL endpoint" link to access the SPARQL endpoint interface.

Note this module is not yet optimized for big sites, and the indexing can take some time. For reference, indexing 300 nodes resulting 1650 triples takes about 1min 30s.

The ARC2 library will create its own tables in the Drupal database (sparql_ep_arc2_*).

AUTHOR/MAINTAINER
======================
scor (Stéphane Corlosquet) http://drupal.org/user/52142
