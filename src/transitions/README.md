# Transition effects

Transition effects runs during the transition state and animates the change from the old slide to the new.

## Implementing a new transition

For simple effects all that is needed is a GLSL fragment shader with the uniform `s` and two texture units `texture_0` (old slide) and `texture_1` (new slide). `s` is a float going from 0 to 1 where 1 is the finished transition.

It is good practice to allow the transition effect to work both forwards and backwards (i.e. `s` going from 1.0 to 0.0).

## Advanced (fully customizable)

For advanced effects override `int module_init(transition_module_t*)` setting a custom rendering callback:

    int EXPORT module_init(transition_module_t module){
    	module->render = my_render_callback;
    	return 0;
    }

In addition you may also override `module_alloc` and `module_free` in order to allocate a custom struct which other data. Take care to only allocate memory in `module_alloc` and defer the initialization to `module_init`. `module_cleanup` is called to release any resources.

The render callback will have to bind the texture units and uniforms itself.
