#!/bin/bash

#
# Export and sanitize the Drupal database to remove sensitive information.
# 

if [ $# -ne 3 ]; then
  echo "Syntax (in Drupal docroot): $0 DB_NAME DB_USER DB_PWD"
  echo "  DB_NAME: Name of a temporary database"
  echo "  DB_USER: User of a temporary database"
  echo "  DB_PWD:  Password of a temporary database"
  exit
fi

MYSQL="mysql $1 -u$2 -p$3"
DIR=`dirname $0`

if [ $(echo "SHOW TABLES" | ${MYSQL} | wc -l) -gt 0 ]; then
  echo "The temporary database should be empty."
  exit
fi

drush sql-dump --structure-tables-key=structure | $MYSQL
cat $DIR/sanitize.sql | $MYSQL
mysqldump $1 -u$2 -p$3 --opt --skip-extended-insert > $DIR/../export/dump.sql
grep ^DROP $DIR/../export/dump.sql | $MYSQL

