#pragma once

#include <midas.h>

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

extern const char *frontend_name;
extern EQUIPMENT equipment[];
extern HNDLE hDB;
extern INT max_event_size;

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, PTYPE adr);
int readEvent(char * const pevent, int const off);

#ifndef NEED_NO_EXTERN_C
}
#endif
