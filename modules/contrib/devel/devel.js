// $Id: devel.js,v 1.3 2007/11/07 19:13:01 weitzman Exp $

    
/**
  *  @name    jQuery Logging plugin
  *  @author  Dominic Mitchell
  *  @url     http://happygiraffe.net/blog/archives/2007/09/26/jquery-logging
  */
jQuery.fn.log = function (msg) {
    console.log("%s: %o", msg, this);
    return this;
};