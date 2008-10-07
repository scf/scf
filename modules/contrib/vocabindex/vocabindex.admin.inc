<?php
/**
*Admin general settings page
*/
function vocabindex_admin()
{
	$form['vocabindex_terms_order']=array(
		'#type'=>'select',
		'#title'=>t('Terms order'),
		'#description'=>t('Select the order in which the terms should be displayed.'),
		'#default_value'=>variable_get('vocabindex_terms_order', 'weight'),
		'#options'=>array(
			'weight'=>t('By weight'),
			'name'=>t('By name'),
		),
	);
	
	$form['vocabindex_list_style']=array(
		'#type'=>'select',
		'#title'=>t('List style'),
		'#description'=>t('Select the display style for the terms lists.'),
		'#default_value'=>variable_get('vocabindex_list_style', 'threaded'),
		'#options'=>array(
			'threaded'=>t('Threaded'),
			'flat'=>t('Flat'),
			'flat-toplevel'=>t('Flat (top-level terms only)'),
		),
	);
	
	$form['vocabindex_stylesheet']=array(
		'#type'=>'checkbox',
		'#title'=>t('Use default stylesheet'),
		'#description'=>t('Uncheck if you don\'t want to use the default Vocabindex stylesheet.'),
		'#default_value'=>variable_get('vocabindex_stylesheet', TRUE),
	);
	
	return system_settings_form($form);
}

/**
*Admin paths page
*/
function vocabindex_page_admin_paths()
{
	$count=db_result(db_query("SELECT COUNT(*) FROM {vocabulary}"));
	if($count!=0)
	{
		$output=t('Paths may only contain alphanumeric characters and dashes. For best SEO results, use dashes as word separators. Paths will automatically be converted to lowercase.');
		$output.=drupal_get_form('vocabindex_form_admin_paths');
	}
	else
	{
		$output=t('There are no vocabularies to create index pages for. You can create vocabularies at <a href="!link">the Taxonomy page</a>.', array('!link'=>url(_vocabindex_menu_paths('taxonomy'))));
	}
	
	return $output;
}
function vocabindex_form_admin_paths()
{
	$result=db_query("SELECT vi.path, v.name, v.vid FROM {vocabulary} v LEFT JOIN {vocabindex} vi ON v.vid=vi.vid ORDER BY v.name ASC");
	while($row=db_fetch_object($result))
	{
		if($row->path)
		{
			$description=t('Currently located at <a href="!url">/!relative_url</a>', array('!url'=>url($row->path), '!relative_url'=>$row->path));
		}
		else
		{
			$description=t('There is currently no index page set for this vocabulary.');
		}
		
		$form['vocabindex_path_'.$row->vid]=array(
			'#type'=>'textfield',
			'#title'=>t('Path for %title index page', array('%title'=>$row->name)),
			'#default_value'=>$row->path,
			'#maxlength'=>'128',
			'#description'=>$description,
		);
	}
	
	$form['submit']=array(
		'#type'=>'submit',
		'#value'=>t('Save')
	);
	
	return $form;
}

function vocabindex_form_admin_paths_validate($form, &$form_state)
{
	foreach($form_state['values'] as $element=>$path)
	{
		if(strpos($element, 'vocabindex_path_')!==FALSE)
		{
			$vid=str_replace('vocabindex_path_', '', $element);
			if(!empty($path) && !preg_match('#[a-z0-9-]#i', $path))
			{
				form_set_error($element, t('Paths may only contain alphanumeric characters and dashes.'));
			}
			else
			{
				$vid=db_result(db_query("SELECT vid FROM {vocabindex} WHERE path = '%s'", $path));
				$message=vocabindex_check_index($vid, $path);
				if($message=='used')
				{
					form_set_error($element, t('Path already exists.'));
				}
			}
		}
	}
}

function vocabindex_form_admin_paths_submit($form, &$form_state)
{	
	foreach($form_state['values'] as $element=>$path)
	{
		if(strpos($element, 'vocabindex_path_')!==FALSE)
		{
			$vid=str_replace('vocabindex_path_', '', $element);
			
			$old_path=db_result(db_query("SELECT path FROM {vocabindex} WHERE vid = %d", $vid));
			vocabindex_create_index($vid, $path);
			
		}
	}
	
	//Present the user with a confirmation message
	drupal_set_message(t('The paths have been updated.'));
}

/**
*Check if the given path is already being used. Returns 'vocab' if the path for an index hasn't changed, 'unused' if it isn't used or 'used' when it is in use.
*/
function vocabindex_check_index($vid, $path)
{
	//Check if path is already used for this vocabulary. If false, check if it's already being used by other items or nodes. If true, do nothing.
	$count=db_result(db_query("SELECT COUNT(*) FROM {vocabindex} WHERE vid = %d AND path = '%s'", $vid, $path));
	if($count==0)
	{
		//Check for existing aliases and menu paths
		$count=db_result(db_query("SELECT COUNT(*) FROM {menu_links} WHERE link_path LIKE '%s'", $path));
		if(drupal_lookup_path('source', $path)==TRUE || $count>0)
		{
			$message='used';
		}
		else
		{
			$message='unused';
		}
	}
	else
	{
		$message='vocab';
	}
	
	return $message;
}

/**
*Creates or deletes the paths for the index pages
*/
function vocabindex_create_index($vid, $path)
{
	//Delete the old vocabindex path
	vocabindex_delete_index(NULL, $vid);
	
	if($vid && $path)
	{
		//Create the new path
		$vocab=taxonomy_vocabulary_load($vid);
		$description=$vocab->description;
		$title=$vocab->name;
		$path=strtolower($path);
		
		db_query("INSERT INTO {vocabindex} (vid, path) VALUES (%d, '%s')", $vid, $path);
		
		//Rebuild the menu
		menu_rebuild();
	}
}

/**
*Deletes index pages.
*/
function vocabindex_delete_index($path=NULL, $vid=NULL)
{
	if(!$path && $vid)
	{
		$path=db_result(db_query("SELECT path FROM {vocabindex} WHERE vid = %d", $vid));
	}
	
	//Delete the index page
	db_query("DELETE FROM {vocabindex} WHERE path = '%s'", $path);
	
	_vocabindex_menu_delete_item($path);
}

/**
*Deletes menu items by force. Some parts are a rough copy of _menu_delete_item(), but without the check for System links or updated items.
*/
function _vocabindex_menu_delete_item($path)
{
	//Select the menu item that matches the path.
	$result=db_query("SELECT * FROM {menu_links} WHERE link_path = '%s'", $path);
	while($link=db_fetch_array($result))
	{
		//Check for child elements and append them to their new parent
		if($link['has_children'])
		{			$result=db_query("SELECT mlid FROM {menu_links} WHERE plid = %d", $link['mlid']);			while($m=db_fetch_array($result))
			{				$child=menu_link_load($m['mlid']);				$child['plid']=$link['plid'];				menu_link_save($child);			}		}		db_query("DELETE FROM {menu_links} WHERE mlid = %d", $link['mlid']);			// Update the has_children status of the parent.		_menu_update_parental_status($link);		menu_cache_clear($link->menu_name);		_menu_clear_page_cache();
	}

}