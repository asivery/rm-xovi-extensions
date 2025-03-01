# Disclaimers and general info

I am NOT responsible for any damages that you may incur to your device using my software. If you end up permanently bricking your device, or damaging the screen, you cannot expect me to fix it. I have written these extensions for myself, and I'm sharing them in hopes that someone might also find them useful.

You are installing this at your own risk.

XOVI is a tethered extension manager - after a restart, the tablet will get back to the stock OS. To get XOVI back, simply run `xovi/start` from the SSH prompt. Your tablet will restart, and you will get XOVI back.

Please do not attempt to get xovi running on an encrypted tablet untethered by editing the xochitl systemd files in the root partition!! You WILL end up with a bootlooping device, and have to go through manual recovery.

## To install XOVI:
- Download [the extensions zip for your device](https://github.com/asivery/rmpp-xovi-extensions/releases)
- Copy `install` over from the archive to the device, into `/tmp/`:
```
scp install root@10.11.99.1:/tmp/
```
- Start the install script over SSH - on the device run: `chmod a+x /tmp/install && /tmp/install`

Installing XOVI won't install any extensions. You need to choose what extensions you might need, and install them manually.

## To install extensions:
- Before installing an extension, read its readme to make sure you also installed all of its dependencies. An extension without dependencies will either fail to load, or crash on startup.
- Download an extension's `.so` file (in case of this repository, [from the `extensions.zip` file](https://github.com/asivery/rmpp-xovi-extensions/releases/), then unpack it to get all the currently available xovi extensions)
- Copy the `.so` file to `/home/root/xovi/extensions.d/` on the tablet
- Run `systemctl restart xochitl` to restart xovi

## To update hashtab (required for UI mods)
If your device updates, or if you're installing qt-resource-rebuilder for the first time or updating it, be sure to update the hashtab file. To do that, run the script by [@hasechris](https://github.com/hasechris): `xovi/rebuild_hashtable` and wait for it to complete.
