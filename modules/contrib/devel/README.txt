README.txt
==========

A module containing helper functions for Drupal developers and
inquisitive admins. This module can print a log of
all database queries for each page request at the bottom of each page. The
summary includes how many times each query was executed on a page, and how long each query
 took.
 
 It also
 - a block for running custom PHP on a page
 - a block for quickly accessing devel pages
 - a block for masquerading as other users (useful for testing)
 - reports memory usage at bottom of page
 - more
 
 This module is safe to use on a production site. Just be sure to only grant
 'access development information' permission to developers.

Also a dpr() function is provided, which pretty prints arrays and strings. Useful during
development. Many other nice functions like dpm(), dvm().

You might also download Krumo from http://krumo.sourceforge.net/. Unpack it into a subdirectory 
called krumo. Devel will automatically start using it. You may also call krumo($variable) to get 
a pretty print of any variable.

- You should grab fb.php from http://www.firephp.org/ and put it in the devel directory. Then you can log 
php variables to the firebug console. Is quite useful. See http://www.firephp.org/ for usage information.

Included in this package is also: 
- devel_themer.module which outputs deep information related to all theme calls on a page.
- devel_node_access module which prints out the node_access records for a given node. Also offers hook_node_access_explain for all node access modules to implement. Handy.
- devel_generate.module which bulk creates nodes, users, comment, terms for development
- macro.module which records form submissions and can pay them back later or on another site. More
information available at http://drupal.org/node/79900.


AUTHOR/MAINTAINER
======================
-moshe weitzman
weitzman at tejasa DOT com
