# subcommander

**Roll your own subcommands the Unix way.**

Easily partition your shell command namespace by creating new top
level commands which offer a directory of subcommands of your choice.

The tool is extremely simple because it's done the Unix way, using
symbolic links and executable scripts. The tool is completely shell
agnostic and written in portable ANSI C.

## Example

Assume you want to create a `repl` command whose subcommands start
REPLs for different programming languages. You keep forgetting which
REPLs provide their own line editing and which ones don't. You want to
wrap the latter kind in `rlwrap` so that all `repl` subcommands
consistently offer line editing. Here's how to do it.

Create a personal `bin` directory if you don't have one already.

```Shell
$ mkdir -p ~/.local/bin
$ export PATH="$HOME/.local/bin:$PATH"
```

Create your new `repl` command.

```Shell
$ ln -s "$(which subcommander)" ~/.local/bin/repl
$ mkdir -p ~/.config/subcommander/repl
```

Add some subcommands for the `repl` command.

```Shell
$ ln -s "$(which python)" ~/.config/subcommander/repl/python

$ cat >> ~/.config/subcommander/repl/sbcl
#!/bin/sh
exec rlwrap sbcl "$@"

$ cat >> ~/.config/subcommander/repl/sml
#!/bin/sh
exec rlwrap sml "$@"
```

Now you can use `repl` as follows.

```Shell
$ repl
python
sbcl
sml

$ repl python
>>>

$ repl sbcl
*

$ repl sml
-
```
