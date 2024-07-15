#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "raylib-nuklear.h"
#include "tinyspline.h"

#include "demo_eval.h"
#include "demo_samples.h"
#include "demo_interpolation.h"
#include "demo_frames.h"
#include "common.h"

int screen_width = 865;
int screen_height = DRAW_WINDOW_HEIGHT + TAB_OFFSET;

typedef void (*demo_function) ();
typedef void (*demo_run) (struct nk_context*);
typedef struct
{
	demo_function initialize;
	demo_run run;
	demo_function draw;
	demo_function cleanup;
} demo_t;

enum
{
	DEMO_EVAL,
	DEMO_SAMPLES,
	DEMO_INTERPOLATION,
	DEMO_FRAMES,
	DEMO_COUNT
};

void demo_initialize (demo_t* demo, demo_function initialize, demo_run run, demo_function draw, demo_function cleanup)
{
	demo->initialize = initialize;
	demo->run = run;
	demo->draw = draw;
	demo->cleanup = cleanup;

	demo->initialize ();
}

demo_t demos[DEMO_COUNT];

int main ()
{
	SetTraceLogLevel (LOG_WARNING);
	SetConfigFlags (FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow (screen_width, screen_height, "tinyspline");
	SetTargetFPS (60);

	Font font = LoadFontEx ("resources/font/cousine/Cousine-Regular.ttf", 16, NULL, 0);

	struct nk_context* context = InitNuklearEx (font, 16);

	demo_initialize (&demos[DEMO_EVAL], demo_eval_initialize, demo_eval_run, demo_eval_draw, demo_eval_cleanup);
	demo_initialize (&demos[DEMO_SAMPLES], demo_samples_initialize, demo_samples_run, demo_samples_draw, demo_samples_cleanup);
	demo_initialize (&demos[DEMO_INTERPOLATION], demo_interpolation_initialize, demo_interpolation_run, demo_interpolation_draw, demo_interpolation_cleanup);
	demo_initialize (&demos[DEMO_FRAMES], demo_frames_initialize, demo_frames_run, demo_frames_draw, demo_frames_cleanup);

	int current_demo = DEMO_FRAMES;

	// texture for demos to render all the spline stuff to
	// 	so we dont have to worry about the screen position of what they're drawing
	// 	(demo ui elements are _not_ rendered to this texture)
	RenderTexture2D render_texture = LoadRenderTexture (DRAW_WINDOW_WIDTH, DRAW_WINDOW_HEIGHT);
	BeginTextureMode (render_texture);
	ClearBackground (BLACK);
	EndTextureMode ();

	while (!WindowShouldClose ())
	{
		if (nk_begin (context, "demo tabs", nk_rect (0, 0, screen_width, TAB_OFFSET), NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic (context, 30, 4);
			if (nk_button_label (context, "Eval"))
			{
				current_demo = DEMO_EVAL;
			}
			if (nk_button_label (context, "Samples"))
			{
				current_demo = DEMO_SAMPLES;
			}
			if (nk_button_label (context, "Interpolation"))
			{
				current_demo = DEMO_INTERPOLATION;
			}
			if (nk_button_label (context, "Frames"))
			{
				current_demo = DEMO_FRAMES;
			}
		}
		nk_end (context);

		demos[current_demo].run (context);

		UpdateNuklear (context);

		BeginTextureMode (render_texture);
		demos[current_demo].draw ();
		EndTextureMode ();

		BeginDrawing ();
		{
			ClearBackground (BLACK);

			// render texture must be y-flipped due to default opengl coordinates (left-bottom)
			DrawTextureRec (render_texture.texture, (Rectangle) {0, 0, (float) render_texture.texture.width, (float) -render_texture.texture.height}, (Vector2) {0, TAB_OFFSET}, WHITE);

			DrawNuklear (context);
		}
		EndDrawing ();
	}

	for (int iter = 0; iter < 2; iter++)
	{
		demos[iter].cleanup ();
	}

	UnloadRenderTexture (render_texture);
	UnloadFont (font);
	UnloadNuklear (context);
}
