Running the program with no argument yields some help text.

  $ $MAIN
  Usage: <exe> command file...
  Commands:
    translate   Read the provided Minor C source files and send equivalent C code to stdout.

An error message is displayed when the specified command is unrecognized.

  $ $MAIN non-existent-command
  Unknown command "non-existent-command".
  [1]
