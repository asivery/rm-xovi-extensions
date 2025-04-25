# xovi-message-broker

A simple xovi extension for mediating transmission of simple packets between xovi extensions and QML.
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
import net.asivery.XoviMessageBroker

XoviMessageBroker {
    id: demoBroker
    listeningFor: ["demoSignal"]

    onSignalReceived: (sgn, message) => {
        console.log("Got message", sgn, message);
    }
}

MouseArea {
    onClicked: () => {
        demoBroker.sendSimpleSignal("mySignalName", "Hello from QML!");
    }
}
```
