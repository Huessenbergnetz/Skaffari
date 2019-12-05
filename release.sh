#!/bin/bash

PRJNAME=skaffari

if [ $# -lt 2 ]
then
    echo "Missing version and output directory"
    exit 1
fi

VERSION=$1
shift
OUTPUTDIR=$1
shift

RUNNPM=1
SIGNTARBALL=1

if [ -z $VERSION ]
then
    echo "Missing version";
    exit 1;
fi

if [ -z $OUTPUTDIR ]
then
    echo "Missing output directory";
    exit 2;
fi

while [ -n "$1" ]
do
    case $1 in
        --no-npm) RUNNPM=0;;
        --no-signing) SIGNTARBALL=0;;
    esac
    shift
done

DIR="/tmp/${PRJNAME}-${VERSION}"

compasset() {
    FILE=$1
    FUG=$(stat --printf="%U:%G" ${FILE})
    FM=$(stat --printf="%a" ${FILE})
    echo "Compressing: ${FILE}"
    zopfli -i30 $FILE
    chown ${FUG} ${FILE}.gz
    chmod ${FM} ${FILE}.gz
    brotli -f -Z -k $FILE
    chown ${FUG} ${FILE}.br
    chmod ${FM} ${FILE}.br
}
export -f compasset

rm -rf $DIR

mkdir $DIR

mkdir ${DIR}/templates

pushd templates
cp CMakeLists.txt ${DIR}/templates

cp -r static ${DIR}/templates

for TMPL in default
do
    pushd $TMPL
    if [ $RUNNPM -gt 0 ]
    then
        npm run prod
        pushd static
        find $PWD -type f -name "*.css" -o -type f -name "*.js" -o -type f -name "*.json" | parallel compasset
        popd
    fi
    mkdir ${DIR}/templates/${TMPL}
    cp metadata.json ${DIR}/templates/${TMPL}
    cp CMakeLists.txt ${DIR}/templates/${TMPL}
    cp -r site ${DIR}/templates/${TMPL}
    cp -r static ${DIR}/templates/${TMPL}
    cp -r l10n ${DIR}/templates/${TMPL}
    popd
done

popd

for SRCDIR in common cmd sql src doc l10n grantlee supplementary tests
do
    cp -r $SRCDIR $DIR
done

for SRCFILE in CMakeLists.txt LICENSE CHANGELOG contribute.json
do
    cp $SRCFILE $DIR
done

pushd /tmp
rm -f "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz"
rm -f "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz"
tar -cJf "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz" ${PRJNAME}-${VERSION}
tar -czf "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz" ${PRJNAME}-${VERSION}
rm -rf ${PRJNAME}-${VERSION}
popd

if [ $SIGNTARBALL -gt 0 ]
then
    gpg --armor --detach-sign --yes --default-key 6607CA3F41B25F45 --output "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz.sig" "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz"
    gpg --armor --detach-sign --yes --default-key 6607CA3F41B25F45 --output "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz.sig" "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz"
fi
exit 0
