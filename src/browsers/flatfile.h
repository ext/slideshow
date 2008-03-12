#ifndef FLATFILE_BROWSER_H
#define FLATFILE_BROWSER_H

#include "Browser.h"

class FlatFileBrowser: public Browser {
	public:
		FlatFileBrowser(const char* filename);
		virtual ~FlatFileBrowser();
		
		virtual const char* get_next_file();
		
	private:
		void read_file();
		void clear_records();
		void get_record(unsigned int n);
		
		const char* _filename;
		const char** _records
		unsigned int _index;
		unsigned int _record_cnt;
};

#endif // FLATFILE_BROWSER_H
