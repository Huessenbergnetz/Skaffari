#!/bin/bash

PRJNAME=skaffari

if [ ! $1 ]
then
    echo "Missing version";
    exit 1;
fi

if [ ! $2 ]
then
    echo "Missing output directory";
    exit 2;
fi

DIR="/tmp/${PRJNAME}-${1}"

compasset() {
    FILE=$1
    FUG=$(stat --printf="%U:%G" ${FILE})
    FM=$(stat --printf="%a" ${FILE})
    echo "Compressing: ${FILE}"
    zopfli -i30 $FILE
    chown ${FUG} ${FILE}.gz
    chmod ${FM} ${FILE}.gz
    bro --force --quality 99 --input $FILE --output ${FILE}.br
    chown ${FUG} ${FILE}.br
    chmod ${FM} ${FILE}.br
}
export -f compasset

rm -rf $DIR

mkdir $DIR

mkdir ${DIR}/templates

pushd templates
cp CMakeLists.txt ${DIR}/templates

for TMPL in default
do
    pushd $TMPL
    npm update --dev
    ./gulp --production
    pushd static
    find $PWD -type f -name "*.css" -o -type f -name "*.js" | parallel compasset
    popd
    mkdir ${DIR}/templates/${TMPL}
    cp CMakeLists.txt ${DIR}/templates/${TMPL}
    cp -r site ${DIR}/templates/${TMPL}
    cp -r static ${DIR}/templates/${TMPL}
    popd
done

popd

for SRCDIR in common ctl sql src
do
    cp -r $SRCDIR $DIR
done

for SRCFILE in CMakeLists.txt LICENSE
do
    cp $SRCFILE $DIR
done

pushd /tmp
rm -f ${2}/${PRJNAME}-${1}.tar.xz
rm -f ${2}/${PRJNAME}-${1}.tar.gz
tar -cJf ${2}/${PRJNAME}-${1}.tar.xz ${PRJNAME}-${1}
tar -czf ${2}/${PRJNAME}-${1}.tar.gz ${PRJNAME}-${1}
rm -rf ${PRJNAME}-${1}
popd

exit 0
