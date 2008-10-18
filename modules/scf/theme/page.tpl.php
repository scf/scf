<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language ?>" lang="<?php print $language->language ?>">

<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
</head>
<body class="<?php print $body_classes; ?> <?php print $url_path_token; ?> <?php print $url_path_alias_token; ?>">
  <div id="page_wrapper">

    <div id="header_wrapper">
      <div id="header">
<?php if (!empty($header)): ?>
        <div id="header_region"><?php print $header; ?></div>
<?php endif; ?>
        <div id="site_name_slogan">
<?php if (!empty($site_name)): ?>
          <h1>
            <a href="<?php print $base_path ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
          </h1>
<?php endif; ?>
<?php if (!empty($site_slogan)): ?>
          <div id="site_slogan"><?php print $site_slogan; ?></div>
<?php endif; ?>
        </div>
        
<?php if (!empty($account)): ?>
        <div id="account_information"><?php print $account; ?></div>
<?php endif; ?>
<?php if (!empty($search_box)): ?>
          <div id="search_box"><?php print $search_box; ?></div>
<?php endif; ?>
      </div> <!-- /header -->
    </div> <!-- /header_wrapper -->

    
    <div id="nav_wrapper" class="clear-block">
      <div id="nav">
      
<?php if (!empty($primary_links)): ?>
        <div id="nav_primary">
          <?php print theme('links', $primary_links); ?>
        </div> <!-- /nav-primary -->
<?php endif; ?>
        
      </div>
    </div> <!-- /nav_wrapper -->
    

    <div id="body_wrapper" class="clear-block">
      <div id="body_">

        <div id="sidebar_left_wrapper" class="sidebar">
          <div id="sidebar_left">
<?php if (!empty($mission)): ?>
            <div id="mission_wrapper" class="box">
              <div id="mission" class="box-inner">
                <?php print $mission; ?>
  <?php // Example of manually inserting an admin link for the mission ?>
  <?php // statement. Ideally, you should put this into the variables  ?>
  <?php // using a preprocess function.                                ?>
  <?php if (user_access('administer site configuration')): ?>
                <?php $links['mission_edit'] = array(
                  'title' => t('Edit'), 
                  'href' => 'admin/settings/site-information', 
                  'attributes' => array('title' => t('Edit the site configuration which includes this mission statement')),
                  'query' => drupal_get_destination()); ?>
                <?php print '<div class="clear-block">' . theme('links', $links, array('class' => 'action_links links inline')) . '</div>'; ?>
  <?php endif; ?>
              </div>
            </div>
<?php endif; ?>
            <?php print $left; ?>
          </div> <!-- /sidebar_left -->
        </div> <!-- /sidebar_left_wrapper -->
  
        <div id="main_wrapper" class="clear-block">
          <div id="main">
<?php if (!empty($breadcrumb)): ?>
            <div id="breadcrumb"><?php print $breadcrumb; ?></div>
<?php endif; ?>
            <div id="content">
              <?php if (!empty($display_title)) { ?><h2 class="title clear-block" id="page-title"><?php print $display_title; ?></h2>
              <?php } else if (!empty($title)) { ?><h2 class="title clear-block" id="page-title"><?php print $title; ?></h2><?php } ?>
              <?php if (!empty($tabs)): ?><div class="tabs clear-block"><?php print $tabs; ?></div><?php endif; ?>
              <?php if (!empty($messages)): print $messages; endif; ?>
              <?php if (!empty($help)): print $help; endif; ?>
              <div id="content-content" class="clear">
                <?php print $content; ?>
              </div> <!-- /content-content -->
              <?php print $feed_icons; ?>
            </div> <!-- /content -->
          </div> <!-- /main -->
        </div> <!-- /main_wrapper -->
        
        <div id="sidebar_right_wrapper" class="sidebar">
          <div id="sidebar_right">
            <?php print $right; ?>
          </div> <!-- /sidebar_right -->
        </div> <!-- /sidebar_right_wrapper -->

      </div>
    </div> <!-- /body_wrapper -->

    <div id="footer_wrapper">
      <div id="footer">
        <p id="badge">Powered by the Scientific Collaborative Framework (SCF).</p>
        <div class="content">
          <?php print $footer_message; ?>
          <?php if (!empty($footer)): print $footer; endif; ?>
        </div>
      </div> <!-- /footer -->
    </div> <!-- /footer_wrapper -->

    <?php print $closure; ?>

  </div> <!-- /page_wrapper -->

<?php if ($user->uid == 1): ?>
  <div id="admin_region"><?php print $site_admin; ?></div>
<?php endif; ?>
  
</body>
</html>
