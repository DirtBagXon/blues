
#include <getopt.h>
#include <sys/stat.h>

#include "game.h"
#include "resource.h"
#include "sys.h"
#include "util.h"

static const char *CAPTION = "Blues Brothers : Jukebox Adventure";

static const char *DEFAULT_DATA_PATH = ".";

static const int DEFAULT_SCALE_FACTOR = 2;

static const char *DEFAULT_SCALE_FILTER = "nearest";

static const char *USAGE =
	"Usage: %s [OPTIONS]...\n"
	"  --datapath=PATH   Path to data files (default '.')\n"
	"  --level=NUM       Start at level NUM\n"
	"  --cheats=MASK     Cheats mask\n"
	"  --startpos=XxY    Start at position (X,Y)\n"
	"  --fullscreen      Enable fullscreen\n"
	"  --scale=N         Graphics scaling factor (default 2)\n"
	"  --filter=NAME     Graphics scaling filter (default 'nearest')\n"
	"  --screensize=WxH  Graphics screen size (default 320x200)\n"
;

int main(int argc, char *argv[]) {
	g_vars.start_xpos16 = -1;
	g_vars.start_ypos16 = -1;
	g_vars.screen_w = 320;
	g_vars.screen_h = 200;
	const char *data_path = DEFAULT_DATA_PATH;
	int scale_factor = DEFAULT_SCALE_FACTOR;
	const char *scale_filter = DEFAULT_SCALE_FILTER;
	bool fullscreen = false;
	if (argc == 2) {
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISDIR(st.st_mode)) {
			data_path = strdup(argv[1]);
		}
	}
	g_debug_mask = 0; // DBG_GAME | DBG_MIXER | DBG_RESOURCE | DBG_UNPACK;
	while (1) {
		static struct option options[] = {
			{ "datapath",   required_argument, 0, 1 },
			{ "level",      required_argument, 0, 2 },
			{ "debug",      required_argument, 0, 3 },
			{ "cheats",     required_argument, 0, 4 },
			{ "startpos",   required_argument, 0, 5 },
			{ "fullscreen", no_argument,       0, 6 },
			{ "scale",      required_argument, 0, 7 },
			{ "filter",     required_argument, 0, 8 },
			{ "screensize", required_argument, 0, 9 },
			{ 0, 0, 0, 0 },
		};
		int index;
		const int c = getopt_long(argc, argv, "", options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 1:
			data_path = strdup(optarg);
			break;
		case 2:
			g_vars.start_level = atoi(optarg);
			break;
		case 3:
			g_debug_mask = atoi(optarg);
			break;
		case 4:
			g_vars.cheats = atoi(optarg);
			break;
		case 5:
			sscanf(optarg, "%dx%d", &g_vars.start_xpos16, &g_vars.start_ypos16);
			break;
		case 6:
			fullscreen = true;
			break;
		case 7:
			scale_factor = atoi(optarg);
			break;
		case 8:
			scale_filter = strdup(optarg);
			break;
		case 9:
			if (sscanf(optarg, "%dx%d", &g_vars.screen_w, &g_vars.screen_h) == 2) {
				// align to tile 16x16
				g_vars.screen_w =  (g_vars.screen_w + 15) & ~15;
				g_vars.screen_h = ((g_vars.screen_h + 15) & ~15) + PANEL_H;
			}
			break;
		default:
			fprintf(stdout, USAGE, argv[0]);
			return -1;
		}
	}
	res_init(data_path, GAME_SCREEN_W * GAME_SCREEN_H);
	g_sys.init();
	g_sys.set_screen_size(GAME_SCREEN_W, GAME_SCREEN_H, CAPTION, scale_factor, scale_filter, fullscreen);
	game_main();
	g_sys.fini();
	res_fini();
	if (data_path != DEFAULT_DATA_PATH) {
		free((char *)data_path);
	}
	if (scale_filter != DEFAULT_SCALE_FILTER) {
		free((char *)scale_filter);
	}
	return 0;
}
