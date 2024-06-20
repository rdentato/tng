:: Build script for Micrsoft C

cl /std:c11 /O2 /I..\extlibs /c /Fe:val.obj ..\extlibs\val.c
cl /std:c11 /O2 /I..\extlibs /Fe:tng.exe boot\tng_boot.c val.obj
tng -o tng.c tng.md
cl /std:c11 /O2 /I..\extlibs /Fe:tng.exe tng.c val.obj
del *.obj
