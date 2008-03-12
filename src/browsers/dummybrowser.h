#ifndef DUMMY_BROWSER_H
#define DUMMY_BROWSER_H

#include "Browser.h"

class DummyBrowser: public Browser {
	public:
		DummyBrowser();
		virtual ~DummyBrowser();
		
		virtual const char* get_next_file();
		
	private:
		unsigned int n;
		const char* _img[4];
};

#endif // DUMMY_BROWSER_H
