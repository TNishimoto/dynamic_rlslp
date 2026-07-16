
#!/bin/bash
cd -- "$(dirname -- "${BASH_SOURCE[0]}")" || exit 1

mkdir -p ../build/experiment/logs

list=("world_leaders")
#list=("cere" "coreutils" "einstein.de.txt" "einstein.en.txt" "Escherichia_Coli" "influenza" "kernel" "para" "world_leaders" )

for item in "${list[@]}" ; do
  set -- $item
  echo $1
  echo "Printing restricted_recompression - fast - text: $1"
  /usr/bin/time -l ../build/print -i ../build/experiment/indexes/$1.rr.fa.ds >> ../build/experiment/logs/index_memory.rr.fa.log 2>&1
  echo "Printing restricted_recompression - standard - text: $1"
  /usr/bin/time -l ../build/print -i ../build/experiment/indexes/$1.rr.st.ds >> ../build/experiment/logs/index_memory.rr.st.log 2>&1
  echo "Printing signature_encoding - fast - text: $1"
  /usr/bin/time -l ../build/print -i ../build/experiment/indexes/$1.se.fa.ds >> ../build/experiment/logs/index_memory.se.fa.log 2>&1
  echo "Printing signature_encoding - standard - text: $1"
  /usr/bin/time -l ../build/print -i ../build/experiment/indexes/$1.se.st.ds >> ../build/experiment/logs/index_memory.se.st.log 2>&1
done
