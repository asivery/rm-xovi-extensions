# xovi-message-broker

A simple xovi extension for mediating transmission of simple packets between xovi extensions, QML and external scripts.
It can be used whenever the extension does not require major changes to the QML, but still needs to transfer small packets of data to the UI.

## Examples

### Extension

C code:
```c
char *signalHandler(const char *value) {
    printf("I got a message: %s!\n", value);
    return NULL;
}

void sendSignal(){
    xovi_message_broker$broadcast("demoSignal", "Hello from C!");
}
```

Attaching the signal handler needs to be done via xovi metadata chains (in the project.xovi file):

```
export signalHandler
with
    xovi-message-broker$simpleSignal = "mySignalName"
    xovi-message-broker$version = 1
end
```

### QML

```qml
import net.asivery.XoviMessageBroker 2.0

XoviMessageBroker {
    id: demoBroker
    listeningFor: ["demoSignal"]

    function onSignalReceived(sgn, message) {
        console.log("Got message", sgn, message);
    }
}

MouseArea {
    onClicked: () => {
        demoBroker.sendSimpleSignal("mySignalName", "Hello from QML!");
    }
}
```

## Scripting

Xovi-message-broker has basic scripting support.
Different signals can be sent to both QML and native extensions via the `/run/xovi-mb` pipe.

The packets sent to the pipe have the format:
```
[>]<u/e><signal name>:[signal value]
```
`>` - if provided, the return value of the signal will have been preserved, and readable from the `/run/xovi-mb-out` pipe

`u/e` - u: Send the signal to the UI part of xovi-message-broker, e - to the extension part.

`signal name` - the name of the signal to send

`signal value` - optional data to pass to the signal

### Examples:

Assuming the following signal is declared in the QML:
```qml
XoviMessageBroker {
    id: xovimb
    listeningFor: ["ping"]

    function onSignalReceived(signal: string, message: string): string {
        console.log(signal);
        if(signal == "ping") {
            const out = "pong: " + message;
            console.log("Responding: " + out);
            return out;
        }
    }
}
```

One can use the following command to invoke the handler:

```sh
$ echo '>uping:hello' > /run/xovi-mb
$ cat /run/xovi-mb-out
pong: hello
```

or, if the output can be discarded:

```sh
$ echo 'udemoSignal:hello' > /run/xovi-mb
```

To communicate with the native part of xovi-message-broker, the following command can be used:

```sh
$ echo 'emySignalName:Hello from shell!' > /run/xovi-mb
```
