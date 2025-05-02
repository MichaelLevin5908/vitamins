# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
do-nothing: exit(111)
EOF
pass;
