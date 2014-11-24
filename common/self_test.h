/*
 * self_test.h
 *
 *  Created on: May 17, 2013
 *      Author: trosen
 */

#ifndef SELF_TEST_H_
#define SELF_TEST_H_

namespace self_test {
//check pulse controller register n
bool check_register(volatile void *pulse_addr, char n);
bool check_timing(volatile void *pulse_addr);
bool other_test(volatile void *pulse_addr);
};

#endif /* SELF_TEST_H_ */
