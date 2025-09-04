PSAS ERS board ADC sample app
------------------------------

Build in current directory with::

  $ west build -b ers-v3p1 -p always -- -DBOARD_ROOT=../..

Flash target hardware with:

  $ ./flash-manually.lnk w
