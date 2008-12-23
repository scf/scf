// $Id: popups.js,v 1.9.2.18 2008/11/20 21:18:57 starbow Exp $

/**
 * Popup Modal Dialog API
 *
 * Provide an API for building and displaying JavaScript, in-page, popups modal dialogs.
 * Modality is provided by a fixed, semi-opaque div, positioned in front of the page contents.
 *
 */

/**
 * Create the popups object/namespace.
 */
Drupal.popups = function() {};

function isset(v) {
  return (typeof(v) !== 'undefined');
}

/**
 * Attach the popups bevior to the all the requested links on the page.
 *
 * @param context
 *   The jQuery object to apply the behaviors to.
 */
Drupal.behaviors.popups = function(context) {
  var $body = $('body');
  if(!$body.hasClass('popups-processed')) {
    $body.addClass('popups-processed');
    $(document).bind('keydown', Drupal.popups.keyHandle);
    $popit = $('#popit');
    if ($popit.length) {
      $popit.remove();
      Drupal.popups.message($popit.html());
    }
  }
  
  // Add the popups-link-in-dialog behavior to links defined in Drupal.settings.popups.links array.
  if (Drupal.settings.popups.links) {
    jQuery.each(Drupal.settings.popups.links, function (link, options) { 
      if (isset(options.noReload)) { // Using obsolete name.
        options.noUpdate = options.noReload; // Don't break existing sites with name change.
      }
      Drupal.popups.attach(context, link, options);
    });
  }
  
  Drupal.popups.attach(context, '.popups', {noUpdate: true});  
  Drupal.popups.attach(context, '.popups-form', {}); // ajax reload.
  Drupal.popups.attach(context, '.popups-form-reload', {reloadWhenDone: true}); // whole page reload. 
  Drupal.popups.attach(context, '.popups-form-noupdate', {noUpdate: true});  // no reload at all.
  Drupal.popups.attach(context, '.popups-form-noreload', {noUpdate: true});  // Obsolete.
  
};

/**
 * Attach the popups behavior to a particular link.
 *
 * @param selector
 *   jQuery selector for links to attach popups behavior to.
 * @param options
 *   Hash of options associated with these links.
 */
Drupal.popups.attach = function(context, selector, options) {
  $(selector, context).not('.popups-processed').each(function() {
    var $element = $(this);
    // Mark the element as attached.    
    var title = $element.attr('title') || '';
    $element.attr('title', title + Drupal.t('[Popup]')); // Append note to link title.
    $element.addClass('popups-processed');
    
    // Attach the on-click popup behavior to the element.
    $element.click(function(e){ 
      var element = this;

      // If element is inside of a #popup div, show alert and bail out. 
      if ($(element).parents('#popups').length) { 
        alert("Sorry, popup chaining is not supported (yet).");
        return false;
      }

      // If the element contains a on-popups-options attribute, use it instead of options param.
      if ($(element).attr('on-popups-options')) {
        options = eval('(' + $(element).attr('on-popups-options') + ')'); 
      }

      // If the option is distructive, check if the page is already modified, and offer to save.
      var pageIsDirty = $('span.tabledrag-changed').size() > 0;
      var willModifyOriginal = !options.noUpdate;
      if (pageIsDirty && willModifyOriginal) {
        // The user will lose modifications, so popups dialog offering to save current state.
        var body = Drupal.t("There are unsaved changes on this page, which you will lose if you continue.");
        var buttons = {
         'popup_save': {title: Drupal.t('Save Changes'), func: function(){Drupal.popups.savePage(element, options);}},
         'popup_submit': {title: Drupal.t('Continue'), func: function(){Drupal.popups.removePopup(); Drupal.popups.openPath(element, options);}},
         'popup_cancel': {title: Drupal.t('Cancel'), func: Drupal.popups.close}
        };
        return Drupal.popups.open( Drupal.t('Warning: Please Confirm'), body, buttons );
      }
      else {
        return Drupal.popups.openPath(element, options);
      } 
    });    
  });
};


/**
 * Generic dialog builder.
 */
Drupal.popups.open = function(title, body, buttons, width) {
  Drupal.popups.addOverlay(); // TODO - nonModal option.
  var $popups = $(Drupal.theme('popupDialog', title, body, buttons));
  // Start with dialog off the side. Making it invisible causes flash in FF2.
  $popups.css('left', '-9999px');
  if (width) {
    $popups.css('width', width);
  }
  $('body').append( $popups ); // Add the popups to the DOM.

  // Adding button functions
  if (buttons) {
    jQuery.each(buttons, function (id, button) { 
      $('#'+id).click(button.func);
    });  
  
//    for (var id in buttons) {
//      if (buttons[id]) { // to make jslint happy.
//        var func = buttons[id].func;
//        $('#'+id).click( func );
//      }
//    }
  }
  $('#popups-close').click( Drupal.popups.close );
  $('a.popups-close').click( Drupal.popups.close );
    
  // center on the screen, adding in offsets if the window has been scrolled
  var popupWidth = $popups.width();  
  var windowWidth = $(window).width();
  var left = (windowWidth / 2) - (popupWidth / 2) + Drupal.popups.scrollLeft();
  
  // Get popups's height on the page.
  // Causes flash in FF2 if popups is not visible!
  var popupHeight = $popups.height(); 
  var windowHeight = $(window).height();
  if (popupHeight > (0.9 * windowHeight) ) { // Must fit in 90% of window.
    popupHeight = 0.9 * windowHeight;
    $popups.height(popupHeight);
  }  
  var top = (windowHeight / 2) - (popupHeight / 2) + Drupal.popups.scrollTop();

  $popups.css('top', top).css('left', left); // Position the popups to be visible.
  
  this.refocus(); // TODO: capture the focus when it leaves the dialog.
  Drupal.popups.removeLoading(); // Remove the loading img.
   
  return false;
};

/**
 *  Simple popups that functions like the browser's alert box.
 */
Drupal.popups.message = function(title, message) {
  message = message || '';
  var buttons = {
    'popup_ok': {title: Drupal.t('OK'), func: Drupal.popups.close}
  };
  Drupal.popups.open(title, message, buttons);
};

/**
 * Handle any special keys when popups is active.
 */
Drupal.popups.keyHandle = function(e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 27: // esc
      Drupal.popups.close();
      break;
    case 191: // '?' key, show help.
      if (e.shiftKey && e.ctrlKey) {
        var $help = $('a.popups.more-help');
        if ($help.size()) {
          $help.click();
        }
        else {
          Drupal.popups.message(Drupal.t("Sorry, there is no additional help for this page"));
        }
      }
      break;
  }
};

/*****************************************************************************
 * Appearence Functions (overlay, loading graphic, remove popups)     *********
 *****************************************************************************/

Drupal.popups.removePopup = function() {
  $('#popups').remove();  
}; 
 
Drupal.popups.addOverlay = function() {
  var $overlay = $('#popups-overlay');
  if (!$overlay.size()) { // Overlay does not already exist, so create it.
    $overlay = $(Drupal.theme('popupOverlay'));
    $overlay.css('opacity', '0.4'); // for ie6(?)
    // Doing absolute positioning, so make overlay's size equal the entire body.
    $doc = $(document);
    $overlay.width($doc.width()).height($doc.height()); 
    $overlay.click(Drupal.popups.close);
    $('body').prepend($overlay);
  }
};

Drupal.popups.removeOverlay = function() {
  $('#popups-overlay').remove();
};

Drupal.popups.addLoading = function() {
  var $loading = $('#popups-loading');
  if (!$loading.size()) { // Overlay does not already exist, so create it.
    var waitImageSize = 100;
    var left = ($(window).width() / 2) - (waitImageSize / 2)  + Drupal.popups.scrollLeft();
    var top = ($(window).height() / 2) - (waitImageSize / 2)  + Drupal.popups.scrollTop();
    $loading = $( Drupal.theme('popupLoading', left, top) );
    $('body').prepend($loading);
  }
};

Drupal.popups.removeLoading = function() {
  $('#popups-loading').remove();
};

/**
 * Remove everything.
 */
Drupal.popups.close = function() {
  Drupal.popups.removePopup();
  Drupal.popups.removeLoading();
  Drupal.popups.removeOverlay();
  return false;
};

/**
 * Set the focus on the popups to the first visible form element, or the first button, or the close link.
 */
Drupal.popups.refocus = function() {
  $focus = $('#popups input:visible:eq(0)');
  if (!isset(focus)) {
    $focus = $('#popups-close'); // Doesn't seem to work.
  }
  $focus.focus();
};

/****************************************************************************
 * Theme Functions   ********************************************************
 ****************************************************************************/

Drupal.theme.prototype.popupLoading = function(left, top) {
  var loading = '<div id="popups-loading">';
  loading += '<div style="left:' + left +'px; top:' + top +'px;">';
  loading += '<img src="'+ Drupal.settings.basePath + Drupal.settings.popups.modulePath + '/ajax-loader.gif" />';
  loading += '</div></div>';
  return loading;
};

Drupal.theme.prototype.popupOverlay = function() {
  return '<div id="popups-overlay"></div>';
};

Drupal.theme.prototype.popupButton = function(title, id) {
  return '<input type="button" value="'+ title +'" id="'+ id +'" />';
};

Drupal.theme.prototype.popupDialog = function(title, body, buttons) {
  var template = Drupal.settings.popups.template;
  var popups = template.replace('%title', title).replace('%body', body);
  
  var themedButtons = '';
  if (buttons) {
    jQuery.each(buttons, function (id, button) { 
      themedButtons += Drupal.theme('popupButton', button.title, id);
    });  
  }  
  popups = popups.replace('%buttons', themedButtons);  
  return popups;
};

// Stolen jQuery offset
Drupal.popups.scrollLeft = function() {
  return Math.max(document.documentElement.scrollLeft, document.body.scrollLeft);
};

// Stolen jQuery offset
Drupal.popups.scrollTop = function() {
  return Math.max(document.documentElement.scrollTop, document.body.scrollTop);
};

/****************************************************************************
 * Page & Form in popups functions                                         ***
 ****************************************************************************/

/**
 * Use Ajax to open the link in a popups window.
 *
 * @param element
 *   Element that was clicked to open the popups.
 * @param options
 *   Hash of options controlling how the popups interacts with the underlying page.
 */
Drupal.popups.openPath = function(element, options) {
  // let the user know something is happening
  $('body').css("cursor", "wait");
  
  // TODO - get nonmodal working.
  if (!options.nonModal) {
    Drupal.popups.addOverlay(); 
  }
  Drupal.popups.addLoading();
  
  var href = options.href ? options.href : element.href;
  var params = {};
  
  // Force the popups to return back to the orignal page when forms are done.
  if (!options.forceReturn) { // If forceReturn, requestor wants data from different page.
    href = href.replace(/destination=[^;&]*[;&]?/, ''); // Strip out any existing destination param.
    params.destination = Drupal.settings.popups.originalPath; // Set the destination to the original page.    
  }

  ajaxOptions = {
    url: href,
    dataType: 'json',
    data: params,
    beforeSend: Drupal.popups.beforeSend,
    success: function(json) { 
      Drupal.popups.openContent(json.title, json.messages + json.content, options);
    },
    complete: function() {
      $('body').css("cursor", "auto"); // Return the cursor to normal state.      
    }
  };

  if (options.reloadOnError) {
    ajaxOptions.error = function() {
//      Drupal.popups.close(); // close everything;
//      console.log( "href = " + href + ", location = " + location.href );
      location.reload(); // Reload on error ?
    };    
  }
  else {
   ajaxOptions.error = function() {
      Drupal.popups.message("Unable to open: " + href);
    };
  }
  $.ajax(ajaxOptions);
/*
  $.ajax({
    url: href,
    dataType: 'json',
    beforeSend: Drupal.popups.beforeSend,
    success: function(json) { 
      Drupal.popups.openContent(json.title, json.messages + json.content, options);
    },
    error: function() {
      Drupal.popups.message("Unable to open: " + href);
    },
    complete: function() {
      $('body').css("cursor", "auto"); // Return the cursor to normal state.      
    }
  });
*/
        
  return false;         
};

/**
 * Open content in an ajax popups.
 *
 * @param title
 *   String title of the popups.
 * @param content
 *   HTML to show in the popups.
 * @param options
 *   Hash of options controlling how the popups interacts with the underlying page.
 */
Drupal.popups.openContent = function(title, content, options) {
  Drupal.popups.open(title, content, null, options.width); 
  // Add behaviors to content in popups. 
  // TODO: d-n-d: need to click to let go of selection.
  delete Drupal.behaviors.tableHeader; // Work-around for bug in tableheader.js (http://drupal.org/node/234377)
  delete Drupal.behaviors.teaser; // Work-around for bug in teaser.js (sigh).
  Drupal.attachBehaviors($('#popups-body'));
  // Adding collapse moves focus.
  Drupal.popups.refocus();

  // If the popups contains a form, capture submits.
  var $form = $('form', '#popups-body');
  $form.ajaxForm({   
    dataType: 'json',   
    beforeSubmit: Drupal.popups.beforeSubmit,
    beforeSend: Drupal.popups.beforeSend,
    success: function(response, status) {
      Drupal.popups.formSuccess(response, options);
    },
    error: function() {
      Drupal.popups.message("Bad Response form submission");
    }
  });
};

Drupal.popups.beforeSend = function(xhr) {
  xhr.setRequestHeader("X-Drupal-Render-Mode", 'json/popups');
};

/**
 * Do before the form in the popups is submitted.
 */
Drupal.popups.beforeSubmit = function(formData, $form, options) {
  Drupal.popups.removePopup(); // Remove just the dialog, but not the overlay.
  Drupal.popups.addLoading();
//  console.log("Before Submit");
};

/**
 * The form in the popups was successfully submitted
 * Update the originating page.
 * Show any messages in a popups (TODO - make this a configurable option).
 * 
 * @param response
 *   JSON object from server with status of form submission.
 * @param options
 *   Hash of options controlling how the popups interacts with the underlying page.
 *     noUpdate: bool, does the popups effect the underlying page.
 *     nonModal: bool, does the popups block access to the underlying page.
 *     targetSelectors: hash of jQuery selectors, overrides defaultTargetSelector.
 *     titleSelectors: array of jQuery selectors, where to put the the new title of the page.
 */
Drupal.popups.formSuccess = function(data, options) {  
  // Determine if we are at an end point, or just moving from one popups to another.
  var done = (data.path === Drupal.settings.popups.originalPath) || (data.path === options.forceReturn);
  if (!done) { // Not done yet, so show new page in new popups.
    Drupal.popups.removeLoading();
    Drupal.popups.openContent(data.title, data.messages + data.content, options);
  }
  else { // Done.
    if (options.reloadWhenDone) { // Force a non-ajax, complete reload of the page.
      location.reload(); 
    }
    else { // Normal ajax reload behavior
      // show messages in dialog and embed the results in the original page.
      var showMessage = data.messages.length && !options.noMessage;
      if (showMessage) {
        Drupal.popups.message(data.messages);
        if (!Drupal.settings.popups.popupFinalMessage) {
          setTimeout(Drupal.popups.close, 2500); // Autoclose the message box in 2.5 seconds.
        }
  
        // Insert the message into the page above the content.
        // Might not be the standard spot, but it is the easiest to find.
        var $next = $(Drupal.settings.popups.defaultTargetSelector);
        $next.parent().find('div.messages').remove(); // Remove the current messages.
        $next.before(data.messages);
      }
          
      // Update the content area (defined by 'targetSelectors').
      if (!options.noUpdate) { 
        Drupal.popups.testContentSelector();
        if (isset(options.targetSelectors)) { // Pick and choose what returned content goes where.
          jQuery.each(options.targetSelectors, function(t_new, t_old) {
            if(!isNaN(t_new)) {
              t_new = t_old; // handle case where targetSelectors is an array, not a hash.
            }
            var new_content = $(t_new, data.content);
            var $c = $(t_old).html(new_content); // Inject the new content into the original page.
            Drupal.attachBehaviors($c);  
          });
        }
        else { // Put the entire new content into default content area.
          $c = $(Drupal.settings.popups.defaultTargetSelector).html(data.content);
          Drupal.attachBehaviors($c);                    
        }
      }
      
      // Update the title of the page.
      if (isset(options.titleSelectors)) {
        jQuery.each(options.titleSelectors, function() {
          $(''+this).html(data.title);
        });
      }
              
      // Done with changes to the original page, remove effects.
      Drupal.popups.removeLoading();
      if (!showMessage) { 
        // If there is not a messages popups, so remove the overlay.
        Drupal.popups.removeOverlay();
      }
    }
  }  // End of updating original page.
}; 

/**
 * Submit the page and reload the results, before popping up the real dialog.
 *
 * @param element
 *   Element that was clicked to open the popups.
 * @param options
 *   Hash of options controlling how the popups interacts with the underlying page.
 */
Drupal.popups.savePage = function(element, options) {
  var target = Drupal.settings.popups.defaultTargetSelector;
  var $form = $('form', target);
  var ajaxOptions = {
    dataType: 'json',
    beforeSubmit: Drupal.popups.beforeSubmit,   
    beforeSend: Drupal.popups.beforeSend,
    success: function(response, status) { 
      // Sync up the current page contents with the submit.
      var $c = $(target).html(response.content); // Inject the new content into the page.
      Drupal.attachBehaviors($c);
      // The form has been saved, the page reloaded, now safe to show the link in a popup.
      Drupal.popups.openPath(element, options); 
    } 
  };
  $form.ajaxSubmit(ajaxOptions); // Submit the form. 
};

/**
 * Warn the user if ajax updates will not work
 *   due to mismatch between the theme and the theme's popup setting.
 */
Drupal.popups.testContentSelector = function() {
  var target = Drupal.settings.popups.defaultTargetSelector;
  var hits = $(target).length;
  if (hits !== 1) { // 1 is the corrent answer.
    msg = Drupal.t('The popup content area for this theme is misconfigured.') + '\n';
    if (hits === 0) {
      msg += Drupal.t('There is no element that matches ') + '"' + target + '"\n';
    }
    else if (hits > 1) {
      msg += Drupal.t('There are multiple elements that match: ') + '"' + target + '"\n';
    }
    msg += Drupal.t('Go to admin/build/themes/settings, select your theme, and edit the "Content Selector" field'); 
    alert(msg);
  }
};
