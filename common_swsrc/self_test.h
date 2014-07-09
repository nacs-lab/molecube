/*
 * self_test.h
 *
 *  Created on: May 17, 2013
 *      Author: trosen
 */

#ifndef SELF_TEST_H_
#define SELF_TEST_H_

namespace self_test
{
bool check_register(char n); //check pulse controller register n
bool check_timing();
bool other_test();
};

#endif /* SELF_TEST_H_ */
