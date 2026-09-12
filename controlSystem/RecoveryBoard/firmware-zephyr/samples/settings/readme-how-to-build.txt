
How to build
------------

west build -b lpcxpresso55s69/lpc55s69/cpu0 -p always -- -DBOARD_ROOT=../../../zephyr

How to flash
------------

For LPC55S69 devkit:

  $ west flash --reset   . . . for Segger Jlink based flashing

For ERS board:

  $ ../../flash-manually.sh w --conf-dir ../../scripts

Sample output from ERS board
----------------------------

 *** Booting Zephyr OS build v3.7.0 ***

*** Settings usage example ***

settings subsys initialization: OK.
subtree <alpha> handler registered: OK
subtree <alpha/beta> has static handler

##############
# iteration 0
##############

=================================================
basic load and save using registered handlers

load all key-value pairs using registered handlers
loading all settings under <beta> handler is done
loading all settings under <alpha> handler is done

save <alpha/beta/voltage> key directly: OK.

load <alpha/beta> key-value pairs using registered handlers
<alpha/beta/voltage> = -3025
loading all settings under <beta> handler is done

save all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

load all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

=================================================
loading subtree to destination provided by the caller

direct load: <alpha/length/2>
direct load: <alpha/length/1>
direct load: <alpha/length>
  direct.length = 100
  direct.length_1 = 41
  direct.length_2 = 59

=================================================
Delete a key-value pair

immediate load: OK.
  <alpha/length> value exist in the storage
delete <alpha/length>: OK.
  Can't to load the <alpha/length> value as expected

=================================================
Service a key-value pair without dedicated handlers

<gamma> = 0 (default)
save <gamma> key directly: OK.

##############
# iteration 1
##############

=================================================
basic load and save using registered handlers

load all key-value pairs using registered handlers
<alpha/length/2> = 59
<alpha/length/1> = 41
<alpha/angle/1> = 1
<alpha/beta/source> = a
<alpha/beta/voltage> = -3025
loading all settings under <beta> handler is done
loading all settings under <alpha> handler is done

save <alpha/beta/voltage> key directly: OK.

load <alpha/beta> key-value pairs using registered handlers
<alpha/beta/source> = a
<alpha/beta/voltage> = -3050
loading all settings under <beta> handler is done

save all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

load all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

=================================================
loading subtree to destination provided by the caller

direct load: <alpha/length>
direct load: <alpha/length/2>
direct load: <alpha/length/1>
  direct.length = 100
  direct.length_1 = 42
  direct.length_2 = 58

=================================================
Delete a key-value pair

immediate load: OK.
  <alpha/length> value exist in the storage
delete <alpha/length>: OK.
  Can't to load the <alpha/length> value as expected

=================================================
Service a key-value pair without dedicated handlers

immediate load: OK.
<gamma> = 1
save <gamma> key directly: OK.

##############
# iteration 2
##############

=================================================
basic load and save using registered handlers

load all key-value pairs using registered handlers
<alpha/length/2> = 58
<alpha/length/1> = 42
<alpha/angle/1> = 2
<alpha/beta/source> = ab
<alpha/beta/voltage> = -3050
loading all settings under <beta> handler is done
loading all settings under <alpha> handler is done

save <alpha/beta/voltage> key directly: OK.

load <alpha/beta> key-value pairs using registered handlers
<alpha/beta/source> = ab
<alpha/beta/voltage> = -3075
loading all settings under <beta> handler is done

save all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

load all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

=================================================
loading subtree to destination provided by the caller

direct load: <alpha/length>
direct load: <alpha/length/2>
direct load: <alpha/length/1>
  direct.length = 100
  direct.length_1 = 43
  direct.length_2 = 57

=================================================
Delete a key-value pair

immediate load: OK.
  <alpha/length> value exist in the storage
delete <alpha/length>: OK.
  Can't to load the <alpha/length> value as expected

=================================================
Service a key-value pair without dedicated handlers

immediate load: OK.
<gamma> = 2
save <gamma> key directly: OK.

##############
# iteration 3
##############

=================================================
basic load and save using registered handlers

load all key-value pairs using registered handlers
<alpha/length/2> = 57
<alpha/length/1> = 43
<alpha/angle/1> = 3
<alpha/beta/source> = abc
<alpha/beta/voltage> = -3075
loading all settings under <beta> handler is done
loading all settings under <alpha> handler is done

save <alpha/beta/voltage> key directly: OK.

load <alpha/beta> key-value pairs using registered handlers
<alpha/beta/source> = abc
<alpha/beta/voltage> = -3100
loading all settings under <beta> handler is done

save all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

load all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

=================================================
loading subtree to destination provided by the caller

direct load: <alpha/length>
direct load: <alpha/length/2>
direct load: <alpha/length/1>
  direct.length = 100
  direct.length_1 = 44
  direct.length_2 = 56

=================================================
Delete a key-value pair

immediate load: OK.
  <alpha/length> value exist in the storage
delete <alpha/length>: OK.
  Can't to load the <alpha/length> value as expected

=================================================
Service a key-value pair without dedicated handlers

immediate load: OK.
<gamma> = 3
save <gamma> key directly: OK.

##############
# iteration 4
##############

=================================================
basic load and save using registered handlers

load all key-value pairs using registered handlers
<alpha/length/2> = 56
<alpha/length/1> = 44
<alpha/angle/1> = 4
<alpha/beta/source> = abcd
<alpha/beta/voltage> = -3100
loading all settings under <beta> handler is done
loading all settings under <alpha> handler is done

save <alpha/beta/voltage> key directly: OK.

load <alpha/beta> key-value pairs using registered handlers
<alpha/beta/source> = abcd
<alpha/beta/voltage> = -3125
loading all settings under <beta> handler is done

save all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

load all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

=================================================
loading subtree to destination provided by the caller

direct load: <alpha/length>
direct load: <alpha/length/2>
direct load: <alpha/length/1>
  direct.length = 100
  direct.length_1 = 45
  direct.length_2 = 55

=================================================
Delete a key-value pair

immediate load: OK.
  <alpha/length> value exist in the storage
delete <alpha/length>: OK.
  Can't to load the <alpha/length> value as expected

=================================================
Service a key-value pair without dedicated handlers

immediate load: OK.
<gamma> = 4
save <gamma> key directly: OK.

##############
# iteration 5
##############

=================================================
basic load and save using registered handlers

load all key-value pairs using registered handlers
<alpha/length/2> = 55
<alpha/length/1> = 45
<alpha/angle/1> = 5
<alpha/beta/source> is not compatible with the application
<alpha/beta/voltage> = -3125
loading all settings under <beta> handler is done
loading all settings under <alpha> handler is done

save <alpha/beta/voltage> key directly: OK.

load <alpha/beta> key-value pairs using registered handlers
<alpha/beta/source> is not compatible with the application
<alpha/beta/voltage> = -3150
loading all settings under <beta> handler is done

save all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

load all key-value pairs using registered handlers
export keys under <beta> handler
export keys under <alpha> handler

=================================================
loading subtree to destination provided by the caller

direct load: <alpha/length>
direct load: <alpha/length/2>
direct load: <alpha/length/1>
  direct.length = 100
  direct.length_1 = 46
  direct.length_2 = 54

=================================================
Delete a key-value pair

immediate load: OK.
  <alpha/length> value exist in the storage
delete <alpha/length>: OK.
  Can't to load the <alpha/length> value as expected

=================================================
Service a key-value pair without dedicated handlers

immediate load: OK.
<gamma> = 5
save <gamma> key directly: OK.

=================================================
Inject the value to the setting destination in runtime

<alpha/beta/source> = RT
injected <alpha/beta/source>: OK.
  The settings destination off the key <alpha/beta/source> has got value: "RT"


=================================================
Read a value from the setting destination in runtime

fetched <alpha/beta/source>: OK.
  String value "rtos" was retrieved from the settings destination off the key <alpha/beta/source>

*** THE END  ***

CTRL-A Z for help | 115200 8N1 | NOR | Minicom 2.8 | VT102 | Offline | ttyACM0                                               


