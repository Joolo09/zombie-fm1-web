#!/bin/sh
# Host compiler with AddressSanitizer + UBSan for tests/run_groovebox_audit.py --fuzz (POSIX clang/gcc).
# -std=gnu99 for clock_gettime; shift-base is off: the platform's fastmath scales floats with n << 23.
for x in "$@"; do [ "$x" = "-std=c99" ] && x="-std=gnu99"; [ "$x" = "-O2" ] && x="-O1"; set -- "$@" "$x"; shift; done
exec "${ASAN_CC:-cc}" -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-sanitize=shift-base -fno-omit-frame-pointer "$@"
