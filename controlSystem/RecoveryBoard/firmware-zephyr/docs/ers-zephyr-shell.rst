

ERS Zephyr based shell commands as of 2026-01-04.  Note that most commands are default and come with the shell (not obvious how to disable or remove these).  The added ERS commands all have descriptions which begin with "- ERS -":

uart:~$ help
Please press the <Tab> button to see all available commands.
You can also use the <Tab> button to prompt or auto-complete all commands or its subcommands.
You can try to call commands with <-h> or <--help> parameter for more information.

Shell supports following meta-keys:
  Ctrl + (a key from: abcdefklnpuw)
  Alt  + (a key from: bf)
Please refer to shell documentation for more details.

Available commands:
  can      : CAN controller commands
  clear    : Clear screen.
  dac      : - ERS - DAC info and set commands
  device   : Device commands
  devmem   : Read/write physical memory
            Usage:
            Read memory at address with optional width:
            devmem address [width]
            Write memory at address with mandatory width and value:
            devmem address <width> <value>
  diag     : - ERS - diagnostics
  ers      : - ERS - development commands
  hall     : - ERS - show and set Hall sensor limit values (in ADC counts)
  help     : Prints the help message.
  history  : Command history.
  kernel   : Kernel commands
  rem      : Ignore lines beginning with 'rem '
  resize   : Console gets terminal screen size or assumes default in case the
            readout fails. It must be executed after each terminal width change
            to ensure correct text display.
  retval   : Print return value of most recent command
  ring     : - ERS - lock ring commands
  shell    : Useful, not Unix-like shell commands.
  stats    : Stats commands
uart:~$
