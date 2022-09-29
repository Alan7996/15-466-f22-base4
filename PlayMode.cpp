#include "PlayMode.hpp"

#include "ColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <thread>
#include <random>
#include <iostream>
#include <fstream>

PlayMode::PlayMode() {
	
	// bit of help from David Lyons on where to get started with this
	// based on https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
	FT_Init_FreeType(&ft_library);
	FT_New_Face(ft_library, data_path("NotoSansMono-Bold.ttf").c_str(), 0, &ft_face);
	FT_Set_Char_Size(ft_face, 36 * 64, 36 * 64, 0, 0);

	hb_font = hb_ft_font_create(ft_face, NULL);
	hb_buffer = hb_buffer_create();
	
	game_state = WAITING;
	msg_stack.insert(msg_stack.begin(), {0, "Welcome!", 0, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "login: ", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "login: b", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "login: bo", 300, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "login: bos", 300, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "login: boss", 300, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "password: ", 500, choice_color});
	msg_stack.insert(msg_stack.begin(), {2, "", 500, default_color}); // err what?
	msg_stack.insert(msg_stack.begin(), {0, "Incorrect! Try again: ", 300, incorrect_color});
	msg_stack.insert(msg_stack.begin(), {0, "password: ", 3000, choice_color});
	msg_stack.insert(msg_stack.begin(), {2, "", 500, default_color}); // realboss
	msg_stack.insert(msg_stack.begin(), {0, "Incorrect! Try again: ", 500, incorrect_color});
	msg_stack.insert(msg_stack.begin(), {0, "Hint: look at where you launched me!", 2000, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "Make sure to delete it before proceeding! ", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {3, "password: ", 1000, choice_color});
	msg_stack.insert(msg_stack.begin(), {2, "", 500, default_color}); // xxxrealbossxxx
	msg_stack.insert(msg_stack.begin(), {0, "", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "Welcome back, boss!", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "How can I help you today?", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {2, "", 500, default_color}); // be my gf

	msg_stack.insert(msg_stack.begin(), {0, "Oh, silly! Maybe in 2^32 seconds!", 1000, default_color}); // select be my gf
	msg_stack.insert(msg_stack.begin(), {0, "How can I help you today?", 3000, default_color});
	msg_stack.insert(msg_stack.begin(), {2, "", 500, default_color}); // minesweeper

	msg_stack.insert(msg_stack.begin(), {0, "Wait", 1000, default_color}); // select nuclear
	msg_stack.insert(msg_stack.begin(), {1, "Wait.", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "Wait..", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "Wait...", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {1, "Wait....", 1000, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "You aren't my boss", 3000, incorrect_color});
	msg_stack.insert(msg_stack.begin(), {0, "", 3000, default_color});
	msg_stack.insert(msg_stack.begin(), {4, "", 3000, default_color});

	msg_stack.insert(msg_stack.begin(), {0, "minesweeper it is~", 1000, default_color}); // select minesweeper
	msg_stack.insert(msg_stack.begin(), {0, "Your code is THEENDOFTHEWORLD", 3000, default_color});
	msg_stack.insert(msg_stack.begin(), {0, "You Win!", 3000, win_color});


	choice_stack.insert(choice_stack.begin(), {{"Errr,what?", 0}, {"password", 0}, {"12345678!", 0}});
	choice_stack.insert(choice_stack.begin(), {{"realboss", 0}, {"asdqwe123", 0}, {"ThisGameSucks", 0}});
	choice_stack.insert(choice_stack.begin(), {{"xxxrealbossxxx", 0}});
	choice_stack.insert(choice_stack.begin(), {{"be my gf", 0}, {"minesweeper", 11}, {"nuclear code", 3}});
	choice_stack.insert(choice_stack.begin(), {{"minesweeper", 8}, {"nuclear code", 1}});


	task_stack.insert(task_stack.begin(), 0);
	task_stack.insert(task_stack.begin(), 100);
}

PlayMode::~PlayMode() {
}

void PlayMode::add_message(std::string msg, int wait, glm::vec3 color) {

	std::chrono::milliseconds timespan(wait);
	std::this_thread::sleep_for(timespan);

	if (messages.size() == MAX_MSG_SIZE) {
		messages.pop_back();
		colors.pop_back();
	}
	messages.insert(messages.begin(), msg);
	colors.insert(colors.begin(), color);
}

void PlayMode::update_message(std::string msg, int wait) {

	std::chrono::milliseconds timespan(wait);
	std::this_thread::sleep_for(timespan);

	messages.at(0) = msg;
}

void create_key() {

	std::ofstream keyFile(data_path("key.txt"));

	keyFile << "Password : xxxactualbossxxx" << std::endl;

	keyFile.close();
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (game_state == PLAYING && evt.key.keysym.sym == SDLK_LEFT) {
			hovering_text = hovering_text == 0 ? 0 : hovering_text - 1;
			return true;
		} else if (game_state == PLAYING && evt.key.keysym.sym == SDLK_RIGHT) {
			hovering_text = hovering_text == static_cast<uint8_t>(user_choices.size()) - 1? static_cast<uint8_t>(user_choices.size()) - 1: hovering_text + 1;
			return true;
		} else if (game_state == PLAYING && evt.key.keysym.sym == SDLK_RETURN) {
			// continue with player selected option
			// remove the next n choices depending on the option chosen
			add_message(std::get<0>(user_choices[hovering_text]), 0, choice_color);
			for (int i = 0; i < std::get<1>(user_choices[hovering_text]); i++) msg_stack.pop_back();
			user_choices = {};
			hovering_text = 0;
			game_state = WAITING;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	if (task_index != -1) {
		if (task_index == 0) {
			std::ifstream f(data_path("key.txt").c_str());
			if (!f.good()) {
				task_index = -1;
			}
		} else if (task_index == 100) {
			exit(0);
		}
		return;
	}

	if (game_state == WAITING && msg_stack.size() > 0) {
		std::tuple<int, std::string, int, glm::vec3> new_action = msg_stack.back();
		msg_stack.pop_back();
		if (std::get<0>(new_action) == 0) { // print new message
			add_message(std::get<1>(new_action), std::get<2>(new_action), std::get<3>(new_action));
		} else if (std::get<0>(new_action) == 1) { // update last message
			update_message(std::get<1>(new_action), std::get<2>(new_action));
		} else if (std::get<0>(new_action) == 2){ // provide a choice and wait for input
			user_choices = choice_stack.back();
			choice_stack.pop_back();
			game_state = PLAYING;
		} else if (std::get<0>(new_action) == 3){ // perform a special check / action
			add_message(std::get<1>(new_action), std::get<2>(new_action), std::get<3>(new_action));

			task_index = task_stack.back();
			task_stack.pop_back();

			if (task_index == 0) {
				create_key();
			}
		} else if (std::get<0>(new_action) == 4){ // gameover
			task_index = 100;
		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{ //render texts
		glDisable(GL_DEPTH_TEST);
		
		for (int i = 0; i < static_cast<int>(messages.size()); i++) {
			render_text(messages[i], glm::vec2(drawable_size.x / 10.0f, drawable_size.y * static_cast<float>(i + 2) / 10.0f), drawable_size, colors[i]);
		}

		render_text(">", glm::vec2(drawable_size.x / 20.0f, drawable_size.y / 10.0f), drawable_size, default_color);

		for (int i = 0; i < static_cast<int>(user_choices.size()); i++) {
			render_text(std::get<0>(user_choices[i]), glm::vec2(drawable_size.x * static_cast<float>(2.0f * i + 1) / 10.0f, drawable_size.y / 10.0f), drawable_size, colors[0]);
		}
		if (game_state == PLAYING) render_text("____", glm::vec2(drawable_size.x * static_cast<float>(2.0f * hovering_text + 1) / 10.0f, drawable_size.y / 20.0f), drawable_size, default_color);
	}
	GL_ERRORS();
}

void PlayMode::render_text(std::string text, glm::vec2 pos_xy, glm::uvec2 const& drawable_size, glm::vec3 color) {

	// this function is heavily inspired by David Lyon's work
	glm::vec2 pos = pos_xy;

	// based on https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
	hb_buffer_clear_contents(hb_buffer);
	hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
	hb_buffer_guess_segment_properties(hb_buffer);

	hb_shape(hb_font, hb_buffer, NULL, 0);

	uint32_t len = hb_buffer_get_length(hb_buffer);
	hb_glyph_info_t* glyph_infos = hb_buffer_get_glyph_infos(hb_buffer, NULL);
	hb_glyph_position_t* glyph_positions = hb_buffer_get_glyph_positions(hb_buffer, NULL);

	// based on https://learnopengl.com/In-Practice/Text-Rendering (including the for loop below)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(color_texture_program->program);
	glUniform3f(color_texture_program->textColor, color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	
	// based on https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text_rendering.cpp
	glm::mat4 projection = glm::ortho(0.0f, (float)drawable_size.x, 0.0f, (float)drawable_size.y);
	glUniformMatrix4fv(color_texture_program->projection, 1, GL_FALSE, &projection[0][0]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (uint32_t i = 0; i < len; i++) {
		// based on https://freetype.org/freetype2/docs/tutorial/step1.html
		hb_codepoint_t glyph_index = glyph_infos[i].codepoint;
		FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT);
		FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                ft_face->glyph->bitmap.width,
                ft_face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                ft_face->glyph->bitmap.buffer
            );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        float w = static_cast<float>(ft_face->glyph->bitmap.width);
        float h = static_cast<float>(ft_face->glyph->bitmap.rows);

		float xpos = pos.x + ft_face->glyph->bitmap_left;
        float ypos = pos.y + ft_face->glyph->bitmap_top - h;

        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
		// render glyph texture over quad
        // glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        pos += glm::vec2(glyph_positions[i].x_advance >> 6, glyph_positions[i].y_advance >> 6); // bitshift by 6 to get value in pixels in both x and y (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}