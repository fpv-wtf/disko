/*
 * mmscanvaswidget.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: sxs
 */

#include "mmsgui/mmscanvaswidget.h"
#include "mmsgui/mmscanvasfactory.h"
#include <iostream>
#include <sstream>


MMSCanvasWidget::MMSCanvasWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
	create(root,className, theme);
	this->initialized = false;
}

MMSCanvasWidget::~MMSCanvasWidget() {

}

MMSWidget *MMSCanvasWidget::copyWidget() {
    // create widget
	MMSCanvasFactory factory;
    MMSCanvasWidget *newWidget = factory.constructCanvas(factoryname.c_str(), rootwindow, className, this->da->theme);

    newWidget->className = this->className;
    newWidget->canvasWidgetClass = this->canvasWidgetClass;
    newWidget->myCanvasWidgetClass = this->myCanvasWidgetClass;

    newWidget->attributes = this->attributes;
    newWidget->attributemap = this->attributemap;
    newWidget->canvastheme = this->canvastheme;

    // copy base widget
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    // call canvas method
    this->copyFunc((MMSWidget *)newWidget);

	return newWidget;
}

void MMSCanvasWidget::updateFromThemeClass(MMSCanvasWidgetClass *themeClass) {

	if(!themeClass)
		return;

	if( themeClass->isAttributes()) {
		this->setAttributes(themeClass->getAttributes());


	}

	MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

bool MMSCanvasWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_CANVAS;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->canvastheme = this->da->theme;
    this->canvasWidgetClass = this->da->theme->getCanvasWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->canvasWidgetClass.widgetClass);
    if (this->canvasWidgetClass) this->da->widgetClass = &(this->canvasWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    // clear
	this->current_fgset = false;

    // create widget base
	return MMSWidget::create(root, true, true, true, true, true, true, true);

}

bool MMSCanvasWidget::init() {
    // init widget basics

    initFunc();

    if (!MMSWidget::init())
        return false;
    return true;
}

bool MMSCanvasWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    return releaseFunc();
}

bool MMSCanvasWidget::draw(bool *backgroundFilled) {

	canvasSurface = this->surface;

	if (MMSWidget::draw(backgroundFilled)) {
    	drawingFunc(this->surface, this->surfaceGeom, backgroundFilled);
    }

	/* draw widgets debug frame */
	return MMSWidget::drawDebug();
}


bool MMSCanvasWidget::enableRefresh(bool enable) {
	if (!MMSWidget::enableRefresh(enable)) return false;

	// mark foreground as not set
	this->current_fgset = false;

	return true;
}

bool MMSCanvasWidget::checkRefreshStatus() {
	if (MMSWidget::checkRefreshStatus()) return true;

	if (this->current_fgset) {
		// current foreground initialized
			return false;
	}

	// (re-)enable refreshing
	enableRefresh();

	return true;
}

/*void MMSCanvasWidget::handleInput(MMSInputEvent *inputevent) {
	if(!handleInputFunc(inputevent))
		throw MMSWidgetError(1,"input not handled");
}
*/
void MMSCanvasWidget::setAttributes(string attr) {
	std::stringstream ss(attr);
	std::string item, item2;

	while(std::getline(ss,item, ';')) {
	std::stringstream ss2(item);
		std::string key = "";
		std::string val = "";
		while(std::getline(ss2,item2, ':')) {
				//trim result
				trim(item2);
			if(key.empty())
				key = item2;
			else
				val = item2;
		}
		if(!key.empty())
			attributemap.insert(std::make_pair(key,val));
	}
}

MMSFontManager *MMSCanvasWidget::getFontManager() {
	return this->rootwindow->fm;
}

void MMSCanvasWidget::checkInit() {
	if(!initialized)
		this->init();
}

MMSImageManager *MMSCanvasWidget::getImageManager() {
	return this->rootwindow->im;
}

void MMSCanvasWidget::add(MMSWidget *widget) {
	this->children.push_back(widget);
	widget->setParent(this);
	if (this->rootwindow)
		this->rootwindow->add(widget);
}
