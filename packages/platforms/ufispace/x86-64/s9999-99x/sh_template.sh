#!/bin/bash

function _platform_adjust {
    local src_pattern1="s9999-99x"
    local src_pattern2="s9999_99x"
    local src_pattern3="S9999-99X"
    local src_pattern4="S9999_99X"
    local src_pattern5="\.9999\.99"
    local src_pattern6="s9999\*99x"
    
    local dst_pattern1="s9705-48d"
    local dst_pattern2="s9705_48d"
    local dst_pattern3="S9705-48D"
    local dst_pattern4="S9705_48D"
    local dst_pattern5="\.9705\.48"
    local dst_pattern6="s9705\*48d"

    find . -type d -name "*${src_pattern2}*" | while read f; do mv $f $(echo $f | sed "s/${src_pattern2}/${dst_pattern2}/"); done
    find . -type d -name "*${src_pattern2}*" | while read f; do mv $f $(echo $f | sed "s/${src_pattern2}/${dst_pattern2}/"); done
    find . -type d -name "*${src_pattern2}*" | while read f; do mv $f $(echo $f | sed "s/${src_pattern2}/${dst_pattern2}/"); done
    find . -type f -name "*${src_pattern2}*" | while read f; do mv $f $(echo $f | sed "s/${src_pattern2}/${dst_pattern2}/"); done
    find . -type f -name "*${src_pattern1}*" | while read f; do mv $f $(echo $f | sed "s/${src_pattern1}/${dst_pattern1}/"); done

    sed -i "s/${src_pattern1}/${dst_pattern1}/g"  `grep "${src_pattern1}" -rl ./`
    sed -i "s/${src_pattern2}/${dst_pattern2}/g"  `grep "${src_pattern2}" -rl ./`
    sed -i "s/${src_pattern3}/${dst_pattern3}/g"  `grep "${src_pattern3}" -rl ./`
    sed -i "s/${src_pattern4}/${dst_pattern4}/g"  `grep "${src_pattern4}" -rl ./`
    sed -i "s/${src_pattern5}/${dst_pattern5}/g"  `grep "${src_pattern5}" -rl ./`
    sed -i "s/${src_pattern6}/${dst_pattern6}/g"  `grep "${src_pattern6}" -rl ./`
}

function _main {
    _platform_adjust
    echo "Please remove the ${0} before code committed!!"
}

_main


