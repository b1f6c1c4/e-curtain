#!/bin/sh

set -eux

BINARY="$1"

# shellcheck disable=SC2087
ssh "pi@$HOST0" <<EOF
hostname
sudo whoami
sudo systemctl stop $BINARY.service
sudo systemctl disable $BINARY.service
sudo rm -f /usr/bin/$BINARY
sudo rm -f /etc/systemd/system/$BINARY.service
EOF
