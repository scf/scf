// Stolen from poll module:
// If we're using the javascript version (99% use-case), change the button
// title to 'Add another participant' to reflect the javascript behavior.
if (Drupal.jsEnabled) {
    $(document).ready(function() {
        $('span.named_content').each(function () {
            var ref = $(this).attr("noderef");
            // HACK
            if (ref != null && ref.indexOf("entrezgene=") == 0) {
                var egid = ref.substr(11);
                var np_href = "http://purl.org/commons/record/ncbi_gene/" + egid;
                var href = "/nodeproxy/get/gene?sc_uri=" + np_href;
                $(this).wrapInner('<a href="' + href + '"></a>');
            }
        });
    });
}
