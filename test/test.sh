#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
shopt -s nullglob
for file in $DIR/*.abl; do
  baseName=${file%.abl}
  echo $file
  $DIR/../OpenABL --lint-only -i $file 2> $baseName.out
  rm -f $baseName.diff
  if [ -f $baseName.exp ]; then
    diff -b $baseName.out $baseName.exp > $baseName.diff
	if [ $? -ne 0 ]; then
		echo "DIFF $baseName.diff"
		cat $baseName.diff
	fi
  else
	echo "OUT Missing .exp file"
	cat $baseName.out
  fi
done
