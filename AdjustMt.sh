#!/bin/bash
#
# corrects src/src.pro, so it uses the right serialization library
# if $1 is STATIC_SER, it tries to do link Boost Ser statically; if it can't, it reverts to dynamic linking

#if lib $1 exists changes src/src.pro and terminates the script
function tryLib
{
    rm -f -r tstMt
    mkdir tstMt
    echo "int main() {}" > tstMt/a.cpp
    g++ tstMt/a.cpp -l$1 -o tstMt/a.out 2> /dev/null
    libExists=$?
    #echo $noMt
    rm -f -r tstMt

    if [ $libExists -eq 0 ] ; then
        cat src/src.pro | sed 's%.*boost_serialization.*%LIBS += -l'$1% > src/src.pro1
        mv -f src/src.pro1 src/src.pro
        echo Serialization Library set as $1
        exit 0
    fi

    echo Serialization Library $1 not found

    #return $libExists
}


if [[ "STATIC_SER" == $1 ]] ; then
    tryLib :libboost_serialization-mt.a
    tryLib :libboost_serialization.a # ttt0 not sure if this should be considered
    tryLib boost_serialization-mt
    tryLib boost_serialization
else
    tryLib boost_serialization-mt
    tryLib boost_serialization
fi

echo Boost Serialization not found
