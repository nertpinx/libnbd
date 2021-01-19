#!/bin/sh

set -e

main() {
    autoreconf -if
    ./configure --enable-gcc-warnings --enable-fuse --enable-ocaml --enable-python --enable-golang --with-gnutls --with-libxml2

    $MAKE

    if test -n "$CROSS"
    then
       return 0
    fi

    $MAKE check || find . -name test-suite.log -exec grep -l '^X\?FAIL:' '{}' \+ | xargs cat

    if test "$DIST" == "force"
    then
        $MAKE distcheck
    fi
}

main "$@"
