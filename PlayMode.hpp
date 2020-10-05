#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

#include <hb.h>
#include <hb-ft.h>

#include "GL.hpp"

#include "ColorTextureProgram.hpp"
#include "game4_util.h"
#include "hbshaper.h"

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;

	// text drawing
#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 target_color = HEX_TO_U8VEC4(0x42f566ff);
	const glm::u8vec4 white_color = HEX_TO_U8VEC4(0xffffffff);
	const glm::u8vec4 base_color = HEX_TO_U8VEC4(0xa709abcc);
#undef HEX_TO_U8VEC4

	void initGL();

	FreeTypeLib lib;
	HBShaper choiceShaper = HBShaper("C:\\Windows\\Fonts\\Arial.ttf", &lib, 18);


	std::vector<VMesh*> meshes;

	//Shader program that draws transformed, vertices tinted with vertex colors:
	//ColorTextureProgram *texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	GLuint background_tex;
	GLuint jean_tex;
	GLuint angry_tex;
	GLuint white_tex;

	float box_padding = 25.0f;
	glm::vec2 promptbox_botright;
	glm::vec2 optionbox_botright;

	struct Textscreen {
		std::vector< HBText > prompt;
		std::vector< HBText > options;
		float promptX = 200.0f;
		float promptY = 480.0f;
		float promptSpacing = 40.0f;
		float optionsX = 350.0f;
		float optionsY = 200.0f;
		float optionSpacing = 40.0f;
		int num_choices = 0;
	};

	void drawScreen(Textscreen& screen);
	void deleteScreen(Textscreen& screen);
	void add_prompt(Textscreen& screen, std::string text);
	void add_option(Textscreen& screen, std::string text);
	void switchToScreen(Textscreen& screen);

	Textscreen intro;
	Textscreen screen1;
	Textscreen screen2;
	Textscreen screen3;
	Textscreen screen4;
	Textscreen screen5;
	Textscreen screen6;
	Textscreen screen7;
	Textscreen screen8;
	Textscreen screen9;
	Textscreen screen10;
	Textscreen screen11;
	Textscreen screen12;
	Textscreen screen13;

	Textscreen* activescreen;
	int choice = 0;
	bool failed_flag = false;
};
