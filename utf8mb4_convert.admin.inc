<?php

function utf8mb4_convert_profile_list() {
  //$db_status = utf8mb4_convert_check_db();
  //$return = $db_status . "\n";
  $profile_list = db_select('utf8mb4_convert_profiles', 'ucp')
    ->fields('ucp')
    ->execute()
    ->fetchAssoc();
  if(!empty($profile_list)) {
    foreach($profile_list as $profile) {

    }
  }
  else {
    $return .= 'No profiles have been created yet.';
  }
  $return .= print_r($profile_list, TRUE);
  return $return;
}

function utf8mb4_convert_create_profile(){
  return drupal_get_form('utf8mb4_convert_profile_form');
}

function utf8mb4_convert_profile_form($form, &$form_state){
  $form = array();
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Profile name'),
    '#description' => t('Machine name used with drush command.  Letters, numbers and underscore only.'),
    '#default_value' => 'default',
    '#size' => 12,
    '#maxlength' => 32,
    '#required' => TRUE,
  );
  $form['drupal_tables_only'] = array(
    '#type' => 'radios',
    '#title' => t('Tables to convert to utf8mb4'),
    '#default_value' => 1,
    '#options' => array(0 => t('Convert all tables'), 1 => t('Convert tables defined in Drupal\'s schema')),
    '#description' => t('Default behaviour for table selection, can be overriden per table after creating profile.'),
    '#access' => 'administer utf8mb4 convert',
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  $form['#validate'] = array('utf8mb4_convert_profile_form_validate');
  $form['#submit'] = array('utf8mb4_convert_profile_form_submit');
  utf8mb4_convert_get_drupal_schema_list();
  return $form;
}

function utf8mb4_convert_profile_form_validate($form, &$form_state) {
  $profile_name = $form_state['values']['name'];
  //$drupal_tables_only = $form_state['values']['drupal_tables_only'];
  if(!preg_match('%^([A-Za-z0-9_])+$%', $profile_name)) {
    form_set_error('profile_name', t('Profile name can only contain letters, numbers, and underscore ("_") characters.'));
  }
  else {
    $query = db_query('SELECT name FROM {utf8mb4_convert_profiles} WHERE name = :name LIMIT 1', array(':name' => $profile_name));
    $record = $query->fetchField(0);
    $profile_name_id = isset($form_settings['values']['profile_name_id']) ? $form_settings['values']['profile_name_id'] : FALSE;
    if ($profile_name_id && ($profile_name_id !== $profile_name)) {

    }
  }
}

function utf8mb4_convert_profile_form_submit($form, &$form_state) {
  $profile_name = $form_state['values']['name'];
  $drupal_tables_only = $form_state['values']['drupal_tables_only'];
  $query = db_query('SELECT name FROM {utf8mb4_convert_profiles} WHERE name = :name LIMIT 1', array(':name' => $profile_name));
  $record = $query->fetchField(0);
  //drupal_set_message('test' . "$record");

}

function utf8mb4_convert_get_drupal_schema_list() {
  global $databases;
  drupal_set_message('<pre>' . print_r($databases, true) . '</pre>');
}

function utf8mb4_convert_check_db() {
  $results = array();
  $status = '';

  //check MySQL variables
  $mysql_variables = db_query('SHOW VARIABLES');

  $req_vars = array(
    'innodb_large_prefix' => array('setting' => '', 'valid_setting' => 'ON', 'severity' => 2),
    'innodb_file_format' => array('setting' => '', 'valid_setting' => 'Barracuda', 'severity' => 2),
    'innodb_file_per_table' => array('setting' => '', 'valid_setting' => 'ON', 'severity' => 2),
    'character_set_client' => array('setting' => '', 'valid_setting' => 'utf8mb4', 'severity' => 2),
    'collation_connection' => array('setting' => '', 'valid_setting' => 'utf8mb4_general_ci', 'severity' => 2),
    'character_set_database' => array('setting' => '', 'valid_setting' => 'utf8mb4', 'severity' => 1),
    'collation_database' => array('setting' => '', 'valid_setting' => 'utf8mb4_general_ci', 'severity' => 1),
    'character_set_server' => array('setting' => '', 'valid_setting' => 'utf8mb4', 'severity' => 1),
    'collation_server' => array('setting' => '', 'valid_setting' => 'utf8mb4_general_ci', 'severity' => 1),
    'version' => array('setting' => '', 'valid_setting' => '5.5.14', 'compare' => '>', 'severity' => 2),
  );

  //$status .= print_r($mysql_variables, TRUE);
  if (is_object($mysql_variables)) {
    foreach ($mysql_variables as $val) {
      $name = $val->Variable_name;
      $value = $val->Value;
      if(array_key_exists($name, $req_vars)) {
        $compare = isset($req_vars[$name]['compare']) ? $req_vars[$name]['compare'] : 0;
        $valid_setting = $req_vars[$name]['valid_setting'];
        $req_vars[$name]['setting'] = $value;
        $row_status = FALSE;
        if ($req_vars[$name]['valid_setting'] == $value
          || ($compare && version_compare($value, $valid_setting, $compare))) {
          $results[$name] = array('title' => $name, 'value' => 'Passed',);
          $row_status = TRUE;
        }
        else {
          $results[$name] = array('title' => $name, 'value' => 'Failed',);
        }
        $results[$name]['found'] = $name . "=" . $value;
        $results[$name] = array_merge($results[$name], utf8mb4_convert_check_db_status_string($name, $row_status, $compare));
      }
    }
  }
  else {
    $status .= 'Unable to Query MySQL variables from the database to confirm utf8mb4 capabilities.';
  }
  if(is_array($results)) {
    foreach ($results as $rkey => $rvals) {
      if(!empty($rvals['found'])) {
        $results[$rkey]['description'] .= "  Found: " . $rvals['found'];
      }
    }
    //Use the status report page's theme
    $status .= theme('status_report', array('requirements' => $results));
  }

  return $status;
}

function utf8mb4_convert_check_db_status_string($key, $status, $compare){
  $strings = array(
    'innodb_large_prefix' => array(
      'title' => 'MySQL server setting: innodb_large_prefix',
      'severity' => 2,
      'description' => 'MySQL InnoDB must be configured to support large prefixes.',
    ),
    'innodb_file_format' => array(
      'title' => 'MySQL server setting: innodb_file_format',
      'severity' => 2,
      'description' => 'MySQL InnoDB must be configured to support the Barricuda file format.',
    ),
    'innodb_file_per_table' => array(
      'severity' => 2,
      'description' => 'MySQL InnoDB must be configured to store individual files per table.',
    ),
    'character_set_client' => array(
      'severity' => 2,
      'description' => 'MySQL client must be configured to utilize the utf8mb4 character set.',
    ),
    'collation_connection' => array(
      'severity' => 2,
      'description' => 'MySQL client needs to use a utf8mb4 collation.',
    ),
    'character_set_database' => array(
      'severity' => 1,
      'description' => 'The currently configured character set for the database needs to be utf8mb4.  This can be overridden in the converted MySQL dump file.',
    ),
    'collation_database' => array(
      'severity' => 1,
      'description' => 'The currently configured collation for the database needs to use utf8.  This can be .',
    ),
    'character_set_server' => array(
      'severity' => 1,
      'description' => 'T',
    ),
    'collation_server' => array(
      'severity' => 1,
    ),
    'version' => array(
      'severity' => 2,
      'description' => 'The database server version should be greater than 5.5.14.'
    ),
  );
  if (array_key_exists($key, $strings)) {
    $return = $strings[$key];
    if ($status) {
      unset($return['severity']);
    };
    return $return;
  }
  else {
    return array();
  }
}
 ?>
