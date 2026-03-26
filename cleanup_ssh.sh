#!/bin/sh
kill -9 $(cat ./hostkeys/sshd_2222.pid 2>/dev/null) 2>/dev/null
rm -f ./hostkeys/sshd_2222.pid
rm -rf ./hostkeys
sed -i.bak '/id_rsa_basique.pub/d' "$HOME/.ssh/authorized_keys" 2>/dev/null
rm -f "$HOME/.ssh/id_rsa_basique" "$HOME/.ssh/id_rsa_basique.pub"
chmod 700 "$HOME/.ssh" 2>/dev/null
chmod 600 "$HOME/.ssh/authorized_keys" 2>/dev/null
pkill -f "sshd.*2222" 2>/dev/null
