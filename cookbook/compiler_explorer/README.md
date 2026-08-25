# Compiler Explorer

Everyone is familiar with Matt Godbolt's excellent [Compiler Explorer](https://godbolt.org). This Voy recipe shows how to set something similar up locally to explore the compiled assembly from a program.

Run the following:

```bash
voy watch
```

Then make changes to either [`main.cpp`](./main.cpp) or [`error.cpp`](./error.cpp) to view the assembly or compiler errors.
