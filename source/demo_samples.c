#include <stdlib.h>

#include "tinyspline.h"
#include "raylib.h"
#include "raylib-nuklear.h"

#include "common.h"

#define SAMPLES_POINT_COUNT 7
#define SAMPLES_DIMENSION 2
#define SAMPLES_SPLINE_DEGREE 3

static tsReal control_points[SAMPLES_POINT_COUNT * SAMPLES_DIMENSION];

static tsBSpline spline_clamped;
static tsBSpline spline_opened;

static int samples;
static size_t sample_count;
static tsReal* points_clamped;
static tsReal* points_opened;
static tsReal* points;  // points at either points_clamped or points_opened
static tsStatus status;

static nk_bool show_sampled_points;
static nk_bool show_control_points;
static nk_bool clamped;
static nk_bool opened;

void demo_samples_initialize ()
{
	control_points[0]  = 50;  control_points[1]  = 50;  // P1
	control_points[2]  = 180; control_points[3]  = 30;  // P2
	control_points[4]  = 200; control_points[5]  = 350; // P3
	control_points[6]  = 500; control_points[7]  = 30;  // P4
	control_points[8]  = 480; control_points[9]  = 400; // P5
	control_points[10] = 330; control_points[11] = 500; // P6
	control_points[12] = 50;  control_points[13] = 380; // P7

	samples = 50;

	show_sampled_points = true;
	show_control_points = true;
	clamped = true;
	opened = false;

	ts_bspline_new (SAMPLES_POINT_COUNT, SAMPLES_DIMENSION, SAMPLES_SPLINE_DEGREE, TS_CLAMPED, &spline_clamped, NULL);
	ts_bspline_new (SAMPLES_POINT_COUNT, SAMPLES_DIMENSION, SAMPLES_SPLINE_DEGREE, TS_OPENED, &spline_opened, NULL);

	ts_bspline_set_control_points (&spline_clamped, control_points, NULL);
	ts_bspline_set_control_points (&spline_opened, control_points, NULL);

	ts_bspline_sample (&spline_clamped, samples, &points_clamped, &sample_count, &status);
	ts_bspline_sample (&spline_opened, samples, &points_opened, &sample_count, &status);

	points = points_clamped;
}

void demo_samples_run (struct nk_context* context)
{
	if (nk_begin (context, "samples controls", nk_rect (525, TAB_OFFSET, 340, 525), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic (context, 20, 1);
		if (nk_slider_int (context, 0, &samples, 100, 1))
		{
			free (points_clamped);
			free (points_opened);

			ts_bspline_sample (&spline_clamped, samples, &points_clamped, &sample_count, &status);
			ts_bspline_sample (&spline_opened, samples, &points_opened, &sample_count, &status);
		}
		nk_labelf (context, NK_TEXT_CENTERED, "Samples: %i", samples);
		nk_break (context);

		nk_label (context, "Knot vector:", NK_TEXT_LEFT);
		nk_layout_row_dynamic (context, 20, 2);
		clamped = nk_option_label (context, "Clamped", clamped);
		if (!clamped && !opened)
		{
			clamped = true;
		}
		if (clamped)
		{
			opened = false;
			points = points_clamped;
		}
		opened = nk_option_label (context, "Opened", opened);
		if (!opened && !clamped)
		{
			opened = true;
		}
		if (opened)
		{
			clamped = false;
			points = points_opened;
		}
		nk_break (context);

		nk_layout_row_dynamic (context, 20, 1);
		nk_checkbox_label (context, "Show sampled points", &show_sampled_points);
		nk_checkbox_label (context, "Show control points", &show_control_points);
	}
	nk_end (context);
}

void demo_samples_draw ()
{
	ClearBackground (WHITE);

	if (show_control_points)
	{
		for (int iter = 0; iter < SAMPLES_POINT_COUNT; iter++)
		{
			DrawRectangle (control_points[iter * SAMPLES_DIMENSION] - 3, control_points[(iter * SAMPLES_DIMENSION) + 1] - 3, 6, 6, RED);
		}
	}

	for (int iter = 0; iter < sample_count - 1; iter++)
	{
		DrawLine (points[iter * SAMPLES_DIMENSION], points[(iter * SAMPLES_DIMENSION) + 1], points[(iter * SAMPLES_DIMENSION) + 2], points[(iter * SAMPLES_DIMENSION) + 3], BLACK);
	}

	if (show_sampled_points)
	{
		for (int iter = 0; iter < sample_count; iter++)
		{
			DrawRectangle (points[iter * SAMPLES_DIMENSION] - 2, points[(iter * SAMPLES_DIMENSION) + 1] - 2, 4, 4, BLUE);
		}
	}
}

void demo_samples_cleanup ()
{
	ts_bspline_free (&spline_clamped);
	ts_bspline_free (&spline_opened);
	free (points_clamped);
	free (points_opened);
}

