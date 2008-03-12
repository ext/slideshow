#ifndef MYSQL_BROWSER_H
#define MYSQL_BROWSER_H

#include "Browser.h"

#ifndef NULL
#	define NULL 0
#endif

class MySQLBrowser: public Browser {
	public:
		
		MySQLBrowser(const char* username, const char* password = NULL, const char* database = "slideshow", const char* host = "localhost");
		virtual ~MySQLBrowser();
		
		virtual const char* get_next_file();
		virtual void reload();
		
		virtual void dump_queue();
		
		void set_username(const char* username);
		void set_password(const char* password);
		void set_database(const char* database);
		void set_hostname(const char* hostname);
		
	private:
		void connect();
		void disconnect();
		struct st_mysql_res* query(const char* str, ...);
		
		void clear_fields();
		void allocate_fields(unsigned int n);
		void set_field(unsigned int n, const char* str);
		const char* get_field(unsigned int n);
		
		char* _username;
		char* _password;
		char* _database;
		char* _hostname;
		struct st_mysql *_conn;
		
		char** _fields;
		unsigned int _nr_of_fields;
		unsigned int _current_field;
};

#endif // MYSQL_BROWSER_H
