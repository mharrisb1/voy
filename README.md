# Voy

> [!CAUTION]
> Pre-alpha

Voy is a lightweight, embeddable filesystem event pipeline and multi-route orchestrator. Similar to tools like [Watchman](https://github.com/facebook/watchman), [Watchexec](https://github.com/watchexec/watchexec), and [entr](https://github.com/eradman/entr), Voy watches files for events and optionally executes commands in response to those events. Where Voy shines is:

- Small footprint
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

Voy requires a C++23 compliant compiler (e.g., GCC 14+ or Clang 18+). You can build and install it using CMake:

```bash
git clone https://github.com/mharrisb1/voy.git
cd voy
./bin/install.sh
```

## CLI Usage

To use Voy from the command line, create a `.voy.json` configuration file in the root of your project. This file defines the routes and commands you want to execute when files change.

### Example `.voy.json`

```json
{
  "debounce_ms": 150,
  "routes": [
    {
      "name": "compile_project",
      "watch": ["src/**/*.cpp", "include/**/*.hpp"],
      "ignore": ["build/**"],
      "events": ["modify", "create", "delete"],
      "action": {
        "command": "make build",
        "workdir": ".",
        "env": {
          "BUILD_ENV": "development"
        }
      }
    }
  ]
}
```

> **Note on Globs:** Voy uses a lightweight, custom glob-to-regex engine.
>
> **Supported:**
>
> - `**` : Recursive directory matching (e.g., `src/**/*.cpp`)
> - `*` : Any sequence of characters within a single directory (e.g., `src/*.cpp`)
> - `?` : Any single character (e.g., `test_?.cpp`)
>
> **Not Supported (Escaped as literals):**
>
> - Brace expansion (e.g., `*.{cpp,hpp}`) - you must specify these as separate watch rules.
> - Bracket character classes (e.g., `[a-z]*.cpp`).
> - Extended regex syntax (e.g., `+`, `()`, `|`).

### Running Voy

Start the file watcher in the foreground. By default, it looks for a `voy.json` file in the current directory:

```bash
voy watch
```

You can also specify a custom configuration file path using the `--config` flag:

```bash
voy -c configs/voy.json watch
```

**Options:**

- `-c, --config <file>`: Path to the JSON config file (default: `voy.json`)
- `-h, --help`: Print the help message and exit

## Embedded Usage

The core of Voy is `libvoy` which is a zero-dependency embeddable filesystem event pipeline and multi-route orchestrator. With `libvoy` you can create your own file watching systems by leveraging all of the components that make Voy work.

Using `libvoy` over other options like [chokidar](https://github.com/paulmillr/chokidar) or [watchdog](https://github.com/gorakhargosh/watchdog) allows you to use a file watcher utility where interpreted languages, larger memory overhead, and garbage collection are prohibitively expensive. It also shines when integrated in existing C++ pipelines.

```cpp
#include <chrono>
#include <vector>

#include <voy/voy.h>

namespace Renderer {
  void compile_and_inject_shader(const std::string& shader_path) {...}
}

int main() {
  auto engine = voy::Engine::builder()
    .with_debounce_window(std::chrono::milliseconds(10))
    .on_stdout([](std::string_view chunk) {
      std::cout << chunk;
    })
    .on_stderr([](std::string_view chunk) {
      std::cerr << "\033[31m[voy error]\033[0m " << chunk;
    })
    .add_route(voy::Route::builder("shader_hot_reload")
      .watch("assets/shaders/**/*.frag")
      .watch("assets/shaders/**/*.vert")
      .on_events(voy::EventType::Modify)
      .with_callback([](const std::vector<voy::Event>& events) {
        for (const auto& evt : events) {
          Renderer::compile_and_inject_shader(evt.path.string());
        }
      })
      .build())
    .build();

  if (engine) {
    engine->run();
  }
}
```

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
