#ifndef _ANIM_H_
#define _ANIM_H_

#include <stddef.h>
#include <stdbool.h>

#define NUM_PROPS 4

enum prop_e {
	W, H, X, Y,
};

struct animation_t {
	int length, progress;
	double start_value, end_value;

	double (*interp)(const double v);
};
struct animation_t *make_animation(const int length, const double start_value,
				   const double end_value,
				   double (*interp)(const double v));
void destroy_animation(struct animation_t *an);

struct property_t {
	enum {
		SINT, UINT,
	} type;
	union {
		int ival;
		unsigned int uval;
	};

	struct animation_t *animation;
};

struct object_t {
	struct property_t w, h, x, y;

	size_t word_size;
	char data[];
};
struct object_t *from_bits(const unsigned char *data, const size_t size,
			   const unsigned w, const unsigned h, const int x,
			   const int y, const char c);
void destroy_object(struct object_t *obj);
struct property_t *get_object_prop(struct object_t *obj, const enum prop_e idx);
void tick_object(struct object_t *obj);
void animate(struct object_t *obj, const enum prop_e idx,
	     struct animation_t *an);

struct canvas_t {
	unsigned width, height;
	unsigned unit;

	size_t num_objects, cap_objects;
	struct object_t **objects;

	void (*filter)(char data[], const unsigned width,
		       const unsigned height);

	char data[];
};
struct canvas_t *make_canvas(const unsigned width, const unsigned height,
			     const unsigned unit);
void destroy_canvas(struct canvas_t *cv);
void tick_canvas(struct canvas_t *cv);
void draw_canvas(struct canvas_t *cv);
void print_canvas(const struct canvas_t *cv);
void add_object(struct canvas_t *cv, struct object_t *obj);
void draw_object(struct canvas_t *cv, const struct object_t *obj);

#endif//_ANIM_H_
