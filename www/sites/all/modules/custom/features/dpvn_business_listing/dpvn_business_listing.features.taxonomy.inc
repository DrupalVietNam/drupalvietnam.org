<?php
/**
 * @file
 * dpvn_business_listing.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function dpvn_business_listing_taxonomy_default_vocabularies() {
  return array(
    'thanh_pho' => array(
      'name' => 'Thành phố',
      'machine_name' => 'thanh_pho',
      'description' => 'Tỉnh thành.',
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
