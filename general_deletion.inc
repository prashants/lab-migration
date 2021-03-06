<?php

/******************************************************************************/
/****************************** DELETION FUNCTIONS ****************************/
/******************************************************************************/

function lab_migration_delete_solution($solution_id)
{
  global $user;
  $root_path = lab_migration_path();
  $status = TRUE;

  $solution_q = db_query("SELECT * FROM {lab_migration_solution} WHERE id = %d", $solution_id);
  $solution_data = db_fetch_object($solution_q);
  if (!$solution_data)
  {
    drupal_set_message(t('Invalid solution.'), 'error');
    return FALSE;
  }

  $experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE id = %d", $solution_data->experiment_id);
  $experiment_data = db_fetch_object($experiment_q);
  if (!$experiment_data)
  {
    drupal_set_message(t('Invalid experiment.'), 'error');
    return FALSE;
  }

  /* deleting solution files */
  $solution_files_q = db_query("SELECT * FROM {lab_migration_solution_files} WHERE solution_id = %d", $solution_id);
  while ($solution_files_data = db_fetch_object($solution_files_q))
  {
    if (!file_exists($root_path . $solution_files_data->filepath))
    {
      $status = FALSE;
      drupal_set_message(t('Error deleting !file. File does not exists.', array('!file' => $solution_files_data->filepath)), 'error');
      continue;
    }

    /* removing solution file */
    if (!unlink($root_path . $solution_files_data->filepath))
    {
      $status = FALSE;
      drupal_set_message(t('Error deleting !file', array('!file' => $solution_files_data->filepath)), 'error');

      /* sending email to admins */
      $email_to = variable_get('lab_migration_emails', '');
      $param['standard']['subject'] = "[ERROR] Error deleting example file";
      $param['standard']['body'] = "Error deleting solution files by " . $user->uid . " at " . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'] . " :
        solution id : " . $solution_id . "
        file id : " .  $solution_files_data->id . "
        file path : " . $solution_files_data->filepath;
      if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
        drupal_set_message('Error sending email message.', 'error');
    } else {
      /* deleting example files database entries */
      db_query("DELETE FROM {lab_migration_solution_files} WHERE id = %d", $solution_files_data->id);
    }
  }

  if (!$status)
    return FALSE;

  /* removing code folder */
  $ex_path = $experiment_data->proposal_id . '/EXP' . $experiment_data->number . '/CODE' . $solution_data->code_number;
  $dir_path = $root_path . $ex_path;
  if (is_dir($dir_path))
  {
    if (!rmdir($dir_path))
    {
      drupal_set_message(t('Error deleting folder !folder', array('!folder' => $dir_path)), 'error');

      /* sending email to admins */
      $email_to = variable_get('lab_migration_emails', '');
      $param['standard']['subject'] = "[ERROR] Error deleting folder";
      $param['standard']['body'] = "Error deleting folder " . $dir_path . " by " . $user->uid . " at " . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
      if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
        drupal_set_message('Error sending email message.', 'error');
      return FALSE;
    }
  } else {
    drupal_set_message(t('Cannot delete solution folder. !folder does not exists.', array('!folder' => $dir_path)), 'error');
    return FALSE;
  }

  /* deleting solution dependency and solution database entries */
  db_query("DELETE FROM {lab_migration_solution_dependency} WHERE solution_id = %d", $solution_id);
  db_query("DELETE FROM {lab_migration_solution} WHERE id = %d", $solution_id);

  return $status;
}

function lab_migration_delete_experiment($experiment_id)
{
  $status = TRUE;
  $root_path = lab_migration_path();

  $experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE id = %d", $experiment_id);
  $experiment_data = db_fetch_object($experiment_q);
  if (!$experiment_data)
  {
    drupal_set_message('Invalid experiment.', 'error');
    return FALSE;
  }

  /* deleting solutions */
  $solution_q = db_query("SELECT * FROM {lab_migration_solution} WHERE experiment_id = %d", $experiment_id);
  $delete_exp_folder = FALSE;
  while ($solution_data = db_fetch_object($solution_q))
  {
  	$delete_exp_folder = TRUE;
    if (!lab_migration_delete_solution($solution_data->id))
      $status = FALSE;
  }

  if (!$delete_exp_folder)
  {
  	return TRUE;
  }

  if ($status)
  {
    $dir_path = $root_path . $experiment_data->proposal_id . '/EXP' . $experiment_data->number;

    if (is_dir($dir_path))
    {
      $res = rmdir($dir_path);
      if (!$res)
      {
        drupal_set_message(t('Error deleting experiment folder !folder', array('!folder' => $dir_path)), 'error');
  
        /* sending email to admins */
        $email_to = variable_get('lab_migration_emails', '');
        $param['standard']['subject'] = "[ERROR] Error deleting experiment folder";
        $param['standard']['body'] = "Error deleting folder " . $dir_path;
        if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
          drupal_set_message('Error sending email message.', 'error');
        return FALSE;
      } else {
        return TRUE;
      }
    } else {
      drupal_set_message(t('Cannot delete experiment folder. !folder does not exists.', array('!folder' => $dir_path)), 'error');
      return FALSE;
    }
  }
  return FALSE;
}

function lab_migration_delete_lab($lab_id)
{
  $status = TRUE;
  $root_path = lab_migration_path();

  $proposal_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE id = %d", $lab_id);
  $proposal_data = db_fetch_object($proposal_q);
  if (!$proposal_data)
  {
    drupal_set_message('Invalid Lab.', 'error');
    return FALSE;
  }

  /* delete experiments */
  $experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE proposal_id = %d", $proposal_data->id);
  while ($experiment_data = db_fetch_object($experiment_q))
  {
    if (!lab_migration_delete_experiment($experiment_data->id))
    {
      $status = FALSE;
    }
  }
  return $status;
}

function lab_migration_del_lab_pdf($lab_id)
{
  $root_path = lab_migration_path();
  $dir_path = $root_path . "latex/";
  $pdf_filename = "lab_" . $lab_id . ".pdf";
  if (file_exists($dir_path . $pdf_filename))
    unlink($dir_path . $pdf_filename);
}


function lab_migration_delete_dependency($dependency_id)
{
  global $user;
  $root_path = lab_migration_path();
  $status = TRUE;

  $dependency_files_q = db_query("SELECT * FROM {lab_migration_dependency_files} WHERE id = %d", $dependency_id);
  $dependency_files_data = db_fetch_object($dependency_files_q);
  if (!$dependency_files_data)
  {
    drupal_set_message(t('Invalid dependency.'), 'error');
    return FALSE;
  }

	if (!file_exists($root_path . $dependency_files_data->filepath))
	{
		drupal_set_message(t('Error deleting !file. File does not exists.', array('!file' => $dependency_files_data->filepath)), 'error');
    return FALSE;
	}

	/* removing dependency file */
	if (!unlink($root_path . $dependency_files_data->filepath))
	{
		$status = FALSE;
		drupal_set_message(t('Error deleting !file', array('!file' => $dependency_files_data->filepath)), 'error');

		/* sending email to admins */
		$email_to = variable_get('lab_migration_emails', '');
		$param['standard']['subject'] = "[ERROR] Error deleting dependency file";
		$param['standard']['body'] = "Error deleting dependency files by " . $user->uid . " at " . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'] . " :
			dependency id : " . $dependency_id . "
			file id : " .  $dependency_files_data->id . "
			file path : " . $dependency_files_data->filepath;
		if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $param, variable_get('lab_migration_from_email', NULL), TRUE))
			drupal_set_message('Error sending email message.', 'error');
	} else {
		/* deleting dependency files database entries */
		db_query("DELETE FROM {lab_migration_dependency_files} WHERE id = %d", $dependency_id);
	}

  return $status;
}

