GIT_SHA=`git log -1 --pretty=%H`
GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`

SPECIFIC_IMAGE_NAME=registry.son/szygi-foosball-monitor:$GIT_SHA
BRANCH_IMAGE_NAME=registry.son/szygi-foosball-monitor:$GIT_BRANCH
