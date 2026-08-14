# Voy

> [!CAUTION]
> Pre-alpha

Voy is a lightweight, embeddable filesystem event pipeline and multi-route orchestrator. Similar to tools like [Watchman](https://github.com/facebook/watchman), [Watchexec](https://github.com/watchexec/watchexec), and [entr](https://github.com/eradman/entr), Voy watches files for events and optionally executes commands in response to those events. Where Voy shines is:

- Small footprint and performant
- Fully embeddable
- Foreground or background (daemon) command execution
- Multi-route orchestrator for more complex needs
- Process group isolation

## Feature Matrix

| Feature                        | Voy | Watchman | Watchexec | entr |
| ------------------------------ | --- | -------- | --------- | ---- |
| Embeddable                     | Yes | No       | Yes       | No   |
| Foreground Execution           | Yes | No       | Yes       | Yes  |
| Background Execution           | Yes | Yes      | No        | No   |
| Multi-Route Orchestration      | Yes | Yes      | No        | No   |
| Built-in Globbing              | Yes | Yes      | Yes       | No   |
| Environment Variable Injection | Yes | Yes      | Yes       | No   |
| Recursive Traversal            | Yes | Yes      | Yes       | No   |
| Dynamic Directory Watching     | Yes | Yes      | Yes       | No   |
| Multi-OS Support               | No  | Yes      | Yes       | Yes  |
| Naive Polling (Fallback)       | No  | Yes      | Yes       | Yes  |

## Installation

[TODO]

## CLI Usage

[TODO]

## libvoy

[TODO]

## Environment Variable Parameters

> [!WARNING]
> Volatile before stable release

For each process created by Voy, the following environment variables are passed in:

| Name             | Description                                          |
| ---------------- | ---------------------------------------------------- |
| `VOY_ROUTE_NAME` | Optional name of route that handled the event        |
| `VOY_BATCH_SIZE` | Number of unique events collected in debounce window |
| `VOY_EVENT_TYPE` | Type of event                                        |
| `VOY_EVENT_TIME` | Original timestamp of the event (ISO)                |
| `VOY_EVENT_PATH` | Absolute path of file or subdirectory                |

| `VOY_EVT

### A Note on Batch Size

The `VOY_BATCH_SIZE` value is provided because currently we do not support reporting all events in a window through parameters. We need to come up with a good solution for this that avoids potential issues with max environment variable size. Right now, we just report the first event from the observed window but the batch size allows the user to create a script that will do something different if the batch size is larger than one.

For example something like this:

```bash
if [ "$VOY_BATCH_SIZE" -gt 1 ]; then
  echo "Multiple files changed, running full build..."
else
  echo "Only $VOY_EVENT_PATH changed, running fast build..."
fi
```
