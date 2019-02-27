#!/bin/bash

. version.sh

docker build . -t $SPECIFIC_IMAGE_NAME

if [ $? != 0 ]; then
    echo "Failed to build image $SPECIFIC_IMAGE_NAME"
    exit 1
fi

docker tag $SPECIFIC_IMAGE_NAME $BRANCH_IMAGE_NAME
