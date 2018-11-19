
/* game screens */

#include "game.h"
#include "resource.h"
#include "sys.h"

struct vars_t g_vars;

void update_input() {
	g_sys.process_events();

	g_vars.input_key_left  = (g_sys.input.direction & INPUT_DIRECTION_LEFT) != 0  ? 0xFF : 0;
	g_vars.input_key_right = (g_sys.input.direction & INPUT_DIRECTION_RIGHT) != 0 ? 0xFF : 0;
	g_vars.input_key_up    = (g_sys.input.direction & INPUT_DIRECTION_UP) != 0    ? 0xFF : 0;
	g_vars.input_key_down  = (g_sys.input.direction & INPUT_DIRECTION_DOWN) != 0  ? 0xFF : 0;
	g_vars.input_key_space = g_sys.input.space ? 0xFF : 0;

	g_vars.input_keystate[2] = g_sys.input.digit1;
	g_vars.input_keystate[3] = g_sys.input.digit2;
	g_vars.input_keystate[4] = g_sys.input.digit3;
}

static void do_splash_screen() {
	clear_palette();
	load_file("titus.eat");
	video_copy_vga(0x7D00);
	fade_in_palette();
	fade_out_palette();
	load_file("tiny.eat");
	video_copy_vga(0x7D00);
	fade_in_palette();
}

static void copy_screen_palette() {
}

static void scroll_screen_palette() {
	g_vars.level_time += 3;
	if (g_vars.level_time >= 90) {
		g_vars.level_time = 0;
	}
	const int count = 90 - g_vars.level_time;
	for (int i = 0; i < count; i += 3) {
		g_sys.set_palette_color(225 + i / 3, g_res.tmp + 225 * 3 + g_vars.level_time + i);
	}
	g_sys.update_screen(g_res.tmp + 768, 1);
}

static void do_select_screen_scroll_palette(int al, int ah, int step, int count) {
	uint8_t *palette_buffer = g_res.tmp;
	do {
		for (int i = al * 3; i < ah * 3; ++i) {
			int color = g_vars.palette_buffer[i];
			if ((step > 0 && color != palette_buffer[i]) || (step < 0 && color != 0)) {
				color += step;
			}
			g_vars.palette_buffer[i] = color;
		}
		g_sys.set_screen_palette(g_vars.palette_buffer + al * 3, al, ah - al + 1);
		g_sys.update_screen(g_res.tmp + 768, 1);
	} while (--count != 0);
}

static void do_select_screen_scroll_palette_pattern1() {
	do_select_screen_scroll_palette(0x10, 0x4F, -1, 0x19);
}

static void do_select_screen_scroll_palette_pattern2() {
	do_select_screen_scroll_palette(0x60, 0x9F, -1, 0x19);
}

static void do_select_screen_scroll_palette_pattern3() {
	do_select_screen_scroll_palette(0x10, 0x4F,  1, 0x19);
}

static void do_select_screen_scroll_palette_pattern4() {
	do_select_screen_scroll_palette(0x60, 0x9F,  1, 0x19);
}

static void do_select_screen() {
	load_file("select.eat");
	clear_palette();
	video_copy_vga(0x7D00);
	fade_in_palette();
	do_select_screen_scroll_palette_pattern2();
	int bl = 2;
	while (!g_sys.input.quit) {
		int bh = bl;
		video_vsync(60);
		if (g_sys.input.direction & INPUT_DIRECTION_RIGHT) {
			g_sys.input.direction &= ~INPUT_DIRECTION_RIGHT;
			++bl;
			bl &= 3;
			if (bl == 0) {
				bl = 1;
			}
			video_vsync(60);
		}
		if (g_sys.input.direction & INPUT_DIRECTION_LEFT) {
			g_sys.input.direction &= ~INPUT_DIRECTION_LEFT;
			--bl;
			bl &= 3;
			if (bl == 0) {
				bl = 3;
			}
			video_vsync(60);
		}
		bh ^= bl;
		if (bh & 1) {
			if (bl & 1) {
				do_select_screen_scroll_palette_pattern4();
			} else {
				do_select_screen_scroll_palette_pattern2();
			}
		}
		if (bh & 2) {
			if (bl & 2) {
				do_select_screen_scroll_palette_pattern3();
			} else {
				do_select_screen_scroll_palette_pattern1();
			}
		}
		if (g_sys.input.space) {
			g_sys.input.space = 0;
			g_vars.player = (bl & 3) - 1;
			fade_out_palette();
			break;
		}
		update_input();
		g_sys.sleep(30);
	}
}

void do_difficulty_screen() {
	char name[16];
	snprintf(name, sizeof(name), "dif%02d.eat", (g_vars.level >> 3) + 1);
	load_file(name);
	clear_palette();
	video_copy_vga(0x7D00);
	fade_in_palette();
	// ...
	fade_out_palette();
}

void do_level_number_screen() {
	clear_palette();
	load_file("fond.eat");
	video_draw_string("LEVEL NUMBER", 0x5E0C, 11);
	char buf[8];
	snprintf(buf, sizeof(buf), "%02d", g_vars.level);
	video_draw_string(buf, 0x5E9B, 11);
	fade_in_palette();
	fade_out_palette();
}

static void display_password_image() {
	clear_palette();
	load_file("password.eat");
	video_copy_vga(0x7D00);
	copy_screen_palette();
}

static uint16_t get_password_seed(uint16_t ax) {
	ax ^= 0xAA31;
	// ax *=
	// rol ax
	return ax;
}

void do_level_password_screen() {
	display_password_image();
	uint8_t dx = g_vars.level - 1;
	uint16_t ax = g_vars.player * 50 + dx;
	char str[5];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) { // reverse 4 high bits of ax in dx
			dx = (dx << 1) | ((ax >> (15 - j)) & 1);
		}
		dx &= 15;
		dx += '0';
		if (dx > '9') {
			dx += 7;
		}
		str[i] = dx;
	}
	str[4] = 0;
	// ...
	video_draw_string("STAGE NUMBER", 0x7E96, 11);
	video_draw_string(str, 0xABB4, 20);
	fade_in_palette();
	scroll_screen_palette();
	// ...
	ax = get_password_seed(dx);
	// ...
	fade_out_palette();
}

static void do_password_screen() {
	display_password_image();
	video_draw_string("ENTER PASSWORD", 0x7E96, 11);
	fade_in_palette();
	char str[5] = "0000";
	video_draw_string(str, 0xABB4, 20);
}

static int do_menu_screen() {
	load_file("menu.eat");
	clear_palette();
	video_copy_vga(0x7D00);
	fade_in_palette();
	copy_screen_palette();
	memset(g_vars.input_keystate, 0, sizeof(g_vars.input_keystate));
	g_vars.level_time = 0;
	while (!g_sys.input.quit) {
		scroll_screen_palette();
		if (g_vars.input_keystate[2] || g_vars.input_keystate[0x4F] || g_sys.input.space) {
			g_sys.input.space = 0;
			fade_out_palette();
			return 1;
		}
		if (g_vars.input_keystate[3] || g_vars.input_keystate[0x50]) {
			fade_out_palette();
			return 2;
		}
		if (g_vars.input_keystate[4] || g_vars.input_keystate[0x51]) {
			return 3;
		}
		update_input();
		g_sys.sleep(30);
	}
	return 0;
}

static int do_options_screen() {
	fade_out_palette();
	load_file("fond.eat");
	video_copy_vga(0x7D00);
	copy_screen_palette();
	video_draw_string("GAME SPEED", 0x3EE9, 11);
	video_draw_string("1 FAST", 0x647E, 11);
	video_draw_string("2 NORMAL", 0x89FE, 11);
	fade_in_palette();
	memset(g_vars.input_keystate, 0, sizeof(g_vars.input_keystate));
	while (!g_sys.input.quit) {
		scroll_screen_palette();
		if (g_vars.input_keystate[2] || g_vars.input_keystate[0x4F]) {
			// _options |= OPT_GAME_SPEED_FAST;
			fade_out_palette();
			return 1;
		}
		if (g_vars.input_keystate[3] || g_vars.input_keystate[0x50]) {
			// _options &= ~OPT_GAME_SPEED_FAST;
			fade_out_palette();
			return 2;
		}
		update_input();
		g_sys.sleep(30);
	}
	return 0;
}

void do_game_over_screen() {
	load_file("fond.eat");
	clear_palette();
	video_copy_vga(0x7D00);
	video_draw_string("GAME OVER", 0x5E2E, 11);
	fade_in_palette();
	// wait_for_key_action();
	fade_out_palette();
}

void do_game_win_screen() {
	clear_palette();
	load_file("win.eat");
	video_copy_vga(0x7D00);
	fade_in_palette();
	fade_out_palette();
	clear_palette();
	load_file("end.eat");
	video_copy_vga(0x7D00);
	static const struct {
		uint16_t offset;
		const char *str;
	} text[] = {
		{ 0x0F68, "CONGRATULATION" },
		{ 0x3B34, "YOU HAVE BEATEN" },
		{ 0x5CE8, "THE EVIL JUKEBOXE" },
		{ 0x7EB1, "NOW YOU ARE FREE" },
		{ 0xA072, "AND YOUR CONCERT" },
		{ 0xC22D, "WILL BE A SUCCESS" }
	};
	for (int j = 0; j < 5; ++j) {
		for (int i = 0; i < 6; ++i) {
			video_draw_string(text[i].str, text[i].offset, 11);
		}
		fade_in_palette();
		// wait_for_key_action();
		fade_out_palette();
		video_copy_vga(0x7D00);
	}
}

void game_main() {
	sound_init();
	play_music(0);
	do_splash_screen();
	fade_out_palette();
	g_sys.set_screen_palette(common_palette_data, 0, 128);
	video_load_sprites();
	render_set_sprites_clipping_rect(0, 0, TILEMAP_SCREEN_W, TILEMAP_SCREEN_H);
	while (!g_sys.input.quit) {
		update_input();
		g_vars.level = g_vars.start_level;
		if (g_vars.level == 0) {
			g_vars.level = 1;
		}
		const int ret = do_menu_screen();
		g_vars.players_table[0].lifes_count = 3;
		g_vars.players_table[1].lifes_count = 3;
		if (ret == 1) {
			do_select_screen();
			do_difficulty_screen();
		} else if (ret == 2) {
			g_vars.level = -1;
			do_password_screen();
			if (g_vars.level < 0) {
				continue;
			}
			++g_vars.level;
			do_difficulty_screen();
		} else {
			do_options_screen();
			continue;
		}
		do_level();
	}
}
