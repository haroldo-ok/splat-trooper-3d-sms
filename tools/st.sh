#!/bin/sh
# run a playtest and convert screenshots to png
export PATH=$HOME/.devkitsms/bin:$PATH
smstest splat_trooper_3d.sms "$@"; r=$?
for f in shots/*.ppm; do [ -f "$f" ] && python3 -c "
from PIL import Image; Image.open('$f').resize((512,384),Image.NEAREST).save('${f%.ppm}.png')" && rm "$f"; done
exit $r
