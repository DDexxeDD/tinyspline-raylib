#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyspline.h"
#include "raylib.h"
#include "raylib-nuklear.h"
#include "cvector.h"

#include "demo_interpolation.h"
#include "common.h"

#define INTERPOLATION_POINT_COUNT 7
#define INTERPOLATION_DIMENSION 2
#define INTERPOLATION_SAMPLES 100

enum
{
	TYPE_CUBIC_NATURAL,
	TYPE_CATMULL_ROM
};

static tsBSpline spline_cubic;
static tsBSpline spline_catmull;

static cvector (tsReal) points;
static cvector (tsReal) points_draw;

static tsReal demo_alpha;
static int selected;
static nk_bool draw_cubic;
static nk_bool draw_catmull;
static int drag_index = 0;

void reset ()
{
	cvector_clear (points);

	cvector_push_back (points, 50); cvector_push_back (points, 200);
	cvector_push_back (points, 200); cvector_push_back (points, 100);
	cvector_push_back (points, 400); cvector_push_back (points, 120);
	cvector_push_back (points, 450); cvector_push_back (points, 300);
	cvector_push_back (points, 210); cvector_push_back (points, 250);
	cvector_push_back (points, 300); cvector_push_back (points, 400);
	cvector_push_back (points, 170); cvector_push_back (points, 430);

	demo_alpha = 0.5;
}

void interpolate_splines (cvector (tsReal) points, int dimension, tsReal alpha, tsReal epsilon)
{
	if (dimension == 0)
	{
		printf ("interpolate_splines: dimension 0 unsupported\n");
		return;
	}
	if (cvector_size (points) % dimension != 0)
	{
		printf ("interpolate_splines: number of points not a multiple of dimension\n");
		return;
	}

	tsStatus status;

	ts_bspline_free (&spline_cubic);
	spline_cubic = ts_bspline_init ();
	ts_bspline_interpolate_cubic_natural (points, cvector_size (points) / dimension, dimension, &spline_cubic, &status);

	ts_bspline_free (&spline_catmull);
	spline_catmull = ts_bspline_init ();
	ts_bspline_interpolate_catmull_rom (points, cvector_size (points) / dimension, dimension, alpha, NULL, NULL, epsilon, &spline_catmull, &status);

}

void demo_interpolation_initialize ()
{
	points = NULL;
	cvector_init (points, INTERPOLATION_POINT_COUNT * INTERPOLATION_DIMENSION, NULL);
	points_draw = NULL;
	cvector_init (points_draw, INTERPOLATION_POINT_COUNT * INTERPOLATION_DIMENSION, NULL);

	reset ();

	draw_cubic = true;
	draw_catmull = true;
	selected = -1;

	spline_cubic = ts_bspline_init ();
	spline_catmull = ts_bspline_init ();

	// TODO/FIXME
	// 	put these cvector ops in interpolate_splines
	// 		when they are there we get a segfault though
	// 		? why ?
	cvector_clear (points_draw);
	cvector_copy (points, points_draw);
	interpolate_splines (points_draw, INTERPOLATION_DIMENSION, demo_alpha, 0.1f);
}

void demo_interpolation_run (struct nk_context* context)
{
	if (IsMouseButtonReleased (MOUSE_BUTTON_LEFT))
	{
		drag_index = -1;
	}

	if (IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
	{
		Vector2 mouse_position = GetMousePosition ();
		// the spline and points are rendered offset from their actual location
		// 	so we need to test the mouse at the offset location
		mouse_position.y -= TAB_OFFSET;

		for (int iter = 0; iter < cvector_size (points) / INTERPOLATION_DIMENSION; iter++)
		{
			if ((mouse_position.x > points[iter * 2] - 4
					&& mouse_position.x < points[iter * 2] + 4)
				&& (mouse_position.y > points[(iter * 2) + 1] - 4
					&& mouse_position.y < points[(iter * 2) + 1] + 4))
			{
				drag_index = iter * 2;
				break;
			}
		}
	}

	if (drag_index >= 0 && IsMouseButtonDown (MOUSE_BUTTON_LEFT))
	{
		Vector2 delta = GetMouseDelta ();

		if (!(delta.x == 0.0f && delta.y == 0.0f))
		{
			tsReal new_x = points[drag_index] + delta.x;
			tsReal new_y = points[drag_index + 1] + delta.y;

			if (new_x < 0.0)
			{
				new_x = 0.0;
			}
			if (new_x > 525)
			{
				new_x = 525;
			}

			if (new_y < 0.0)
			{
				new_y = 0.0;
			}
			if (new_y > 525)
			{
				new_y = 525;
			}

			points[drag_index] = new_x;
			points[drag_index + 1] = new_y;

			cvector_clear (points_draw);
			cvector_copy (points, points_draw);
			interpolate_splines (points_draw, INTERPOLATION_DIMENSION, demo_alpha, TS_POINT_EPSILON);
		}
	}

	if (nk_begin (context, "interpolation controls", nk_rect (525, TAB_OFFSET, 340, 525), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic (context, 20, 1);

		nk_checkbox_label (context, "Cubic Natural", &draw_cubic);
		nk_break (context);
		nk_checkbox_label (context, "Catmull-Rom", &draw_catmull);
		nk_break (context);

		if (nk_slider_float (context, 0.0f, &demo_alpha, 1.0f, 0.01f))
		{
			cvector_clear (points_draw);
			cvector_copy (points, points_draw);
			interpolate_splines (points_draw, INTERPOLATION_DIMENSION, demo_alpha, TS_POINT_EPSILON);
		}
		nk_labelf (context, NK_TEXT_CENTERED, "alpha: %.2f", demo_alpha);
		nk_break (context);
		nk_break (context);

		if (nk_button_label (context, "Reset"))
		{
			reset ();

			cvector_clear (points_draw);
			cvector_copy (points, points_draw);
			interpolate_splines (points_draw, INTERPOLATION_DIMENSION, demo_alpha, TS_POINT_EPSILON);
		}
	}
	nk_end (context);
}

void draw_interpolated_spline (int type)
{
	tsBSpline* spline_draw;
	Color color;

	if (type == TYPE_CUBIC_NATURAL)
	{
		spline_draw = &spline_cubic;
		color = GREEN;
	}
	else if (type == TYPE_CATMULL_ROM)
	{
		spline_draw = &spline_catmull;
		color = BLUE;
	}
	else
	{
		return;
	}

	tsReal* sample_points; 
	size_t sample_count;
	tsStatus status;

	ts_bspline_sample (spline_draw, INTERPOLATION_SAMPLES, &sample_points, &sample_count, &status);

	for (int iter = 0; iter < (sample_count - 1); iter ++)
	{
		DrawLineEx ((Vector2) {sample_points[iter * INTERPOLATION_DIMENSION], sample_points[(iter * INTERPOLATION_DIMENSION) + 1]}, (Vector2) {sample_points[(iter * INTERPOLATION_DIMENSION) + 2], sample_points[(iter * INTERPOLATION_DIMENSION) + 3]}, 1.0f, color);
	}

	free (sample_points);
}

void demo_interpolation_draw ()
{
	ClearBackground (WHITE);

	for (int iter = 0; iter < cvector_size (points) / INTERPOLATION_DIMENSION; iter++)
	{
		DrawRectangle (points[iter * INTERPOLATION_DIMENSION] - 3, points[(iter * INTERPOLATION_DIMENSION) + 1] - 3, 6, 6, RED);
	}

	if (draw_cubic)
	{
		draw_interpolated_spline (TYPE_CUBIC_NATURAL);
	}
	if (draw_catmull)
	{
		draw_interpolated_spline (TYPE_CATMULL_ROM);
	}
}

void demo_interpolation_cleanup ()
{
	ts_bspline_free (&spline_cubic);
	ts_bspline_free (&spline_catmull);

	cvector_free (points);
	cvector_free (points_draw);
}

