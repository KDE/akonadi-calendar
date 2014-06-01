#! /bin/sh
$EXTRACTRC `find . src -name \*.ui -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT `find . src -name "*.cpp" -o -name "*.h" | grep -v "/tests"` -o $podir/libakonadi-calendar5.pot
rm -f rc.cpp
