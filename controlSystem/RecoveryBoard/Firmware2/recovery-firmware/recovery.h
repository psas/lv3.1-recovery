#ifndef _RECOVERY_H_
#define _RECOVERY_H_

#define DEBUG_SD  (BaseSequentialStream *) &SD2

enum mainchutecommandtype {idle_m, stop_m, fire_m, reset_m};
enum droguecommandtype {idle, stop, fire, lock};
enum recoverystatetype {armed, disarmed};

extern volatile enum recoverystatetype recoveryState;
extern volatile enum droguecommandtype drogueCommand;
extern volatile enum mainchutecommandtype mainchuteCommand;

#endif

