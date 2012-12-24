#!/bin/sh

TOP_DIR="${TOP_SRCDIR:-..}"
LIB_DIR="${TOP_DIR}/lib"
GIT_DIR="${TOP_DIR}/.git"

VERSION_HEADER="${LIB_DIR}/marisa2/version.h"
echo "VERSION_HEADER = ${VERSION_HEADER}"

VERSION_MACRO="MARISA2_VERSION"

if [ -d ${GIT_DIR} ]
then
  CURRENT_VERSION=`git describe --abbrev=7 HEAD 2>/dev/null`
  echo "CURRENT_VERSION = ${CURRENT_VERSION}"

  if [ -r ${VERSION_HEADER} ]
  then
    OLD_VERSION=`grep "^#define ${VERSION_MACRO} " ${VERSION_HEADER} |\
                 sed -e "s/^#define ${VERSION_MACRO} \"\(.*\)\"/\1/"`
  fi
  echo "OLD_VERSION = ${OLD_VERSION}"

  if [ "${OLD_VERSION}" != "${CURRENT_VERSION}" ]
  then
    echo "#ifndef MARISA2_VERSION_H" > ${VERSION_HEADER}
    echo "#define MARISA2_VERSION_H" >> ${VERSION_HEADER}
    echo >> ${VERSION_HEADER}
    echo "#define ${VERSION_MACRO} \"${CURRENT_VERSION}\"" >> ${VERSION_HEADER}
    echo >> ${VERSION_HEADER}
    echo "#endif  // MARISA2_VERSION_H" >> ${VERSION_HEADER}
  fi
fi
