#include "dummybrowser.h"

DummyBrowser::DummyBrowser():
	n(0){
	
	_img[0] = "IMG_0298.JPG";
	_img[1] = "IMG_0299.JPG";
	_img[2] = "IMG_0300.JPG";
	_img[3] = "IMG_0301.JPG";
}

DummyBrowser::~DummyBrowser(){
	
}

const char* DummyBrowser::get_next_file(){
	return _img[++n%4];
}
