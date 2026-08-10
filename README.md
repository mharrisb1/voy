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

| Tool      | Embeddable | Foreground Execution | Background Execution | Parameters | Built-in Globbing |
| --------- | ---------- | -------------------- | -------------------- | ---------- | ----------------- |
| Voy       | Yes        | Yes                  | Yes                  | Yes        | Yes               |
| Watchman  | No         | No                   | Yes                  | Yes[^1]    | Yes               |
| Watchexec | Yes        | Yes                  | No                   | Yes[^1]    | Yes               |
| entr      | No         | Yes                  | No                   | Yes[^2]    | No                |

[^1]: Only through environment variables

[^2]: Only supports absolute path
