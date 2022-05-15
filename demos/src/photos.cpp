/*
 * photos.cpp
 *
 *  Created on: Feb 07, 2010
 *      Author: jys
 */

#include "photos.h"

Photos::Photos(MMSDialogManager &dm, string name) {
	if(!name.empty()) {
		this->controlname = name;
	} else {
		controlname = "Photos";
	}
	try {
		dm.loadChildDialog(config.getData() + "/photos.xml");
		MMSWindow *win = dm.getWindow();
		this->controlwin = win->findWindow("photos");
		this->controlwin->setName(this->controlname);

		this->photos_menuwin = this->controlwin->findWindow("photos_menuwin");
		this->photos_menu = (MMSMenuWidget*)this->photos_menuwin->findWidget("photos_menu");
		this->photos_menu->onReturn->connect(sigc::mem_fun(this,&Photos::onReturn));

		// get access to the zoomwin
		this->photos_zoomwin = this->controlwin->findWindow("photos_zoomwin");
		this->photos_zoomwin->hide();
		this->photos_image = (MMSImageWidget *)this->photos_zoomwin->findWidget("photos_image");
//		this->photos_image_caption = (MMSLabelWidget *)this->photos_zoomwin->findWidget("photos_image_caption");
//		this->photos_zoomwin->onHandleInput->connect(sigc::mem_fun(this,&Photos::onHandleInput));

		// get access to the finalwin
		this->photos_finalwin = this->controlwin->findWindow("photos_finalwin");
		this->photos_finalwin->hide();
		this->photos_fimage = (MMSImageWidget *)this->photos_finalwin->findWidget("photos_fimage");
		this->photos_fimage_caption = (MMSLabelWidget *)this->photos_finalwin->findWidget("photos_fimage_caption");
		this->photos_finalwin->onHandleInput->connect(sigc::mem_fun(this,&Photos::onHandleInput));

		MMSFBRectangle rect = this->photos_finalwin->getGeometry();
		printf("photos_finalwin geometry %d,%d,%d,%d\n", rect.x,rect.y,rect.w,rect.h);

	    // add animation callbacks
	    this->onBeforeAnimation_connection = this->pulser.onBeforeAnimation.connect(sigc::mem_fun(this, &Photos::onBeforeAnimation));
	    this->onAnimation_connection = this->pulser.onAnimation.connect(sigc::mem_fun(this, &Photos::onAnimation));
	    this->onAfterAnimation_connection = this->pulser.onAfterAnimation.connect(sigc::mem_fun(this, &Photos::onAfterAnimation));
	    this->animIsRunning = false;

	} catch(MMSDialogManagerError *err) {
		printf("error: %s\n", err->getMessage().c_str());
	}
}

Photos::~Photos() {

}

#define MAX_OFFSET	30

bool Photos::onBeforeAnimation(MMSPulser *pulser) {

	if (this->animMode == ANIM_MODE_ZOOM_IN) {
		// zoomwin should appear
		MMSImageWidget *item = (MMSImageWidget *)this->photos_menu->getSelectedItem();
		MMSFBRectangle wrect = this->controlwin->getGeometry();
		MMSFBRectangle zrect = this->photos_zoomwin->getGeometry();
		MMSFBRectangle rect = item->getGeometry();

		// calc ratio to the middle
		double xx = (double)rect.x + (double)rect.w / 2 - (double)wrect.w / 2;
		xx = xx * 100 / (double)zrect.w;
		double yy = (double)rect.y + (double)rect.h / 2 - (double)wrect.h / 2;
		yy = yy * 100 / (double)zrect.h;

		double f  = wrect.w - zrect.w;
		f/= zrect.w;
		double f2 = wrect.h - zrect.h;
		f2/= zrect.h;
		if (f2 < f) f = f2;
		f/= 2;
		f*= 100;

		this->sleft = 100 - xx;
		this->fleft = (f + (100 - this->sleft)) / MAX_OFFSET;
		this->sup	= 100 - yy;
		this->fup	= (f + (100 - this->sup)) / MAX_OFFSET;
		this->sright= 100 + xx;
		this->fright= (f + (100 - this->sright)) / MAX_OFFSET;
		this->sdown = 100 + yy;
		this->fdown = (f + (100 - this->sdown)) / MAX_OFFSET;

		if (item) {
			// set pictures
			this->photos_image->setImageName(item->getImageName());
			string is;
			item->getData(is);
			this->photos_fimage->setImageName(is);

			// set picture description
			MMSLabelWidget *label = (MMSLabelWidget *)item->findWidget("photos_image_caption");
//			this->photos_image_caption->setText((label)?label->getText():"");
			this->photos_fimage_caption->setText((label)?label->getText():"");
		}

		this->photos_menuwin->setOpacity(192);

		this->photos_zoomwin->stretch(this->sleft, this->sup, this->sright, this->sdown);
		this->photos_zoomwin->show();
		this->photos_zoomwin->waitUntilShown();
	}
	else
	if (this->animMode == ANIM_MODE_ZOOM_OUT) {
		// zoomwin should disappear
		this->photos_zoomwin->stretch(	this->sleft + MAX_OFFSET * this->fleft,
										this->sup   + MAX_OFFSET * this->fup,
										this->sright+ MAX_OFFSET * this->fright,
										this->sdown + MAX_OFFSET * this->fdown);
		this->photos_zoomwin->show();
		this->photos_zoomwin->waitUntilShown();
		this->photos_finalwin->hide();
		this->photos_finalwin->waitUntilHidden();

	}

	// animation is started
	this->animIsRunning = true;

	return true;
}


bool Photos::onAnimation(MMSPulser *pulser) {

	// get the offset needed to calculate my next things
	double animOffs = pulser->getOffset();

printf("X) offset = %f, real duration = %d\n", animOffs, pulser->getRealDuration());

	// stretch the window with the offset
	this->photos_zoomwin->stretch(	this->sleft + animOffs * this->fleft,
									this->sup   + animOffs * this->fup,
									this->sright+ animOffs * this->fright,
									this->sdown + animOffs * this->fdown);

	return true;
}

void Photos::onAfterAnimation(MMSPulser *pulser) {

	// animation finished
	if (this->animMode == ANIM_MODE_ZOOM_IN) {
		// set window to final state
		this->photos_finalwin->setOpacity(255);
		this->photos_finalwin->show();
		this->photos_finalwin->waitUntilShown();
		this->photos_zoomwin->hide();
		this->photos_zoomwin->waitUntilHidden();

	}
	else
	if (this->animMode == ANIM_MODE_ZOOM_OUT) {
		// set window to final state
		this->photos_zoomwin->stretch(this->sleft, this->sup, this->sright, this->sdown);
		this->photos_zoomwin->hide();
		this->photos_zoomwin->waitUntilHidden();
		this->photos_zoomwin->stretch();
		this->photos_menuwin->setOpacity(255);
	}

	// animation is stopped
	this->animIsRunning = false;

	printf("framerate = %d\n", pulser->getFrameRate());
	printf("frames    = %d\n", pulser->getFrames());
	printf("Y) real duration = %d\n", pulser->getRealDuration());

}


void Photos::onReturn(MMSWidget* widget) {
	if (widget == (MMSWidget *)this->photos_menu) {
    	// do the animation
		if (!this->animIsRunning) {
			this->animMode = ANIM_MODE_ZOOM_IN;
			this->pulser.setStepsPerSecond(MAX_OFFSET * 5);
			this->pulser.setMaxOffset(MAX_OFFSET, MMSPULSER_SEQ_LOG_SOFT_END, MAX_OFFSET / 3);
			this->pulser.start(false);
		}
	}
}

bool Photos::onHandleInput(MMSWindow *win, MMSInputEvent *event) {
	if (win == this->photos_finalwin) {
		if (!this->animIsRunning) {
			this->animMode = ANIM_MODE_ZOOM_OUT;
			this->pulser.setStepsPerSecond(MAX_OFFSET * 5);
			this->pulser.setMaxOffset(MAX_OFFSET, MMSPULSER_SEQ_LOG_DESC_SOFT_END, MAX_OFFSET / 3);
			this->pulser.start(false);
		}
	}

	return true;
}

