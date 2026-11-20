#!/bin/sh
# build + run every playtest; non-zero exit on any failure
cd "$(dirname "$0")/.." || exit 1
make || exit 1
fail=0
for t in boot walkout fight flow boss over soak sound text; do
  printf '%-6s ' $t
  tools/st.sh tests/$t.txt > /tmp/st_$t.log 2>&1 && echo PASS || { echo FAIL; fail=1; tail -5 /tmp/st_$t.log; }
done
exit $fail
