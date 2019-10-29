#!/bin/bash

export INKSCAPE_CMD=`which inkscape`
export CONVERT_CMD=`which convert`
export BC_CMD=`which bc`
export ZOPFLI_CMD=`which zopflipng`
export ZOPFLI_OPTS="-y --iterations=250 --filters=01234mepb --lossy_8bit --lossy_transparent"
PARALLEL_CMD=`which parallel`

export BACKGROUND_COLOR="transparent"
export TARGET_DIR=../static/img/favicons
export SRC_FILE=../static/img/favicons/favicon.svg
SIZES="512 384 256 192 144 96 72 48 36 32 16"

export APPLE_TOUCH_BACKGOUND="transparent"
APPLE_TOUCH_SIZES="180 152 144 120 114 76 72 60 57"

if [ -z $INKSCAPE_CMD ] || [ ! -f $INKSCAPE_CMD ] || [ ! -x $INKSCAPE_CMD ]
then
    echo "Can not find \"inkscape\" executable!"
    exit 1
fi

if [ -z $CONVERT_CMD ] || [ ! -f $CONVERT_CMD ] || [ ! -x $CONVERT_CMD ]
then
    echo "Can not find ImageMagick \"convert\" executable!"
    exit 1
fi

if [ -z $ZOPFLI_CMD ] || [ ! -f $ZOPFLI_CMD ] || [ ! -x $ZOPFLI_CMD ]
then
    echo "Can not find \"zopflipng\" executable!"
    exit 1
fi

if [ -z $BC_CMD ] || [ ! -f $BC_CMD ] || [ ! -x $BC_CMD ]
then
    echo "Can not find \"bc\" calculator executable!"
    exit 1
fi

if [ -z $SRC_FILE ] || [ ! -f $SRC_FILE ] || [ ! -r $SRC_FILE ]
then
    echo "Can not find source file \"$SRC_FILE\"!"
    exit 1
fi

if [ ! -d $TARGET_DIR ]
then
    mkdir -p $TARGET_DIR

    if [ ! -d $TARGET_DIR ]
    then
        echo "Can not create target directory!"
        exit 1
    fi

    if [ ! -w $TARGET_DIR ]
    then
        echo "Can not write to target directory!"
        exit 1
    fi
fi

function create_favicon {
    local SIZE=$1

    local OUT_FILE="$TARGET_DIR/favicon-$SIZE.png"

    if [ ! -f $OUT_FILE ]
    then
        local TMP_FILE=$(mktemp --suffix .png)

        echo "Creating $OUT_FILE"

        $INKSCAPE_CMD -z -e $TMP_FILE -w $SIZE -h $SIZE $SRC_FILE &> /dev/null
        $ZOPFLI_CMD $ZOPFLI_OPTS $TMP_FILE $OUT_FILE

        rm $TMP_FILE
    else
        echo "${OUT_FILE} already exists"
    fi
}
export -f create_favicon

if [ -z $PARALLEL_CMD ] || [ ! -f $PARALLEL_CMD ] || [ ! -x $PARALLEL_CMD ]
then
    for SIZE in $SIZES
    do
        create_favicon $SIZE
    done
else
    $PARALLEL_CMD create_favicon ::: $SIZES
fi

if [ ! -f ../static/favicon.ico ]
then
    $CONVERT_CMD $TARGET_DIR/favicon-16.png \
                 $TARGET_DIR/favicon-32.png \
                 $TARGET_DIR/favicon-48.png \
                 ../static/favicon.ico
fi

function create_apple_touch {
    local OUTER=$1
    local INNER=$(echo "$OUTER*0.84" | $BC_CMD)
    INNER=$(LC_ALL=C printf '%.0f' $INNER)
    local OUT_FILE="$TARGET_DIR/apple-touch-icon-${OUTER}x${OUTER}.png"

    if [ ! -f $OUT_FILE ]
    then
        echo "Creating $OUT_FILE"
        local TMP_FILE1=$(mktemp --suffix .png)
        local TMP_FILE2=$(mktemp --suffix .png)

        $INKSCAPE_CMD -z -e $TMP_FILE1 -w $INNER -h $INNER -b "$APPLE_TOUCH_BACKGOUND" $SRC_FILE &> /dev/null
        $CONVERT_CMD $TMP_FILE1 -quiet -background $APPLE_TOUCH_BACKGOUND -gravity center -extent ${OUTER}x${OUTER} $TMP_FILE2
        $ZOPFLI_CMD $ZOPFLI_OPTS $TMP_FILE2 $OUT_FILE

        rm $TMP_FILE1
        rm $TMP_FILE2
    else
        echo "${OUT_FILE} already exists"
    fi
}
export -f create_apple_touch

if [ -z $PARALLEL_CMD ] || [ ! -f $PARALLEL_CMD ] || [ ! -x $PARALLEL_CMD ]
then
    for SIZE in $APPLE_TOUCH_SIZES
    do
        create_apple_touch $SIZE
    done
else
    $PARALLEL_CMD create_apple_touch ::: $APPLE_TOUCH_SIZES
fi

function create_mstile {
    local INNER=$1
    local OUTER=$2
    local NAME=$3
    local OUT_FILE="$TARGET_DIR/${NAME}tile.png"

    if [ ! -f $OUT_FILE ]
    then
        local TMP_FILE=$(mktemp --suffix .png)

        echo "Creating $OUT_FILE"

        $INKSCAPE_CMD -z -e $TMP_FILE -w $INNER -h $INNER $SRC_FILE &> /dev/null
        $CONVERT_CMD $TMP_FILE -quiet -background transparent -gravity center -extent ${OUTER}x${OUTER} $OUT_FILE

        rm $TMP_FILE
    fi
}

# create_mstile 96 128 small
# create_mstile 128 270 medium
# create_mstile 260 558 large
# create_mstile 128 558x270 wide
