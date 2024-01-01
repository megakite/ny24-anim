#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../include/anim.h"
#include "../include/util.h"

#define LEFTMOST_BIT 0x80
#define EMPTY_CHAR ' '

// impl struct canvas_t
struct canvas_t *make_canvas(const unsigned width, const unsigned height,
			     const unsigned unit)
{
	if (width == 0 || height == 0 || unit == 0)
		return NULL;
	
	struct canvas_t *new = malloc(sizeof(*new) + width * height);
	new->width = width;
	new->height = height;
	new->unit = unit;
	new->num_objects = 0;
	new->cap_objects = 0;
	new->objects = NULL;
	new->filter = NULL;

	return new;
}
void destroy_canvas(struct canvas_t *cv)
{
	if (!cv)
		return;

	for (size_t i = 0; i < cv->num_objects; ++i) {
		destroy_object(cv->objects[i]);
	}
	
	free(cv->objects);
	free(cv);
	cv = NULL;
}
void add_object(struct canvas_t *cv, struct object_t *obj)
{
	if (!cv || !obj)
		return;

	if (cv->cap_objects == cv->num_objects) {
		cv->cap_objects = cv->cap_objects * 2 + 1;
		cv->objects = realloc(cv->objects,
				      sizeof(*(cv->objects)) * cv->cap_objects);
	}
	cv->objects[cv->num_objects] = obj;
	++cv->num_objects;

}
void draw_canvas(struct canvas_t *cv)
{
	if (!cv)
		return;
	
	memset(cv->data, EMPTY_CHAR, cv->width * cv->height);

	for (size_t i = 0; i < cv->num_objects; ++i) {
		const struct object_t *obj = cv->objects[i];
		draw_object(cv, obj);
	}

	if (cv->filter)
		cv->filter(cv->data, cv->width, cv->height);
}
void tick_canvas(struct canvas_t *cv)
{
	for (size_t i = 0; i < cv->num_objects; ++i) {
		struct object_t *obj = cv->objects[i];
		tick_object(obj);
	}
}
void print_canvas(const struct canvas_t *cv)
{
	if (!cv)
		return;

	for (size_t i = 0; i < cv->width * cv->height; ++i) {
		for (size_t j = 0; j < cv->unit; ++j) {
			putchar(cv->data[i]);
		}

		if (i % cv->width == cv->width - 1)
			putchar('\n');
	}
}

// impl struct object_t
struct object_t *from_bits(const unsigned char *data, const size_t size,
			   const unsigned w, const unsigned h, const int x,
			   const int y, const char c)
{
	if (!data)
		return NULL;

	const size_t data_size = 8 * size;

	struct object_t *new = malloc(sizeof(*new) + data_size);
	new->w = (struct property_t)
		 { .type = UINT, .uval = w, .animation = NULL };
	new->h = (struct property_t)
		 { .type = UINT, .uval = h, .animation = NULL };
	new->x = (struct property_t)
		 { .type = SINT, .ival = x, .animation = NULL };
	new->y = (struct property_t)
		 { .type = SINT, .ival = y, .animation = NULL };
	// XXX: implicit calculation
	new->word_size = 8 * ceil_byte(w);

	for (size_t row = 0; row < size; ++row) {
		for (size_t col = 0; col < 8; ++col) {
			new->data[row * 8 + col] = data[row] &
				(LEFTMOST_BIT >> col) ? c : EMPTY_CHAR;
		}
	}

	return new;
}
void destroy_object(struct object_t *obj)
{
	free(obj);
	obj = NULL;
}
struct property_t *get_object_prop(struct object_t *obj, const enum prop_e idx)
{
	switch (idx) {
	case W:
		return &obj->w;
	case H:
		return &obj->h;
	case X:
		return &obj->x;
	case Y:
		return &obj->y;
	default:
		return NULL;
	}
}
void tick_object(struct object_t *obj)
{
	for (size_t j = 0; j < NUM_PROPS; ++j) {
		struct property_t *prop = get_object_prop(obj, j);

		struct animation_t *an = prop->animation;
		if (!an)
			continue;

		++an->progress;

		double fac = an->interp((double)an->progress / an->length);
		switch (prop->type) {
		case SINT:
			prop->ival = (int)
				     round((1 - fac) * an->start_value +
					   fac * an->end_value);
			break;
		case UINT:
			prop->uval = (unsigned)
				     round((1 - fac) * an->start_value +
					   fac * an->end_value);
			break;
		}

		if (an->progress >= an->length) {
			// TODO: make animation reusable
			// an->progress = 0;
			prop->animation = NULL;
		}
	}
}
void animate(struct object_t *obj, const enum prop_e idx,
	     struct animation_t *an)
{
	if (!obj || !an)
		return;

	struct property_t *prop = get_object_prop(obj, idx);
	if (!prop)
		return;
		
	prop->animation = an;
}
void draw_object(struct canvas_t *cv, const struct object_t *obj)
{
	const size_t row_step = obj->word_size;
	
	const ptrdiff_t topleft = obj->x.ival + (signed)cv->width * obj->y.ival;
	for (size_t row = 0; row < obj->h.uval; ++row) {
		for (size_t col = 0; col < obj->w.uval; ++col) {
			const ptrdiff_t pos = topleft +
					      (signed)(row * cv->width + col);
			if (pos < 0 || pos >= cv->width * cv->height)
				continue;

			cv->data[pos] = obj->data[row * row_step + col];
		}
	}
}

// impl struct animation_t
struct animation_t *make_animation(const int length, const double start_value,
				   const double end_value,
				   double (*interp)(double v))
{
	if (length <= 0 || interp == NULL)
		return NULL;

	struct animation_t *new = calloc(1, sizeof(*new));
	new->length = length;
	new->progress = 0;
	new->start_value = start_value;
	new->end_value = end_value;
	new->interp = interp;

	return new;
}
void destroy_animation(struct animation_t *an)
{
	free(an);
	an = NULL;
}

