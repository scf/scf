// $Id: ajax_view.js,v 1.13 2008/12/02 18:35:50 merlinofchaos Exp $

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

  var $view = $(target);

  // Check the 'display' for data.
  if (response.status && response.display) {
    var $newView = $(response.display);
    $view.replaceWith($newView);
    $view = $newView;
    Drupal.attachBehaviors($view.parent());
  }
 
  if (response.messages) {
    // Show any messages (but first remove old ones, if there are any).
    $view.find('.views-messages').remove().end().prepend(response.messages);
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
      var view = '.view-dom-id-' + settings.view_dom_id;
      if (!$(view).size()) {
        // Backward compatibility: if 'views-view.tpl.php' is old and doesn't
        // contain the 'view-dom-id-#' class, we fall back to the old way of
        // locating the view:
        view = '.view-id-' + settings.view_name + '.view-display-id-' + settings.view_display_id;
      }


      // Process exposed filter forms.
      $('form#views-exposed-form-' + settings.view_name.replace(/_/g, '-') + '-' + settings.view_display_id.replace(/_/g, '-'))
      .filter(':not(.views-processed)')
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
      .addClass('views-processed')
      .submit(function () {
        $('input[@type=submit]', this).after('<span class="views-throbbing">&nbsp</span>');
        var object = this;
        $(this).ajaxSubmit({
          url: ajax_path,
          type: 'GET',
          success: function(response) {
            // Call all callbacks.
            if (response.__callbacks) {
              $.each(response.__callbacks, function(i, callback) {
                eval(callback)(view, response);
              });
              $('.views-throbbing', object).remove();
            }
          },
          error: function() { alert(Drupal.t("An error occurred at @path.", {'@path': ajax_path})); $('.views-throbbing', object).remove(); },
          dataType: 'json'
        });

        return false;
      });

      $(view).filter(':not(.views-processed)').each(function() {
        var target = this;
        $(this)
        .addClass('views-processed')
        // Process pager links.
        .find('ul.pager > li > a')
        .each(function () {
          var viewData = Drupal.Views.parseQueryString($(this).attr('href'));
          if (!viewData['view_name']) {
            $.each(settings, function (key, setting) {
              viewData[key] = setting;
            });
          }

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
                error: function() { $(this).removeClass('views-throbbing'); alert(Drupal.t("An error occurred at @path.", {'@path': ajax_path})); },
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
                error: function() { $(this).removeClass('views-throbbing'); alert(Drupal.t("An error occurred at @path.", {'@path': ajax_path})); },
                dataType: 'json'
              });

              return false;
            });
        }); // .each 'th.views-field a'
      }); // $view.filter().each
    }); // .each Drupal.settings.views.ajaxViews
  } // if
};
