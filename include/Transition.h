#ifndef TRANSITION_H
#define TRANSITION_H

class Transition {
	public:
		Transition(){}
		virtual ~Transition(){}
		
		virtual void render(unsigned int texture_0, unsigned int texture_1, float state) = 0;
};

#endif // TRANSITION_H
