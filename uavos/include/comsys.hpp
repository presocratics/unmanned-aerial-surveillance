/*
 * comsys.hpp
 *
 *  Created on: Jul 31, 2012
 *      Author: Xichen Shi
 */

#ifndef COMSYS_HPP_
#define COMSYS_HPP_

#include <UAS_comm.hpp>
#include <UAS_serial.hpp>
#include <mavlink/mavlink_types.h>
 
void initComsys();

void *runApmLink(void*);

void *runGcsLink(void*);

void *runComsys(void*);

#endif /* COMSYS_HPP_ */
