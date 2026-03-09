#!/bin/bash
# One-time setup for Stormgate MUD on Oracle Cloud ARM Ubuntu VM.
# Usage: ssh into the VM, clone the repo, then run this script.
#   git clone https://github.com/rayhe/stormgate.git /opt/stormgate
#   sudo /opt/stormgate/deploy/setup.sh
set -euo pipefail

INSTALL_DIR="/opt/stormgate"
MUD_PORT=4000

if [ "$(id -u)" -ne 0 ]; then
    echo "Run as root: sudo $0"
    exit 1
fi

echo "=== Installing build dependencies ==="
apt-get update
apt-get install -y gcc g++ make libcrypt-dev

echo "=== Creating stormgate user ==="
id stormgate &>/dev/null || useradd --system --no-create-home --shell /usr/sbin/nologin stormgate

echo "=== Building stormgate ==="
cd "$INSTALL_DIR/src"
make clean
make -j$(nproc) -f makefile.prod

echo "=== Setting up directories ==="
mkdir -p "$INSTALL_DIR/log"
chown -R stormgate:stormgate "$INSTALL_DIR/player" "$INSTALL_DIR/log" "$INSTALL_DIR/area"

echo "=== Installing systemd service ==="
cp "$INSTALL_DIR/deploy/stormgate.service" /etc/systemd/system/
systemctl daemon-reload
systemctl enable stormgate

echo "=== Opening port $MUD_PORT in iptables ==="
if ! iptables -C INPUT -p tcp --dport "$MUD_PORT" -j ACCEPT 2>/dev/null; then
    iptables -I INPUT 1 -p tcp --dport "$MUD_PORT" -j ACCEPT
    netfilter-persistent save 2>/dev/null || iptables-save > /etc/iptables/rules.v4
fi

echo "=== Done ==="
echo "Start with: sudo systemctl start stormgate"
echo "Logs:       journalctl -u stormgate -f"
echo "Connect:    telnet <vm-ip> $MUD_PORT"
