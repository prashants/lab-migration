<?php
// $Id$

function lab_migration_settings_form($form_state)
{
  $form['emails'] = array(
    '#type' => 'textfield',
    '#title' => t('Notification emails'),
    '#description' => t('A comma separated list of email addresses to receive notifications emails'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('lab_migration_emails', ''),
  );

  $form['from_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Outgoing from email address'),
    '#description' => t('Email address to be display in the from field of all outgoing messages'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('lab_migration_from_email', ''),
  );

  $form['extensions']['source'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed source file extensions'),
    '#description' => t('A comma separated list WITHOUT SPACE of source file extensions that are permitted to be uploaded on the server'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('lab_migration_source_extensions', ''),
  );
  $form['extensions']['dependency'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed dependency file extensions'),
    '#description' => t('A comma separated list WITHOUT SPACE of dependency file extensions that are permitted to be uploaded on the server'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('lab_migration_dependency_extensions', ''),
  );
  $form['extensions']['result'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed result file extensions'),
    '#description' => t('A comma separated list WITHOUT SPACE of result file extensions that are permitted to be uploaded on the server'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('lab_migration_result_extensions', ''),
  );
  $form['extensions']['xcos'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed xcos file extensions'),
    '#description' => t('A comma separated list WITHOUT SPACE of xcos file extensions that are permitted to be uploaded on the server'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('lab_migration_xcos_extensions', ''),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );
  return $form;
}

function lab_migration_settings_form_validate($form, &$form_state)
{
  return;
}

function lab_migration_settings_form_submit($form, &$form_state)
{
  variable_set('lab_migration_emails', $form_state['values']['emails']);
  variable_set('lab_migration_from_email', $form_state['values']['from_email']);
  variable_set('lab_migration_source_extensions', $form_state['values']['source']);
  variable_set('lab_migration_dependency_extensions', $form_state['values']['dependency']);
  variable_set('lab_migration_result_extensions', $form_state['values']['result']);
  variable_set('lab_migration_xcos_extensions', $form_state['values']['xcos']);
  drupal_set_message(t('Settings updated'), 'status');
}

