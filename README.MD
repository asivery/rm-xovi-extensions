# rMPP XOVI extensions

This is a monorepo for all my XOVI extensions for the rMPP tablet.

Note: I call extensions which themselves do not add functionality, but are a dependency for
other extensions 'modules'

All buildscripts require you to have set the XOVI_REPO environment variable to the directory
you have cloned [XOVI](https://github.com/asivery/xovi) to, and TMP to the directory which will be the
parent of all temporary build directories.

# Currently available extensions:

- [fileman](fileman/) - A module for managing filesystem redirections
- [random-suspend-screen](random-suspend-screen) - Lets you randomize your rM tablet's suspend screens
- [palm-rejection](palm-rejection) - Disables the touchscreen whenever the marker is undocked
- [qt-command-executor](qt-command-executor) - Injects a QT module, which makes it possible to execute shell commands from QML.
- [qt-resource-rebuilder](qt-resource-rebuilder) - Injects itself into QT, and rebuilds resource databases on the fly. This makes it possible to replace or add QML files, images, etc...
- [framebuffer-spy](framebuffer-spy) - A module for exposing the address of the system framebuffer
- [webserver-remote](webserver-remote) - Exposes the USB webserver to all interfaces, and adds a confirmation dialog for any inbound connection to it
