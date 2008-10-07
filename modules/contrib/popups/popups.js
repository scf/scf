// $Id: popups.js,v 1.9 2008/03/06 20:32:50 starbow Exp $

/**
 * Popup Modal Dialog API
 *
 * Provide an API for building and displaying JavaScript, in-page, popup modal dialogs.
 * Modality is provided by a fixed, semi-opaque div, positioned in front of the page contents 
 * TODO: I believe fixed positioning is not supported in IE6.
 * The dialog itself is positioned absolutely, which allows for scrolling if the dialog is bigger than the page.
 * Also, the dialog is created off-screen (left: -9999px) to allow for verticle centering without flicker. 
 *
 * Here is an example of using this API to create a dialog:
 * 
 * Drupal.popups.prototype.open_unsaved = function( a ) {
 *   body = Drupal.t("There are unsaved changes on this page.");
 *   buttons = {
 *    'popups_save': { title: Drupal.t('Save Changes'), func: function(){$('#edit-submit').click()} },
 *    'popups_submit': { title: Drupal.t('Discard Changes and Continue'), func: function(){window.location = a.href} },
 *    'popups_cancel': { title: Drupal.t('Cancel'), func: this.close }
 *  };
 *  this.open( "Warning - Please Confirm", body, buttons );
 *  return false;
 * };
 */


/**
 * Create the popups object, and set any defaults.
 */
Drupal.popups = function() {
  this.default_width = 600;
};
/**
 * Generic dialog builder.
 */
Drupal.popups.prototype.open = function( title, body, buttons, width ) {
  if( !width ) {
    width = this.default_width;
  }

  var $overlay = Drupal.popups.get_overlay();
  if ($overlay.size() == 0) {  
    // overlay does not exist yet so create a new one.
    $overlay = Drupal.popups.add_overlay();
  }
    
  // center on the screen, add in offsets if the window has been scrolled
  var overlay_width = $overlay.width();
  var overlay_height = $overlay.height();
  var left = (overlay_width / 2) - (width / 2) + Drupal.popups.f_scrollLeft();
  var top = 0; // we will reposition after adding to page
  // Start with dialog off the side. Making it invisible causes flash.
  var popup = Drupal.theme('popupsDialog', -9999, top, width, title, body, buttons);

  $overlay.before( popup ); // Dialog is added, but still hidden.  

  // Adding button functions
  if (buttons) {
    for (var id in buttons) {
      if (buttons[id]) { // to make jslint happy.
        var func = buttons[id].func;
        $('#'+id).click( func );
      }
    }
  }
  $('#popups-close').click( Drupal.popups.close );
  
  // Get popups's height on the page, and center vertically before showing.
  var popups_height = $('#popups').height(); // Causes flash if not visible!
  if (popups_height < overlay_height) {
    top = (overlay_height / 2) - (popups_height / 2) + Drupal.popups.f_scrollTop();
  }
  else { // popups is too big to fit on screen
    top = 20  + Drupal.popups.f_scrollTop();
  }

  $('#popups').css('top', top).css('left', left); // Make the popup visible.
  
  this.refocus(); // TODO: capture the focus when it leaves the dialog.
  Drupal.popups.remove_loading(); // Remove the loading img.
   
  return false;
};

Drupal.popups.message = function (message, body) {
  var popup = new Drupal.popups();
  var buttons = {
    'popups_ok': { title: Drupal.t('OK'), func: Drupal.popups.close }
  };
  popup.open( message, body, buttons );
};

/************************************************************************************
 * Theme Functions ******************************************************************
 */

Drupal.theme.prototype.popupsLoading = function (left, top) {
  var loading = '<div id="popups-loading">';
  loading += '<div style="left:' + left +'px; top:' + top +'px;">';
  // TODO - remove hardcoded path to image.
  loading += '<img src="'+ Drupal.settings.basePath +'/sites/all/modules/popups/ajax-loader.gif" />';
//  loading += '<img src="'+ Drupal.settings.basePath +'/misc/ajax-loader.gif" />';
  loading += '</div></div>';
  return loading;
};

Drupal.theme.prototype.popupsOverlay = function () {
  return '<div id="popups-overlay"></div>';
};

Drupal.theme.prototype.popupsDialog = function(left, top, width, title, body, buttons) {
  var popups = '<div  id="popups" style="left:'+ left +'px; top:'+ top +'px;';
  popups += ' width:'+ width +'px;';
  popups += '" >';
  popups += '<div id="popups-title">'+
            '  <div id="popups-close"><a>close [X]</a></div>'+
            '  <div class="title">' + title +'</div>'+
            '  <div class="clear"></div>'+
            '</div>';
  if (body) {
    popups += '<div id="popups-body">' + body +'</div>';
  }
  popups += '  <div id="popups-buttons">';

  for ( var id in buttons) {
    if (buttons[id]) {
      var button = buttons[id];
      popups += '<input type="button" value="'+ button.title +'" id="'+ id +'" />';
    }
  }
  
  popups += '  </div>'; // end buttons
  popups +=  '</div>'; // close popups
  return popups;
};

/***************************************************************************************
 * Appearence Functions (overlay, loading graphic, remove popup) ***********************
 **************************************************************************************/
 
Drupal.popups.add_overlay = function() {
  var $overlay = $( Drupal.theme('popupsOverlay') );

  $overlay.css( 'opacity', '0.4' ); // for ie (?)
  $('body').prepend( $overlay );
  return $overlay;
};

Drupal.popups.get_overlay = function() {
  return $('#popups-overlay');
};

Drupal.popups.remove_overlay = function() {
  $('#popups-overlay').remove();
};

Drupal.popups.add_loading = function() {
  var $overlay = Drupal.popups.get_overlay();
  var wait_image_size = 100;
  var left = ( $overlay.width() / 2 ) - ( wait_image_size / 2 );
  var top = ( $overlay.height() / 2 ) - ( wait_image_size / 2 );
  var $loading = $( Drupal.theme('popupsLoading', left, top) );
  $('body').prepend($loading);
};

Drupal.popups.remove_loading = function() {
  $('#popups-loading').remove();
};

Drupal.popups.remove_popup = function() {
  $('#popups').remove();
};

/**
 *  Remove everything.
 */
Drupal.popups.close = function() {
  Drupal.popups.remove_popup();
  Drupal.popups.remove_loading();
  Drupal.popups.remove_overlay();
};

/**
 *  Set the focus on the popup to the first visible form element, or the first button, or the close link.
 */
Drupal.popups.prototype.refocus = function() {
  $focus = $('#popups input:visible:eq(0)')
  if (!focus) {
    $focus = $('#popups-close');
  }
  $focus.focus()
}

/****************************************************************************
 * Utility functions
 ****************************************************************************/
 
/**
 * Break the URL down into it's componant parts.
 * Might be worth turning into a core function.
 *  @param
 *    String version of URL.
 *  @return
 *    Object version of URL.
 */ 
Drupal.popups.parseUrl = function(url) {
  var params = [];
  var temp = url.split('#');
  url = temp[0];
  var fragment = temp[1];
  
  // Convert the existing parameter string into an array.
  if (url.indexOf('?') > -1) { 
    temp = url.split('?');
    url = temp[0];
    if (temp[1]) {
      params = temp[1].split('&');    
    }
  }
  return { 'url': url, 'params': params, 'fragment': fragment};
}

/**
 *  Rebuild the url string from the obj returned by parseUrl.
 *  @param
 *    Object version of URL.
 *  @return
 *    String version of URL.
 */
Drupal.popups.buildUrl = function(u) {
  url = u.url + '?' + u.params.join('&');
  if (u.fragment) {
    url += '#' + u.fragment;
  }
  return url;
}

/***********************************************************************************************
 * Utility functions taken from http://www.softcomplex.com/docs/get_window_size_and_scrollbar_position.html
 */

Drupal.popups.f_scrollLeft = function() {
  return Drupal.popups.f_filterResults (
    window.pageXOffset ? window.pageXOffset : 0,
    document.documentElement ? document.documentElement.scrollLeft : 0,
    document.body ? document.body.scrollLeft : 0 );
};

Drupal.popups.f_scrollTop = function() {
  return Drupal.popups.f_filterResults (
    window.pageYOffset ? window.pageYOffset : 0,
    document.documentElement ? document.documentElement.scrollTop : 0,
    document.body ? document.body.scrollTop : 0 );
};

Drupal.popups.f_filterResults = function(n_win, n_docel, n_body) {
  var n_result = n_win ? n_win : 0;
  if (n_docel && (!n_result || (n_result > n_docel))) {
    n_result = n_docel;
  }
  return n_body && (!n_result || (n_result > n_body)) ? n_body : n_result;
};

/***************************************************************************************
 * Page-in-Popup Behavior
 **************************************************************************************/

/**
 * This part uses the Popup API to build dialogs that show the content from a Drupal page.
 * 
 * It is assumed that the pages being returned from the Drupal server will be XML
 *  and have the following format:
 *
 * <popup>
 *  <title>Page Title</title>
 *   <messages>Error, warning and status messages, as HTML</messages>
 *   <path>The page's URL</path>
 *   <content>The content of the page, as HTML</content>
 * </popup>
 */

/**
 * Attach the popup bevior to the all the requested links on the page.
 *
 * @param context: The jQuery object to apply the behaviors to.
 */
Drupal.behaviors.popups = function(context) {
  var popups = new Drupal.popups();
  
  // Add the popup-link-in-dialog behavior to links defined in Drupal.settings.popups.links array.
  // TODO: how to handle popups-in-popups?
  if (Drupal.settings.popups) {
    for (var link in Drupal.settings.popups.links) {
      var options = Drupal.settings.popups.links[link];
      popups.attach(context, link, options); // Needs to be seperate function for closure.
    }
  }
    
//  $('a.popups', context).click( function() {return popups.open_path(this, {});} );
  popups.attach(context, 'a.popups', {});
};

/**
 * Attach the popup behavior to a particular link.
 *
 * @param link - link that was clicked.
 * @param options - options associated with the link.
 */
Drupal.popups.prototype.attach = function(context, link, options) {
  var popups = this;
  $(link, context).not('.popups-processed').each( function() {
    $(this).click( function(e){ 
      var a = this;
      // If the option is distructive, check if the page is already modified, and offer to save.
      var page_is_dirty = $('span.tabledrag-changed').size() > 0;
      var will_modify_original = !options.noReload && !options.singleRow;
      if( page_is_dirty && will_modify_original ) {
        // The user will lose modifications, so popup dialog offering to save current state.
        var body = Drupal.t("There are unsaved changes on this page, which you will lose if you continue.");
        var buttons = {
         'popups_save': {title: Drupal.t('Save Changes'), func: function(){popups.save_page(a, options)}},
         'popups_submit': {title: Drupal.t('Continue'), func: function(){Drupal.popups.close(); popups.open_path(a, options)}},
         'popups_cancel': {title: Drupal.t('Cancel'), func: Drupal.popups.close}
        };
        return popups.open( Drupal.t('Warning: Please Confirm'), body, buttons );
      }
      else {
        return popups.open_path(a, options);
      } 
    });
    $(this).addClass('popups-processed');
  });

};


/**
 * Deal with the param string of a url to make the response popup friendly.
 * Add 'page_override=popup' param.
 * 
 * @param url 
 *   String: The original url.
 * @return url 
 *   String: The url with the corrected parameters.
 */
Drupal.popups.prepUrl = function(url) {
  var u = Drupal.popups.parseUrl(url);  
  // Add our param to the filtered array of existing params.
  if (jQuery.inArray('page_override=popup', u.params) == -1) {
    u.params.unshift('page_override=popup');
  }  
  // Rebuild the url with the new param, the old params and the old fragment.
  return Drupal.popups.buildUrl(u);
}

/**
 * Use Ajax to open the link in a popup window.
 *
 * @param a - link that was clicked.
 * @param options - options associated with the link.
 */
Drupal.popups.prototype.open_path = function( a, options ) {
  var popup = this;
  // let the user know something is happening
  $('body').css("cursor", "wait");
  var $overlay = Drupal.popups.add_overlay(); 
  Drupal.popups.add_loading();

//  var type = options.type ? options.type : 'json'; // Default to json.
  var url = Drupal.popups.prepUrl(a.href); 
//  if (type=='html') { // Special handling for html.
//    $.get(url, function(data) {  
//      var $response = $('<div></div>');
//      $response.html(data);
//      var body = $response.html();
//      popup.open_content(document.title, body, options, a);
//       $('body').css("cursor", "auto"); // Return the cursor to normal state.
//    });
//  }
//  else { // It is a json object from Drupal.
  $.getJSON(url, function(json) {  
    popup.open_content(json.title, json.messages + json.content, options, a);
    $('body').css("cursor", "auto"); // Return the cursor to normal state.  
  });
//  }
/*  
  $.get(url, function(data) {  
    var $data = $(data);
    if ($data.size() == 1) { // Expecting XML with single root.
     var title = $data.find('title').text();
     var messages = $data.find('messages').text();
     var content = messages + $data.find('content').text();
     popup.open_content(title, content, options, a);
     $('body').css("cursor", "auto"); // Return the cursor to normal state.
    }
    else { // Not XML, so show entire HTML page in popup.
      // Filter the html - There must be a there a better way?
      var $response = $('<div></div>');
      $response.html(data);
      var body = $response.html();
//      var msg = 'Messages: ' + $('.messages', $response).text();
//      Drupal.popups.message('Error: Bad response.', msg);
//      Drupal.popups.remove_loading();
      var title = document.title;
      popup.open_content(title, body, options, a);
      $('body').css("cursor", "auto"); // Return the cursor to normal state.
    }
  });   
*/     
  return false;         
};


Drupal.popups.prototype.open_content = function(title, content, options, a) {
  this.open(title, content); 
  // Add behaviors to content in popup. 
  // TODO: d-n-d: need to click to let go of selection.
  Drupal.attachBehaviors($('#popups-body'));
  // Adding collapse moves focus.
  this.refocus();

  // If the popup contains a form, capture submits.
  var $form = $('form', '#popups-body');
  $form.ajaxForm({ 
    dataType: 'json',     
    beforeSubmit: Drupal.popups.beforeSubmit,
    success: function(response, status) { Drupal.popups.formSuccess(response, options, a) },
  });
}

/**
 * Do before the form in the popup is submitted.
 *
 */
Drupal.popups.beforeSubmit = function(form_data, $form, options) {
  Drupal.popups.remove_popup(); // Remove just the dialog, but not the overlay.
  Drupal.popups.add_loading();
  // Send the original page back to Drupal with a flag to return the form results unthemed. 
  options.url = Drupal.popups.prepUrl(options.url);
};

/**
 * The form in the popup was successfully submitted
 * Update the originating page.
 * Show any messages in a popup (TODO - make this a configurable option).
 * 
 * @param response - specially formated page contents from server.
 * @param options - hash of per link options.
 * @param a - the link that was clicked.
 */
Drupal.popups.formSuccess = function (response, options, a) {
  var $data = $(response);
  if ($data.size() > 1) { // Bad html response, show an error message.
    var $response = $('<div></div>');
    $response.html( response );
    var msg = 'Messages: ' + $('.messages', $response).text();
    Drupal.popups.message('Error: Bad response.', msg);
    Drupal.popups.remove_loading();
  }
  else { // Got a good response back from the server.
    // Get into common format for testing.
    var data = {
      title: response.title, // $data.find('title').text();
      messages: response.messages, // $data.find('messages').text();
      path: response.path, // $data.find('path').text();
      content: response.content // $data.find('content').text()
    };
  
//    var messages = $data.find('messages').text();
        
    // Are we at an end point, or just moving from one popup to another?
//    var path = $data.find('path').text();
    if (!location.href.match(data.path)) { // Not done yet, so show results in new popup.
//      var title = $data.find('title').text();
//      var content = $data.find('content').text();
      Drupal.popups.remove_loading();
      var popups = new Drupal.popups();     
      popups.open_content(data.title, data.messages + data.content, options, a);
    }
    else { // Done, so show messages in dialog and embed the results in the original page.
      if (data.messages) {
        Drupal.popups.message(data.messages);
        // Also insert the message into the page above the content.
        // Might not be the standard spot, but it is the easiest to find.
        var $next = $(Drupal.settings.popups.defaultTargetSelector);
        $next.parent().find('div.messages').remove(); // Remove the current messages.
        $next.before(data.messages);
      }
          
      // Just update a single row out of a table (still expiremental). 
      // Loop through, with special case for first element.
      if (options.singleRow) {
        var href = $(a).attr('href');
        var selector = 'table a[href=' + href + ']';
        var $new_row = $data.find(selector).parents('tr'); // new tr
        var $target_row = $(selector).parents('tr'); // target tr.
          for (var i in options.singleRow) {
            var col = options.singleRow[i];
            $new_row.find(col).contents().not('div.indentation').wrapAll('<div id="newvalue"/>');
            $target_row.find(col).contents()
              .not('a.tabledrag-handle').not('span.warning').not('div.indentation')
              .wrapAll('<div id="killme"/>');
            $('#killme').replaceWith( $new_row.find('#newvalue').html() );
          }
      }
      // Update the entire content area (defined by 'target selector').
      else if (!options.noReload) { 
        var target = options.targetSelector;
        if (!target) {
          target = Drupal.settings.popups.defaultTargetSelector;
        }
        
        // Remove page_override=popup param from form's action. 
        var action = $data.find('form').attr('action');
        if (action) { 
          action = Drupal.popups.parseUrl(action);
          action.params = jQuery.grep(action.params, function(n, i){
            return n != 'page_override=popup';
          });
          $data.find('form').attr( 'action', Drupal.popups.buildUrl(action) );
        }
        
        // Update the original page.      
//        var content = $data.find('content').text();
        var $c = $(target).html(data.content); // Inject the new content into the page.
        Drupal.attachBehaviors($c);
      }
      
      // Update the title of the page.
      if (options.updateTitle) {
//        var title = $data.find('title').text();
        $(Drupal.settings.popups.defaultTitleSelector).html(data.title);
        document.title = data.title; // Also update the browser page title (TODO: include site name?).
      }
      
      // Done with changes to the original page, remove effects.
      Drupal.popups.remove_loading();
      if (!data.messages) { 
        // If there is not a messages popup remove the overlay.
        Drupal.popups.remove_overlay();
      }
    }  // End of updating original page.
  } // End of good response.
}; 

/**
 * Submit the page and reload the results, before popping up the real dialog.
 *
 * @param a - link that was clicked.
 * @param options - options associated with the link.
 */
Drupal.popups.prototype.save_page = function(a, options) {
  var popups = this;
  // TODO - what if clicking on link with option['targetSelector']?
  var target = Drupal.settings.popups.defaultTargetSelector;
  var $form = $('form', target);
  var ajaxOptions = {
    dataType: 'json',
    beforeSubmit: Drupal.popups.beforeSubmit,
    success: function(response, status) { // Sync up the current page contents with the submit.     
//      var $data = $(response);
//      var content = $data.find('content').text();
      var $c = $(target).html(response.content); // Inject the new content into the page.
      Drupal.attachBehaviors($c);
      Drupal.popups.close();
      // The form has been saved, the page reloaded, now safe to show the link in a popup.
      popups.open_path(a, options); 
    }     
  };
  $form.ajaxSubmit( ajaxOptions ); // Submit the form. 
};

