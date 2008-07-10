<?php

/**
 * Base class for SCF modules
 */
abstract class ScfModule {
  
  public $name;

  protected function __construct ($name) {
    $this->name = $name;
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * scf's admin_settings_subform hook
   * 
   * default impl just returns print-friendly checkbox
   ****************************************************************************/
  public function adminSettingsSubform () {
  }

  // ------------------------------------------------------- template methods

  // ------------------------------------------------------- utility
  
  /**
   * add a CSS file (defaults to 'modulename.css') in module's path
   */
  protected function addCss ($path = NULL) {
    if ($path === NULL) {
      $path = '/' . $this->name . '.css';
    }
    drupal_add_css(drupal_get_path('module', $this->name) . $path);
  }

}
