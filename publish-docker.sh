#!/bin/bash

. version.sh

echo "# Publishing image $SPECIFIC_IMAGE_NAME #"
docker push $SPECIFIC_IMAGE_NAME

echo "# Publishing image $BRANCH_IMAGE_NAME #"
docker push $BRANCH_IMAGE_NAME
