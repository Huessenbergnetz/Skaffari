#!/bin/bash

PRJNAME=skaffari

if [ ! $1 ]
then
    echo "Missing version";
    exit 1;
fi

if [ ! "$2" ]
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

for TMPL in default
do
    pushd $TMPL
    npm run prod
    pushd static
    find $PWD -type f -name "*.css" -o -type f -name "*.js" -o -type f -name "*.json" | parallel compasset
    popd
    mkdir ${DIR}/templates/${TMPL}
    cp metadata.json ${DIR}/templates/${TMPL}
    cp CMakeLists.txt ${DIR}/templates/${TMPL}
    cp -r site ${DIR}/templates/${TMPL}
    cp -r static ${DIR}/templates/${TMPL}
    cp -r l10n ${DIR}/templates/${TMPL}
    popd
done

popd

for SRCDIR in common cmd sql src doc l10n grantlee
do
    cp -r $SRCDIR $DIR
done

for SRCFILE in CMakeLists.txt LICENSE CHANGELOG contribute.json
do
    cp $SRCFILE $DIR
done

pushd /tmp
rm -f "${2}/${PRJNAME}-${1}.tar.xz"
rm -f "${2}/${PRJNAME}-${1}.tar.gz"
tar -cJf "${2}/${PRJNAME}-${1}.tar.xz" ${PRJNAME}-${1}
tar -czf "${2}/${PRJNAME}-${1}.tar.gz" ${PRJNAME}-${1}
rm -rf ${PRJNAME}-${1}
popd

gpg --armor --detach-sign --yes --default-key 3A70A936614C3258 "${2}/${PRJNAME}-${1}.tar.xz"
gpg --armor --detach-sign --yes --default-key 3A70A936614C3258 "${2}/${PRJNAME}-${1}.tar.gz"

exit 0
