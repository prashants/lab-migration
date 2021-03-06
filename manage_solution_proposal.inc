<?php
// $Id$

function lab_migration_solution_proposal_pending()
{
  /* get list of solution proposal where the solution_provider_uid is set to some userid except 0 and solution_status is also 1 */
  $pending_rows = array();
  $pending_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE solution_provider_uid != 0 AND solution_status = 1 ORDER BY id DESC");
  while ($pending_data = db_fetch_object($pending_q))
  {
    $pending_rows[$pending_data->id] = array(l($pending_data->name, 'user/' . $pending_data->uid), $pending_data->lab_title, l('Approve', 'lab_migration/manage_proposal/solution_proposal_approve/' . $pending_data->id));
  }

  /* check if there are any pending proposals */
  if (!$pending_rows)
  {
    drupal_set_message(t('There are no pending solution proposals.'), 'status');
    return '';
  }

  $pending_header = array('Proposer Name', 'Title of the Lab', 'Action');
  $output = theme_table($pending_header, $pending_rows);
  return $output; 
}

/******************************************************************************/
/***************** SOLUTION PROPOSAL APPROVAL FORM ****************************/
/******************************************************************************/

function lab_migration_solution_proposal_approval_form($form_state)
{
  global $user;

  /* get current proposal */
  $proposal_id = (int)arg(3);
  $proposal_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE id = %d", $proposal_id);
  if ($proposal_q)
  {
    if ($proposal_data = db_fetch_object($proposal_q))
    {
      /* everything ok */
    } else {
      drupal_set_message(t('Invalid proposal selected. Please try again.'), 'error');
      drupal_goto('lab_migration/manage_proposal/pending_solution_proposal');
      return;
    }
  } else {
    drupal_set_message(t('Invalid proposal selected. Please try again.'), 'error');
    drupal_goto('lab_migration/manage_proposal/pending_solution_proposal');
    return;
  }

  $form['name'] = array(
    '#type' => 'item',
    '#value' => l($proposal_data->name_title . ' ' . $proposal_data->name, 'user/' . $proposal_data->uid),
    '#title' => t('Proposer Name'),
  );
  $form['email_id'] = array(
    '#type' => 'item',
    '#value' => user_load($proposal_data->uid)->mail,
    '#title' => t('Email'),
  );
  $form['contact_ph'] = array(
    '#type' => 'item',
    '#value' => $proposal_data->contact_ph,
    '#title' => t('Contact No.'),
  );
  $form['department'] = array(
    '#type' => 'item',
    '#value' => $proposal_data->department,
    '#title' => t('Department/Branch'),
  );
  $form['university'] = array(
    '#type' => 'item',
    '#value' => $proposal_data->university,
    '#title' => t('University/Institute'),
  );
  $form['lab_title'] = array(
    '#type' => 'item',
    '#value' => $proposal_data->lab_title,
    '#title' => t('Title of the Lab'),
  );

  /* get experiment details */
  $experiment_list = '<ul>';
  $experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE proposal_id = %d ORDER BY id ASC", $proposal_id);
  while ($experiment_data = db_fetch_object($experiment_q))
  {
    $experiment_list .= '<li>' . $experiment_data->title . '</li>';
  }
  $experiment_list .= '</ul>';

  $form['experiment'] = array(
      '#type' => 'item',
      '#value' => $experiment_list,
      '#title' => t('Experiments'),
  );

  $form['solution_display'] = array(
	  '#type' => 'item',
		'#title' => t('Display the solution on the www.scilab.in website'),
		'#value' => ($proposal_data->solution_display == 1) ? "Yes" : "No",
  );

  if ($proposal_data->solution_provider_uid == 0) {
  	$solution_provider = "Open";
  } else if ($proposal_data->solution_provider_uid == $proposal_data->uid) {
  	$solution_provider = "Proposer will provide the solution of the lab";
  } else {
  	$solution_provider_user_data = user_load($proposal_data->solution_provider_uid);
  	if ($solution_provider_user_data) {
  		$solution_provider .= '<ul>' .
        '<li><strong>Solution Provider:</strong> ' . l($solution_provider_user_data->name, 'user/' . $proposal_data->solution_provider_uid) . '</li>' .
        '<li><strong>Solution Provider Name:</strong> ' . $proposal_data->solution_provider_name_title . ' ' . $proposal_data->solution_provider_name . '</li>' .
        '<li><strong>Department:</strong> ' . $proposal_data->solution_provider_department . '</li>' .
        '<li><strong>University:</strong> ' . $proposal_data->solution_provider_university . '</li>' .
        '<li><strong>Date of Completion:</strong> ' . $proposal_data->expected_completion_date . '</li>' .
        '</ul>';
  	} else {
  		$solution_provider = "User does not exists";
  	}
  }
  $form['solution_provider_uid'] = array(
	  '#type' => 'item',
		'#title' => t('Solution Provider'),
		'#value' => $solution_provider,
  );

  $form['approval'] = array(
	  '#type' => 'radios',
		'#title' => t('Solution Provider'),
		'#options' => array('1' => 'Approve', '2' => 'Disapprove'),
		'#required' => TRUE,
	);

  $form['message'] = array(	
    '#type' => 'textarea',
    '#title' => t('Reason for disapproval'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );

  $form['cancel'] = array(
    '#type' => 'markup',
    '#value' => l(t('Cancel'), 'lab_migration/manage_proposal/pending_solution_proposal'),
  );

  return $form;
}

function lab_migration_solution_proposal_approval_form_validate($form, &$form_state)
{
  $proposal_id = (int)arg(3);

  $solution_provider_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE id = %d", $proposal_id);
  $solution_provider_data = db_fetch_object($solution_provider_q);

 	$solution_provider_present_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE solution_provider_uid = %d AND approval_status IN (0, 1) AND id != %d", $solution_provider_data->uid, $proposal_id);
	if ($x = db_fetch_object($solution_provider_present_q)) {
		drupal_set_message($proposal_id);
		form_set_error('', t('Solution provider has already one proposal active'));
	}
}

function lab_migration_solution_proposal_approval_form_submit($form, &$form_state)
{
	global $user;

  /* get current proposal */
  $proposal_id = (int)arg(3);
  $proposal_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE id = %d", $proposal_id);
  if ($proposal_q)
  {
    if ($proposal_data = db_fetch_object($proposal_q))
    {
      /* everything ok */
    } else {
      drupal_set_message(t('Invalid proposal selected. Please try again.'), 'error');
      drupal_goto('lab_migration/manage_proposal/pending_solution_proposal');
      return;
    }
  } else {
    drupal_set_message(t('Invalid proposal selected. Please try again.'), 'error');
    drupal_goto('lab_migration/manage_proposal/pending_solution_proposal');
    return;
  }

  $user_data = user_load($proposal_data->solution_provider_uid);

  if ($form_state['values']['approval'] == 1) {
  	db_query("UPDATE {lab_migration_proposal} SET solution_status = 2 WHERE id = %d", $proposal_id);

    /* sending email */
    $email_to = $user_data->mail;
    $param['solution_proposal_approved']['proposal_id'] = $proposal_id;
    $param['solution_proposal_approved']['user_id'] = $proposal_data->uid;
    if (!drupal_mail('lab_migration', 'solution_proposal_approved', $email_to , language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
      drupal_set_message('Error sending email message.', 'error');

    $email_to = $user->mail . ', ' . variable_get('lab_migration_emails', '');
    if (!drupal_mail('lab_migration', 'solution_proposal_approved', $email_to , language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
      drupal_set_message('Error sending email message.', 'error');

    drupal_set_message('Lab migration solution proposal approved. User has been notified of the approval.', 'status');
    drupal_goto('lab_migration/manage_proposal/pending_solution_proposal');
    return;
  } else if ($form_state['values']['approval'] == 2) {
    db_query("UPDATE {lab_migration_proposal} SET solution_provider_uid = 0, solution_status = 0, solution_provider_name_title = '', solution_provider_name = '', solution_provider_contact_ph = '', solution_provider_department = '', solution_provider_university = '', expected_completion_date = '' WHERE id = %d", $proposal_id);

    /* sending email */
    $email_to = $user_data->mail;
    $param['solution_proposal_disapproved']['proposal_id'] = $proposal_id;
    $param['solution_proposal_disapproved']['user_id'] = $proposal_data->uid;
    $param['solution_proposal_disapproved']['message'] = $form_state['values']['message'];
    if (!drupal_mail('lab_migration', 'solution_proposal_disapproved', $email_to , language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
      drupal_set_message('Error sending email message.', 'error');

    $email_to = $user->mail . ', ' . variable_get('lab_migration_emails', '');;
    if (!drupal_mail('lab_migration', 'solution_proposal_disapproved', $email_to , language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
      drupal_set_message('Error sending email message.', 'error');

    drupal_set_message('Lab migration solution proposal dis-approved. User has been notified of the dis-approval.', 'status');
    drupal_goto('lab_migration/manage_proposal/pending_solution_proposal');
    return;
  }
}

