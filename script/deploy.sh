#!/bin/sh

set -eux

BINARY="$1"

scp "$DIR/$BINARY" "pi@$HOST0:/tmp"


# shellcheck disable=SC2087
ssh "pi@$HOST0" <<EOF
set -eux
hostname
sudo whoami
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
ExecStart=/usr/bin/$BINARY

[Install]
WantedBy=multi-user.target
EOFF
sudo systemctl enable $BINARY.service
sudo systemctl restart $BINARY.service
rm -f /tmp/$BINARY /tmp/$BINARY.service
EOF
