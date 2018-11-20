#include <midas.h>
#include <caen/handle.hxx>

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char frontend_name[] = "fe_v1720";
/* The frontend file name, don't change it */
char frontend_file_name[] = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 1000;

/* maximum event size produced by this frontend */
INT max_event_size = 100 * 1024;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 200 * 1024;

extern INT run_state;
extern HNDLE hDB;

/*-- Function declarations -----------------------------------------*/
INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_event(char *pevent, INT off);
/*-- Bank definitions ----------------------------------------------*/

/*-- Equipment list ------------------------------------------------*/

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
	caen::Handle handle(0, 0, 0);
	return 0;
}
