// $Id: autocomplete.js,v 1.22 2007/10/21 18:59:01 goba Exp $

/**
 * Requires misc/autocomplete.js to be loaded BEFORE this file loads.
 */

/**
 * Attaches the autocomplete behavior to all required fields
 */
Drupal.behaviors.autocomplete = function (context) {
  var acdb = [];
  $('input.autocomplete:not(.autocomplete-processed)', context).each(function () {
    var uri = this.value;
    if (!acdb[uri]) {
      acdb[uri] = new Drupal.ACDB(uri);
    }
    // --------------------------------------------------------------
    // CHANGED/ADDED BY TG
    var input_id = this.id.substr(0, this.id.length - 13);
    var input = $('#' + input_id).attr('autocomplete', 'OFF')[0];
    $(input.form).submit(Drupal.autocompleteSubmit);
    var ac = new Drupal.jsAC(input, acdb[uri]);
    ac.setSeparator('^^');
    // FIXME: hardcoded 'mid'
    var id_input_id = getIdInputId(input_id, 'mid');
    // tell the ac where the "ID input" field is...
    ac.setIdInput($('#' + id_input_id)[0]);
    // --------------------------------------------------------------
    $(this).addClass('autocomplete-processed');
  });
};


// --------------------------------- new and overridden methods, functions


Drupal.jsAC.prototype.setSeparator = function (sep) {
    this.separator = sep;
};


/**
 * The "ID input" is a hidden input element that receives the ID from 
 * the autocompletion.  You must create it manually, and its name follows 
 * a strict naming convention in relation to the primary (autocompleted)
 * text field (see getIdInputId() below).
 */
Drupal.jsAC.prototype.setIdInput = function (input) {
    this.idInput = input;
};


/**
 * value may contain multiple fields separated by this.separator.  
 * First field is used for the actual text field value, and the last 
 * field is the ID.
 */
Drupal.jsAC.prototype.setInputValue = function (value) {
    this.input.value = "";
    if (value) {
        var vals = value.split(this.separator);
        this.input.value = vals[0];
        this.idInput.value = vals[vals.length - 1];
    }
};


/**
 * Puts the currently highlighted suggestion into the autocomplete field
 */
Drupal.jsAC.prototype.select = function (node) {
    // TG: changed the line below to call setInputValue()
    this.setInputValue(node.autocompleteValue);
};


/**
 * Hides the autocomplete suggestions
 */
Drupal.jsAC.prototype.hidePopup = function (keycode) {
    // Select item if the right key or mousebutton was pressed
    if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
        // TG: changed the line below to call setInputValue()
        this.setInputValue(this.selected.autocompleteValue);
    }
    // Hide popup
    var popup = this.popup;
    if (popup) {
        this.popup = null;
        $(popup).fadeOut('fast', function() { $(popup).remove(); });
    }
    this.selected = false;
};

/**
 * the name is confusing, but there is basically another input element which
 * is called the "ID input", which receives the (hidden) ID from the autocompletion.
 * This function calculates the ID of the ID input field, given the ID of the
 * actual (autocompleting) input field.
 */
function getIdInputId (input_id, suffix) {
    var dashpos = input_id.lastIndexOf('-');
    if (dashpos < 0)
        return suffix;
    else
        return input_id.substring(0, dashpos + 1) + suffix;
}

