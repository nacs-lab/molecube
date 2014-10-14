#ifndef UTIL_H_
#define UTIL_H_

void enable_interrupts();
void disable_interrupts();
void set_priority(int prio);
int get_priority();
void yield_execution();

double restrict(double v, double bottom, double top);


#endif /*UTIL_H_*/
