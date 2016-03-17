#!/bin/bash
RELEASE="ns-3.24"
NS3DIR="${HOME}/source/${RELEASE}"

DIR=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)

cd ${DIR}

rsync -r --delete ./src/my-module ${NS3DIR}/src/
rsync -r --delete ./scratch ${NS3DIR}/
