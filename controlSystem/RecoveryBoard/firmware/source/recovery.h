#ifndef _RECOVERY_H_
#define _RECOVERY_H_

#define DEBUG_SD  (BaseSequentialStream *) &SD2

enum mainchutecommandtype {idle_m, stop_m, fire_m, reset_m};
enum recoverystatetype {armed, disarmed};
enum PositionCommandtype {lock, unlock, idle};
enum BeepCommandtype{Beep};

extern volatile enum recoverystatetype recoveryState;
extern volatile enum mainchutecommandtype mainchuteCommand;
extern volatile enum PositionCommandtype PositionCommand;
extern volatile enum BeepCommandtype BeepCommand;

#endif

