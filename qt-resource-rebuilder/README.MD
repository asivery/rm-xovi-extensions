# qt-resource-rebuilder

This extension lets you rebuild the QT resources (e.g., icons, fonts, QML files which dictate the UI) loaded by an application on the fly. By rebuilding QT resources loaded by Xochitl, reMarkable's main application, on the fly as XOVI starts up, qt-resource-rebuilder enables UI modifications.  

qt-resource-rebuilder reads modification files in `$XOVI_EXTHOME/exthome/qt-resource-rebuilder` when XOVI starts and modifies the application's UI accordingly.

Note: By default, `$XOVI_EXTHOME` is `/home/root/xovi` (i.e., the location where XOVI is installed by default).

There are three types of modification files that qt-resource-rebuilder supports: `.qrr` files for replacing existing QT resources, `.qmd` files for modifying exsting QT resources, and `.rcc` files for adding new resources.

Note: Xochitl's frontend elements change from version to version; that means that a mod file that works for one version (particularly `.qmd` files, which build on Xochitl's frontend elements) may not work for the next.

## .qrr files
`.qrr` files are used to wholesale replace an existing QT resource file in Xochitl with a local file (e.g., if you wanted to replace all occurrences of a specific icon with a new icon of your choice).

QRR files follow the format:

```
R <resource path within Xochitl> <local file path>
R <resource path within Xochitl 2> <local file path 2>
...
```
Note: Both absolute and relative paths are supported for local file path (relative paths' base is `$XOVI_EXTHOME/exthome/qt-resource-rebuilder`)

For example:
```
R /images/test.png test.png
R /images/example.qml /home/root/example.qml
``` 
## .rcc files
RCC files let you add new resources - QML files, images, etc... Simply compile a new `.rcc` file from a `.qrc` file with that enumerates the QT resources you want added. Learn more about `.qrc` and `.rcc` files [here](https://doc.qt.io/qt-6/resources.html).

## .qmd files
`.qmd` files are written in [QMLDiff](https://github.com/asivery/qmldiff) language and are meant to partially modify the QML files that define Xochitl's UI, enabling new functionalities or minor UI modificiations while keeping the tablet's UI largely intact. For more about how to write `.qmd` files, please refer to the [QMLDiff repo](https://github.com/asivery/qmldiff?tab=readme-ov-file#the-diff-language).

### (Re)building hashtable
`.qmd`s made available by the community of reMarkable modders will be ["hashed"](https://github.com/asivery/qmldiff?tab=readme-ov-file#hashing) instead of referencing QML files and their contents by their actual names. This is by design. Thus, to ensure that qt-resource-rebuilder knows how to actually apply the `.qmd` files, there has to be a version-specific hashtable that lets qt-resource-rebuilder translate between the hashes and the actual QML elements.

The script to build this hashtable is included in the installation of XOVI, at `$XOVI_EXTHOME/rebuild_hashtable`. Be sure to run the script after installing qt-resource-rebuilder for the first time, updating qt-resource-rebuilder, or updating the tablet's OS.

### Please remember to clear your qmlcache directory after creating / deleting qrr files, to make sure no cached versions are loaded

`QML_DISABLE_DISK_CACHE=1` is your friend; when qt-resource-rebuilder is installed, this environment variable is set by default when starting XOVI.
