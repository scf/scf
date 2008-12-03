/* $Id: README.txt,v 1.20.2.4 2008/07/14 21:44:13 sun Exp $ */

-- SUMMARY --

Drupal Administration Menu displays the whole menu tree below /admin including
most local tasks in a drop-down menu. So administrators need less time
to access pages which are only visible after one or two clicks normally.

Admin menu also provides hook_admin_menu() that allows other modules to add
menu links.

For a full description visit the project page:
  http://drupal.org/project/admin_menu
Bug reports, feature suggestions and latest developments:
  http://drupal.org/project/issues/admin_menu


-- REQUIREMENTS --

None.


-- INSTALLATION --

* Install as usual, see http://drupal.org/node/70151 for further information.


-- CONFIGURATION --

* Configure user permissions in Administer >> User management >> Permissions
  >> admin_menu module:

  - access administration menu: Displays Drupal Administration Menu.

  - display drupal links: Displays additional links to Drupal.org and issue
    queues of all enabled contrib modules in the Drupal Administration Menu icon.

  Please bear in mind that the displayed menu items in Drupal Administration Menu
  depend on the actual permissions of a user.  For example, if a user does not
  have the permission 'administer access control' and 'administer users', the
  whole 'User management' menu item will not be displayed.

* Customize module settings in Administer >> Site configuration >> Administration
  Menu.


-- CUSTOMIZATION --

* You have two options to override the admin menu icon:

  1) Disable it via CSS in your theme:
<code>
body #admin-menu-icon { display: none; }
</code>

  2) Alter the image by overriding the theme function:

     Copy the whole function theme_admin_menu_icon() into your template.php,
     rename it to f.e. phptemplate_admin_menu_icon() and customize the output
     according to your needs.

  Please bear in mind that admin_menu's output is cached. You need to clear your
  site's cache (probably best using Devel module, or by manually truncating the
  cache_menu database table) to see any changes of your theme override function.

* You can override the font size by adding a line to your stylesheet in your
  theme like the following:
<code>
body #admin-menu { font-size: 10px; }
</code>


-- TROUBLESHOOTING --

* If admin menu is not displayed, check the following steps:

  - Is the 'access administration menu' permission enabled?

  - Does your theme output $closure? (See FAQ below for more info)

* If admin menu is rendered behind a flash movie object, you need to add the
  following property to your flash object(s):
<code>
<param name="wmode" value="transparent" />
</code>
  See http://drupal.org/node/195386 for further information.


-- FAQ --

Q: When admin_menu is enabled, plenty of blank space is added to the bottom of
   my theme. Why?

A: This is caused by a long list of links to module issue queues at Drupal.org.
   Just go to Administer >> User management >> Permissions >> admin_menu and
   disable the permission "display drupal links" for your or all roles.
   Since uid 1 always has all permissions, this link list cannot be disabled
   for uid 1.

Q: After upgrading to 6.x-1.x, admin_menu disappeared. Why?

A: This should not happen. If it did, visit
   http://<yoursitename>/admin/build/modules to re-generate your menu.

Q: Can I configure admin_menu to display another (or the Navigation) menu just
   like the administration menu?

A: No. As the name implies, Drupal Administration Menu is for the administrative
   menu only. However, if you know CSS, you can simply copy'n'paste the contents
   of admin_menu.css into your theme's stylesheet and alter #admin-menu to any
   other menu block id, for example #block-menu-1 or the like.

Q: Sometimes the user counter displays a lot of anonymous users, but when
   comparing the site's statistics (f.e. in Google Analytics) there is no such
   huge amount of users and/or requests reported.

A: If your site was concurrently spidered by search-engine robots, it's commonly
   known to have a giant amount of anonymous users for a short time-frame.
   Most web statistics systems like Google Analytics filter such requests
   already.

Q: I enabled "Aggregate and compress CSS files", but I found admin_menu.css is
   still there, is it normal?

A: Yes, this is the intended behavior. Since admin_menu is only visible for
   logged-on administrative users, it would not make sense to load its
   stylesheet for all, including anonymous users.


-- CONTACT --

Current maintainers:
* Daniel F. Kudwien (sun) - dev@unleashedmind.com
* Peter Wolanin (pwolanin) - http://drupal.org/user/49851
* Stefan M. Kudwien (smk-ka) - dev@unleashedmind.com

Major rewrite for Drupal 6 by Peter Wolanin (pwolanin).

This project has been sponsored by:
* UNLEASHED MIND
  Specialized in consulting and planning of Drupal powered sites, UNLEASHED
  MIND offers installation, development, theming, customization, and hosting
  to get you started. Visit http://www.unleashedmind.com for more information.

