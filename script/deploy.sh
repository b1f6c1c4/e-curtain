#!/bin/sh

set -eux

DIR="$1"
BINARY="${DIR##*/}"
shift
HOST="$1"
shift

scp "$DIR" "pi@$HOST:/tmp"

# shellcheck disable=SC2087
ssh "pi@$HOST" <<EOF
set -eux
sudo install /tmp/$BINARY /usr/bin
sudo tee /etc/systemd/system/$BINARY.service <<EOFF
[Unit]
Description=e-curtain/$BINARY
After=network.target
StartLimitIntervalSec=0

[Service]
Type=simple
Restart=always
RestartSec=5
User=root
ExecStart=/usr/bin/$BINARY $*

[Install]
WantedBy=multi-user.target
EOFF
sudo systemctl enable $BINARY.service
sudo systemctl restart $BINARY.service
rm -f /tmp/$BINARY
EOF
