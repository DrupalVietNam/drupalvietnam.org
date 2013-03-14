<?php

/**
 * @file
 * Inspired from paranoia module.
 */

/**
 * Implements hook_drush_command().
 */
function sanitize_drush_sql_sync_sanitize($site) {
  drush_sql_register_post_sync_op('variable_keys',
    dt('Remove variables that contain names that indicate potential sensitive data.'),
    "DELETE FROM variable WHERE name LIKE '%key%' OR name LIKE 'cron_semaphore';");
  drush_sql_register_post_sync_op('users_blocked',
    dt('Blocked user accounts may contain inappropriate information and are not accessible to the public in general.'),
    "
    DELETE FROM users WHERE status <> 1 AND uid NOT IN (0, 1);
    DELETE FROM node WHERE status <> 1;
    DELETE FROM comment WHERE status <> 0;
    DELETE users_roles FROM users_roles LEFT JOIN users ON users_roles.uid = users.uid WHERE users.uid IS NULL;
    UPDATE comment SET mail = CONCAT(name, '@localhost'), hostname = '';
    DELETE node FROM node LEFT JOIN users ON node.uid = users.uid WHERE users.uid IS NULL;
    DELETE node_revision FROM node_revision LEFT JOIN node ON node.nid = node_revision.nid WHERE node.nid IS NULL;
    DELETE comment FROM comment LEFT JOIN node ON node.nid = comment.nid WHERE node.nid IS NULL;
    DELETE comment FROM comment LEFT JOIN users ON comment.uid = users.uid WHERE users.uid IS NULL;
    DELETE comment FROM comment LEFT JOIN comment c2 ON comment.pid = c2.cid WHERE c2.cid IS NULL AND comment.pid <> 0;
    ");
}

