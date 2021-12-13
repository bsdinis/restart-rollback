#!/usr/bin/env zsh

if [[ "$#" -ne 1 ]]
then
    echo "usage: ./clone.sh <new_name>"
    echo "eg: ./clone.sh epicQP # creates a root level dir called epicQP, with the corresponding QP and client interface"
    exit 1;
fi

orig=$(basename $(pwd))
target=$(basename $1)

echo "Are you sure you want to convert your ${orig} to ${target}? [y/n]"
read ans

make -C clnt clean;
make -C noQP clean;
make -C QP clean;

echo ${ans} "|"
case "${ans}" in
    y|Y)
        echo "Converting to ${target}";
        cp -r ../${orig} ../${target};
        cd ../${target}
        rm clnt/*generated*
        rm QP/enclave/*generated*
        rm noQP/*generated*

        mv proto/${orig}.fbs proto/${target}.fbs;
        mv proto/${orig}_req.fbs proto/${target}_req.fbs;
        mv proto/${orig}_resp.fbs proto/${target}_resp.fbs;
        mv QP/app/${orig}.cc QP/app/${target}.cc
        mv QP/app/${orig}.h QP/app/${target}.h
        mv noQP/${orig}.cc noQP/${target}.cc
        sed -i -e 's/'${orig}'/'${target}'/g' .gitignore proto/*.fbs proto/*.sh clnt/Makefile clnt/*.cc clnt/*.h QP/sgxMakefile QP/Makefile QP/app/*.cc QP/app/*.h QP/enclave/*.cc QP/enclave/enclave.edl QP/enclave/*.h QP/include/*.h noQP/Makefile noQP/*.h noQP/*.cc README.md clnt/README.md noQP/README.md QP/README.md proto/README.md

        echo "Compiling..."
        echo "Protocol..."
        (cd proto && ./clean.sh && ./compile.sh && ./install.sh);
        make -C clnt depend;
        make -C noQP depend;
        echo "Client..."
        (cd clnt && make all);
        echo "QP..."
        (cd QP && make all);
        echo "noQP..."
        (cd noQP && make all);
        ;;
    n|N)
        echo "Aborted";
        ;;
esac
