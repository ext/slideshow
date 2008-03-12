#ifndef SLIDESHOW_IPC_H
#define SLIDESHOW_IPC_H

class Kernel;

class IPC {
	public:
		IPC(Kernel* kernel): _kernel(kernel){}
		virtual ~IPC(){}
		
		virtual void poll() = 0;
		
	protected:
		Kernel* kernel(){ return _kernel; }
		
	private:
		Kernel* _kernel;
};

#endif // SLIDESHOW_IPC_H
