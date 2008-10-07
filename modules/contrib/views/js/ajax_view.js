// $Id: ajax_view.js,v 1.5 2008/05/27 22:31:59 merlinofchaos Exp $

/**
 * @file ajaxView.js
 *
 * Handles AJAX fetching of views, including filter submission and response.
 */

Drupal.Views.Ajax = Drupal.Views.Ajax || {};

/**
 * An ajax responder that accepts a packet of JSON data and acts appropriately.
 *
 * The following fields control behavior.
 * - 'display': Display the associated data in the view area.
 */
Drupal.Views.Ajax.ajaxViewResponse = function(target, response) {

  if (response.debug) {
    alert(response.debug);
  }

  // Check the 'display' for data.
  if (response.status && response.display) {
    var view = $(target).replaceWith(response.display).get(0);
    Drupal.attachBehaviors(view);
  }
};

/**
 * Ajax behavior for views. 
 */
Drupal.behaviors.ViewsAjaxView = function() {
  var ajax_path = Drupal.settings.views.ajax_path;
  // If there are multiple views this might've ended up showing up multiple times.
  if (ajax_path.constructor.toString().indexOf("Array") != -1) {
    ajax_path = ajax_path[0];
  }
  if (Drupal.settings && Drupal.settings.views && Drupal.settings.views.ajaxViews) {
    $.each(Drupal.settings.views.ajaxViews, function(i, settings) {
      var target;
      $('.view-id-'+ settings.view_name +'.view-display-id-'+ settings.view_display_id +':not(.views-processed)')
        .addClass('views-processed')
        .each(function () {
          target = $(this).get(0);
        })
        // Process exposed filter forms.
        .find('form#views-exposed-form')
        .each(function () {
          // remove 'q' from the form; it's there for clean URLs
          // so that it submits to the right place with regular submit
          // but this method is submitting elsewhere.
          $('input[name=q]', this).remove();
          var form = this;
          // ajaxSubmit doesn't accept a data argument, so we have to
          // pass additional fields this way.
          $.each(settings, function(key, setting) {
            $(form).append('<input type="hidden" name="'+ key + '" value="'+ setting +'"/>');
          });
        })
        .submit(function () {
          $('input[@type=submit]', this).after('<span class="views-throbbing">&nbsp</span>');
          $(this).ajaxSubmit({
            url: ajax_path,
            type: 'GET',
            success: function(response) {
              // Call all callbacks.
              if (response.__callbacks) {
                $.each(response.__callbacks, function(i, callback) {
                  eval(callback)(target, response);
                });
              }
            },
            error: function() { alert(Drupal.t("An error occurred at ") + ajax_path); },
            dataType: 'json'
          });
  
          return false;
        })
        .end()
        // Process pager links.
        .find('ul.pager > li > a')
        .each(function () {
          var viewData = Drupal.Views.parseQueryString($(this).attr('href'));
          $.each(settings, function (key, setting) {
            viewData[key] = setting;
          });

          $(this)
            .click(function () {
              $(this).addClass('views-throbbing');
              $.ajax({
                url: ajax_path,
                type: 'GET',
                data: viewData,
                success: function(response) {
                  $(this).removeClass('views-throbbing');
                  // Call all callbacks.
                  if (response.__callbacks) {
                    $.each(response.__callbacks, function(i, callback) {
                      eval(callback)(target, response);
                    });
                  }
                },
                error: function() { $(this).removeClass('views-throbbing'); alert(Drupal.t("An error occurred at ") + ajax_path); },
                dataType: 'json'
              });

              return false;
            });
        })
        .end()
        // Process tablesort links.
        .find('th.views-field a')
        .each(function () {
          var viewData = Drupal.Views.parseQueryString($(this).attr('href'));
          $.each(settings, function (key, setting) {
            viewData[key] = setting;
          });

          $(this)
            .click(function () {
              $(this).addClass('views-throbbing');
              $.ajax({
                url: ajax_path,
                type: 'GET',
                data: viewData,
                success: function(response) {
                  $(this).removeClass('views-throbbing');
                  // Call all callbacks.
                  if (response.__callbacks) {
                    $.each(response.__callbacks, function(i, callback) {
                      eval(callback)(target, response);
                    });
                  }
                },
                error: function() { $(this).removeClass('views-throbbing'); alert(Drupal.t("An error occurred at ") + ajax_path); },
                dataType: 'json'
              });

              return false;
            });
        });
    });
  }
};