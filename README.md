# eta - A generic tool for monitoring progress and ETA

`eta` is a command line tool that takes two arguments:

1. A target progress value
2. A command to inspect the current progress value

`eta` will execute the given command repeatedly, parse it's output, and estimate the time until it reaches the given target value.

## Example 1: File copy
Use `du -bs` to monitor the size of a directory.

<img src="http://aioo.be/eta/demo1b.gif" />

To avoid running an `ssh` command every second, you can lower the rate with `--interval`.

