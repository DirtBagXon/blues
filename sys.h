
#ifndef SYS_H__
#define SYS_H__

#include "intern.h"

#define INPUT_DIRECTION_LEFT  (1 << 0)
#define INPUT_DIRECTION_RIGHT (1 << 1)
#define INPUT_DIRECTION_UP    (1 << 2)
#define INPUT_DIRECTION_DOWN  (1 << 3)

#define SYS_AUDIO_FREQ 22050

struct input_t {
	uint8_t direction;
	bool quit;
	bool escape;
	bool space;
	bool digit1, digit2, digit3;
};

typedef void (*sys_audio_cb)(void *, uint8_t *data, int len);

struct sys_rect_t {
	int x, y;
	int w, h;
};

struct sys_t {
	struct input_t	input;
	int	(*init)();
	void	(*fini)();
	void	(*set_screen_size)(int w, int h, const char *caption, int scale, const char *filter, bool fullscreen);
	void	(*set_screen_palette)(const uint8_t *colors, int offset, int count);
	void	(*set_palette_color)(int i, const uint8_t *colors);
	void	(*fade_in_palette)();
	void	(*fade_out_palette)();
	void	(*update_screen)(const uint8_t *p, int present);
	void	(*process_events)();
	void	(*sleep)(int duration);
	uint32_t	(*get_timestamp)();
	void	(*start_audio)(sys_audio_cb callback, void *param);
	void	(*stop_audio)();
	void	(*lock_audio)();
	void	(*unlock_audio)();
};

extern struct sys_t g_sys;

#define RENDER_SPR_GAME  0 /* player sprites */
#define RENDER_SPR_LEVEL 1 /* level sprites */

extern void	render_load_sprites(int spr_type, int count, const struct sys_rect_t *r, const uint8_t *data, int w, int h, int palette_offset);
extern void	render_unload_sprites(int spr_type);
extern void	render_add_sprite(int spr_type, int frame, int x, int y, int xflip);
extern void	render_clear_sprites();
extern void	render_set_sprites_clipping_rect(int x, int y, int w, int h);

#endif /* SYS_H__ */
