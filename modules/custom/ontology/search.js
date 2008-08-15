	function search(term, id) {
	// remove prior search results
	$('span.searched').each(function() { 
		$(this).replaceWith(this.innerHTML);
	});
    
	$('div.pubnode').remove('a#search_result_link');

	if (!term) return;  

	// look through the page content and find instances where the term occurs  
	$('div.pubnode p,div.pubnode div.ref').each(function() {
		var flag = 0;
		var html = $(this).html();
    	 
		var pieces = html.split(/\.\s/);
		for (var i = 0; i < pieces.length; i++) {
			var piece = pieces[i];
			var stripped_piece = piece.replace(/<[^>]+?>/g, '');
			stripped_piece = stripped_piece.replace(/^\s+|\s+$|\.$/g, '');

			if (stripped_piece == term) {
				pieces[i] = '<span class="searched"><a name="search_result' + id + '" id="search_result_link"></a>' + piece + '</span>';
				flag = 1;
			}
		}

		if (flag) {
			html = pieces.join('. ');
			$(this).html(html);
			return false;
		}   
	});
    
	self.location.hash = 'search_result' + id;
	window.scrollBy(0, -300);
}