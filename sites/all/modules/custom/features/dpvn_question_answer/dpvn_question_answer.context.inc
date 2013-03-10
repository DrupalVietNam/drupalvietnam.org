<?php
/**
 * @file
 * dpvn_question_answer.context.inc
 */

/**
 * Implements hook_context_default_contexts().
 */
function dpvn_question_answer_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'question_detail';
  $context->description = '';
  $context->tag = 'Question Answer';
  $context->conditions = array(
    'path' => array(
      'values' => array(
        'hoi-dap/*' => 'hoi-dap/*',
      ),
    ),
  );
  $context->reactions = array(
    'block' => array(
      'blocks' => array(
        'views-9b49839544f048036ea1526252173b86' => array(
          'module' => 'views',
          'delta' => '9b49839544f048036ea1526252173b86',
          'region' => 'sidebar_second',
          'weight' => '-10',
        ),
      ),
    ),
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Question Answer');
  $export['question_detail'] = $context;

  return $export;
}
