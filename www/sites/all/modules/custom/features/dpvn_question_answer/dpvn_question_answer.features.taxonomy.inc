<?php
/**
 * @file
 * dpvn_question_answer.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function dpvn_question_answer_taxonomy_default_vocabularies() {
  return array(
    'question_tags' => array(
      'name' => 'Question Tags',
      'machine_name' => 'question_tags',
      'description' => 'Question Tags',
      'hierarchy' => '0',
      'module' => 'taxonomy',
      'weight' => '0',
      'rdf_mapping' => array(
        'rdftype' => array(
          0 => 'skos:ConceptScheme',
        ),
        'name' => array(
          'predicates' => array(
            0 => 'dc:title',
          ),
        ),
        'description' => array(
          'predicates' => array(
            0 => 'rdfs:comment',
          ),
        ),
      ),
    ),
  );
}
