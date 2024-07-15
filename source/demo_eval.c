#include <stdlib.h>
#include <stdbool.h>

#include "tinyspline.h"
#include "raylib.h"
#include "raylib-nuklear.h"

#include "demo_eval.h"
#include "common.h"

#define EVAL_POINT_COUNT 7
#define EVAL_DIMENSION 2
#define EVAL_SAMPLES 100

static tsReal control_points[EVAL_POINT_COUNT * EVAL_DIMENSION];

static tsBSpline spline;

static int degree;

static tsReal knot;
static tsDeBoorNet net;

static tsReal* points;
static size_t points_count;

static tsReal* result;
static size_t result_count;

static tsReal* sample_points;
static size_t sample_count;

static nk_bool draw_net;
static nk_bool autoplay;

void demo_eval_initialize ()
{
	control_points[0]  = 50;  control_points[1]  = 50;  // P1
	control_points[2]  = 180; control_points[3]  = 30;  // P2
	control_points[4]  = 200; control_points[5]  = 350; // P3
	control_points[6]  = 500; control_points[7]  = 30;  // P4
	control_points[8]  = 480; control_points[9]  = 400; // P5
	control_points[10] = 330; control_points[11] = 500; // P6
	control_points[12] = 50;  control_points[13] = 380; // P7

	degree = 3;

	ts_bspline_new (EVAL_POINT_COUNT, EVAL_DIMENSION, degree, TS_CLAMPED, &spline, NULL);
	ts_bspline_set_control_points (&spline, control_points, NULL);
	
	// TODO
	// 	error handling :D
	tsStatus status;
	ts_bspline_sample (&spline, EVAL_SAMPLES, &sample_points, &sample_count, &status);

	knot = 0.1f;

	ts_bspline_eval (&spline, knot, &net, &status);

	ts_deboornet_points (&net, &points, &status);
	points_count = ts_deboornet_len_points (&net);
	ts_deboornet_result (&net, &result, &status);
	result_count = ts_deboornet_len_result (&net);

	draw_net = true;
	autoplay = false;
}

void demo_eval_run (struct nk_context* context)
{
	bool knot_update = false;

	if (nk_begin (context, "eval controls", nk_rect (525, TAB_OFFSET, 340, 525), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic (context, 20, 1);
		if (nk_slider_float (context, 0.0f, &knot, 1.0f, 0.01f))
		{
			knot_update = true;
		}
		nk_labelf (context, NK_TEXT_CENTERED, "Knot: %.2f", knot);
		nk_break (context);

		nk_checkbox_label (context, "Autoplay", &autoplay);
		nk_checkbox_label (context, "Show De Boor control net", &draw_net);
		nk_break (context);

		if (nk_slider_int (context, 0, &degree, 6, 1))
		{
			demo_eval_cleanup ();

			ts_bspline_new (EVAL_POINT_COUNT, EVAL_DIMENSION, degree, TS_CLAMPED, &spline, NULL);
			ts_bspline_set_control_points (&spline, control_points, NULL);
			
			tsStatus status;
			ts_bspline_sample (&spline, EVAL_SAMPLES, &sample_points, &sample_count, &status);

			ts_bspline_eval (&spline, knot, &net, &status);

			ts_deboornet_points (&net, &points, &status);
			points_count = ts_deboornet_len_points (&net);
			ts_deboornet_result (&net, &result, &status);
			result_count = ts_deboornet_len_result (&net);
		}
		nk_labelf (context, NK_TEXT_CENTERED, "Degree: %i", degree);
	}
	nk_end (context);

	if (autoplay)
	{
		// if we are running at our target fps of 60
		// 	it will take 10 seconds for the knot to traverse the entire spline
		knot += (0.1f / 60.0f);
		if (knot > 1.0f)
		{
			knot = 0.0f;
		}

		knot_update = true;
	}

	if (knot_update)
	{
		ts_deboornet_free (&net);
		free (points);
		free (result);

		tsStatus status;
		ts_bspline_eval (&spline, knot, &net, &status);

		ts_deboornet_points (&net, &points, &status);
		points_count = ts_deboornet_len_points (&net);
		ts_deboornet_result (&net, &result, &status);
		result_count = ts_deboornet_len_result (&net);
	}
}

void demo_eval_draw ()
{
	ClearBackground (WHITE);

	// draw spline
	if (ts_bspline_degree (&spline))
	{
		// if a spline is a sequence of lines (degree 1)
		// 	use its control points to draw the lines
		if (ts_bspline_degree (&spline) == 1)
		{
			for (int iter = 0; iter < (EVAL_POINT_COUNT - 1); iter++)
			{
				DrawLine (control_points[iter * EVAL_DIMENSION], control_points[(iter * EVAL_DIMENSION) + 1], control_points[(iter * EVAL_DIMENSION) + 2], control_points[(iter * EVAL_DIMENSION) + 3], BLACK);
			}
		}
		else
		{
			for (int iter = 0; iter < (sample_count - 1); iter++)
			{
				DrawLine (sample_points[iter * EVAL_DIMENSION], sample_points[(iter * EVAL_DIMENSION) + 1], sample_points[(iter * EVAL_DIMENSION) + 2], sample_points[(iter * EVAL_DIMENSION) + 3], BLACK);
			}
		}
	}

	// draw control points
	for (int iter = 0; iter < EVAL_POINT_COUNT; iter++)
	{
		DrawRectangle (control_points[iter * EVAL_DIMENSION] - 3, control_points[(iter * EVAL_DIMENSION) + 1] - 3, 6, 6, RED);
	}

	// draw deboor control net
	if (draw_net)
	{
		Color color;
		int offset = 0;
		size_t insertion_count = ts_deboornet_num_insertions (&net);
		for (int iter = insertion_count + 1; iter > 1; iter--)
		{
			switch (iter)
			{
				case 2:
					color = MAGENTA;
					break;
				case 3:
					color = (Color) {0, 255, 255, 255};
					break;
				case 4:
					color = GREEN;
					break;
				case 5:
					color = ORANGE;
					break;
				case 6:
					color = PURPLE;
					break;
				case 7:
					color = LIGHTGRAY;
					break;
				default:
					color = BLACK;
					break;
			}

			for (int jter = 0; jter < iter - 1; jter++)
			{
				DrawLineEx ((Vector2) {points[offset + (jter * EVAL_DIMENSION)], points[offset + (jter * EVAL_DIMENSION) + 1]}, (Vector2) {points[offset + (jter * EVAL_DIMENSION) + 2], points[offset + (jter * EVAL_DIMENSION) + 3]}, 2.0f, color);
			}

			offset += iter * 2;
		}
	}

	// draw evaluated point
	DrawRectangle (result[0] - 3, result[1] - 3, 6, 6, BLUE);
}

void demo_eval_cleanup ()
{
	ts_bspline_free (&spline);
	ts_deboornet_free (&net);
	free (points);
	free (result);
	free (sample_points);
}
