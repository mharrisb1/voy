# Voy

> [!CAUTION]
> Pre-alpha

Voy is a lightweight, embeddable filesystem event pipeline and multi-route orchestrator. Similar to tools like [Watchman](https://github.com/facebook/watchman), [Watchexec](https://github.com/watchexec/watchexec), and [entr](https://github.com/eradman/entr), Voy watches files for events and optionally executes commands in response to those events. Where Voy shines is:

- Small bundle size
- Foreground or background (daemon) command execution
- Multi-route orchestrator for more complex needs
- Easy-to-use parameters for commands instead of environment variables
- Process group isolation

## Feature Matrix

| Feature              | Voy | Watchman | Watchexec | entr    |
| -------------------- | --- | -------- | --------- | ------- |
| Embeddable           | Yes | No       | Yes       | No      |
| Foreground Execution | Yes | No       | Yes       | Yes     |
| Background Execution | Yes | Yes      | No        | No      |
| Parameters           | Yes | Yes[^1]  | Yes[^1]   | Yes[^2] |
| Built-in Globbing    | Yes | Yes      | Yes       | No      |

[^1]: Only through environment variables

[^2]: Only supports absolute path
