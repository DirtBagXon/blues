
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
	"  --screensize=WxH  Graphics screen size (default 320x200)\n"
;

int main(int argc, char *argv[]) {
	const char *data_path = DEFAULT_DATA_PATH;
	g_vars.start_xpos16 = -1;
	g_vars.start_ypos16 = -1;
	g_vars.screen_w = 320;
	g_vars.screen_h = 200;
	if (argc == 2) {
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISDIR(st.st_mode)) {
			data_path = strdup(argv[1]);
		}
	}
	g_debug_mask = DBG_GAME | DBG_MIXER | DBG_RESOURCE | DBG_UNPACK;
	while (1) {
		static struct option options[] = {
			{ "datapath",   required_argument, 0, 1 },
			{ "level",      required_argument, 0, 2 },
			{ "debug",      required_argument, 0, 3 },
			{ "cheats",     required_argument, 0, 4 },
			{ "startpos",   required_argument, 0, 5 },
			{ "screensize", required_argument, 0, 6 },
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
	g_sys.set_screen_size(GAME_SCREEN_W, GAME_SCREEN_H, CAPTION, DEFAULT_SCALE_FACTOR, DEFAULT_SCALE_FILTER, false);
	game_main();
	g_sys.fini();
	res_fini();
	return 0;
}
