$Id: README.txt,v 1.1.2.2 2008/02/18 14:31:01 fago Exp $

Content Profile Module
------------------------
by Wolfgang Ziegler, nuppla@zites.net

With this module you can build user profiles with drupal's content types.


Installation 
------------
 * Copy the module's directory to your modules directory and activate the module.
 
 Usage:
--------
 * There will be a new content type "profile". Customise its settings at
   admin/content/types.
 * When you edit a profile content type there will be a further tab "Content profile",
   which provides content profile specific settings.
 * At the bottom of each content type edit form, there is a checkbox, which allows
   you to mark a content type as profile.


Content profiles per role:
--------------------------
You may, but you need not, mark multiple content types as profile. By customizing 
the permissions of a content type, this allows you to create different profiles for
different roles.