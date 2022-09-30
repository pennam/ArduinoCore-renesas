#!/bin/bash

CORE_PATH=$(pwd -P)/../../../
TARGET=MUXTO

# this part changes the makefile generated by e2studio (removing ra_gen from the build)
# then it cleans the build and re-build all again

if ! grep -q "#\-include ra_gen\/subdir.mk" ./Debug/makefile; then
    sed -i 's/-include ra_gen\/subdir.mk/#-include ra_gen\/subdir.mk/g' ./Debug/makefile
    echo "not commented"
else 
    echo "already commented!"    
fi

cd Debug
make clean
make
cd ..
echo `pwd`


LIBRARY=`find . | grep "\.a$"`

echo Copying ${LIBRARY} to ${CORE_PATH}/variants/${TARGET}/libs/libfsp.a
if [ ! -d ${CORE_PATH}/variants/${TARGET}/libs ]
then
    mkdir ${CORE_PATH}/variants/${TARGET}/libs
fi
cp ${LIBRARY} ${CORE_PATH}/variants/${TARGET}/libs/libfsp.a

LINKER_SCRIPTS=`find . | grep "\.ld$"`
# cp ${LINKER_SCRIPTS} ${CORE_PATH}/variants/${TARGET}/

#-----------------------------------------------------------------------
# TEMPORARY CHANGE - TO BE REMOVED - used to copy temporary some genrated files
# those are the files that have been removed from the build at the beginning
# and now are copied to a temporary path (so that we can continue to build)
#------------------------------------------------------------------------
if [ ! -d ${CORE_PATH}/variants/${TARGET}/tmp_gen_c_files ]
then
    mkdir ${CORE_PATH}/variants/${TARGET}/tmp_gen_c_files
fi

cp ./ra_gen/*.c ${CORE_PATH}/variants/${TARGET}/tmp_gen_c_files/
# no more need of the following file
if test -f "${CORE_PATH}/variants/${TARGET}/tmp_gen_c_files/vector_data.c"; then
    echo "removed"
    rm ${CORE_PATH}/variants/${TARGET}/tmp_gen_c_files/vector_data.c
fi
#-------------------------------------------------------------------------


FILE_MK=`find . | grep all/subdir.mk`

CCCOMMAND=`cat $FILE_MK | grep \$\(file | cut -f2 -d","`

echo $CCCOMMAND

DEFINES=()
INCLUDES=()
FLAGS=()

OIFS=$IFS
IFS=' '
tokens=$CCCOMMAND
for x in $tokens
do
    if [[ $x == -D* ]]; then DEFINES+=( $x ); fi
    if [[ $x == -I* ]]; then INCLUDES+=( $x ); fi
    if [[ $x == -m* ]]; then FLAGS+=( $x ); fi
    if [[ $x == -f* ]]; then FLAGS+=( $x ); fi
    if [[ $x == -W* ]]; then FLAGS+=( $x ); fi
done
IFS=$OIFS

# Clean files
rm ${CORE_PATH}/variants/${TARGET}/defines.txt
rm ${CORE_PATH}/variants/${TARGET}/includes.txt
rm ${CORE_PATH}/variants/${TARGET}/cflags.txt
rm ${CORE_PATH}/variants/${TARGET}/cxxflags.txt

for value in "${DEFINES[@]}"
do
    echo $value >> ${CORE_PATH}/variants/${TARGET}/defines.txt
done

echo ${INCLUDES[@]}

if [ ! -d ${CORE_PATH}/variants/${TARGET}/includes/ ]
then
    mkdir ${CORE_PATH}/variants/${TARGET}/includes/
else
    rm -r ${CORE_PATH}/variants/${TARGET}/includes/*
fi

for value in "${INCLUDES[@]}"
do
    INCLUDE_PATH=`echo $value | cut -f2 -d"\"" | cut -f1 -d"\""`
    echo $INCLUDE_PATH
    PROJECT_FOLDER=$(pwd -P)
    # temporarily, copy everything staring with "ra_" in variant/includes/ , everything with ra in core folder
    if [[ $INCLUDE_PATH == */ra* ]]; then
        INCLUDE_PATH_REL=`echo ${INCLUDE_PATH} | sed 's/.*ra//'`
        INCLUDE_PATH_REL=ra${INCLUDE_PATH_REL}
        #cp -r --parent $INCLUDE_PATH ${CORE_PATH}/variants/${TARGET}/includes/
        echo "-iwithprefixbefore/variants/${TARGET}/includes/$INCLUDE_PATH_REL" >> ${CORE_PATH}/variants/${TARGET}/includes.txt
    fi
    #rel_path=`echo $value | sed -e "s#-I$PWD#-iwithprefixbefore/fsp#g"`
    #echo $rel_path >> ${CORE_PATH}/variants/${TARGET}/includes.txt

    # TODO: check how many include folders are generated and if it makes sense to track them manually
done

rm -rf ${CORE_PATH}/variants/${TARGET}/includes/ra/*
rm -rf ${CORE_PATH}/variants/${TARGET}/includes/ra_gen/*

RA_INCLUDES=`find ra/ -iname *.h`
cp --parent $RA_INCLUDES ${CORE_PATH}/variants/${TARGET}/includes/

RA_INCLUDES=`find ra_gen/ -iname *.h`
cp --parent $RA_INCLUDES ${CORE_PATH}/variants/${TARGET}/includes/

RA_INCLUDES=`find ra_cfg/ -iname *.h`
cp --parent $RA_INCLUDES ${CORE_PATH}/variants/${TARGET}/includes/

ELC_TABLE=`find ra/ | grep bsp_elc.h`
ELC_ENTRIES=`cat ${ELC_TABLE} | grep "ELC_EVENT" | cut -f1 -d "="`
for entry in $ELC_ENTRIES
do
    echo "#define $entry $entry" >> ${CORE_PATH}/variants/${TARGET}/elc_defines.h
done

for value in "${FLAGS[@]}"
do
    echo $value >> ${CORE_PATH}/variants/${TARGET}/cflags.txt
    echo $value >> ${CORE_PATH}/variants/${TARGET}/cxxflags.txt
done
