/*
 * carrousel.cpp
 *
 *  Created on: Jan 26, 2010
 *      Author: sxs
 */

#include "carrousel.h"

MMSLabelWidget *fps;
int FPS1old = 0;
int FPS1 = 0;
int FPS2 = 0;
struct timeval perf_stime;
struct timeval perf_etime;


#ifdef __HAVE_MMS3D__

typedef enum {
	MODE_ROTATE_Y = 0,
	MODE_ROTATE_X,
	MODE_ROTATE_Z,
	MODE_ROTATE_XY,
	MODE_ROTATE_YZ,
	MODE_ROTATE_XZ,
	MODE_ROTATE_XYZ
} MODE;

MODE mode = MODE_ROTATE_Y;
float angle_x = 45;
bool angle_x_inc = true;
float angle_y = 0;
bool angle_y_inc = true;
float angle_z = 0;
bool angle_z_inc = true;

float min_speed = 0.25;
float max_speed = 2;
float speed = min_speed;
bool speed_inc = true;

#define CALC_ANGLE(a,ai) \
	if (ai) { \
		a+=speed; \
		if (a>=360) { \
			a = 360; \
			ai = false; \
		} \
	} \
	else { \
		a-=speed; \
		if (a<=0) { \
			a = 0; \
			ai = true; \
		} \
	}

void process_modes() {
	switch (mode) {
	case MODE_ROTATE_Y:
		CALC_ANGLE(angle_y,angle_y_inc);
		break;
	case MODE_ROTATE_X:
		CALC_ANGLE(angle_x,angle_x_inc);
		break;
	case MODE_ROTATE_Z:
		CALC_ANGLE(angle_z,angle_z_inc);
		break;
	case MODE_ROTATE_XY:
		CALC_ANGLE(angle_x,angle_x_inc);
		CALC_ANGLE(angle_y,angle_y_inc);
		break;
	case MODE_ROTATE_YZ:
		CALC_ANGLE(angle_y,angle_y_inc);
		CALC_ANGLE(angle_z,angle_z_inc);
		break;
	case MODE_ROTATE_XZ:
		CALC_ANGLE(angle_x,angle_x_inc);
		CALC_ANGLE(angle_z,angle_z_inc);
		break;
	case MODE_ROTATE_XYZ:
		CALC_ANGLE(angle_x,angle_x_inc);
		CALC_ANGLE(angle_y,angle_y_inc);
		CALC_ANGLE(angle_z,angle_z_inc);
		break;
	}
}


class MyScene : public MMSThread {
    private:
        MMSWindow *window;

        MMS3DTexture	mms3dtex;
        MMS3DScene		scene;
        MMS3DObject		*HEAD;
        MMS3DObject		*CUBE;

        MMSImageManager	im;



        void render(MMSFBSurface *surface) {
        	// head: clear matrix operations
        	HEAD->reset();

        	// head: rotate the object
        	HEAD->rotate(angle_z, 0, 0, 1);
        	HEAD->rotate(angle_y+180, 0, 1, 0);
        	HEAD->rotate(angle_x, 1, 0, 0);

        	// head: rotate the object with a radius around the center
        	HEAD->translate(0, 0, -200);
        	HEAD->rotate(angle_z, 0, 0, 1);
        	HEAD->rotate(-angle_y, 0, 1, 0);
        	HEAD->rotate(-(angle_x-45), 1, 0, 0);


        	// cube: clear matrix operations
        	CUBE->reset();

        	// cube: rotate the object
        	CUBE->rotate(angle_x, 1, 0, 0);
        	CUBE->rotate(angle_y, 0, 1, 0);
        	CUBE->rotate(angle_z, 0, 0, 1);


        	// (re-)generate matrices of the whole scene
        	scene.genMatrices();


        	// render scene...
        	MMS3D_VERTEX_ARRAY	**varrays;
        	MMS3D_INDEX_ARRAY	**iarrays;
        	MMS3D_MATERIAL		*materials;
        	MMSFBSurface		**textures;
        	MMS3D_OBJECT		**objects;
        	scene.getMeshArrays(&varrays, &iarrays);
        	MMS3DMaterial mat;
        	mat.getBuffer(&materials);
        	mms3dtex.getBuffer(&textures);
        	scene.getObjects(&objects);
        	surface->renderScene(varrays, iarrays, materials, textures, objects);
        }

        bool onDraw(MMSFBSurface *surface, bool clear) {
        	render(surface);
        	return true;
        }

        void threadMain() {
        	int cnter = 0;
        	sleep(1);
        	int usleep_time = 10000;

        	while(1) {

        		if (fps->getRootWindow()->isShown(true)) {
        			// all my parent windows are shown
					gettimeofday(&perf_stime, NULL);

					fps->setText(iToStr((int)angle_x) + "°, "
									+ iToStr((int)angle_y) + "°, "
									+ iToStr((int)angle_z) + "°, "
									+ iToStr(FPS1) + "." + iToStr(FPS2) + "fps");

		        	if (this->window) {
		        		window->refresh();
		        	}

					process_modes();

					gettimeofday(&perf_etime, NULL);

					FPS1old = FPS1;

					if (perf_etime.tv_usec >= perf_stime.tv_usec)
						FPS1 = perf_etime.tv_usec - perf_stime.tv_usec;
					else
						FPS1 = 1000000 - perf_stime.tv_usec + perf_etime.tv_usec;

					float xxx = FPS1;
					FPS2 = 10000000 / FPS1 - (1000000 / FPS1) * 10;
					FPS1 = 1000000 / FPS1;
					if (FPS1 > 1000) {
						FPS1=0;
						FPS2=0;
					}

					if (FPS1old > 0) {
						FPS1 = (FPS1 + FPS1old) / 2;
					}

					if (cnter < 3) {
						// the first 3 loops are used to set min/max/speed
						float dur = xxx + usleep_time;
						min_speed = 10.0f / (1000000 / dur);
						max_speed = speed * 8;
						speed = min_speed;
						cnter++;
					}
        		}

				usleep(usleep_time);

        	}
        }

    public:
    	MyScene(MMSWindow *window) {
        	// window on which the 3D scene has to be drawn
    		if (!window) return;
        	this->window = window;

        	// connect window's onDraw callback
        	window->onDraw->connect(sigc::mem_fun(this,&MyScene::onDraw));

        	// setup the base matrix for scene
			int w, h;
			window->getSurface()->getSize(&w, &h);
			MMS3DMatrix matrix;
			loadIdentityMatrix(matrix);
			frustumMatrix(matrix, -w/2, w/2, -h/2, h/2, 200, 1700);
    		scene.setBaseMatrix(matrix);

    		// move whole scene into depth
    		scene.translate(0, 0, -500);

    		// generate objects & meshes
    		MMS3DObject *object;

    		// head
    		HEAD = new MMS3DObject(&scene);
#ifdef __HAVE_GL2__
//TODO: currently our GLES2 shaders do not support materials and normals

    		HEAD->show();

    		// head: face
    		object = (MMS3DObject *) new MMS3DSphere(&scene, MMS3DMaterial::MAT_RED, MMS3DTexture::TEX_NOTSET, 150, 75);
    		object->show();
    		object->cullFace(true);
    		HEAD->addObject(object);

    		// head: hair
    		object = (MMS3DObject *) new MMS3DSphere(&scene, MMS3DMaterial::MAT_BLACK, MMS3DTexture::TEX_NOTSET, 150, 75);
    		object->show();
    		object->cullFace(true);
    		object->scale(0.9, 0.9, 0.9);
    		object->translate(0, -15, 0);
    		HEAD->addObject(object);

    		// head: eye #1 (left)
    		object = (MMS3DObject *) new MMS3DSphere(&scene, MMS3DMaterial::MAT_WHITE, MMS3DTexture::TEX_NOTSET, 50, 15);
    		object->show();
    		object->cullFace(true);
    		object->translate(0, 0, -65);
    		object->rotate(-5, 1, 0, 0);
    		HEAD->addObject(object);

    		// head: eye #2 (right)
    		object = (MMS3DObject *) new MMS3DSphere(&scene, MMS3DMaterial::MAT_WHITE, MMS3DTexture::TEX_NOTSET, 50, 15);
    		object->show();
    		object->cullFace(true);
    		object->translate(0, 0, -65);
    		object->rotate(-5, 1, 0, 0);
    		object->rotate(50, 0, 1, 0);
    		HEAD->addObject(object);

    		// head: nose
    		object = (MMS3DObject *) new MMS3DSphere(&scene, MMS3DMaterial::MAT_YELLOW, MMS3DTexture::TEX_NOTSET, 50, 15);
    		object->show();
    		object->cullFace(true);
    		object->scale(1, 2, 1);
    		object->rotate(30, 1, 0, 0);
    		object->translate(0, 0, -70);
    		object->rotate(25, 0, 1, 0);
    		HEAD->addObject(object);

    		// head: mouth
    		object = (MMS3DObject *) new MMS3DTorus(&scene, MMS3DMaterial::MAT_WHITE, MMS3DTexture::TEX_NOTSET, 30, 18, 45, 20);
    		object->show();
    		object->cullFace(true);
    		object->rotate(-45, 1, 0, 0);
    		object->translate(0, 0, -17);
    		object->rotate(25, 0, 1, 0);
    		HEAD->addObject(object);

    		// head: cap
    		object = (MMS3DObject *) new MMS3DCylinder(&scene, MMS3DMaterial::MAT_YELLOW, MMS3DTexture::TEX_NOTSET, 16, 80, 30);
    		object->show();
    		object->cullFace(false);
    		object->rotate(115, 0, 1, 0);
    		object->translate(0, -70, 0);
    		HEAD->addObject(object);
#endif

    		// cube
    		CUBE = new MMS3DObject(&scene);
    		CUBE->show();

    		// cube: for each face
    		for (int i = 0; i < 6; i++) {
    			MMSFBSurface *texsurface;
    			switch (i) {
    			case 0:
    				texsurface = im.getImage("share/themes/default/photos", "fuerteventura_1_small.png");

    				object = (MMS3DObject *) new MMS3DRectangle(&scene,
													(texsurface)?MMS3DMaterial::MAT_NOTSET:MMS3DMaterial::MAT_GREEN,
													(texsurface)?mms3dtex.genTexture(texsurface):MMS3DTexture::TEX_NOTSET,
													100, 100);

    				object->translate(0, 0, 50);
    				break;
    			case 1:
    				texsurface = im.getImage("share/themes/default/photos", "fuerteventura_2_small.png");

    				object = (MMS3DObject *) new MMS3DRectangle(&scene,
													(texsurface)?MMS3DMaterial::MAT_NOTSET:MMS3DMaterial::MAT_GREEN,
													(texsurface)?mms3dtex.genTexture(texsurface):MMS3DTexture::TEX_NOTSET,
													100, 100);

    				object->translate(0, 0, 50);
    				object->rotate(90, 0, 1, 0);
    				break;
    			case 2:
    				texsurface = im.getImage("share/themes/default/photos", "fuerteventura_3_small.png");

    				object = (MMS3DObject *) new MMS3DRectangle(&scene,
													(texsurface)?MMS3DMaterial::MAT_NOTSET:MMS3DMaterial::MAT_GREEN,
													(texsurface)?mms3dtex.genTexture(texsurface):MMS3DTexture::TEX_NOTSET,
													100, 100);

    				object->translate(0, 0, 50);
    				object->rotate(180, 0, 1, 0);
    				break;
    			case 3:
    				texsurface = im.getImage("share/themes/default/photos", "gran_canaria_1_small.png");

    				object = (MMS3DObject *) new MMS3DRectangle(&scene,
													(texsurface)?MMS3DMaterial::MAT_NOTSET:MMS3DMaterial::MAT_GREEN,
													(texsurface)?mms3dtex.genTexture(texsurface):MMS3DTexture::TEX_NOTSET,
													100, 100);

    				object->translate(0, 0, 50);
    				object->rotate(270, 0, 1, 0);
    				break;
    			case 4:
    				texsurface = im.getImage("share/themes/default/photos", "gran_canaria_2_small.png");

    				object = (MMS3DObject *) new MMS3DRectangle(&scene,
													(texsurface)?MMS3DMaterial::MAT_NOTSET:MMS3DMaterial::MAT_GREEN,
													(texsurface)?mms3dtex.genTexture(texsurface):MMS3DTexture::TEX_NOTSET,
													100, 100);

    				object->translate(0, 0, 50);
    				object->rotate(90, 1, 0, 0);
    				break;
    			case 5:
    				texsurface = im.getImage("share/themes/default/photos", "gran_canaria_3_small.png");

    				object = (MMS3DObject *) new MMS3DRectangle(&scene,
													(texsurface)?MMS3DMaterial::MAT_NOTSET:MMS3DMaterial::MAT_GREEN,
													(texsurface)?mms3dtex.genTexture(texsurface):MMS3DTexture::TEX_NOTSET,
													100, 100);

    				object->translate(0, 0, 50);
    				object->rotate(270, 1, 0, 0);
    				break;
    			}

    			object->show();
    			object->cullFace(true);
    			CUBE->addObject(object);
    		}


        }
};

#endif

Carrousel::Carrousel(MMSDialogManager &dm, string name) {
	if(!name.empty()) {
		this->controlname = name;
	} else {
		controlname = "Carrousel";
	}
	try {
		dm.loadChildDialog(config.getData() + "/carrousel.xml");
		MMSWindow *win = dm.getWindow();
		this->controlwin = win->findWindow("carrousel");
		this->controlwin->setName(this->controlname);

		this->last_si = 0;

		MMSWidget *menu = this->controlwin->findWidget("menu");
		this->onReturnConnection = menu->onReturn->connect(sigc::mem_fun(this,&Carrousel::onReturn));

		fps = (MMSLabelWidget *)this->controlwin->findWidget("fps");

#ifdef __HAVE_MMS3D__
		MyScene *myscene = new MyScene(this->controlwin->findWindow("scene_window"));
		myscene->start();
#endif

	} catch(MMSDialogManagerError *err) {
		printf("error: %s\n", err->getMessage().c_str());
	}
}

Carrousel::~Carrousel() {

}


void Carrousel::onReturn(MMSWidget* widget) {
#ifdef __HAVE_MMS3D__
	if (widget->getType()!=MMSWIDGETTYPE_MENU)
		return;

	MMSMenuWidget *menu = (MMSMenuWidget *)widget;

	unsigned si = menu->getSelected();

	if (si != this->last_si) {
		switch (si) {
		case 0:
			mode = MODE_ROTATE_Y;
			break;
		case 1:
			mode = MODE_ROTATE_X;
			break;
		case 2:
			mode = MODE_ROTATE_Z;
			break;
		case 3:
			mode = MODE_ROTATE_XY;
			break;
		case 4:
			mode = MODE_ROTATE_YZ;
			break;
		case 5:
			mode = MODE_ROTATE_XZ;
			break;
		case 6:
			mode = MODE_ROTATE_XYZ;
			break;
		}
	}
	else {
		if (speed_inc) {
			speed+=min_speed;
			if (speed>=max_speed) {
				speed=max_speed;
				speed_inc = false;
			}
		}
		else {
			speed-=min_speed;
			if (speed<=min_speed) {
				speed=min_speed;
				speed_inc = true;
			}
		}
	}

	this->last_si = si;
#endif
}

