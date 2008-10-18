/***
 * @file IE fixes
 */
if (Drupal.jsEnabled) {

  $(document).ready( function() {
  
    /**
     * Fix child selector issues in IE6
     */
    $(".box > div").addClass('box-inner');
    $(".node.teaser.sticky > div").addClass('node-teaser-sticky');
    $(".pubgroup.teaser .content > img").addClass('pubgroup-teaser-content-img');
    $(".pubgroup .content > img").addClass('pubgroup-content-img');
    $(".block > div > h2").addClass('block-div-h2');
    $(".block > div > h2 .action_links").addClass('block-div-h2-action_links');
    $(".pubnode.nlm > h2").addClass('pubnode-nlm-h2');
    $(".member.teaser.odd > div").addClass("member-teaser-odd");
    $(".member.teaser.even > div").addClass("member-teaser-even");
    $("td > a.add_term").addClass('td-a-add_term');
    
    /**
     * Fix chained class issues in IE6
     */
    $(".node.teaser.sticky").css('border', '1px solid #dcdcdc');
    $(".node.not-teaser.sticky").css('border', '0 !important');
    $(".node.odd, .node.even").css('width','100%');
    $(".member.teaser.odd, .member.teaser.even").css('width','50%');
    $("th.bio-relationships, th.bio-comments").css({ height:'20px', textIndent:'-999em'});

    /**
     * Fix clear:both problem in IE6 by inserting additional clearing blocks
     */
    $("body.page-member .node.member.teaser.even").after("<div style='clear:both'></div>");
    
  });
  
}