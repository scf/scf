<?php
// $Id: join.handlers.inc,v 1.2 2008/05/20 00:19:03 merlinofchaos Exp $
/**
 * @file
 * Views' join handlers.
 */
/**
 * @defgroup views_join_handlers Views' join handlers
 * @{
 * Handlers to tell Views how to join tables together.

 * Here is how you do complex joins:
 *
 * @code
 * class views_join_complex extends views_join {
 *   // PHP 4 doesn't call constructors of the base class automatically from a
 *   // constructor of a derived class. It is your responsibility to propagate
 *   // the call to constructors upstream where appropriate.
 *   function construct($table, $left_table, $left_field, $field, $extra = array(), $type = 'LEFT') {
 *     parent::construct($table, $left_table, $left_field, $field, $extra, $type);
 *   }
 *
 *   function join($table, &$query) {
 *     $output = parent::join($table, $query);
 *   }
 *   $output .= "AND foo.bar = baz.boing";
 *   return $output;
 * }
 * @endcode
 */
/**
 * A function class to represent a join and create the SQL necessary
 * to implement the join.
 *
 * This is the Delegation pattern. If we had PHP5 exclusively, we would
 * declare this an interface.
 *
 * Extensions of this class can be used to create more interesting joins.
 *
 * join definition
 *   - table: table to join (right table)
 *   - field: field to join on (right field)
 *   - left_table: The table we join to
 *   - left_field: The field we join to
 *   - type: either LEFT (default) or INNER
 *   - extra: Either a string that's directly added, or an array of items:
 *   - - table: if not set, current table; if NULL, no table. This field can't
 *       be set in the cached definition because it can't know aliases; this field
 *       can only be used by realtime joins.
 *   - - field: Field or formula
 *   - - operator: defaults to =
 *   - - value: Must be set. If an array, operator will be defaulted to IN.
 *   - - numeric: If true, the value will not be surrounded in quotes.
 *   - extra type: How all the extras will be combined. Either AND or OR. Defaults to AND.
 */
class views_join {
  /**
   * Construct the views_join object.
   */
  function construct($table = NULL, $left_table = NULL, $left_field = NULL, $field = NULL, $extra = array(), $type = 'LEFT') {
    $this->extra_type = 'AND';
    if (!empty($table)) {
      $this->table = $table;
      $this->left_table = $left_table;
      $this->left_field = $left_field;
      $this->field = $field;
      $this->extra = $extra;
      $this->type = strtoupper($type);
    }
    else if (!empty($this->definition)) {
      // if no arguments, construct from definition.
      // These four must exist or it will throw notices.
      $this->table = $this->definition['table'];
      $this->left_table = $this->definition['left_table'];
      $this->left_field = $this->definition['left_field'];
      $this->field = $this->definition['field'];
      if (!empty($this->definition['extra'])) {
        $this->extra = $this->definition['extra'];
      }
      if (!empty($this->definition['extra type'])) {
        $this->extra = strtoupper($this->definition['extra_type']);
      }

      $this->type = !empty($this->definition['type']) ? strtoupper($this->definition['type']) : 'LEFT';
    }
  }

  /**
   * Build the SQL for the join this object represents.
   */
  function join($table, &$query) {
    $left = $query->get_table_info($this->left_table);
    $output = " $this->type JOIN {" . $this->table . "} $table[alias] ON $left[alias].$this->left_field = $table[alias].$this->field";

    // Tack on the extra.
    if (isset($this->extra)) {
      if (is_array($this->extra)) {
        $extras = array();
        foreach ($this->extra as $info) {
          $extra = '';
          // Figure out the table name. Remember, only use aliases provided
          // if at all possible.
          $join_table = '';
          if (!array_key_exists('table', $info)) {
            $join_table = $table['alias'] . '.';
          }
          elseif (isset($info['table'])) {
            $join_table = $info['table'] . '.';
          }

          // And now deal with the value and the operator.  Set $q to
          // a single-quote for non-numeric values and the
          // empty-string for numeric values, then wrap all values in $q.
          $raw_value = $this->db_safe($info['value']);
          $q = (empty($info['numeric']) ? "'" : '');

          if (is_array($raw_value)) {
            $operator = !empty($info['operator']) ? $info['operator'] : 'IN';
            // Transform from IN() notation to = notation if just one value.
            if (count($raw_value) == 1) {
              $value = $q . array_shift($raw_value) . $q;
              $operator = $operator == 'NOT IN' ? '!=' : '=';
            }
            else {
              $value = "($q" . implode("$q, $q", $raw_value) . "$q)";
            }
          }
          else {
            $operator = !empty($info['operator']) ? $info['operator'] : '=';
            $value = "$q$raw_value$q";
          }
          $extras[] = "$join_table$info[field] $operator $value";
        }

        if ($extras) {
          if (count($extras) == 1) {
            $output .= ' AND ' . array_shift($extras);
          }
          else {
            $output .= ' AND (' . implode(' ' . $this->extra_type . ' ', $extras) . ')';
          }
        }
      }
      else if ($this->extra && is_string($this->extra)) {
        $output .= " AND ($this->extra)";
      }
    }
    return $output;
  }

  /**
   * Ensure that input is db safe. We only check strings and ints tho
   * so something that needs floats in their joins needs to do their
   * own type checking.
   */
  function db_safe($input) {
    if (is_array($input)) {
      $output = array();
      foreach ($input as $value) {
        if (empty($info['numeric'])) {
          $output[] = db_escape_string($value);
        }
        else {
          $output[] = intval($value);
        }
      }
    }
    else if (empty($info['numeric'])) {
      $output = db_escape_string($input);
    }
    else {
      $output = intval($input);
    }

    return $output;
  }
}

/**
 * @}
 */

