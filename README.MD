# rMPP XOVI extensions

This is a monorepo for all my XOVI extensions for the rMPP tablet.

Note: I call extensions which themselves do not add functionality, but are a dependency for
other extensions 'modules'

All buildscripts require you to have set the XOVI_REPO environment variable to the directory
you have cloned [XOVI](https://github.com/asivery/xovi) to, and TMP to the directory which will be the
parent of all temporary build directories.

# Disclaimers

I am NOT responsible for any damages that you may incur to your device using my software. If you end up permanently bricking your device, or damaging the screen, you cannot expect me to fix it. I have written these extensions for myself, and I'm sharing them in hopes that someone might also find them useful.

You are installing this at your own risk.

XOVI is a tethered extension manager - after a restart, the tablet will get back to the stock OS. To get XOVI back, simply run `xovi/start` from the SSH prompt. Your tablet will restart, and you will get XOVI back.

Please do not attempt to get xovi running on an encrypted tablet untethered by editing the xochitl systemd files in the root partition!! You WILL end up with a bootlooping device, and have to go through manual recovery.

If you'd like to be able to start XOVI without an external device, it is recommended that you install a separate XOVI starter, such as [xovi-tripletap](https://github.com/rmitchellscott/xovi-tripletap).

## To install XOVI:
- Download [the extensions zip for your device](https://github.com/asivery/rmpp-xovi-extensions/releases)
- Copy `install-xovi-for-rm` over from the archive to the device, into `/tmp/`:
```
scp install-xovi-for-rm root@10.11.99.1:/tmp/
```
- Start the install-xovi-for-rm script over SSH - on the device run: `chmod a+x /tmp/install-xovi-for-rm && /tmp/install-xovi-for-rm`

Installing XOVI won't install any extensions. You need to choose what extensions you might need, and install them manually. **You do not need to install all extensions. Choose only the ones you want**

## To install extensions:
- Before installing an extension, read its readme to make sure you also installed all of its dependencies. An extension without dependencies will either fail to load, or crash on startup.
- Download an extension's `.so` file (in case of this repository, [from the `extensions.zip` file](https://github.com/asivery/rmpp-xovi-extensions/releases/), then unpack it to get all the currently available xovi extensions)
- Copy the `.so` file to `/home/root/xovi/extensions.d/` on the tablet
- Run `systemctl restart xochitl` to restart xovi

## To update hashtab (required for UI mods)
If your device updates, or if you're installing qt-resource-rebuilder for the first time or updating it, be sure to update the hashtab file. To do that, run the script by [@hasechris](https://github.com/hasechris): `xovi/rebuild_hashtable` and wait for it to complete.

# Currently available extensions:

- [fileman](fileman/) - A module for managing filesystem redirections
- [random-suspend-screen](random-suspend-screen) - Lets you randomize your rM tablet's suspend screens
- [qt-command-executor](qt-command-executor) - Injects a QT module, which makes it possible to execute shell commands from QML.
- [qt-resource-rebuilder](qt-resource-rebuilder) - Injects itself into QT, and rebuilds resource databases on the fly. This makes it possible to replace or add QML files, images, etc...
- [framebuffer-spy](framebuffer-spy) - A module for exposing the address of the system framebuffer
- [webserver-remote](webserver-remote) - Exposes the USB webserver to all interfaces, and adds a confirmation dialog for any inbound connection to it

# Deprecated:

- [palm-rejection](palm-rejection) - Disables the touchscreen whenever the marker is undocked
