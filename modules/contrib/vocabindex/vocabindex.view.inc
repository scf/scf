<?php
/**
*Views a vocabulary index page
*/
function vocabindex_view_page($path)
{
	$vid=db_result(db_query("SELECT vid FROM {vocabindex} WHERE path='%s'", $path));
	$list_style=variable_get('vocabindex_list_style', 'threaded');
	$sort_by=variable_get('vocabindex_terms_order', 'weight');
	
	$list_tmp=taxonomy_get_tree($vid);
	
	//Don't try to render the list if there are no terms to display
	if(count($list_tmp)!=0)
	{
		//The eventual list. The different types of keys are used for sorting
		$list=array();
		if($sort_by=='weight')
		{
			foreach($list_tmp as $term)
			{
				//Sort by weight. Use the name as well to prevent duplicate keys and to sort by name after the terms have been sorted by weight
				$list[$term->weight.$term->name]=$term;
			}
		}
		else if($sort_by=='name')
		{
			foreach($list_tmp as $term)
			{
				//Sort by name
				$list[$term->name]=$term;
			}
		}
		
		//Check list-style and call the right function
		if($list_style!='threaded')
		{
			$output=vocabindex_render_list_flat($list, $sort_by, $list_style);
		}
		else
		{
			$output=vocabindex_render_list_threaded($list, $sort_by);
		}
			
		//And render the top list and page
		$output=theme('vocabindex_list', $output, $list_style);
		$vocab=taxonomy_vocabulary_load($vid);		
		$output=theme('vocabindex_page', $vocab->description, $output);
	}
	else
	{
		$output=t('There are no categories to display.');
	}
	
	//Only add the stylesheet if the site administrator wants it
	if(variable_get('vocabindex_stylesheet', TRUE))
	{
		drupal_add_css(drupal_get_path('module', 'vocabindex') . '/vocabindex.css', 'module', 'screen', FALSE);
	}	
	
	return $output;
}

/**
*Function to render a flat list
*/
function vocabindex_render_list_flat($list, $sort_by, $list_style)
{
	ksort($list);
	
	if($list_style=='flat-toplevel')
	{
		//Filter $list so only terms with $term->parents[0]==0 will be kept
		$list_tmp=array();
		foreach($list as $key => $term)
		{
			if($term->parents[0]==0)
			{
				$list_tmp[$key]=$term;
			}
		}
		$list=$list_tmp;
	}
	
	//Loop through all the terms from the list and render them
	$i=1;
	foreach($list as $term)
	{
		$zebra=($i%2==0?'even':'odd');
		$url=url('taxonomy/term/'.$term->tid);
		$output.=theme('vocabindex_list_item', $url, $term->name, $term->description, $zebra);
		$i++;
	}
	
	return $output;
}

/**
*Function to render a threaded list
*/
function vocabindex_render_list_threaded($list, $sort_by)
{
	//A list of terms that are parents
	$parents=array();
	//A list of the depths all terms are at.
	$depths=array();
	//Maximum depth.
	$max_depth=0;
	
	//Set up all the arrays necessary for rendering the eventual list
	foreach($list as $term)
	{			
		foreach($term->parents as $parent)
		{
			//Build up a list of terms that are parents
			$parents[$parent][]=$term->tid;
			
			//Set up an array with all the depths so we can render the list from the inside out later on
			$depths[$term->depth][]=$term;
			if($term->depth>$max_depth)
			{
				//And to know where to start rendering we need to know the maximum depth
				$max_depth=$term->depth;
			}
		}
		
	}
		
	//Render the list from the inside out
	$rendered_terms=array();
	for($i=$max_depth; $i>=0; $i--)
	{
		//Loop through all the terms at this depth
		foreach($depths[$i] as $term)
		{
			//Check if term is a parent
			if($parents[$term->tid])
			{
				//The term is a parent. Because we render inside out all the children have been rendered already and can simply be put together
				foreach($parents[$term->tid] as $child)
				{
					$children.=$rendered_terms[$child];
				}
				$children=theme('vocabindex_list', $children);
			}
			
			//Render term
			$url=url('taxonomy/term/'.$term->tid);
			$rendered_terms[$term->tid]=theme('vocabindex_list_item', $url, $term->name, $term->description, NULL, $children);
			
			//And clear all children
			$children=NULL;
		}
	}
	
	//Put the top-level terms together
	foreach($depths[0] as $term)
	{
		$output.=$rendered_terms[$term->tid];
	}
	
	return $output;
}