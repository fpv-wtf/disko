/*
 * mmscanvasfactory.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: sxs
 */

#include "mmsgui/mmscanvasfactory.h"

MMS_CANVAS_MAP MMSCanvasFactory::canvasFactoryList;

MMSCanvasFactory::MMSCanvasFactory() {

}

MMSCanvasFactory::~MMSCanvasFactory() {

}

void MMSCanvasFactory::registerCanvas(string name, MMS_CANVAS_CONSTRUCTOR constructor) {
	MMS_CANVAS_MAP::iterator it = this->canvasFactoryList.find(name);
	if(it != this->canvasFactoryList.end()) {
		throw MMSCanvasFactoryError(1, "a canvas mit name '" + name  + "' already registered");
	}

	this->canvasFactoryList.insert(std::make_pair(name, constructor));
}

MMSCanvasWidget *MMSCanvasFactory::constructCanvas(const char *name, MMSWindow *root, string classname, MMSTheme *theme) {
	MMS_CANVAS_MAP::iterator it = this->canvasFactoryList.find(name);

	if(it == this->canvasFactoryList.end()) {
		return NULL;
	}
	return it->second(root, classname, theme);
}
