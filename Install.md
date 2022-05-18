## Dependencies:
On a debian based system, those are the dependencies I had to install (this might not be a full list since I am sure I had some of the dependencies already installed):
```
apt install scons libsigc++-2.0-dev libxml2-dev libfreetype6-dev libdirectfb-dev libxine2-dev libsqlite3-dev libtiff-dev
```

Running `check` will basicaly tell you what's missing in regards to libraries. That being said not all newly installed libraries were picked up immediately and `.sconsign.dblite` needed to be deleted in order for the dependencies to be picked up.

## Config

### For X11 with OpenGl2:
```
scons graphics_backend=x11 graphics_outputtype=gl2 pixelformats=all enable_tools=yes messages=yes debug=yes profile=yes enable_crypt=no check
```

### For Goggle (directfb and stdfb):
```
scons graphics_backend=dfb graphics_outputtype=stdfb pixelformats=argb enable_tools=yes messages=yes debug=yes profile=yes enable_crypt=no check
```

### All in one go
```
scons graphics_backend=dfb,x11 graphics_outputtype=stdfb,gl2 pixelformats=all enable_tools=yes messages=yes debug=yes profile=yes enable_crypt=no check
```


## Compile and install

```
scons
sudo scons install
```

## Compile Demo
`disko` needs to be compiled and installed beforehand. The demo uses X11 backend and gl2 as outputtype.

```
cd demo
scons
./bin/demo
```
