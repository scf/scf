
$Id$

ENABLE THE THEME

1.  Goto admin/build/themes; enable HSCI StemCenter theme; disable all other 
    themes.
    
2.  Goto admin/build/themes/settings/stemcenter; turn on site slogan and 
    mission statement.
    
    
SET UP THE WELCOME COPY

1.  Goto admin/settings/site-information; Add slogan and mission statements...
Slogan: Where the StemCell Community starts
Mission:
<h2>Welcome to HSCI StemCenter</h2><p><strong>The Harvard Stem Cell Institute (HSCI) 
unites scientists at Harvard and its affiliated hospitals with the shared goal 
of advancing stem cell research to treat injury and disease and the analysis of 
related political, legal, and ethical issues.</strong></p><p>The HSCI StemCenter publishes 
StemBook which provides current, peer-reviewed chapters on topics related to 
stem cell biology; as well as timely perspectives, interviews, and news. It 
serves as a resource for the stem cell research community, educators, and the 
interested non-specialist as well as a tool to foster discussion and build 
community among its readers. The Stem Center also provides links to useful 
biological resources such as genes, antibodies and protocols; links to key 
events and ability to discuss about them.</p><p><a href="" title="">Become a 
member now</a></p>


SET UP THE ACCOUNT INFORMATION DISPLAY

1.  Goto admin/build/block; put user login and logintoboggan block into the
    account information field.
    
2.  Goto admin/build/block/configure/user/0; set title to <none> and the type 
    to Link.
    
3.  Goto admin/build/block/configure/logintoboggan/0; set title to <none>


SET UP THE SEARCH BOX DISPLAY

1.  Goto admin/build/modules; enable Search under 'Core - optional'

2.  Goto admin/user/permissions; enable 'search content' for unauthenticated 
    and authenticated users.

3.  Goto admin/build/themes/settings/stemcenter; turn on search box.

SET UP SECONDARY MENU IN THE RIGHT COLUMN

1.  Goto admin/build/block; move Secondary Menu to the Right sidebar; save!

2.  Goto admin/build/block/configure/menu/secondary-links; set block title to <none>

SET UP THE NEWS ARTICLES ON THE FRONT PAGE

1.  Goto admin/build/themes; enable and seet as default another them and then 
    switch back to the stemcenter theme to re-init any new regions.
    
2.  Goto admin/build/block; move New Article Listing to the Left sidebar; save!

3.  Goto admin/build/block/configure/newsarticle/0; set block title to <none>; 
    in visibility settings, show only on listed pages and put <front> into the 
    Pages field.
    
SET UP THE STEMBOOK INTRO ON THE FRONT PAGE

1.  Goto admin/build/block/add; add the following fields:
Block description:
StemBook Introduction
Block title:
<none>
Block Body:
<div id="stembook_intro"><p class="slogan">The Online Review of Online Stem 
Cell Biology</p><h2>StemBook</h2><p>StemBook is a comprehensive, open-access 
collection of original, peer-reviewed chapters covering topics related to Stem 
Cell Biology. <a class="read-more" href="">Read more</a></p><p>
<a class="contribute" href="">Find out how you can contribute&hellip;</a></p>
</div>
Input format: should be full text

2.  Goto admin/build/block; move StemBook Intro to the Stembook Intro region; save!

SET UP STEMBOOK FEATURE ON THE FRONT PAGE

1.  Goto admin/build/block; move Interview Llisting to the StemBook Feature 
    region; save!

2.  Goto admin/build/block/configure/interview/0; set block title to <none>.

SET UP STEMBOOK CHAPTERS ON THE FRONT PAGE

1.  Goto admin/build/block; movePublication Group listing to the StemBook 
    Chapters region; save!

2.  Goto admin/build/block/configure/pubgroup/0; set block title to Books.

SET UP COMMENTS ON THE FRONT PAGE

1.  Goto admin/build/block; move Recent comments to the Right sidebar region; save!

2.  Goto admin/build/block/configure/comment/0; set block title to StemBook buzz; 
    in visibility settings, show only on listed pages and put <front> into the 
    Pages field.

SET UP THE EDITOR ROLE TO ALLOW ACTION LINKS TO APPEAR

To come!