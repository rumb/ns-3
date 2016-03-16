#!/bin/bash

# 転送するファイル一覧
#scp_list=('wscript'
#'model/my-router-module.cc'
#'model/my-app-packet.cc'
#'model/my-router-app.cc'
#'model/my-endnode-app.cc'
#'model/my-router-module.h'
#'model/my-app-packet.h'
#'model/my-router-app.h'
#'model/my-endnode-app.h'
#'helper/my-app-helper.cc'
#'helper/my-app-helper.h'
#)
scp_list=('wscript' 'model' 'helper')

#dummy_file="scratch-simulator.cc"

# SSHのセッティング
user_name="sunaga"
host_name="133.11.236.15"
key_path="/Users/sunaga/.ssh/ISTCloud/id_rsa_ndnsim"
# 転送先のディレクトリパス
dir_path='/home/sunaga/ns-dev/ns-3/src/my-module/'

if [ -n "$1" ]; then

  for n in $@
  do
    case ${n} in
      "a" )
        scp -i ${key_path} -r ${scp_list[@]} ${user_name}@${host_name}:${dir_path}
        ;;

      [0-9]* )
        scp -i ${key_path} -r ${scp_list[$n]} ${user_name}@${host_name}:${dir_path}
        ;;
    esac
  done
  # 転送完了タイムスタンプ
  date

else

  i=0

  for filename in ${scp_list[@]}
  do
    echo ${i}:${filename}
    ((i++))
  done

fi
