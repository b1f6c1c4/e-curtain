#!/bin/sh

set -eux

BINARY="$1"
HOST="$2"

# shellcheck disable=SC2087
ssh "pi@$HOST" <<EOF
sudo systemctl stop $BINARY.service
sudo systemctl disable $BINARY.service
sudo rm -f /usr/bin/$BINARY
sudo rm -f /etc/systemd/system/$BINARY.service
EOF
