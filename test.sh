#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
shopt -s nullglob

# Save compilation time (FlameGPU)
export SMS=20

# Perform lint-only tests in test/ directory.
# These check that errors are detected correctly.
for file in $DIR/test/*.abl; do
  noExtName=${file%.abl}
  echo $file
  $DIR/OpenABL --lint-only -i $file 2> $noExtName.out
  rm -f $noExtName.diff
  if [ -f $noExtName.exp ]; then
    diff -b $noExtName.out $noExtName.exp > $noExtName.diff
    if [ $? -ne 0 ]; then
        echo "DIFF $noExtName.diff"
        cat $noExtName.diff
    fi
  else
    echo "OUT Missing .exp file"
    cat $noExtName.out
  fi
done

TMP_DIR=$DIR/test-tmp
mkdir -p $TMP_DIR
for file in $DIR/examples/*abl; do
  noExtName=${file%.abl}
  baseName=$(basename $noExtName)
  echo $file

  TARGET_DIR=$TMP_DIR/$baseName
  mkdir -p $TARGET_DIR

  # Lint-only test
  $DIR/OpenABL --lint-only -i $file > $TARGET_DIR/lint.out
  if [ $? -ne 0 ]; then
    echo "LINT-FAIL $TARGET_DIR/lint.out"
    cat $TARGET_DIR/lint.out
    continue
  fi

  for backend in ${BACKENDS:-c flame flamegpu mason}; do
    echo "BACKEND $backend"

    BACKEND_DIR=$TARGET_DIR/$backend
    mkdir -p $BACKEND_DIR

    # Codegen test
    $DIR/OpenABL -i $file -o $BACKEND_DIR -b $backend
    if [ $? -ne 0 ]; then
      echo "CODEGEN-FAIL for backend $backend"
      continue
    fi

    # Build test
    if [ -f $BACKEND_DIR/build.sh ]; then
      echo "BUILD"
      pushd $BACKEND_DIR
      ./build.sh > build.log 2>&1
      if [ $? -ne 0 ]; then
        echo "BUILD-FAIL $BACKEND_DIR/build.log"
        cat build.log
      fi
      popd
    fi
  done
done
