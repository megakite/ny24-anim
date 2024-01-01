#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#include "../include/anim.h"
#include "../include/anim_interp.h"

#define WIDTH 40
#define HEIGHT 20
#define CLEAR "\033[H\033[J"

#define _POSIX_C_SOURCE 200809L

// Glyphs from Konserif 30
// See also: https://github.com/megakite/kons
static const unsigned char QUOTE[] = {
	0x30, 0x78, 0x78, 0x38, 0x30, 0x30, 0x20, 0x40, 0x80,
};
static const unsigned char DIGIT_2[] = {
	0x0F, 0x80, 0x31, 0xE0, 0x40, 0x70, 0x40, 0x30, 0x80, 0x38, 0x80, 0x38,
	0x80, 0x38, 0x80, 0x38, 0x40, 0x70, 0x01, 0xF0, 0x0F, 0xE0, 0x3F, 0x80,
	0x40, 0x00, 0x80, 0x00, 0x80, 0x08, 0x80, 0x08, 0xFF, 0xF8, 0xFF, 0xF8,
	0xFF, 0xF8,
};
static const unsigned char DIGIT_3[] = {
	0x0F, 0x00, 0x30, 0xC0, 0x70, 0x60, 0xE0, 0x70, 0x60, 0x70, 0x20, 0x70,
	0x00, 0xE0, 0x00, 0xC0, 0x03, 0x00, 0x0C, 0xC0, 0x00, 0x70, 0x00, 0x30,
	0x00, 0x38, 0x80, 0x38, 0x80, 0x38, 0x40, 0x70, 0x60, 0xF0, 0x3F, 0xE0,
	0x0F, 0x80,
};
static const unsigned char DIGIT_4[] = {
	0x11, 0xC0, 0x39, 0xC0, 0x3D, 0xC0, 0x71, 0xC0, 0x61, 0xC0, 0x41, 0xC0,
	0x41, 0xC0, 0x81, 0xC0, 0x81, 0xC0, 0x81, 0xC0, 0x81, 0xC0, 0x81, 0xC8,
	0xFF, 0xF8, 0x01, 0xC8, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0,
	0x07, 0xF0,
};

const struct timespec ONE_MS = { .tv_nsec = 1000000 };
const struct timespec ONE_FRAME = { .tv_nsec = 66666667 };

static void SHADOW(char data[], const unsigned width, const unsigned height)
{
	for (size_t i = height - 1; i > 0; --i) {
		for (size_t j = width - 1; j > 0; --j) {
			if (data[(i - 1) * width + (j - 1)] != ' ' &&
			    data[i * width + j] == ' ')
				data[i * width + j] = '\\';
		}
	}
}

// Global variables
_Atomic(int) g_year, g_hour, g_min, g_sec;

void *timer(void *data)
{
	(void) data;

	time_t now;
	struct tm *tm_now;
	while (true) {
		now = time(NULL);
		tm_now = localtime(&now);

		g_year = tm_now->tm_year;
		g_hour = tm_now->tm_hour;
		g_min = tm_now->tm_min;
		g_sec = tm_now->tm_sec;

		if (tm_now->tm_year == 124 && tm_now->tm_sec == 2)
			break;

		nanosleep(&ONE_MS, NULL);
	}

	return NULL;
}

void *player(void *data)
{
	(void) data;

	struct canvas_t *canvas = make_canvas(WIDTH, HEIGHT, 2);
	canvas->filter = SHADOW;

	struct object_t *quote = from_bits(QUOTE, 9, 5, 9, 0, 0, '@');
	struct object_t *digit_2 = from_bits(DIGIT_2, 38, 13, 19, 7, 0, '@');
	struct object_t *digit_3 = from_bits(DIGIT_3, 38, 13, 19, 22, 0, '@');
	struct object_t *digit_4 = from_bits(DIGIT_4, 38, 13, 19, 22, 20, '@');
	add_object(canvas, quote);
	add_object(canvas, digit_2);
	add_object(canvas, digit_3);
	add_object(canvas, digit_4);

	struct animation_t *expand_5 = make_animation(5, 0, 5, interp_linear);
	struct animation_t *expand_13 = make_animation(13, 0, 13, interp_linear);
	struct animation_t *trans_exit = make_animation(15, 0, -20, interp_cubic);
	struct animation_t *trans_enter = make_animation(15, 20, 0, interp_cubic);

	int frame = 0;
	bool played = false;
	while (true) {
		printf(CLEAR "=== %02d:%02d:%02d\n\n", g_hour, g_min, g_sec);

		if (frame == 0) {
			animate(quote, W, expand_5);
			animate(digit_2, W, expand_13);
			animate(digit_3, W, expand_13);
		}

		if (!played && g_year == 124) {
			animate(digit_3, Y, trans_exit);
			animate(digit_4, Y, trans_enter);

			played = true;
		}

		if (g_sec == 2)
			break;

		tick_canvas(canvas);
		draw_canvas(canvas);
		print_canvas(canvas);

		fflush(stdout);
		++frame;
		nanosleep(&ONE_FRAME, NULL);
	}

	destroy_canvas(canvas);
	destroy_animation(expand_5);
	destroy_animation(expand_13);
	destroy_animation(trans_exit);
	destroy_animation(trans_enter);

	puts(CLEAR);

	return NULL;
}

int main(void)
{
	pthread_t thread_timer, thread_player;
	pthread_create(&thread_timer, NULL, timer, NULL);
	pthread_create(&thread_player, NULL, player, NULL);

	pthread_join(thread_timer, NULL);
	pthread_join(thread_player, NULL);

	return 0;
}

