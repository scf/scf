<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

function: format detection
author:   Benjamin Nowack
version:  2008-02-27 (Fix: xml wasn't properly detected when the opening tag was very large, added "xmlns:" as xml trigger)
*/

function ARC2_getFormat($v, $mtype = '', $ext = '') {
  $r = false;
  /* mtype check (atom, rdf/xml, turtle, n3, mp3, jpg) */
  $r = (!$r && preg_match('/\/atom\+xml/', $mtype)) ? 'atom' : $r;
  $r = (!$r && preg_match('/\/rdf\+xml/', $mtype)) ? 'rdfxml' : $r;
  $r = (!$r && preg_match('/\/(x\-)?turtle/', $mtype)) ? 'turtle' : $r;
  $r = (!$r && preg_match('/\/rdf\+n3/', $mtype)) ? 'n3' : $r;
  /* xml sniffing */
  if (!$r && preg_match('/^\s*\<[^\s]/s', $v) && (preg_match('/\<\/[a-z0-9\_\:\-]+\>/i', $v) || preg_match('/\sxmlns\:/', $v))) {
    while (preg_match('/^\s*\<\?xml[^\r\n]+\?\>\s*/s', $v)) {
      $v = preg_replace('/^\s*\<\?xml[^\r\n]+\?\>\s*/s', '', $v);
    }
    while (preg_match('/^\s*\<\!--.+?--\>\s*/s', $v)) {
      $v = preg_replace('/^\s*\<\!--.+?--\>\s*/s', '', $v);
    }
    /* doctype checks (html, rdf) */
    $r = (!$r && preg_match('/^\s*\<\!DOCTYPE\s+html[\s|\>]/is', $v)) ? 'html' : $r;
    $r = (!$r && preg_match('/^\s*\<\!DOCTYPE\s+[a-z0-9\_\-]\:RDF\s/is', $v)) ? 'rdfxml' : $r;
    /* markup checks */
    $v = preg_replace('/^\s*\<\!DOCTYPE\s.*\]\>/is', '', $v);
    $r = (!$r && preg_match('/^\s*\<rss\s+[^\>]*version/s', $v)) ? 'rss' : $r;
    $r = (!$r && preg_match('/^\s*\<feed\s+[^\>]+http\:\/\/www\.w3\.org\/2005\/Atom/s', $v)) ? 'atom' : $r;
    $r = (!$r && preg_match('/^\s*\<opml\s/s', $v)) ? 'opml' : $r;
    $r = (!$r && preg_match('/^\s*\<html[\s|\>]/is', $v)) ? 'html' : $r;
    $r = (!$r && preg_match('/^\s*\<[^\s]*RDF[\s\>]/s', $v)) ? 'rdfxml' : $r;
    $r = (!$r && preg_match('/^\s*\<[^\>]+http\:\/\/www\.w3\.org\/1999\/02\/22\-rdf/s', $v)) ? 'rdfxml' : $r;
    $r = (!$r) ? 'xml' : $r;
  }
  /* turtle/n3 */
  $r = (!$r && preg_match('/\@(prefix|base)/i', $v)) ? 'turtle' : $r;
  $r = (!$r && preg_match('/^(ttl)$/', $ext)) ? 'turtle' : $r;
  $r = (!$r && preg_match('/^(n3)$/', $ext)) ? 'n3' : $r;
  /* ntriples */
  $r = (!$r && preg_match('/^\s*(_:|<).+?\s+<[^>]+?>\s+\S.+?\s*\.\s*$/m', $v)) ? 'ntriples' : $r;
  $r = (!$r && preg_match('/^(nt)$/', $ext)) ? 'ntriples' : $r;
  return $r;
}
