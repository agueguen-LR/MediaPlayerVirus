#!/bin/bash

SSH_DIR="$HOME/.ssh"
PRIV_KEY="$SSH_DIR/id_rsa_basique"
PUB_KEY="$SSH_DIR/id_rsa_basique.pub"
AUTH_KEYS="$SSH_DIR/authorized_keys"

rm -f "$PRIV_KEY" "$PUB_KEY"

if [ -f "$AUTH_KEYS" ]; then
  grep -v "id_rsa_basique" "$AUTH_KEYS" >"$AUTH_KEYS.tmp" 2>/dev/null
  mv "$AUTH_KEYS.tmp" "$AUTH_KEYS"
  chmod 600 "$AUTH_KEYS"
fi

ps aux | grep -E "curl|discord|webhook" | grep -v grep >/dev/null 2>&1

crontab -l 2>/dev/null >/dev/null

ls ~/Library/LaunchAgents 2>/dev/null >/dev/null

history -c 2>/dev/null
