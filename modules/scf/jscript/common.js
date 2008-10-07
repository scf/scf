/***
 * Client side functionality
 */
if (Drupal.jsEnabled) { 

  $(document).ready( function() {
        
    /**
     * Activation of scf them admin region
     */
    $('#admin_region').hide();
    $(document).keypress(function(e) {
      var keycode = e.keyCode? e.keyCode : e.charCode
      if (keycode == 27) $('#admin_region').toggle();
    });
    $('#admin_region').click(function(){
      //$(this).hide();
    });

    /**
     * Animation effects for scf theme messages
     */
    $('.messages h3').click(function(){
      $(this).parent().slideUp("fast"); return false;
    });

    /**
     * Animation effects for action links
     */
		$('#mission').addClass('action-link-enabled');
		$('.node.teaser').addClass('action-link-enabled');
    $('div.block').addClass('action-link-enabled');
		$('#forum table.forum-list td.forum').addClass('action-link-enabled');
		$('#forum table.forum-topic-list td.title').addClass('action-link-enabled');
    $('#account_information div.block').removeClass('action-link-enabled');
		
		$('.action-link-enabled').hover(
		  function() {
			  $(this).addClass('action-link-selected');
				$(this).children('.action_links').show();
		  },
			function() {
        $(this).removeClass('action-link-selected');
        $(this).children('.action_links').hide();
      }
		);
		
  });

}
