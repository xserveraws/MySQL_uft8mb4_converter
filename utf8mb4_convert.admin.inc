<?php

function utf8mb4_convert_main(){
  return drupal_get_form('utf8mb4_convert_profile_form');
  //return 'test';
}

function utf8mb4_convert_profile_form($form, &$form_state){
  $form = array();
  $form['drupal_tables_only'] = array(
    '#type' => 'radios',
    '#title' => t('Tables to convert to utf8mb4'),
    '#default_value' => 1,
    '#options' => array(0 => t('Convert all tables'), 1 => t('Convert tables defined in Drupal\'s schema')),
    '#description' => t('Default behaviour for table selection, can be overriden individually by table'),
    '#access' => 'administer utf8mb4 convert',
  );
  return $form;
}
 ?>
