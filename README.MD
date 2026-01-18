# rM XOVI extensions

This is a monorepo for all my XOVI extensions for rM tablets.

Note: I call extensions which themselves do not add functionality, but are a
dependency for other extensions "modules".

All build scripts require you to have set the XOVI_REPO environment variable to
the directory you have cloned [XOVI](https://github.com/asivery/xovi) to, and
TMP to the directory which will be the parent of all temporary build
directories.

# Disclaimers

I am NOT responsible for any damages that you may incur to your device using my
software. If you end up permanently bricking your device, or damaging the
screen, you cannot expect me to fix it. I have written these extensions for
myself, and I'm sharing them in hopes that someone might also find them useful.

You are installing this at your own risk.

XOVI is a tethered extension manager - after a restart, the tablet will get
back to the stock OS. To get XOVI back, simply run `xovi/start` from the SSH
prompt. Your tablet will restart, and you will get XOVI back.

Please do not attempt to get xovi running on an encrypted tablet untethered by
editing the xochitl systemd files in the root partition!! You WILL end up with
a bootlooping device, and have to go through manual recovery.

If you'd like to be able to start XOVI without an external device, it is
recommended that you install a separate XOVI starter, such as
[xovi-tripletap](https://github.com/rmitchellscott/xovi-tripletap).

# To install XOVI:

## With curl

### Download curl

Everything should be run on the tablet.

For rM1/rM2:

```bash
wget -O /home/root/curl --no-check-certificate \
  https://github.com/moparisthebest/static-curl/releases/download/v8.11.0/curl-armv7 && \
  echo "779a1bd9f486fd5ff1da25d5e5bb99c58bc79ded22344f2b7ff366cf645a6630  /home/root/curl" | \
  sha256sum -c && chmod +x /home/root/curl
```

For rMPP/rMPPM:

```bash
wget -O /home/root/curl --no-check-certificate \
  https://github.com/moparisthebest/static-curl/releases/download/v8.11.0/curl-aarch64 && \
  echo "1b050abd1669f9a2ac29b34eb022cdeafb271dce5a4fb57d8ef8fadff6d7be1f  /home/root/curl" | \
  sha256sum -c && chmod +x /home/root/curl
```

Output should end with `/home/root/curl: OK`.

If the output ends with
`sha256sum: WARNING: 1 of 1 computed checksums did NOT match`, follow the
[manual method](#manually) instead.

### Download the archive

For rM1/rM2:

```bash
./curl -Lfo /tmp/xovi.tar.gz \
  https://github.com/asivery/rm-xovi-extensions/releases/latest/download/xovi-arm32.tar.gz
```

For rMPP/rMPPM:

```bash
./curl -Lfo /tmp/xovi.tar.gz \
  https://github.com/asivery/rm-xovi-extensions/releases/latest/download/xovi-aarch64.tar.gz
```

### Extract the archive

```bash
tar -xzvf /tmp/xovi.tar.gz -C /home/root
```

## Manually

- Download the
  [latest xovi.tar.gz](https://github.com/asivery/rm-xovi-extensions/releases/latest)
  for your device (arm32 for rM1/rM2, aarch64 for rMPP/rMPPM)
- Copy it onto the device, into `/tmp/xovi.tar.gz`:

  ```shell
  scp xovi.tar.gz root@10.11.99.1:/tmp/xovi.tar.gz
  ```

- Extract the archive (on the tablet):

  ```bash
  tar -xzvf /tmp/xovi.tar.gz -C /home/root
  ```

You're all set, use `xovi/start` to start xovi.

To go back to stock either reboot, or run `xovi/stock`.

Installing XOVI will install qt-resource-rebuilder by default. You can choose
what other extensions you might need, and install them manually. **You do not
need to install all extensions. Choose only the ones you want.**

If you plan on using UI mods, make sure to [update the hashtab](#to-update-hashtab-required-for-ui-mods).

# To install extensions:

Before installing an extension, read its readme to make sure you also installed
all of its dependencies. An extension without dependencies will either fail to
load, or crash on startup.

The are four other extensions bundled in with the default install (see
[currently available extensions](#currently-available-extensions)). For these
four extensions, the installation is as simple as running:

```bash
mv /home/root/xovi/inactive-extensions/<extension-name>.so \
  /home/root/xovi/extensions.d/
```

To install other extensions:

- Download the extension's `.so` file(s).
- Copy the `.so` file to `/home/root/xovi/extensions.d/` on the tablet
- Run `systemctl restart xochitl` to restart xovi

To uninstall an extension, simply move it to the inactive extensions folder:

```bash
mv /home/root/xovi/extensions.d/<extension-name>.so \
  /home/root/xovi/inactive-extensions/
```

# To update hashtab (required for UI mods)

If your device updates, or if you're installing qt-resource-rebuilder for the
first time or updating it, be sure to update the hashtab file. To do that, run
the script by [@hasechris](https://github.com/hasechris):
`xovi/rebuild_hashtable` and wait for it to complete.

# Currently available extensions:

- (installed by default) [qt-resource-rebuilder](qt-resource-rebuilder) -
  Injects itself into QT, and rebuilds resource databases on the fly. This
  makes it possible to replace or add QML files, images, etc...
- [webserver-remote](webserver-remote) - Exposes the USB webserver to all
  interfaces, and adds a confirmation dialog for any inbound connection to it

## Modules:

- [framebuffer-spy](framebuffer-spy) - A module for exposing the address of the
  system framebuffer
- [qt-command-executor](qt-command-executor) - Injects a QT module, which makes
  it possible to execute shell commands from QML
- [xovi-message-broker](xovi-message-broker) - A module for mediating the
  transmission of simple packets between xovi extensions and QML

## Deprecated:

- [fileman](fileman) - A module for managing filesystem redirections
- [palm-rejection](palm-rejection) - Disables the touchscreen whenever the
  marker is undocked
- [random-suspend-screen](random-suspend-screen) - Lets you randomize your rM
  tablet's suspend screens
