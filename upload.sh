#!/bin/bash

PRJNAME=skaffari
PRJVER=0.0.99
PRJREL=1
BSPRJ=home:buschmann23:Cutelyst:devel:apps
BSBASE=/home/buschmann/Dokumente/Entwicklung/Build\ Service
BSDIR="${BSBASE}/${BSPRJ}/${PRJNAME}"
# BSDIR=/home/buschmann/Dokumente/Entwicklung/Build\ Service/home\:buschmann23\:Cutelyst\:devel\:apps/${PRJNAME}
BSDIST=openSUSE_Leap_42.3
BSBUILDROOT=/var/tmp/build-root
BSBUILDDIR=/home/abuild/rpmbuild/RPMS
BSARCH=x86_64
SPECFILE="${BSDIR}/${PRJNAME}.spec"
SSHHOST=hbn
SSHDIR=/home/buschmann/packages

if [ ! -d "${BSDIR}" ]
then
    echo "Verzeichnis ${BSDIR} existiert nicht."
    exit 1
fi

CURVER=$(grep ^Version: "${SPECFILE}" | sed 's/^Version: *\([0-9]\+\.[0-9]\+\.[0-9]\+\)/\1/')
CURREL=$(grep ^Release: "${SPECFILE}" | sed 's/^Release: *\([0-9]\+\)/\1/')

if [ $PRJVER != $CURVER ]
then
PRJREL=1
else
PRJREL=$((CURREL+1))
fi

./release.sh $PRJVER "${BSDIR}"

if [ $? -ne 0 ]
then
exit 1
fi

pushd "${BSDIR}" \
&& sed -i "s/^\(Version: *\)\([0-9]\+\.[0-9]\+\.[0-9]\+\)/\1${PRJVER}/" "${SPECFILE}" \
&& sed -i "s/^\(Release: *\)\([0-9]\+\)/\1${PRJREL}/" "${SPECFILE}" \
&& osc build --release=${PRJREL} --no-init $BSDIST $BSARCH ${PRJNAME}.spec \
&& scp ${BSBUILDROOT}/${BSDIST}-${BSARCH}${BSBUILDDIR}/${BSARCH}/${PRJNAME}-${PRJVER}-${PRJREL}.${BSARCH}.rpm ${SSHHOST}:${SSHDIR} \
&& popd \
&& kdialog --title "${PRJNAME}" --passivepopup "Hochladen fertig" 3
