	var id = 0;
	var windowtarget;

	function searchwindow(text, node_id) {
		if (!windowtarget) {
			if (window.opener)
				windowtarget = window.opener;
			else {
				windowtarget = window.open('/node/' + node_id);
			}
		}
	
		windowtarget.search(text, id++);
	}
