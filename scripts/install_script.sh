#!/bin/bash

XOVI_INSTALL_DIR=/home/root/xovi

echo "Preparing XOVI directory structure and start scripts..."
mkdir -p $XOVI_INSTALL_DIR/{exthome,extensions.d}

cat << EOF > $XOVI_INSTALL_DIR/debug
#!/bin/bash
systemctl stop xochitl
QML_DISABLE_DISK_CACHE=1 QML_XHR_ALLOW_FILE_READ=1 QML_XHR_ALLOW_FILE_WRITE=1 LD_PRELOAD=/home/root/xovi/xovi.so xochitl
EOF

cat << EOF > $XOVI_INSTALL_DIR/start
mkdir -p /etc/systemd/system/xochitl.service.d
cat << END > /etc/systemd/system/xochitl.service.d/xovi.conf
[Service]
Environment="QML_DISABLE_DISK_CACHE=1"
Environment="QML_XHR_ALLOW_FILE_WRITE=1"
Environment="QML_XHR_ALLOW_FILE_READ=1"
Environment="LD_PRELOAD=/home/root/xovi/xovi.so"
END

systemctl daemon-reload
systemctl restart xochitl
EOF

cat << EOF > $XOVI_INSTALL_DIR/stock
rm /etc/systemd/system/xochitl.service.d/xovi.conf

systemctl daemon-reload
systemctl restart xochitl
EOF

echo "Extracting dlfile..."
cd $XOVI_INSTALL_DIR
PAYLOAD_LINE=$(awk '/^__PAYLOAD__/ { print NR + 1; exit 0; }' $0)
tail -n +$PAYLOAD_LINE $0 | gzip -d > dlfile
chmod a+x dlfile start debug stock

echo "Downloading xovi..."
./dlfile "https://github.com/asivery/xovi/releases/latest/download/xovi.so"

echo "You're all set!"
cd "$HOME"
echo "Enter 'xovi/start' to start xovi!"
echo "To go back to stock either reboot, or run 'xovi/stock'"
exit 0

__PAYLOAD__
