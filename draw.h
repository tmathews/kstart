#ifndef FILE_draw
#define FILE_draw

#define M_PI 3.14159265358979323846

struct color {
	double a;
	double r;
	double g;
	double b;
};

struct point {
	int x, y;
};

struct rect {
	int x, y;
	int width, height;	
};

struct color hex2rgb(unsigned int val);
void path_rounded_rect(cairo_t *, double x, double y, double width, 
	double height, double r);
void draw_svg_square(cairo_t *, RsvgHandle *, int x, int y, int size);
void draw_img_square(cairo_t *cr, cairo_surface_t *img, int x, int y, int size);
double draw_text(cairo_t *cr, const char *str, int origin_x, int origin_y);
double draw_text_rtl(cairo_t *cr, const char *str, struct point origin);
double draw_text_centered(cairo_t *cr, const char *str, struct point origin);

#endif
