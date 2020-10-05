#include "PlayMode.hpp"

#include "load_save_png.hpp"
#include "ColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "hbshaper.h"

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("blippy-trance-by-kevin-macleod.wav"));
});


// This code is mostly from game0 base code.
void PlayMode::initGL() {
	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}
	
	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(color_texture_program->Position_vec4);
		
		//set up the vertex array object to describe arrays of Vertex:
		glVertexAttribPointer(
			color_texture_program->Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 //offset
		); 
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]
		glEnableVertexAttribArray(color_texture_program->Color_vec4);
		glVertexAttribPointer(
			color_texture_program->Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 4 * 3 //offset
		);

		glEnableVertexAttribArray(color_texture_program->TexCoord_vec2);
		glVertexAttribPointer(
			color_texture_program->TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 4 * 3 + 4 * 1 //offset
		);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);
	}
	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1, 1);
		std::vector< glm::u8vec4 > data(size.x * size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	std::vector< glm::u8vec4 > data;
	glm::uvec2 size(0, 0);
	load_png(data_path("classroom.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &background_tex);
	glBindTexture(GL_TEXTURE_2D, background_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data.clear();
	load_png(data_path("pixel_jean.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &jean_tex);
	glBindTexture(GL_TEXTURE_2D, jean_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data.clear();
	load_png(data_path("pixel_jean_angry.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &angry_tex);
	glBindTexture(GL_TEXTURE_2D, angry_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void PlayMode::add_prompt(Textscreen& screen, std::string text) {
	HBText hb = { text, "en", HB_SCRIPT_LATIN, HB_DIRECTION_LTR };
	screen.prompt.push_back(hb);
}

void PlayMode::add_option(Textscreen& screen, std::string text) {
	HBText hb = { text, "en", HB_SCRIPT_LATIN, HB_DIRECTION_LTR };
	screen.options.push_back(hb);
	screen.num_choices += 1;
}

void PlayMode::drawScreen(Textscreen& screen) {
	float y = screen.promptY;
	promptbox_botright[0] = 0.0f;
	promptbox_botright[1] = 10000.0f;
	for (HBText hb : screen.prompt) {
		for (VMesh* mesh : choiceShaper.drawText(hb, screen.promptX, y, white_color, promptbox_botright)) {
			meshes.push_back(mesh);
		}
		y -= screen.promptSpacing;
	}
	y = screen.optionsY;
	int cnt = 0;
	optionbox_botright[0] = 0.0f;
	optionbox_botright[1] = 10000.0f;
	for (HBText hb : screen.options) {
		if (choice == cnt) {
			for (VMesh* mesh : choiceShaper.drawText(hb, screen.optionsX, y, target_color, optionbox_botright)) {
				meshes.push_back(mesh);
			}
		}
		else {
			for (VMesh* mesh : choiceShaper.drawText(hb, screen.optionsX, y, white_color, optionbox_botright)) {
				meshes.push_back(mesh);
			}
		}
		cnt++;
		y -= screen.optionSpacing;
	}
	activescreen = &screen;
}

void PlayMode::deleteScreen(Textscreen& screen) {
	choiceShaper.deleteMeshes(meshes);
}

PlayMode::PlayMode() {
	//start music loop playing:
	// (note: position will be over-ridden in update())
	leg_tip_loop = Sound::loop(*dusty_floor_sample, 0.7f, 0.0f);

	choiceShaper.init();

	initGL();

	add_prompt(intro, "You are taking the Game Programming Course at Central Michigan University.");
	add_prompt(intro, "The professor is the intelligent and beautiful Jean MacWomann.");
	add_prompt(intro, "You've been struggling in the course, and are attending office hours to try and impress the professor.");
	add_option(intro, "Press spacebar to continue");

	add_prompt(screen1, "Jean: This is your fifth time at OH in the last two weeks. What can I help you with this time?");
	add_option(screen1, "Show her your broken code and ask for help.");
	add_option(screen1, "Show her the code you're most proud of.");

	add_prompt(screen2, "Jean: I'm pretty sure you came to me with this bug yesterday. Did my advice not help?");
	add_option(screen2, "(Truth) I did manage to fix it, but came to OH again to talk to you.");
	add_option(screen2, "(Lie) I didn't quite understand what you said, could you clarify a few things?");

	add_prompt(screen3, "Jean: Well, that's...flattering. Is there something you want to talk about specifically?");
	add_option(screen3, "Your hair is so luscious!");
	add_option(screen3, "I actually wanted to talk about my code.");

	add_prompt(screen4, "Jean: I'm sorry, this has nothing to do with your code. If that's all, I'll need to move on to the next student.");
	add_prompt(screen4, "You didn't learn anything from OH. You received a C in the class.");
	add_option(screen4, "Try again?");

	add_prompt(screen5, "Jean: What do you need clarified?");
	add_option(screen5, "I don't understand how to set up these vertex buffers for OpenGL.");
	add_option(screen5, "I don't understand how you are so sucessful and beautiful at the same time!");

	add_prompt(screen6, "You spend the next thirty minutes going over vertex buffers with Jean, and learn enough to pass the class with flying colors.");
	add_prompt(screen6, "Result: You received a A in the class.");
	add_option(screen6, "This was the best result, but try again?");

	add_prompt(screen7, "Jean: This is really interesting! How did you manage to write this global illumination shader in a week?");
	add_option(screen7, "(Truth) I actually found it online.");
	add_option(screen7, "(Lie) I remembered some things from Computer Graphics, and spent all week grinding.");

	add_prompt(screen8, "Jean: That's alright, but don't forget to cite your source! And post it in the Discord for other students to learn from.");
	add_option(screen8, "Will do! See you later!");
	add_option(screen8, "I actually had this problem with my code...");

	add_prompt(screen9, "With Jean's reminder, you cite your source. But by the time you remember your code is still broken, OH is over.");
	add_prompt(screen9, "Result: You received a B in the class.");
	add_option(screen9, "Try again?");

	add_prompt(screen10, "Jean: I'm impressed. Good work! Is there anything else you want to talk about?");
	add_option(screen10, "Nope, see you later!");
	add_option(screen10, "I actually had this problem with my code...");

	add_prompt(screen11, "After OH, Jean checks your code and finds that you plagiarized.");
	add_prompt(screen11, "She fails youand does her best to get you expelled from Central Michigan University.");
	add_prompt(screen11, "Result: You received a F in the class.");
	add_option(screen11, "Try again?");

	add_prompt(screen12, "Jean: I'm sorry, this has nothing to do with your code. If that's all, I'll need to move on to the next student.");
	add_prompt(screen12, "After OH, Jean checks your code and finds that you plagiarized.");
	add_prompt(screen12, "She fails youand does her best to get you expelled from Central Michigan University.");
	add_prompt(screen12, "Result: You received a F in the class.");
	add_option(screen12, "Try again?");

	add_prompt(screen13, "You spend the next thirty minutes going over vertex buffers with Jean, and learn enough to pass the class with flying colors.");
	add_prompt(screen13, "HOWEVER, after OH, Jean checks your code and finds that you plagiarized.");
	add_prompt(screen13, "She fails youand does her best to get you expelled from Central Michigan University.");
	add_prompt(screen13, "Result: You received a F in the class.");
	add_option(screen13, "Try again?");

	drawScreen(intro);
}

PlayMode::~PlayMode() {
	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_w || evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s || evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE && !space.pressed) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_w || evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s || evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE && space.pressed) {
			space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::switchToScreen(Textscreen& screen) {
	choiceShaper.deleteMeshes(meshes);
	activescreen = &screen;
	choice = 0;
	drawScreen(*activescreen);
}

void PlayMode::update(float elapsed) {

	if (down.downs > 0) {
		if (choice < activescreen->num_choices - 1) {
			choice++;
			choiceShaper.deleteMeshes(meshes);
			drawScreen(*activescreen);
		}
	}
	if (up.downs > 0) {
		if (choice > 0) {
			choice--;
			choiceShaper.deleteMeshes(meshes);
			drawScreen(*activescreen);
		}
	}

	if (space.downs > 0) {
		if (activescreen == &intro) {
			switchToScreen(screen1);
		}
		else if (activescreen == &screen1 && choice == 0) {
			switchToScreen(screen2);
		}
		else if (activescreen == &screen1 && choice == 1) {
			switchToScreen(screen7);
		}
		else if (activescreen == &screen2 && choice == 0) {
			switchToScreen(screen3);
		}
		else if (activescreen == &screen2 && choice == 1) {
			switchToScreen(screen5);
		}
		else if (activescreen == &screen3 && choice == 0) {
			if (failed_flag) switchToScreen(screen12);
			else switchToScreen(screen4);
		}
		else if (activescreen == &screen3 && choice == 1) {
			switchToScreen(screen5);
		}
		else if (activescreen == &screen4 && choice == 0) {
			switchToScreen(intro);
		}
		else if (activescreen == &screen5 && choice == 0) {
			if (failed_flag) switchToScreen(screen13);
			else switchToScreen(screen6);
		}
		else if (activescreen == &screen5 && choice == 1) {
			if (failed_flag) switchToScreen(screen12);
			else switchToScreen(screen4);
		}
		else if (activescreen == &screen6 && choice == 0) {
			switchToScreen(intro);
		}
		else if (activescreen == &screen7 && choice == 0) {
			switchToScreen(screen8);
		}
		else if (activescreen == &screen7 && choice == 1) {
			switchToScreen(screen10);
		}
		else if (activescreen == &screen8 && choice == 0) {
			switchToScreen(screen9);
		}
		else if (activescreen == &screen8 && choice == 1) {
			switchToScreen(screen2);
		}
		else if (activescreen == &screen9 && choice == 0) {
			switchToScreen(intro);
		}
		else if (activescreen == &screen10 && choice == 0) {
			switchToScreen(screen11);
		}
		else if (activescreen == &screen10 && choice == 1) {
			switchToScreen(screen2);
			failed_flag = true;
		}
		else if (activescreen == &screen11 && choice == 0) {
			switchToScreen(intro);
			failed_flag = false;
		}
		else if (activescreen == &screen12 && choice == 0) {
			switchToScreen(intro);
			failed_flag = false;
		}
		else if (activescreen == &screen13 && choice == 0) {
			switchToScreen(intro);
			failed_flag = false;
		}
	}

	//reset button press counters:
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 projection = glm::ortho(0.0f, float(drawable_size.x), 0.0f, float(drawable_size.y));
	glUseProgram(color_texture_program->program);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vertex_buffer_for_color_texture_program);
	glUniformMatrix4fv(color_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(projection));

	//draw background image

	std::vector< Vertex > background_vertices;
	background_vertices.emplace_back(Vertex(glm::vec3(0.0f, float(drawable_size.y), 0.0f), white_color, glm::vec2(0.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(float(drawable_size.x), float(drawable_size.y), 0.0f), white_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(0.0f, 0.0f, 0.0f), white_color, glm::vec2(0.0f, 0.0f)));

	background_vertices.emplace_back(Vertex(glm::vec3(0.0f, 0.0f, 0.0f), white_color, glm::vec2(0.0f, 0.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(float(drawable_size.x), float(drawable_size.y), 0.0f), white_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(float(drawable_size.x), 0.0f, 0.0f), white_color, glm::vec2(1.0f, 0.0f)));

	glBindTexture(GL_TEXTURE_2D, background_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(Vertex), background_vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//draw Jean MacWomann
	background_vertices.clear();
	float leftside = float(drawable_size.x) / 3 * 2;
	float rightside = leftside + 500.0f;
	float topside = 500.0f;
	background_vertices.emplace_back(Vertex(glm::vec3(leftside, topside, 0.0f), white_color, glm::vec2(0.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, topside, 0.0f), white_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(leftside, 0.0f, 0.0f), white_color, glm::vec2(0.0f, 0.0f)));

	background_vertices.emplace_back(Vertex(glm::vec3(leftside, 0.0f, 0.0f), white_color, glm::vec2(0.0f, 0.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, topside, 0.0f), white_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, 0.0f, 0.0f), white_color, glm::vec2(1.0f, 0.0f)));

	if (activescreen == &screen11 || activescreen == &screen12 || activescreen == &screen13)
		glBindTexture(GL_TEXTURE_2D, angry_tex);
	else 
		glBindTexture(GL_TEXTURE_2D, jean_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(Vertex), background_vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//Draw prompt text box
	background_vertices.clear();
	leftside = activescreen->promptX - box_padding;
	rightside = promptbox_botright.x + box_padding;
	float botside = promptbox_botright.y - box_padding;
	topside = activescreen->promptY + box_padding;
	background_vertices.emplace_back(Vertex(glm::vec3(leftside, topside, 0.0f), base_color, glm::vec2(0.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, topside, 0.0f), base_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(leftside, botside, 0.0f), base_color, glm::vec2(0.0f, 0.0f)));

	background_vertices.emplace_back(Vertex(glm::vec3(leftside, botside, 0.0f), base_color, glm::vec2(0.0f, 0.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, topside, 0.0f), base_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, botside, 0.0f), base_color, glm::vec2(1.0f, 0.0f)));

	glBindTexture(GL_TEXTURE_2D, white_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(Vertex), background_vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	//Draw option text box
	background_vertices.clear();
	leftside = activescreen->optionsX - box_padding;
	rightside = optionbox_botright.x + box_padding;
	botside = optionbox_botright.y - box_padding;
	topside = activescreen->optionsY + box_padding;
	background_vertices.emplace_back(Vertex(glm::vec3(leftside, topside, 0.0f), base_color, glm::vec2(0.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, topside, 0.0f), base_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(leftside, botside, 0.0f), base_color, glm::vec2(0.0f, 0.0f)));

	background_vertices.emplace_back(Vertex(glm::vec3(leftside, botside, 0.0f), base_color, glm::vec2(0.0f, 0.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, topside, 0.0f), base_color, glm::vec2(1.0f, 1.0f)));
	background_vertices.emplace_back(Vertex(glm::vec3(rightside, botside, 0.0f), base_color, glm::vec2(1.0f, 0.0f)));

	glBindTexture(GL_TEXTURE_2D, white_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(Vertex), background_vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	int meshidx = 0;
	for (auto mesh : meshes) {
		glBindTexture(GL_TEXTURE_2D, mesh->textureId);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(Vertex), mesh->vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
	
	GL_ERRORS();
}
