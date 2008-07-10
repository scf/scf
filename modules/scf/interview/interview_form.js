// Stolen from poll module:
// If we're using the javascript version (99% use-case), change the button
// title to 'Add another participant' to reflect the javascript behavior.
if (Drupal.jsEnabled) {
    $(document).ready(function() {
        $('#edit-interview-add-participant').val(Drupal.t('Add another participant'));
    });
}
