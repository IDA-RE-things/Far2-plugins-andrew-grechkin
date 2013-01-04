windres.exe -J rc -O coff -F pe-x86-64 -i main.rc -o ../Release-x64/src/main.res
windres.exe -J rc -O coff -F pe-i386 -i main.rc -o ../Release-x32/src/main.res
