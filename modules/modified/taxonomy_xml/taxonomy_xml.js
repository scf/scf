/**
 * UI addition to enhance the inport source switcher
 */

if (Drupal.jsEnabled) {
  $(document).ready(function() {

    $('#data_source').addClass('filtered');
    $('#source_selector').change(
      /* 
      When the select changes, we change the class of the containing fieldset.
      This then uses css to hide the unwanted form elements. 
      */
      function(){
        var methods = new Array('none', 'upload-file', 'batch-upload-file', 'url', 'service');
        for(var m in methods) {
          $('#data_source').removeClass(methods[m]);
        }
        $('#data_source').addClass(this.value)
          .animate({opacity:.5}, 200)
          .animate({opacity:1}, 200)
      }
    );
    // Trigger the filter to update the current display
    $('#source_selector').trigger('change');
  })
}