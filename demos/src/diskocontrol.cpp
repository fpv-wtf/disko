/*
 * control.cpp
 *
 *  Created on: Jan 24, 2010
 *      Author: sxs
 */
#include "diskocontrol.h"

void DiskoControl::getName(string &controlname) {
	controlname = this->controlname;
}

MMSWindow*  DiskoControl::getWindow() const {
	return this->controlwin;
}
