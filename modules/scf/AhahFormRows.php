<?php

abstract class AhahFormRows {
  
  public $name;
  protected $pluralName;
  protected $wrapperName;
  protected $nodeType;
  
  public function __construct ($nodeType, $rowType) {
    $this->name = $rowType;
    $this->pluralName = $rowType . 's';
    $this->wrapperName = $rowType . '_wrapper';
    $this->nodeType = $nodeType;
  }

  protected abstract function defineFormRows (&$node, &$form_state);

  /**
   * Create a single row with no indices, values, reference data or default values
   */
  protected abstract function defineFormRow ();

  protected abstract function blankRowValuesArray ();

  /**
   * Make sure row has the right settings for index $idx
   */
  protected abstract function setRowIndex (&$rowForm, $idx);

  /**
   * (AHAH) set reference data for new row based on AHAH post
   * TODO: use actual POST instead of cached form values
   */
  protected function setReferenceData (&$rowForm, $form) {
    // do nothing by default
  }

  protected function setFormRowValues (&$formRow, $values, $property = '#default_value') {
    foreach (element_children($formRow) as $key) {
      if (isset($values[$key]))
        // special hack for fieldset containing filter radios...
        if (isset($formRow[$key]['#type']) && $formRow[$key]['#type'] == 'fieldset') {
          // filter radio buttons use value of parent element, sorta
          $this->setFilterRadioValues($formRow[$key], $values[$key], $property);
          // also recurse to get any non-radio real fields
          $this->setFormRowValues($formRow[$key], $values, $property);
        } else {
          $formRow[$key][$property] = $values[$key];
        }
      else // recurse to make sure we get everything
        $this->setFormRowValues($formRow[$key], $values, $property);
    }
  }

  protected function setFilterRadioValues (&$element, $value, $property = '#default_value') {
    foreach (element_children($element) as $key) {
      // special hack for fieldset containing filter radios...
      if (isset($element[$key]['#type']) && $element[$key]['#type'] == 'radio') {
        $element[$key][$property] = $value;
      }
    }
  }

  /**
   * copy the '#value' properties from one row's fields to another's
   */
  protected function copyRowValuesFromTo (&$form, $from, $to) {
    $wrap = $this->wrapperName;
    $plur = $this->pluralName;
    $fields = array_keys($this->blankRowValuesArray());
    $subform = &$form[$wrap][$plur];
    foreach ($fields as $field) {
      $fromVal = empty($subform[$from][$field]['#value']) ? '' : $subform[$from][$field]['#value'];
      $subform[$to][$field]['#value'] = $fromVal;
    }
  }

  /**
   * duplicate a whole row
   */
  protected function copyRowFromTo (&$form, $from, $to) {
    $wrap = $this->wrapperName;
    $plur = $this->pluralName;
    $form[$wrap][$plur][$to] = $form[$wrap][$plur][$from];
    $this->setRowIndex($form[$wrap][$plur][$to], $to);
  }

  /**
   * (AHAH) count how many rows are in the form as submitted
   */
  protected function getRowCount () {
    return count($_POST[$this->pluralName]);
  }

  /**
   * (AHAH) get form ID to lookup previously cached form
   */
  protected function getFormBuildId () {
    return $_POST['form_build_id'];
  }

  /**
   * (AHAH) retrieve previously cached form
   */
  protected function getCachedForm (&$form_state) {
    $fbid = $this->getFormBuildId();
    return form_get_cache($fbid, $form_state);
  }

  /**
   * (AHAH) return modified form to cache after handling AHAH request
   */
  protected function storeFormInCache ($form, &$form_state) {
    $fbid = $this->getFormBuildId();
    form_set_cache($fbid, $form, $form_state);
  }

  /**
   * (AHAH) take the rebuilt form, extract just the portion relevant
   * to this AHAH request, render it and send it back as JSON.
   */
  protected function outputRows ($form) {
    // Render the new output.
    $subform = $form[$this->wrapperName][$this->pluralName];
    // Prevent duplicate wrappers.
    unset($subform['#prefix'], $subform['#suffix']);

    $output = theme('status_messages') . drupal_render($subform);

    drupal_json(array('status' => TRUE, 'data' => $output));
  }    


  /**
   * (AHAH) Build the form again with new altered definition and submitted values
   */
  protected function rebuildForm ($form, &$form_state) {
    $form += array(
      '#post' => $_POST,
      '#programmed' => FALSE
    );
    return form_builder($this->nodeType . '_node_form', $form, $form_state);
  }

  /**
   * TODO: improve this!
   */
  protected function outputError () {
    drupal_json(array('status' => FALSE, 'data' => '<div>Error occurred...</div>'));
  }

  /**
   * (AHAH) Delete the row at position $idx
   * @param $idx
   *    index of the row to be deleted
   */  
  public function deleteRow ($idx = 0) {

    $rowCount = $this->getRowCount();
    // detect illegal state
    if ($rowCount < 1 || $idx < 0 || $idx >= $rowCount) {
      $this->outputError();
    } 

    $form_state = array('submitted' => FALSE);
    $form = $this->getCachedForm($form_state);

    // zero out this row, shifting all later ones up
    // can't do this simply because the array mixes indexed (numeric) keys with
    // '#property' keys...
    
    for ($i = $idx + 1; $i < $rowCount; $i++) {
      $this->copyRowFromTo($form, $i, $i - 1);
    }

    // form is rebuilt BEFORE we delete the last row
    $rebuilt = $this->rebuildForm($form, $form_state);

    // get rid of the last one
    unset($form[$this->wrapperName][$this->pluralName][$rowCount - 1]);

    $this->storeFormInCache($form, $form_state);

    // HACK: after form rebuild, must shift the actual field values around 
    for ($i = $idx + 1; $i < $rowCount; $i++) {
      $this->copyRowValuesFromTo($rebuilt, $i, $i - 1);
    }
    // and get rid of the last one in the rebuilt form too (!!)
    unset($rebuilt[$this->wrapperName][$this->pluralName][$rowCount - 1]);

    $this->outputRows($rebuilt);
  }
  
  /**
   * (AHAH) Insert a new row
   * @param $idx
   *    index of location where row is to inserted, or -1 to append at end
   */
  public function insertRow ($idx = -1) {

    $rowCount = $this->getRowCount();
    // add to end if $idx is negative
    if ($idx < 0) {
      $idx = $rowCount;
    }
    $idx = min($idx, $rowCount);

    $form_state = array('submitted' => FALSE);
    $form = $this->getCachedForm($form_state);

    $newRow = $this->defineFormRow();
    $blanks = $this->blankRowValuesArray();
    $this->setFormRowValues($newRow, $blanks);
    $this->setReferenceData($newRow, $form);
    $this->setRowIndex($newRow, $idx);

    // shift other rows down to make room
    for ($i = $rowCount - 1; $i >= $idx; $i--) {
      $this->copyRowFromTo($form, $i, $i + 1);
    }

    // insert the new one
    $form[$this->wrapperName][$this->pluralName][$idx] = $newRow;

    $this->storeFormInCache($form, $form_state);

    $form = $this->rebuildForm($form, $form_state);

    // HACK: after form rebuild, must shift the actual field values around 
    for ($i = $rowCount - 1; $i >= $idx; $i--) {
      $this->copyRowValuesFromTo($form, $i, $i + 1);
    }

    // point $newRow to the row in the rebuilt form
    $newRow = &$form[$this->wrapperName][$this->pluralName][$idx];
    // ANOTHER HACK: and zero out the new row's values
    $this->setFormRowValues($newRow, $blanks, '#value');
    // add 'ahah-new-content' class to added element
    $newRow['#attributes']['class'] =
      empty($newRow['#attributes']['class'])
      ? 'ahah-new-content'
      : $newRow['#attributes']['class'] .' ahah-new-content';

    $this->outputRows($form);
  }
  
}

