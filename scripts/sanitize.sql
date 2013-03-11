UPDATE authmap SET authname = CONCAT(aid, '@localhost');
UPDATE comment SET mail = CONCAT(name, '@localhost'), hostname = '';
DELETE FROM openid_association;
DELETE FROM queue;
UPDATE users SET mail = CONCAT(name, '@localhost'), init = CONCAT('drupalvietnam.org/', uid), pass = MD5(CONCAT('drupalvietnam', name)), login = 1234567890, access = 1234567890;
DELETE FROM variable WHERE name LIKE '%key%';
DELETE FROM variable WHERE name = 'cron_semaphore';
DELETE FROM watchdog;

-- Get rid of unpublished/blocked nodes, users, comments and related data in other tables.
DELETE FROM node WHERE status <> 1;
DELETE FROM comment WHERE status <> 0;
DELETE FROM users WHERE status <> 1 AND uid <> 0;
DELETE node FROM node LEFT JOIN users ON node.uid = users.uid WHERE users.uid IS NULL;
DELETE node_revision FROM node_revision LEFT JOIN node ON node.nid = node_revision.nid WHERE node.nid IS NULL;
DELETE comment FROM comment LEFT JOIN node ON node.nid = comment.nid WHERE node.nid IS NULL;
DELETE comment FROM comment LEFT JOIN users ON comment.uid = users.uid WHERE users.uid IS NULL;
DELETE comment FROM comment LEFT JOIN comment c2 ON comment.pid = c2.cid WHERE c2.cid IS NULL AND comment.pid <> 0;

