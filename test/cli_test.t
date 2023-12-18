Running the program with no argument yields some help text.

  $ $MAIN
  Usage: <exe> command
  Commands:
    show-structs file...     List all the struct definitions in the provided files.

An error message is displayed when the specified command is unrecognized.

  $ $MAIN non-existent-command
  Unknown command "non-existent-command".
  [1]
