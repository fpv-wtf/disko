/*
 * mmscanvasfactory.h
 *
 *  Created on: Nov 3, 2011
 *      Author: sxs
 */

#ifndef MMSCANVASFACTORY_H_
#define MMSCANVASFACTORY_H_

#include "mmsgui/mmscanvaswidget.h"
#include <map>

typedef MMSCanvasWidget *(*MMS_CANVAS_CONSTRUCTOR)(MMSWindow *, string, MMSTheme *);

typedef std::map<std::string, MMS_CANVAS_CONSTRUCTOR> MMS_CANVAS_MAP;


class MMSCanvasFactory {
	public:
		MMSCanvasFactory();
		~MMSCanvasFactory();
		void registerCanvas(string name, MMS_CANVAS_CONSTRUCTOR constructor);
		MMSCanvasWidget *constructCanvas(const char *name, MMSWindow *root, string classname, MMSTheme *theme);

	private:
        static MMS_CANVAS_MAP canvasFactoryList;

};

MMS_CREATEERROR(MMSCanvasFactoryError);

#endif /* MMSCANVASFACTORY_H_ */
