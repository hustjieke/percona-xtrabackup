########################################################################
# Bug #664128: xtrabackup does not support "--loose-" options
########################################################################

cat >$topdir/my.cnf <<EOF
[mysqld]
loose-innodb_log_file_size=5M
maximum-innodb_log_file_size=6M
skip-innodb_log_checksums
EOF

diff -u - <($XB_BIN --defaults-file=$topdir/my.cnf --print-param) <<EOF
# This MySQL options file was generated by XtraBackup.
[mysqld]
innodb_log_file_size=5242880
innodb_log_file_size=5242880
innodb_log_checksums=0
EOF

cat >$topdir/my.cnf <<EOF
[mysqld]
enable-innodb_log_checksums
EOF

diff -u - <($XB_BIN --defaults-file=$topdir/my.cnf --print-param) <<EOF
# This MySQL options file was generated by XtraBackup.
[mysqld]
innodb_log_checksums=1
EOF

cat >$topdir/my.cnf <<EOF
[mysqld]
disable-innodb_log_checksums
EOF

diff -u - <($XB_BIN --defaults-file=$topdir/my.cnf --print-param) <<EOF
# This MySQL options file was generated by XtraBackup.
[mysqld]
innodb_log_checksums=0
EOF
