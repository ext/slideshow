## Slideshow

Slideshow is a kiosk-style application for showing text, image and video in a continious loop on monitors and projectors. Content is edited and updated directly in using a webgui. The application is split into two packages, a backend and a frontend. The backend is written in C++ and OpenGL and the frontend using python. Currently the frontend is very GNU/Linux specific and will not run on any other platform but is meant to be as platform independent as possible, as long as it handles OpenGL.

## Features

* Webgui controls daemon, even starting and stopping.
* Slides consisting of either text, images or video.
* Custom themes for text-slides.
* Automatic aspect-correct resizing with ImageMagick.
* Natively loads most image formats, including but not limited to: BMP, JPEG, PNG, GIF, TGA.
* Hardware acceleration with OpenGL.
* Transition via plugin-based system (fade and spin builtin).
* Licensed under AGPLv3.

## Requirements

* automake-1.11 or later
* DevIL
* GLEW
* libdaemon
* libjson
* libcurl
* [datapack](https://github.com/ext/datapack)

## Optional

* SDL
* sqlite3 or mysql
* DBus
* Ragel

## Debian/ubuntu requirements

    sudo apt-get install build-essentials libtool autoconf pkg-config ragel libdaemon-dev libdevil-dev libjson-c-dev libcurl4-openssl-dev libsqlite3-dev python-cherrypy3 python-genshi python-xlib python-pythonmagick python-gamin
    sudo pip install htmlcolor

## Building daemon

    autoreconf -si
    mkdir build
    cd build
    ../configure --with-sdl
    make

# Usage

    (hack to run inplace)
    export PYTHONPATH=.
    python slideshow/web --install /path/to/storage
    python slideshow/web -f /path/to/storage/settings.conf

## Theme preview

    python slideshow/theme.py THEME OUTPUT.PNG
