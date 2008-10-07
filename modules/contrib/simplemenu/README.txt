# $Id: README.txt,v 1.4.2.16.2.1 2008/02/28 21:47:17 m3avrck Exp $


--- README  -------------------------------------------------------------

SimpleMenu, Version 6.0

Written by Ted Serbinski, aka, m3avrck
  hello@tedserbinski.com
  http://tedserbinski.com

Requirements: Drupal 6.x

jQuery Superfish: http://users.tpg.com.au/j_birch/plugins/superfish/



--- INSTALLATION --------------------------------------------------------

1. Place simplemenu folder in your modules directory

2. Enable "SimpleMenu" under administer > site building > modules

3. Enable access to "view simplemenu" under administer > user management > access control

4. Configure menu to use under administer > site configuration > simplemenu



--- CHANGELOG --------------------------------------------------------

6.0, 2008-xx-xx
----------------------
- compatible with Drupal 6.x
- Superfish 1.4.1
- separate superfish.js into own file
- remove devel links, since devel module links in 6.x can be moved to any menu now


5.0, 2008-Jan-26
----------------------

- #199224, fix display issues in IE6/7 
- #200086, don't load non-existent custom.css file
- #195972, better default CSS to avoid conflicts with themes
- #199715, remove absolute positioning to improve theme and CSS attaching compatibility
- #199882, new options for controlling menu effects and timing


4.0, 2007-Nov-22
----------------------
- new CHANGELOG to keep track of changes
- #156256 upgrade to SuperFish 1.3
- upgrade to bgIframe 2.1.1 (for IE6 compatibility with forms)
- #136478 - fix Opera compatibility
- remove RTL option; this conflicts with other changes and is properly implemented in Drupal 6
- new option to select which theme to style SimpleMenu with, or provide a custom one
- #184051 - don't hardcode CSS, add class to body
- #180106 - fix missing translatable strings
- #144742 - don't show annoying anchor titles
- remove dependency on menu module, now works with menu module off
- new black & blue theme, design by Jeremy Caldwell (http://nerdliness.com/article/2007/11/01/simplemenu-module-customizations)
- alter height of menu and rollover to fix gaps
