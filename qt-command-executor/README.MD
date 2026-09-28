# qt-command-executor

To use this module in your QML code, start by importing CommandExecutor:

```qml
import net.asivery.CommandExecutor 1.0
```

The package comes with two components: CommandExecutor for short, synchronous tasks and AsyncCommandExecutor for longer-running tasks executing in parallel with QMD.

## CommandExecutor (synchronous)

To use, define the CommandExecutor object:

```qml
CommandExecutor {
    id: cmdExecutor
}
```

Finally, to execute any command, invoke:

```qml
const { stdout, stderr } = cmdExecutor.executeCommand("uname", ["-a"]);
```

## AsyncCommandExecutor (asynchronous)

AsyncCommandExecutor is a bit more complex, but the initial step of defining an instance is the same. You can set properties directly in this definition:

```qml
AsyncCommandExecutor {
    id: asyncCmdExecutor
    command: "/usr/bin/env""
    arguments: ["--", "SHOW_THIS=information"]
    environment: {
        "AND_ALSO": "this"
    }
}
```

In order to execute a command you have set up this way, invoke:

```qml
if(asyncCmdExecutor.startCommand(1000)) { // wait up to one second (1000ms) for the program to start
    console.log("Command started sucessfully!");
} else {
    console.log("Could not start command!");
}
```

or, simpler than that, just set `asyncCmdExecutor.running = true`. In that case, the waiting time will be fixed at 100 ms and an exception will be thrown if the program cannot be started within that timeout.

You can capture the standard output and error streams by subscribing to one or both of the signals
```qml
    onStdOutAvailable: (line) => {
        console.log("stdout", line);
    }
    onStdErrAvailable: (line) => {
        console.log("stderr", line);
    }
```
These signals are emitted whenever the running program outputs a newline. If either of the signals is not subscribed to, the corresponding output stream will be forwarded to the parent process.

To track the lifetime of the associated process, use the notify signal of the `running` property, `onRunningChanged()` or of the `exitCode` property, `onExitCodeChanged()`. `exitCode` is set to the return code of the program, when the process has exited normally, and to a negative value otherwise.

A program that has stopped can be started again after it has exited. There is no need to change any of the properties such as `arguments`, but they can be changed if needed. The properties `command`, `arguments` and `environment` can be set using bindings or dynamically. They are captured the moment that a program is started and while they can still be changed during execution of the command, these changes have no influence on the currently running process.

To force termination of a running program, call `asyncCmdExecutor.terminateCommand(timeout)` or set `asyncCmdExecutor.running = false`. The semantics mirror those of `startCommand()` and setting `running = true` respectively.
