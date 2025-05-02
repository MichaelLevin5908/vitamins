# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
CS 111: Operating Systems Principles
write-stdout: exit(0)
EOF
pass;
