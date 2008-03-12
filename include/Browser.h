#ifndef BROWSER_H
#define BROWSER_H

class Browser {
	public:
		Browser(){}
		virtual ~Browser(){}
		
		virtual const char* get_next_file() = 0;
		virtual void reload() = 0;
		virtual void dump_queue() = 0;
};

#endif // BROWSER_H
