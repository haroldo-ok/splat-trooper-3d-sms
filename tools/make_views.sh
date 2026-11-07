#!/bin/bash
# The release ROM at each view width: wolf3d_192.sms (the normal one), wolf3d_160.sms, wolf3d_128.sms.  Narrower is
# faster; the field of view is the same.  Leaves gen/ and build/ made for 192 (the default) again.
set -euo pipefail
cd "$(dirname "$0")/.."
for w in 160 128 192; do
    VIEW_W=$w make >/dev/null
    cp wolf3d.sms "wolf3d_$w.sms"
    echo "wolf3d_$w.sms"
done
