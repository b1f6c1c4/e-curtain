#!/bin/sh

set -eux

DIR="$1"
BINARY="${DIR##*/}"
shift
HOST="$1"
shift

# shellcheck disable=SC2087
ssh "pi@$HOST" <<EOF
sudo systemctl stop $BINARY.service
sudo systemctl disable $BINARY.service
sudo rm -f /usr/bin/$BINARY
sudo rm -f /etc/systemd/system/$BINARY.service
EOF
