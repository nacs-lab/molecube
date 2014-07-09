
/* Generic functions to write values to and read them from file.
 * Calls to these functions can manipulate file-mapped devices in Linux. */
 
#ifdef __cplusplus
extern "C"
{
int write_int_to_file(const char* fname, int val);
int get_int_from_file(const char* fname, int* val);
int write_str_to_file(const char* fname, const char* val);
}
#else
int write_int_to_file(const char* fname, int val);
int get_int_from_file(const char* fname, int* val);
int write_str_to_file(const char* fname, const char* val);
#endif
