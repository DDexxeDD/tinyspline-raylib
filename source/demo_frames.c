#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tinyspline.h"
#include "raylib.h"
#include "raylib-nuklear.h"
#include "cvector.h"
#include "raymath.h"

#include "demo_interpolation.h"
#include "common.h"

#define FRAMES_POINT_COUNT 11
#define FRAMES_DIMENSION 3
#define FRAMES_SAMPLES 100
#define FRAMES_KNOT_COUNT 1001

static Camera3D camera;

static tsReal control_points[FRAMES_POINT_COUNT * FRAMES_DIMENSION];

static tsBSpline spline;

static tsReal* sample_points;
static size_t sample_count;

static tsReal knot;
static tsReal knots[FRAMES_KNOT_COUNT];
static tsFrame frames[FRAMES_KNOT_COUNT];

static nk_bool autoplay;

void demo_frames_initialize ()
{
	control_points[0] = -30.0; control_points[1] = 0; control_points[2] = 0; // P1
	control_points[3] = -10.0; control_points[4] = -8.0; control_points[5] = -10.0; // P2
	control_points[6] = 0; control_points[7] = 35.0; control_points[8] = -8.0; // P3
	control_points[9] = 42.0; control_points[10] = -8.0; control_points[11] = -7.0; // P4
	control_points[12] = 40.0; control_points[13] = 35.0; control_points[14] = -2.0; // P5
	control_points[15] = 25.0; control_points[16] = 45.0; control_points[17] = -1.0; // P6
	control_points[18] = -5.0; control_points[19] = 40.0; control_points[20] = 5.0; // P7
	control_points[21] = -3.0; control_points[22] = 30.0; control_points[23] = 8.0; // P8
	control_points[24] = 0; control_points[25] = 20.0; control_points[26] = 9.0; // P9
	control_points[27] = 3.0; control_points[28] = 23.0; control_points[29] = 1.0; // P10
	control_points[30] = 8.0; control_points[31] = 30.0; control_points[32] = 0; // P11

	ts_bspline_new (FRAMES_POINT_COUNT, FRAMES_DIMENSION, 3, TS_CLAMPED, &spline, NULL);
	ts_bspline_set_control_points (&spline, control_points, NULL);

	tsStatus status;
	ts_bspline_sample (&spline, FRAMES_SAMPLES, &sample_points, &sample_count, &status);

	camera.position = (Vector3) {-24.839f, 18.0f, -28.49f};
	camera.target = (Vector3) {-21.318f, 17.182f, -22.239f};
	camera.up = (Vector3) {0.0f, 1.0f, 0.0f};
	camera.fovy = 90.0f;
	// lets render in perspective 3d
	// 	tinyspline cxx/fltk example renders in orthographic
	camera.projection = CAMERA_PERSPECTIVE;

	knot = 0.0f;

	ts_bspline_uniform_knot_seq (&spline, FRAMES_KNOT_COUNT, knots);
	ts_bspline_compute_rmf (&spline, knots, FRAMES_KNOT_COUNT, false, frames, &status);

	autoplay = false;
}

void demo_frames_run (struct nk_context* context)
{
	if (nk_begin (context, "frames controls", nk_rect (525, TAB_OFFSET, 340, 525), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic (context, 20, 1);
		nk_slider_float (context, 0.0f, &knot, 1.0f, 0.01f);
		nk_labelf (context, NK_TEXT_CENTERED, "Knot: %.2f", knot);
		nk_break (context);

		nk_checkbox_label (context, "Autoplay", &autoplay);
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
	}
}

void demo_frames_draw ()
{
	// draw this demo white on black
	// 	the constrast looks better
	ClearBackground (BLACK);

	BeginMode3D (camera);
	{
		for (int iter = 0; iter < (sample_count - 1); iter++)
		{
			DrawLine3D ((Vector3) {sample_points[iter * FRAMES_DIMENSION], sample_points[(iter * FRAMES_DIMENSION) + 1], sample_points[(iter * FRAMES_DIMENSION) + 2]}, (Vector3) {sample_points[(iter * FRAMES_DIMENSION) + 3], sample_points[(iter * FRAMES_DIMENSION) + 4], sample_points[(iter * FRAMES_DIMENSION) + 5]}, WHITE);
		}

		tsFrame* frame = &frames[(int) (knot * 1000)];
		Vector3 position = (Vector3) {frame->position[0], frame->position[1], frame->position[2]};
		Vector3 tangent = (Vector3) {frame->tangent[0], frame->tangent[1], frame->tangent[2]};
		Vector3 binormal = (Vector3) {frame->binormal[0], frame->binormal[1], frame->binormal[2]};
		Vector3 normal = (Vector3) {frame->normal[0], frame->normal[1], frame->normal[2]};

		DrawLine3D (position, Vector3Add (position, Vector3Scale (tangent, 5.0f)), RED);
		DrawLine3D (position, Vector3Add (position, Vector3Scale (binormal, 5.0f)), GREEN);
		DrawLine3D (position, Vector3Add (position, Vector3Scale (normal, 5.0f)), BLUE);
	}
	EndMode3D ();
}

void demo_frames_cleanup ()
{
	free (sample_points);
	ts_bspline_free (&spline);
}

