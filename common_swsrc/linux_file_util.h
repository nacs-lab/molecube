
/* Generic functions to write values to and read them from file.
 * Calls to these functions can manipulate file-mapped devices in Linux. */

#ifdef __cplusplus
extern "C"
{
#endif

int write_int_to_file(const char* fname, int val);
int get_int_from_file(const char* fname, int* val);
int write_str_to_file(const char* fname, const char* val);

//set status line for web interface in /tmp/progname.status
//progname is only needed at 1st call.  Later, supply 0.
void setProgramStatus(const char* progname, const char* str);

#ifdef __cplusplus
}
#endif
