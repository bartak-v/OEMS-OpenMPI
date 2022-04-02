#!/usr/bin/bash
# ZMĚNIT NA /bin/bash na merlinovi
#pocet cisel zadam nebo 8
numbers=8

#preklad cpp zdrojaku
mpic++ --prefix /usr/local/share/OpenMPI -o oems oems.cpp

#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers status=none

#spusteni PRO 4
mpirun --prefix /usr/local/share/OpenMPI -np 4 oems

#uklid
rm -f oems numbers