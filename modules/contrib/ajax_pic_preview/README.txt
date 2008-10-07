This module uses JQuery and the AJAX File Upload Jquery plugin to present
a preview of a user's new picture as soon as they select one.  Uploads will be
sent through the same validation process and errors will be reported immediately
after selection has been made.

Important! Make sure cron is setup properly or your pictures/tmp/ directory
will grow very large. Preview images are staged there and cron will remove them
once the originating session has expired.

Also note that a subdirectory of pictures named 'tmp' is required. The install
will attempt to create this directory, but you will need to create this manually
if for some reason it fails.


Installation
------------
Copy ajax_pic.module to your module directory and then enable on the admin
modules page.  Make sure pictures is enabled in your User Settings.


Troubleshooting
---------------
If the module isn't working as expected, make sure that there is a subdirectory
in your pictures directory named 'tmp' with the appropriate permissions.


Credits
-------
JQuery AJAX File Upload plugin written by 'logan'.


Author
------
Mike Milano
coder1@gmail.com

Changelog
---------
6.x-1.1
2008-04-25 - pictures/tmp directory now gets created properly upon install.
2008-04-25 - Now works with clean urls disabled.
2008-04-25 - Fixed path to throbber.gif