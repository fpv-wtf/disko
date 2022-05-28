#!/bin/bash
wayback_machine_downloader --directory ./mirror/ -c 5 -p 5000 http://www.diskohq.com/

find ./mirror/ \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 sed -i -e 's/http:\/\/www.diskohq.com\//\//g' -e 's/forum?func/forum_func/g' -e 's/.php?/.php_/g'
find ./mirror/ \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 sed -i 's/"\/\(.*\).php_\(.*\)"/"\/\1.php_\2.html"/g'

find ./mirror/ \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 sed -i 's/href=".*">Join our <br\/> mailing list!/href="https:\/\/github.com\/fpv-wtf\/disko">This is a <br\/> mirror!/g'
cp scripts/speakbubble.png mirror/templates/diskohq/images/speakbubble.png

find ./mirror/ -depth -type d -exec rename 's/forum\?func/forum_func/' '{}' \;
find ./mirror/ -depth -name *.php* -type f -exec rename 's/(.*).php\?(.*)/$1.php_$2.html/g' '{}' \;
