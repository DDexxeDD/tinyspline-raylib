#include "raylib-nuklear.h"

#include "common.h"

void nk_break (struct nk_context* context)
{
	nk_label (context, "", NK_TEXT_LEFT);
}

