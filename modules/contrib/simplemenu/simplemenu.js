// $Id: simplemenu.js,v 1.9.2.12.2.1 2008/02/28 21:47:17 m3avrck Exp $

$(document).ready(function() {
  // get the element to add the menu to
  var element = Drupal.settings.simplemenu.element;
  var menu = $(simplemenu).attr("id", "simplemenu");

  switch (Drupal.settings.simplemenu.placement) {
    case 'prepend':
      $(menu).prependTo(element);
      break;
    case 'append':
      $(menu).appendTo(element);
      break;
    case 'replace':
      $(element).html(menu);
      break;
  }

  $('body').addClass('simplemenu-enabled');

  var animation = {};
  animation[Drupal.settings.simplemenu.effect] = 'toggle';
  
  // Build menu
  $(menu)
    .superfish( { 
      animation: animation,
      delay: Drupal.settings.simplemenu.hideDelay,
      speed: Drupal.settings.simplemenu.effectSpeed
    } )
  	.find(">li:has(ul)")
  		.mouseover(function(){
  			$("ul", this).bgIframe({opacity:false});
  		})
  		.find("a")
  			.focus(function(){
  				$("ul", $(".nav>li:has(ul)")).bgIframe({opacity:false});
  			})
  	  .end()
  	.end()
  	.find("a")
  	  .removeAttr('title');

	 $('#simplemenu').children('li.expanded').addClass('root');
});


/* Copyright (c) 2006 Brandon Aaron (http://brandonaaron.net)
 * Dual licensed under the MIT (http://www.opensource.org/licenses/mit-license.php)
 * and GPL (http://www.opensource.org/licenses/gpl-license.php) licenses.
 *
 * $LastChangedDate: 2007-07-21 18:45:56 -0500 (Sat, 21 Jul 2007) $
 * $Rev: 2447 $
 *
 * Version 2.1.1
 */
(function($){$.fn.bgIframe=$.fn.bgiframe=function(s){if($.browser.msie&&/6.0/.test(navigator.userAgent)){s=$.extend({top:'auto',left:'auto',width:'auto',height:'auto',opacity:true,src:'javascript:false;'},s||{});var prop=function(n){return n&&n.constructor==Number?n+'px':n;},html='<iframe class="bgiframe"frameborder="0"tabindex="-1"src="'+s.src+'"'+'style="display:block;position:absolute;z-index:-1;'+(s.opacity!==false?'filter:Alpha(Opacity=\'0\');':'')+'top:'+(s.top=='auto'?'expression(((parseInt(this.parentNode.currentStyle.borderTopWidth)||0)*-1)+\'px\')':prop(s.top))+';'+'left:'+(s.left=='auto'?'expression(((parseInt(this.parentNode.currentStyle.borderLeftWidth)||0)*-1)+\'px\')':prop(s.left))+';'+'width:'+(s.width=='auto'?'expression(this.parentNode.offsetWidth+\'px\')':prop(s.width))+';'+'height:'+(s.height=='auto'?'expression(this.parentNode.offsetHeight+\'px\')':prop(s.height))+';'+'"/>';return this.each(function(){if($('> iframe.bgiframe',this).length==0)this.insertBefore(document.createElement(html),this.firstChild);});}return this;};})(jQuery);