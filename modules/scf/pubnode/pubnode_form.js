// Stolen from poll module:
// If we're using the javascript version (99% use-case), change the button
// title to 'Add another contributor' to reflect the javascript behavior.
if (Drupal.jsEnabled) {
    $(document).ready(function() {
        $('#edit-pubnode-add-contributor').val(Drupal.t('Add another contributor'));
    });
}
