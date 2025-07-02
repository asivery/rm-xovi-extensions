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
mount -t tmpfs tmpfs /etc/systemd/system/xochitl.service.d
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

cat << EOF > $XOVI_INSTALL_DIR/rebuild_hashtable
#!/bin/bash

if [[ ! -e '/home/root/xovi/extensions.d/qt-resource-rebuilder.so' ]]; then
    echo "Please install qt-resource-rebuilder before updating the hashtable"
    exit 1
fi

# stop systemwide gui process
systemctl stop xochitl.service

if pidof xochitl; then
  kill -15 \$(pidof xochitl)
fi

# make sure the resource-rebuilder folder exists.
mkdir -p /home/root/xovi/exthome/qt-resource-rebuilder

# remove the actual hashtable
rm -f /home/root/xovi/exthome/qt-resource-rebuilder/hashtab

# start update hashtab process
echo -e "#################################"
echo -e "Now we will run the GUI process with the arguments to build a new hashtable."
echo -e "\n\t[...]"
echo -e "\t[qmldiff] [Hashtab Rule Processor]: Hashed derived '...'"
echo -e "\t[qmldiff]: Hashtab saved to /home/root/xovi/exthome/qt-resource-rebuilder/hashtab"
echo -e "\nWhen we find this line in the output the process is stopped and the systemd service is started again."
echo -e "\nPlease enter your password on the rM when prompted."
read -p "Please press enter to continue:"

echo -e "\n\nOutput:"
sleep 3

QMLDIFF_HASHTAB_CREATE=/home/root/xovi/exthome/qt-resource-rebuilder/hashtab QML_DISABLE_DISK_CACHE=1 LD_PRELOAD=/home/root/xovi/xovi.so /usr/bin/xochitl 2>&1 | while IFS= read line; do
  echo -e "\$line"
  if [[ "\$line" == "[qmldiff]: Hashtab saved to /home/root/xovi/exthome/qt-resource-rebuilder/hashtab" ]]; then
    # yep, found the line we are wating for. exiting the process.
    echo -e "\n##############"
    echo -e "Found expected output. Killing gui process and restarting systemd service."
    kill -15 \$(pidof xochitl)
  fi
done

# so, we seem to be finished.
# wait another 5 seconds, then run the normal gui process via systemd
sleep 5
echo -e "Now starting the systemd xochitl service back up..."
systemctl start xochitl.service
EOF

echo "Extracting dlfile..."
PAYLOAD_LINE=$(awk '/^__PAYLOAD__/ { print NR + 1; exit 0; }' $0)
tail -n +$PAYLOAD_LINE $0 | gzip -d > $XOVI_INSTALL_DIR/dlfile
cd $XOVI_INSTALL_DIR
chmod a+x dlfile start debug stock rebuild_hashtable

echo "Downloading xovi..."
if [[ $(uname -m) == "aarch64" ]]; then
    XOVI_FILE="xovi-aarch64.so"
else
    XOVI_FILE="xovi-arm32.so"
fi
./dlfile "https://github.com/asivery/xovi/releases/latest/download/$XOVI_FILE" xovi.so

# Checks if file was downloaded properly
# Throws error and exits script
if [ ! -f $XOVI_INSTALL_DIR/xovi.so ]; then
    echo "\033[0;31m xovi.so not downloaded. Make sure you selected the correct architecture for your tablet, and check if you are connected to WiFi \033[0m" 
    exit 1
fi

echo "You're all set!"
cd "$HOME"
echo "Enter 'xovi/start' to start xovi!"
echo "To go back to stock either reboot, or run 'xovi/stock'"
exit 0

__PAYLOAD__
