#!/usr/bin/bash
numbers=8
mpic++ --prefix /usr/local/share/OpenMPI -o oems oems.cpp
dd if=/dev/random bs=1 count=$numbers of=numbers status=none
mpirun --oversubscribe --prefix /usr/local/share/OpenMPI -np 19 oems 
rm -f oems numbers