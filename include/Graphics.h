#ifndef GRAPHICS_H
#define GRAPHICS_H

class Transition;

class Graphics {
	public:
		Graphics(int width, int height, bool fullscreen);
		~Graphics();
		
		void render(float state);
		void load_image(const char* filename);
		void set_transition(Transition* transition);
		
	private:
		void swap_textures();
		
		Transition* _transition;
		unsigned int texture_0;
		unsigned int texture_1;
};

#endif // GRAPHICS_H
