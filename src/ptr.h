#ifndef __PTR_H
#define __PTR_H

#include <cstdlib>

template <class T>
class Ptr {
public:
	Ptr(T* p = NULL):
		_p(p){}

	~Ptr(){
		reset();
	}

	T* get(){
		return _p;
	}

	void reset(T* p = NULL){
		if ( _p == p ){
			return;
		}

		free(_p);
		_p = p;
	}

private:
	T* _p;
};

#endif /* __PTR_H */
