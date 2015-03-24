/*
 * self_test.h
 *
 *  Created on: May 17, 2013
 *      Author: trosen
 */

#include <nacs-old-pulser/pulser.h>

#ifndef SELF_TEST_H_
#define SELF_TEST_H_

namespace NaCs {

namespace self_test {

//check pulse controller register n
bool check_register(Pulser::Pulser &pulser, int n);
bool check_timing(Pulser::Pulser &pulser);
bool other_test(Pulser::Pulser &pulser);

}

}

#endif
