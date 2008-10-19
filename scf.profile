<?php
// $Id: $

/**
 * Return a description of the profile for the initial installation screen.
 *
 * @return
 *   An array with keys 'name' and 'description' describing this profile,
 *   and optional 'language' to override the language selection for
 *   language-specific profiles.
 */
function scf_profile_details() {
  return array(
    'name' => 'Science Collaboration Framework (SCF)',
    'description' => 'SCF is a reusable, semantically-aware toolkit for building on-line communities.'
  );
}

/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *   An array of modules to enable.
 */
function scf_profile_modules() {
  return array(
    // CORE
    'comment', 'dblog', 'help', 'menu', 'path', 'search', 'taxonomy', 'upload', 'forum', 

    // CONTRIB
    'ajax_pic_preview',
    'biblio',
    'blockclone',
    'logintoboggan',
    'reg_with_pic',
    'simplemenu',    
    'sioc',    
    'taxonomy_csv',
    
    // image et al. can't be installed here.  see below in scf_profile_tasks()
    //    'image',
    //    'image_gallery',
    //    'image_import',
    //    'image_im_advanced',
    
    // SCF
    'iic_util',
    'oo',
    'scf',
    'member',
    'taxonomy_bulk',
    'species',
    'bio_ontologies',
    'gene',
    'antibody',
    'modelorganism',
    'contributor',
    'interview',
    'newsarticle',
    'researchstatement',
    
    // node proxying
    'nodeproxy',
    'rdf', 
  );
}

/**
 * Return a list of tasks that this profile supports.
 *
 * @return
 *   A keyed array of tasks the profile will perform during
 *   the final stage. The keys of the array will be used internally,
 *   while the values will be displayed to the user in the installer
 *   task list.
 */
function scf_profile_task_list() {
}

/**
 * Perform any final installation tasks for this profile.
 *
 * The installer goes through the profile-select -> locale-select
 * -> requirements -> database -> profile-install-batch
 * -> locale-initial-batch -> configure -> locale-remaining-batch
 * -> finished -> done tasks, in this order, if you don't implement
 * this function in your profile.
 *
 * If this function is implemented, you can have any number of
 * custom tasks to perform after 'configure', implementing a state
 * machine here to walk the user through those tasks. First time,
 * this function gets called with $task set to 'profile', and you
 * can advance to further tasks by setting $task to your tasks'
 * identifiers, used as array keys in the hook_profile_task_list()
 * above. You must avoid the reserved tasks listed in
 * install_reserved_tasks(). If you implement your custom tasks,
 * this function will get called in every HTTP request (for form
 * processing, printing your information screens and so on) until
 * you advance to the 'profile-finished' task, with which you
 * hand control back to the installer. Each custom page you
 * return needs to provide a way to continue, such as a form
 * submission or a link. You should also set custom page titles.
 *
 * You should define the list of custom tasks you implement by
 * returning an array of them in hook_profile_task_list(), as these
 * show up in the list of tasks on the installer user interface.
 *
 * Remember that the user will be able to reload the pages multiple
 * times, so you might want to use variable_set() and variable_get()
 * to remember your data and control further processing, if $task
 * is insufficient. Should a profile want to display a form here,
 * it can; the form should set '#redirect' to FALSE, and rely on
 * an action in the submit handler, such as variable_set(), to
 * detect submission and proceed to further tasks. See the configuration
 * form handling code in install_tasks() for an example.
 *
 * Important: Any temporary variables should be removed using
 * variable_del() before advancing to the 'profile-finished' phase.
 *
 * @param $task
 *   The current $task of the install system. When hook_profile_tasks()
 *   is first called, this is 'profile'.
 * @param $url
 *   Complete URL to be used for a link or form action on a custom page,
 *   if providing any, to allow the user to proceed with the installation.
 *
 * @return
 *   An optional HTML string to display to the user. Only used if you
 *   modify the $task, otherwise discarded.
 */
function scf_profile_tasks(&$task, $url) {

  $themes = array('scf_stub', 'scf_demo');

  // forum
  variable_set('node_options_forum', array('status'));
  _scf_comments_on('forum');
  // disable attachments
  variable_set('upload_forum', '0');

  // member
  variable_set('node_options_member', array('status'));
  // don't allow comments on members(?)
  _scf_comments_off('member');
  // disable attachments
  variable_set('upload_member', '1');

  // image
  variable_set('image_default_path', 'images');
  drupal_install_modules(array('image', 'image_gallery', 'image_import', 'image_im_advanced'));
  variable_set('node_options_image', array('status'));
  _scf_comments_off('image');
  variable_set('upload_image', 1);
  variable_set('image_max_upload_size', '5000');
  variable_set('image_sizes',
    array (
      '_original' => 
      array (
        'label' => 'Original',
        'operation' => 'scale',
        'width' => '',
        'height' => '',
        'link' => '1',
        ),
      'thumbnail' => 
      array (
        'label' => 'Thumbnail',
        'operation' => 'scale',
        'width' => '100',
        'height' => '100',
        'link' => '1',
        ),
      'preview' => 
      array (
        'label' => 'Preview',
        'operation' => 'scale',
        'width' => '640',
        'height' => '640',
        'link' => '1',
        )
    ));
  variable_set('image_attach_existing', '1');
  variable_set('image_gallery_nav_vocabulary', '');

  // gene
  variable_set('node_options_gene', array('status'));
  _scf_comments_off('gene');
  // disable attachments
  variable_set('upload_gene', 0);

  // interview
  variable_set('node_options_interview', array('status', 'revision'));
  _scf_comments_on('interview');

  // newsarticle
  variable_set('node_options_newsarticle', array('status', 'revision', 'promote'));
  _scf_comments_on('newsarticle');

  // researchstatement
  variable_set('node_options_researchstatement', array('status'));
  _scf_comments_on('researchstatement');

  // before installing pubnode/pubgroup, set some vars
  variable_set('pubgroup_forum_name', 'Books');
  variable_set('pubnode_forum_name', 'Chapters');
  drupal_install_modules(
    array(
      'pubgroup',
      'pubnode',
      'pubnode_nlm',
    )
  );

  // pubgroup
  variable_set('node_options_pubgroup', array('status'));
  _scf_comments_on('pubgroup');

  // pubnode
  variable_set('node_options_pubnode', array('status'));
  _scf_comments_on('pubnode');
  variable_set('pubnode_default_doctype', 'nlm');

  // install contrib RDF modules after rdf module installed, because
  // that's the only way they can recognize that ARC2 is installed 
  drupal_install_modules(array('rdf_db', 'rdf_import', 'rdf_export', 'rdf_schema', 'sparql'));

  // these modules in turn depend on rdf_import...
  drupal_install_modules(array('rdfnodeproxy', 'sciencecommons_geneproxy'));

  variable_set('filter_html_1', 1);
  variable_set('clean_url', '1');
  // not used if scf is usurping home page
  variable_set('default_nodes_main', '10');
  variable_set('teaser_length', '600');
  // node previewing is optional (set to 1 for required)
  variable_set('node_preview', '0');

  variable_set('javascript_parsed', array());
  variable_set('update_access_fixed', true);
  variable_set('image_toolkit', 'gd');

  // not sure what this does
  // can't blame prev. developer for not knowing
  // maybe has to do with: http://drupal.org/node/61760
  variable_set('array_filter', true);

  // file directory paths
  variable_set('file_directory_path', 'sites/default/files');
  variable_set('file_directory_temp', '/tmp');

  // LoginToboggan
  variable_set('logintoboggan_confirm_email_at_registration', '0');
  variable_set('logintoboggan_pre_auth_role', '2');
  variable_set('logintoboggan_redirect_on_register', '');
  variable_set('logintoboggan_redirect_on_confirm', '');
  variable_set('logintoboggan_login_successful_message', '0');
  variable_set('logintoboggan_minimum_password_length', '0');
  variable_set('logintoboggan_login_block_type', '1');
  variable_set('logintoboggan_login_block_message', '');
  variable_set('logintoboggan_login_with_email', '1');

  // predefined role constants (from bootstrap.inc):
  // define('DRUPAL_ANONYMOUS_RID', 1);
  // define('DRUPAL_AUTHENTICATED_RID', 2);
  db_query("INSERT INTO {role} (rid, name) VALUES (3, 'author')");
  variable_set('scf_author_rid', 3);
  db_query("INSERT INTO {role} (rid, name) VALUES (4, 'editor')");
  db_query("INSERT INTO {role} (rid, name) VALUES (5, 'admin')");

  $type_list = array(
    'antibody' => 'antibodies',
    'gene' => 'genes',
    'interview' => 'interviews',
    'model organism' => 'model organisms',
    'news article' => 'news articles',
    'publication' => 'publications',
    'pubgroup' => 'pubgroups',
    'research statement' => 'research statements'
  );

  $perms = array();
  // base perms for anonymous visitor
  $perms[] = 'access content, access comments, search content';
  // biblio perms (ill-named)
  $perms[] = 'show export links, show filter tab, show sort links, view full text';
  db_query("UPDATE {permission} SET perm = '%s' WHERE rid = 1", implode(', ', $perms));

  // extra perms for authenticated user
  $perms[] = 'post comments, post comments without approval, use advanced search, view uploaded files, change own username';
  // SCF-specific perms: these are still being worked on...
  $perms[] = 'browse member directory, create own member, edit own member, delete own member';
  $perms[] = 'create proxied nodes, search proxy sources';
  $perms[] = 'create proxied sciencecommons gene records';
  db_query("UPDATE {permission} SET perm = '%s' WHERE rid = 2", implode(', ', $perms));

  // extra perms for AUTHOR
  $perms[] = 'create forum topics, delete own forum topics, edit own forum topics, upload files';
  $perms[] = 'create images, edit own images, view original images';
  $perms[] = 'create biblio, edit own biblio entries, import from file';
  foreach ($type_list as $thing => $things) {
    $perms[] = "create $things, edit own $things, delete own $things";
  }
  db_query("INSERT INTO {permission} (rid, perm) VALUES (3, '%s')", implode(', ', $perms));

  // extra perms for EDITOR
  $perms[] = 'administer blocks, administer comments, administer forums, edit any forum topic, delete any forum topic';
  $perms[] = 'administer menu, administer content types, administer nodes, delete revisions, revert revisions, view revisions';
  $perms[] = 'administer search, access administration pages, access site reports, administer actions, administer files';
  $perms[] = 'administer site configuration, administer taxonomy, access user profiles, create url aliases, administer url aliases';
  $perms[] = 'edit images, administer images, import images, administer simplemenu, view simplemenu, administer users';
  $perms[] = 'edit all biblio entries';
  // SCF-specific perms: these are still being worked on...
  foreach ($type_list as $thing => $things) {
    $perms[] = "edit any $thing, delete any $thing";
  }
  $perms[] = 'create any member, edit any member, delete any member';
  db_query("INSERT INTO {permission} (rid, perm) VALUES (4, '%s')", implode(', ', $perms));

  // extra perms for ADMIN (NOT the same as user 1, which can do ANYTHING)
  $perms[] = 'switch users, administer filters, select different theme, administer permissions';
  foreach ($type_list as $thing => $things) {
    $perms[] = "administer $things";
  }
  $perms[] = 'administer node proxies, administer scf, administer members';
  db_query("INSERT INTO {permission} (rid, perm) VALUES (5, '%s')", implode(', ', $perms));

  // "Visitors can create accounts and no administrator approval is required."
  variable_set('user_register', '1');  
  // "Require e-mail verification when a visitor creates an account"
  variable_set('user_email_verification', '0');
  // "This text is displayed at the top of the user registration form
  // and is useful for helping or instructing your users."
  variable_set('user_registration_help', '');

  variable_set('user_signatures', '0');
  variable_set('user_pictures', '1');
  variable_set('user_picture_path', 'pictures');
  variable_set('user_picture_default', '');
  variable_set('user_picture_dimensions', '100x100');
  variable_set('user_picture_file_size', '50'); // KB
  variable_set('user_picture_guidelines', '');

  // role permissions for uploading
  // '0' means no restriction
  variable_set('upload_max_resolution', '0');
  // "Display attached files when viewing a post."
  variable_set('upload_list_default', '1');

  // Default settings (any auth user)
  variable_set('upload_extensions_default', 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp');
  // max size per upload (MB)
  variable_set('upload_uploadsize_default', '1');
  // total max upload size per user (MB)
  variable_set('upload_usersize_default', '1');

  // Author upload settings
  variable_set('upload_extensions_3', 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp zip');
  // max size per upload (MB)
  variable_set('upload_uploadsize_3', '8');
  // total max upload size per user (MB)
  variable_set('upload_usersize_3', '1000');

  // Editor upload settings
  variable_set('upload_extensions_4', 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp zip');
  // max size per upload (MB)
  variable_set('upload_uploadsize_4', '8');
  // total max upload size per user (MB)
  variable_set('upload_usersize_4', '10000');

  // Admin upload settings
  variable_set('upload_extensions_5', 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp zip');
  // max size per upload (MB)
  variable_set('upload_uploadsize_5', '8');
  // total max upload size per user (MB)
  variable_set('upload_usersize_5', '1000');

  $theme_settings = variable_get('theme_settings', array());
  $theme_settings['toggle_logo'] = 0;
  $theme_settings['toggle_name'] = 1;
  $theme_settings['toggle_slogan'] = 1;
  $theme_settings['toggle_mission'] = 1;
  $theme_settings['toggle_node_user_picture'] = 0;
  $theme_settings['toggle_comment_user_picture'] = 0;
  $theme_settings['toggle_search'] = 1;
  $theme_settings['toggle_favicon'] = 1;
  $theme_settings['toggle_primary_links'] = 1;
  $theme_settings['toggle_secondary_links'] = 1;
  $theme_settings['default_logo'] = 0;
  $theme_settings['logo_path'] = '';
  $theme_settings['logo_upload'] = '';
  $theme_settings['default_favicon'] = 1;
  $theme_settings['favicon_path'] = '';
  $theme_settings['favicon_upload'] = '';
  $theme_settings['op'] = 'Save configuration';

  // 'toggle_node_info_X' tells whether to display date and author information for node type X
  $theme_settings['toggle_node_info_member'] = 1;

  variable_set('theme_settings', $theme_settings);
  foreach ($themes as $theme) {
    variable_set('theme_' . $theme . '_settings', $theme_settings);
  }

  // SCF settings
  variable_set('scf_pub_title', 'SCF Book');
//  variable_set('scf_usurp_front_page', 1);

  // Site config
  variable_set('site_name', '');
  variable_set('site_slogan', 'The Science Collaboration Framework (SCF) is a reusable, semantically-aware toolkit for building on-line communities.');
  variable_set('site_footer', '<p>The <a href="http://iic.harvard.edu/projects/scf.html" title="IIC SCF project">Scientific Collaborative Framework</a> is a project of the <a href="http://iic.harvard.edu/" title="IIC web site">Initiative in Innovative Computing</a> at <a href="http://harvard.edu/">Harvard University</a>.</p>
  <p>SCF is licensed under the <a href="http://www.gnu.org/licenses/gpl-3.0.txt">GPL version 3</a> software license.</p>');
  variable_set('site_frontpage', 'node');

  variable_set('simplemenu_theme', 'blackblue');
  variable_set('anonymous', 'Anonymous');

  foreach ($themes as $theme) {
    // all pages
    _scf_block('user', '0', $theme, 'account');
    _scf_block('logintoboggan', '0', $theme, 'account', 0, '', '<none>', 1);
    // only actually populated if current page is a pubnode
    _scf_block('pubnode', 'toc', $theme, 'right', 0, '', '', -10);
    // _scf_block('menu', 'secondary-links', $theme, 'right', 0, '', '<none>', 1);

    // targeted to specific pages:
    // front only:
    _scf_block('newsarticle', 'listing', $theme, 'left', 1, '<front>', 'News', -10);
    _scf_block('pubgroup', 'listing', $theme, 'stemcenter_stembook_books', 1, '<front>', 'Books', -10);
    _scf_block('block', '1', $theme, 'stemcenter_stembook_intro', 1, '<front>', '<none>', -10, 1);

    // stembook page only
    _scf_block('blockclone', '2', $theme, 'left', 1, 'scf/pub', 'Books', -8, 1);
    _scf_block('blockclone', '1', $theme, 'stemcenter_stembook_intro', 1, 'scf/pub', '<none>', -9, 1);

    // stembook page and <front>
    _scf_block('comment', '0', $theme, 'right', 1, "<front>\r\nscf/pub", 'StemBook Buzz');
    _scf_block('interview', 'listing', $theme, 'stembook_features', 1, "<front>\r\nscf/pub", 'Features');
  }

  // add stembook intro block text to boxes table
  $stembook_intro = '<div id="stembook_intro"><h2>SCF</h2><p>This implementation of Science Collaboration Framework will be a comprehensive, open-access collection of original, peer-reviewed chapters.</p>';
  db_query("INSERT INTO {boxes} (bid, body, info, format) VALUES (1, '%s', 'StemBook Introduction', 2)", $stembook_intro);
  // blockclone record to handle cloning of stembook intro
  db_query("INSERT INTO {blockclone} (blockclone_id, mod_module, mod_delta) VALUES (1, 'block', '1')");
  db_query("INSERT INTO {blockclone} (blockclone_id, mod_module, mod_delta) VALUES (2, 'pubgroup', 'listing')");

  foreach ($themes as $theme) {
    system_theme_data();
    db_query("UPDATE {system} SET status = 1 WHERE type = 'theme' and name = '%s'", $theme);
    // run this after all the blocks set up above or it conflicts
    system_initialize_theme_blocks($theme);
  }
  variable_set('theme_default', 'scf_demo');

  _scf_primary_link('newsarticle/list', 'News');
  _scf_primary_link('scf/pub', 'Book');
  _scf_primary_link('forum', 'Forums');
  _scf_primary_link('scf/resources', 'Resources');
  _scf_primary_link('member/dir', 'Members');

  // _scf_secondary_link('node/add/member', 'Member profile');

  // Update the menu router information.
  menu_rebuild();
}

/**
 * Implementation of hook_form_alter().
 *
 * Allows the profile to alter the site-configuration form. This is
 * called through custom invocation, so $form_state is not populated.
 */
function scf_form_alter(&$form, $form_state, $form_id) {
  if ($form_id == 'install_configure') {
    // Set default for site name field.
    $form['site_information']['site_name']['#default_value'] = 'Science Collaboration Framework';
  }
}

/**
 * @param $type the node type to enable comments for
 * @param $mode
 *   possibilities are:
 *   COMMENT_MODE_FLAT_COLLAPSED => t('Flat list - collapsed'),
 *   COMMENT_MODE_FLAT_EXPANDED => t('Flat list - expanded'),
 *   COMMENT_MODE_THREADED_COLLAPSED => t('Threaded list - collapsed'),
 *   COMMENT_MODE_THREADED_EXPANDED => t('Threaded list - expanded')
 * @param $order
 *   possibilities are:
 *   COMMENT_ORDER_NEWEST_FIRST => t('Date - newest first'),
 *   COMMENT_ORDER_OLDEST_FIRST => t('Date - oldest first')
 */
function _scf_comments_on ($type, $mode = COMMENT_MODE_THREADED_EXPANDED, $order = COMMENT_ORDER_NEWEST_FIRST) {
  // default to show per page
  $num = 50;
  // Can users provide a unique subject for their comments?
  $subj = 1; // (enabled)
  // Force user to preview their comments?
  $preview = 1; // (required)
  // Location of comment submission form
  $loc = 0; // display form on sep page (1 means below post or other comments)
  variable_set('comment_' . $type, COMMENT_NODE_READ_WRITE);
  variable_set('comment_default_mode_' . $type, $mode);
  variable_set('comment_default_order_' . $type, $order);
  variable_set('comment_default_per_page_' . $type, $num);
  variable_set('comment_subject_field_' . $type, $subj);
  variable_set('comment_preview_' . $type, $preview);
  variable_set('comment_form_location_' . $type, $loc);
}

function _scf_comments_off ($type) {
  variable_set('comment_' . $type, COMMENT_NODE_DISABLED);
}

function _scf_block ($module, $delta, $theme, $region, $visibility = 0, $pages = '', $title = '<none>', $weight = 0, $cache = -1) {
  $sql = "INSERT INTO {blocks} (module, delta, theme, status, weight, region, visibility, pages, title, cache)";
  $sql .= " VALUES ('%s', '%s', '%s', 1, %d, '%s', %d, '%s', '%s', %d)";
  db_query($sql, $module, $delta, $theme, $weight, $region, $visibility, $pages, $title, $cache);
}

function _scf_primary_link ($href, $text) {
  static $weight = -1;
  $weight++;
  _scf_add_link('primary-links', $href, $text, $weight);
}

function _scf_secondary_link ($href, $text) {
  static $weight = -1;
  $weight++;
  _scf_add_link('secondary-links', $href, $text, $weight);
}

function _scf_add_link ($menu, $href, $text, $weight) {
  $link = array(
    'link_path' => $href,
    'link_title' => st($text),
    'menu_name' => $menu,
    'weight' => $weight,
    'options' => array(
      'attributes' => array(
        'title' => st($text)
      )
    )
  );
  menu_link_save($link);
}
