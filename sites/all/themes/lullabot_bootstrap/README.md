About
=====
This theme is a fork of the [Twitter Boostrap](http://drupal.org/project/twitter_bootstrap) theme. It began with the 7.x-2.x-dev release available as of May 1, and has been significantly cleaned up for use as a base theme for our clients.

Installation
============
Install the Bootstrap library:

* Download the Bootstrap library v2 (http://twitter.github.com/bootstrap/), make sure the resulting folder is named "bootstrap"
* Put the bootstrap folder inside the lullabot_bootstrap theme folder, so that it appears at [path the themes]/lullabot_bootstrap/bootstrap

Update jQuery. Bootstrap requires jQuery 1.7.

* As of this writing, 1.7 is only in the 7.x-2.x-dev release of [jQuery Update module](http://drupal.org/project/jquery_update). Install and enable this version of the module.
* Configure the module to use jQuery 1.7, since it defaults to 1.5. Visit admin/config/development/jquery_update and set version to 1.7.
* If the [CTools module](http://drupal.org/project/ctools) is also in use, it needs to be patched in order for the dependency functionality to work. Apply the latest patch in this issue: http://drupal.org/node/1494860

Put any plugin files into [path the themes]/lullabot_bootstrap/bootstrap/js/