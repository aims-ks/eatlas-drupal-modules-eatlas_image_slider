<?php

function _eatlas_image_slider_overview() {
	$output = '<h3>' . t('Image Sliders') . '</h3>';

	$blocks = eatlas_image_slider_block_load_all();

	$header = array(t('Title'), t('Operations'));
	$rows = array();
	foreach($blocks as $block) {
		$rows[] = array(
			$block->name,
			l(t('Edit'), "admin/config/eatlas/eatlas_image_slider/$block->bid/edit") .
			'<span class="eatlas-image-slider-spacer"></span>' .
			l(t('Delete'), "admin/config/eatlas/eatlas_image_slider/$block->bid/delete")
		);
	}
	$output .= theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('There is no image slider block.')));

	$output .= '<a href="eatlas_image_slider/create">Create a new image slider</a>';

	return $output;
}

function _eatlas_image_slider_create($form, &$form_state) {
	return _eatlas_image_slider_edit($form, $form_state, NULL);
}

function _eatlas_image_slider_edit($form, &$form_state, $bid) {
	$block = eatlas_image_slider_block_load($bid);

	$form['name'] = array(
		'#type' => 'textfield',
		'#title' => t('Name'),
		'#description' => t('Choose a name for this image slider. The name will be use to identify the block.'),
		'#default_value' => $block && property_exists($block, 'name') ? $block->name : '',
		'#required' => TRUE
	);

	// Get all views (module views: https://www.drupal.org/project/views)
	$filtered_view_options = array();
	//$views = views_get_all_views();
	$views = views_get_enabled_views();
	foreach($views as $view_id => $view) {
		$filtered_view_options[$view_id] = $view->human_name;
	}
	// Order by value (without changing the keys)
	uasort($filtered_view_options, 'strnatcasecmp');

	// Get the value from the form state (needed by the 'viewdisplay' field) or from the DB.
	$view_value = isset($form_state['values']['view']) ? $form_state['values']['view'] :
			($block && property_exists($block, 'view') ? $block->view : '');

	// https://api.drupal.org/api/examples/ajax_example!ajax_example.module/function/ajax_example_dependent_dropdown/7
	$form['view'] = array(
		'#type' => 'select',
		'#options' => $filtered_view_options,
		'#title' => t('View'),
		'#description' => t('Select the view associated with this image slider block.'),
		'#default_value' => $view_value,
		'#required' => TRUE,

		// Fill the "View displays" field depending on the selected view
		'#ajax' => array(
			'wrapper' => 'viewdisplay-wrapper',
			'callback' => '_eatlas_image_slider_fill_view_display_ajax_callback',
		)
	);

	// Get the value from the form state (needed by the 'viewdisplay' field) or from the DB.
	$viewdisplay_value = isset($form_state['values']['viewdisplay']) ? $form_state['values']['viewdisplay'] :
			($block && property_exists($block, 'viewdisplay') ? $block->viewdisplay : '');

	$filtered_view_display_options = array();
	if (!empty($view_value)) {
		// Get all display for the choosen view
		// https://api.drupal.org/api/views/views.module/function/views_get_view/7
		$view_obj = views_get_view($view_value);

		// https://api.drupal.org/api/views/includes!view.inc/class/view/7
		$default_display = $view_obj->display['default'];
		foreach($view_obj->display as $display) {
			$filtered_view_display_options[$display->id] = _eatlas_image_slider_get_view_display_name($display, $default_display);
		}
		// Order by value (without changing the keys)
		uasort($filtered_view_display_options, 'strnatcasecmp');
	}

	$form['viewdisplay'] = array(
		'#prefix' => '<div id="viewdisplay-wrapper">',
		'#suffix' => '</div>',
		'#type' => 'select',
		'#empty_option' => '- Select a display -',
		'#default_value' => $viewdisplay_value,
		'#options' => $filtered_view_display_options,
		'#title' => t('View displays'),
		'#required' => TRUE
	);

	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save')
	);

	if ($block && property_exists($block, 'bid')) {
		$form['actions']['delete'] = array(
			'#type' => 'submit',
			'#value' => t('Delete'),
			'#submit' => array('_eatlas_image_slider_delete_redirect')
		);

		$form['bid'] = array(
			'#type' => 'hidden',
			'#value' => $block->bid
		);
	}

	return $form;
}

// https://api.drupal.org/api/examples/ajax_example!ajax_example.module/function/ajax_example_dependent_dropdown/7
function _eatlas_image_slider_fill_view_display_ajax_callback($form, $form_state) {
	return $form['viewdisplay'];
}

function _eatlas_image_slider_get_view_display_name($display, $default_display) {
	$plugin_name = property_exists($display, 'display_plugin') && !empty($display->display_plugin) ? $display->display_plugin : NULL;
	$display_type = property_exists($display, 'display_title') && !empty($display->display_title) ? $display->display_title : NULL;

	if ($display_type === NULL) {
		$display_type = $plugin_name;
	}
	if ($display_type === NULL) {
		// That won't happen...
		$display_type = $display->id;
	}

	$title = property_exists($display, 'display_options') && !empty($display->display_options['title']) ? $display->display_options['title'] : NULL;
	if ($title === NULL && $display->id !== 'default') {
		$title = property_exists($default_display, 'display_options') && !empty($default_display->display_options['title']) ? $default_display->display_options['title'] : NULL;
	}

	return ($title !== NULL ? $title : 'UNTITLED') .
			' (' . $display_type . ')';
}

function _eatlas_image_slider_create_validate($form, &$form_state) {
	// Mandatory fields are managed by Drupal
}

function _eatlas_image_slider_create_submit($form, &$form_state) {
	return _eatlas_image_slider_edit_submit($form, $form_state);
}

function _eatlas_image_slider_edit_validate($form, &$form_state) {
	if (!isset($form_state['values']['bid'])) {
		form_set_error('name', t('ID not found. Try reloading the page.'));
	}
}

function _eatlas_image_slider_edit_submit($form, &$form_state) {
	$block = new stdClass();

	if (isset($form_state['values']['bid'])) {
		$block->bid = $form_state['values']['bid'];
	}
	if (isset($form_state['values']['name'])) {
		$block->name = $form_state['values']['name'];
	}
	if (isset($form_state['values']['view'])) {
		$block->view = $form_state['values']['view'];
	}
	if (isset($form_state['values']['viewdisplay'])) {
		$block->viewdisplay = $form_state['values']['viewdisplay'];
	}

	eatlas_image_slider_block_save($block);

	// Return to the image slider list after submit
	$form_state['redirect'] = 'admin/config/eatlas/eatlas_image_slider';
}

function _eatlas_image_slider_delete_redirect($form, &$form_state) {
	if (isset($form_state['values']['bid'])) {
		$form_state['redirect'] = 'admin/config/eatlas/eatlas_image_slider/' . $form_state['values']['bid'] . '/delete';
	}
}

function _eatlas_image_slider_delete($form, &$form_state, $bid) {
	// Add the image slider block ID to the form so the
	// submit function will know which block needs to be deleted.
	$form['bid'] = array(
		'#type' => 'hidden',
		'#value' => $bid
	);

	// Get some info about the image slider to write better message to the user.
	$block = eatlas_image_slider_block_load($bid);

	return confirm_form($form,
		$question = 'Deleting image slider <em>' . check_plain($block->name) . '</em>',
		'admin/config/eatlas/eatlas_image_slider', // Path to return to if "cancel" is pressed
		$description = 'Do you really want to delete the image slider <em>' . check_plain($block->name) . '</em>?',
		$yes = 'Really?'
	);
}

function _eatlas_image_slider_delete_submit($form, &$form_state) {
	if (isset($form_state['values']['bid'])) {
		$bid = $form_state['values']['bid'];

		eatlas_image_slider_block_delete($bid);

		// Return to the image slider list after submit
		$form_state['redirect'] = 'admin/config/eatlas/eatlas_image_slider';
	}
}

// Display a personalised title for "Edit branding" pages
function _eatlas_image_slider_edit_title($block, $action) {
	switch($action) {
		case 'create':
			return 'Create a new image slider block';
		case 'edit':
			return "Edit $block image slider block settings";
		case 'delete':
			return "Delete $block image slider block";
	}
	return "ERROR invalid action $action";
}

?>
