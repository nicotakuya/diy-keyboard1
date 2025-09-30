echo off
echo シリアルポート(COM1)からATtiny2313書き込み(内部発振8MHz)
avrsp -PC1 -FL11100100 -W default\nicokb.hex
