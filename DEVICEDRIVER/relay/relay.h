#ifndef __RELAY_H
#define __RELAY_H

#include "sys.h"
#include "tpic6c595.h"


void ControlAppointedRelay(u8 ch,u8 state);
void ControlAllRelay(u16 out_put_control_bit,u16 *ch);



































#endif
