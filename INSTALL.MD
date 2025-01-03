# Disclaimers and general info

I am NOT responsible for any damages that you may incur to your device using my software. If you end up permanently bricking your device, or damaging the screen, you cannot expect me to fix it. I have written these extensions for myself, and I'm sharing them in hopes that someone might also find them useful.

You are installing this at your own risk.

XOVI is a tethered extension manager - after a restart, the tablet will get back to the stock OS. To get XOVI back, simply run `xovi/start` from the SSH prompt. Your tablet will restart, and you will get XOVI back.

## To install XOVI:
- Download [install_allmerged.sh](https://raw.githubusercontent.com/asivery/rmpp-xovi-extensions/refs/heads/master/scripts/install_allmerged.sh)
- Copy `install_allmerged.sh` over to the device, into `/tmp/`:
```
scp install_allmerged.sh root@10.11.99.1:/tmp
```
- Start the install script over SSH - on the device run: `chmod a+x /tmp/install_allmerged.sh && /tmp/install_allmerged.sh`

## To install extensions:
- Before installing an extension, read its readme to make sure you also installed all of its dependencies. An extension without dependencies will either fail to load, or crash on startup.
- Download an extension's `.so` file (in case of this repository, download the `extensions.zip` file, then unpack it to get all the currently available xovi extensions)
- Copy the `.so` file to `/home/root/xovi/extensions.d/` on the tablet
- Run `systemctl restart xochitl` to restart xovi

## To update hashtab (required for UI mods)
If your device updates, or you're installing qt-resource-rebuilder for the first time, be sure to update the hashtab file. To do that, issue the following commands:
```
mkdir -p /home/root/xovi/exthome/qt-resource-rebuilder
rm -f /home/root/xovi/exthome/qt-resource-rebuilder/hashtab
systemctl stop xochitl
killall xochitl
QMLDIFF_HASHTAB_CREATE=/home/root/xovi/exthome/qt-resource-rebuilder/hashtab xovi/_start
```
After that, you should start seeing logs scroll up your screen. Enter your passcode on the rM, then watch the logs. When you see the line:
```
[qmldiff]: Hashtab saved to /home/root/xovi/exthome/qt-resource-rebuilder/hashtab
```
scroll up your screen on the PC, press CTRL+C.
After that, start xovi normally (`systemctl start xochitl`)
