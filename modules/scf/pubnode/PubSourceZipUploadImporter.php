<?php

pubnode_load_include("PubSourceImporter.php");

class PubSourceZipUploadImporter extends PubSourceImporter {
  
  protected $fieldname;

  public function __construct ($fieldname = "upload_main") {
    $this->fieldname = $fieldname;
  }

  public function willImport ($form_state) {
    $fieldname = $this->fieldname;
    if (isset($_FILES['files']) && $_FILES['files']['name'][$fieldname] && is_uploaded_file($_FILES['files']['tmp_name'][$fieldname])) {
      $filename = trim(basename($_FILES['files']['name'][$fieldname]), '.');
      return preg_match('{\.zip$}', $filename);
    }
    return FALSE;
  }

  /**
   * 'upload_zip' upload field must refer to a zip file containing 
   *   all files for the pubnode (may be at top-level or inside a subdirectory)
   */
  public function import (&$form_state, $docid = NULL) {
    $validators = array(
      'file_validate_extensions' => array('zip'),
      // NOTE: you must also adjust max upload size in php.ini
      // AND (I think) set the right uploading perms per role in uploads admin area
      'file_validate_size' => array(100000000, 0)
    );
    if ($file = file_save_upload($this->fieldname, $validators, file_directory_temp(), FILE_EXISTS_REPLACE)) {
      $zip = new ZipArchive();
      if ($zip->open($file->filepath) !== TRUE) {
        form_set_error(t("Cannot open !file", array("!file" => $file->filename)), 'error');
        return FALSE;
      }
      // else
      if (empty($docid)) {
        $docid = $this->hashFile($file->filepath);
      }
      $pubpath = $this->constructPubPath($docid);
      //drupal_set_message("PUBPATH: " . $pubpath);
      file_check_directory($pubpath, FILE_CREATE_DIRECTORY);
      $zip->extractTo($pubpath);
      drupal_set_message(t("Extracted !num files to directory !dir",
                           array('!num' => $zip->numFiles, '!dir' => $pubpath)));
      $zip->close();
      $this->pubpath = $pubpath;
      return TRUE;
    }
    // else validations failed and error message will be set by upload function
    return FALSE;
  }

}
