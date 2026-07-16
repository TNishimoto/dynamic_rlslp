
#!/bin/bash
cd -- "$(dirname -- "${BASH_SOURCE[0]}")" || exit 1

mkdir -p ../build/experiment/indexes
mkdir -p ../build/experiment/logs

list=("world_leaders")
#list=("cere" "coreutils" "einstein.de.txt" "einstein.en.txt" "Escherichia_Coli" "influenza" "kernel" "para" "world_leaders" )

for item in "${list[@]}" ; do
  set -- $item
  echo $1
  echo "Building restricted_recompression - fast - text: $1"
  /usr/bin/time -l ../build/build -i ../build/experiment/datasets/$1 -o ../build/experiment/indexes/$1.rr.fa.ds -p restricted_recompression -m fast >> ../build/experiment/logs/online_build.rr.fa.log 2>&1
  echo "Building restricted_recompression - standard - text: $1"
  /usr/bin/time -l ../build/build -i ../build/experiment/datasets/$1 -o ../build/experiment/indexes/$1.rr.st.ds -p restricted_recompression -m standard >> ../build/experiment/logs/online_build.rr.st.log 2>&1
  echo "Building signature_encoding - fast - text: $1"
  /usr/bin/time -l ../build/build -i ../build/experiment/datasets/$1 -o ../build/experiment/indexes/$1.se.fa.ds -p signature_encoding -m fast >> ../build/experiment/logs/online_build.se.fa.log 2>&1
  echo "Building signature_encoding - standard - text: $1"
  /usr/bin/time -l ../build/build -i ../build/experiment/datasets/$1 -o ../build/experiment/indexes/$1.se.st.ds -p signature_encoding -m standard >> ../build/experiment/logs/online_build.se.st.log 2>&1
done

