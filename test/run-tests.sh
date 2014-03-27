#!/bin/sh

for t in $*
do
   echo -n "Test de $t ... "
   ./$t 2>&1 > /dev/null
   if [ $? -eq 0 ]; then
      echo "[OK]"
   else
      echo "[ERREUR]"
   fi
done
