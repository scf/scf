DESCRIPTION
-----------

This module allows you to generate printer friendly versions of any node by
navigating to www.example.com/print/nid, where nid is the node id of content
to render.

A link is inserted in the each node (configurable in the content type
settings), that opens a version of the page with no sidebars, search boxes,
navigation pages, etc.

CONFIGURATION
-------------

- There are several settings that can be configured in the following places:

  Administer › Site building › Modules (admin/build/modules)
    Enable or disable the module. (default: disabled)

  Administer › User management › Access control (admin/user/access)
    Under print module:
    access print: Enable access to the PF page and display of the PF link in
    other pages. (default: disabled)
    administer print: Enable access to the module settings page. (default:
    disabled)

  Administer › Content management › Content types (admin/content/types)
    For each content type it is possible to enable or disable the PF link
    via the "Show printer-friendly version link" checkbox. (default:
    enabled)

  Administer › Content management › Comments › Settings (admin/content/comment/settings)
    It is possible to enabled or disable the PF link in individual comments
    via the "Show printer-friendly version link in individual comments"
    checkbox. (default: disabled)

  Administer › Site configuration › Printer-friendly (admin/settings/print)
    This is where all the module-specific configuration options can be set.

- To modify the template of printer friendly pages, simply edit the
print.tpl.php or the print.css files found in this directory.

- It is possible to set per-content-type and/or theme-specific templates
  which are searched for in the following order: 
   1. print.node-__type__.tpl.php in the theme directory
   2. print.node-__type__.tpl.php in the module directory 
   3. print.tpl.php in the theme directory
   4. print.tpl.php in the module directory (supplied by the module)

API
---

The following function is available to content developers that prefer to
place the printer-friendly link in a custom location. It is advisable to
disable the regular Printer-friendly link so that it is not shown in both
locations.

Calling the function like this:

  print_insert_link()

will return the HTML for a link pointing to a Printer-friendly version of
the current page.

It is also possible to specify the link to the page yourself:

  print_insert_link("print/42")

will return the HTML pointing to the printer-friendly version of node 42.


THEMEABLE FUNCTIONS
-------------------

The following themeable functions are defined:

  theme_print_format_link()
  Returns an array of formatted attributes for the Printer-friendly link.

  theme_print_text
  Returns an array of costumized text strings used in the printer-friendly
  page.

MORE INFORMATION
----------------

For more information, consult the modules' documentation at
http://drupal.org/node/190171.

ACKNOWLEDGMENTS
---------------
The print, pdf and mail icons are copyright Plone Foundation. Thanks for
letting me use them!
