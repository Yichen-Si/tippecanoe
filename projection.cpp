#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include <atomic>
#include <limits>
#include <assert.h>
#include "projection.hpp"
#include "errors.hpp"

unsigned long long (*encode_index)(unsigned int wx, unsigned int wy) = NULL;
void (*decode_index)(unsigned long long index, unsigned *wx, unsigned *wy) = NULL;

struct projection projections[] = {
	{"EPSG:4326", lonlat2tile, tile2lonlat, "urn:ogc:def:crs:OGC:1.3:CRS84"},
	{"EPSG:3857", epsg3857totile, tiletoepsg3857, "urn:ogc:def:crs:EPSG::3857"},
	{"Euclidean", euclidean2tile, tile2euclidean, "euclidean32"},
	{NULL, NULL, NULL, NULL},
};

struct projection *projection = &projections[0];

// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
void lonlat2tile(double lon, double lat, int zoom, long long *x, long long *y) {
	// Place infinite and NaN coordinates off the edge of the Mercator plane

	int lat_class = std::fpclassify(lat);
	int lon_class = std::fpclassify(lon);
	bool bad_lon = false;

	if (lat_class == FP_INFINITE || lat_class == FP_NAN) {
		lat = 89.9;
	}
	if (lon_class == FP_INFINITE || lon_class == FP_NAN) {
		// Keep these far enough from the plane that they don't get
		// moved back into it by 360-degree offsetting

		lon = 720;
		bad_lon = true;
	}

	// Must limit latitude somewhere to prevent overflow.
	// 89.9 degrees latitude is 0.621 worlds beyond the edge of the flat earth,
	// hopefully far enough out that there are few expectations about the shape.
	if (lat < -89.9) {
		lat = -89.9;
	}
	if (lat > 89.9) {
		lat = 89.9;
	}

	if (lon < -360 && !bad_lon) {
		lon = -360;
	}
	if (lon > 360 && !bad_lon) {
		lon = 360;
	}

	double lat_rad = lat * M_PI / 180;
	unsigned long long n = 1LL << zoom;

	long long llx = std::round(n * ((lon + 180) / 360));
	long long lly = std::round(n * (1 - (log(tan(lat_rad) + 1 / cos(lat_rad)) / M_PI)) / 2);

	*x = llx;
	*y = lly;
}

// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
void tile2lonlat(long long x, long long y, int zoom, double *lon, double *lat) {
	unsigned long long n = 1LL << zoom;
	*lon = 360.0 * x / n - 180.0;
	*lat = atan(sinh(M_PI * (1 - 2.0 * y / n))) * 180.0 / M_PI;
}

void epsg3857totile(double ix, double iy, int zoom, long long *x, long long *y) {
	// Place infinite and NaN coordinates off the edge of the Mercator plane
	// input are in meters fro mthe origin
	int iy_class = std::fpclassify(iy);
	int ix_class = std::fpclassify(ix);

	if (iy_class == FP_INFINITE || iy_class == FP_NAN) {
		iy = 40000000.0;
	}
	if (ix_class == FP_INFINITE || ix_class == FP_NAN) {
		ix = 40000000.0;
	}
	// normalize (input range is ~(-R\pi, R\pi) ) -> (0, 2^32)
	// x/(R*pi) * 2^31 + 2^31 (scaled w.r.t. circumference then shift to positive)
	*x = std::round(ix * (1LL << 31) / 6378137.0 / M_PI + (1LL << 31));
	*y = std::round(((1LL << 32) - 1) - (iy * (1LL << 31) / 6378137.0 / M_PI + (1LL << 31)));

	if (zoom != 0) {
		*x = std::round((double) *x / (1LL << (32 - zoom)));
		*y = std::round((double) *y / (1LL << (32 - zoom)));
	}
}

void tiletoepsg3857(long long ix, long long iy, int zoom, double *ox, double *oy) {
	if (zoom != 0) {
		ix <<= (32 - zoom);
		iy <<= (32 - zoom);
	}

	*ox = (ix - (1LL << 31)) * M_PI * 6378137.0 / (1LL << 31);
	*oy = ((1LL << 32) - 1 - iy - (1LL << 31)) * M_PI * 6378137.0 / (1LL << 31);
}

// https://en.wikipedia.org/wiki/Hilbert_curve

void hilbert_rot(unsigned long long n, unsigned *x, unsigned *y, unsigned long long rx, unsigned long long ry) {
	if (ry == 0) {
		if (rx == 1) {
			*x = n - 1 - *x;
			*y = n - 1 - *y;
		}

		unsigned t = *x;
		*x = *y;
		*y = t;
	}
}

unsigned long long hilbert_xy2d(unsigned long long n, unsigned x, unsigned y) {
	unsigned long long d = 0;
	unsigned long long rx, ry;

	for (unsigned long long s = n / 2; s > 0; s /= 2) {
		rx = (x & s) != 0;
		ry = (y & s) != 0;

		d += s * s * ((3 * rx) ^ ry);
		hilbert_rot(s, &x, &y, rx, ry);
	}

	return d;
}

void hilbert_d2xy(unsigned long long n, unsigned long long d, unsigned *x, unsigned *y) {
	unsigned long long rx, ry;
	unsigned long long t = d;

	*x = *y = 0;
	for (unsigned long long s = 1; s < n; s *= 2) {
		rx = 1 & (t / 2);
		ry = 1 & (t ^ rx);
		hilbert_rot(s, x, y, rx, ry);
		*x += s * rx;
		*y += s * ry;
		t /= 4;
	}
}

unsigned long long encode_hilbert(unsigned int wx, unsigned int wy) {
	return hilbert_xy2d(1LL << 32, wx, wy);
}

void decode_hilbert(unsigned long long index, unsigned *wx, unsigned *wy) {
	hilbert_d2xy(1LL << 32, index, wx, wy);
}

unsigned long long encode_quadkey(unsigned int wx, unsigned int wy) {
	unsigned long long out = 0;

	int i;
	for (i = 0; i < 32; i++) {
		unsigned long long v = ((wx >> (32 - (i + 1))) & 1) << 1;
		v |= (wy >> (32 - (i + 1))) & 1;
		v = v << (64 - 2 * (i + 1));

		out |= v;
	}

	return out;
}

static std::atomic<unsigned char> decodex[256];
static std::atomic<unsigned char> decodey[256];

void decode_quadkey(unsigned long long index, unsigned *wx, unsigned *wy) {
	static std::atomic<int> initialized(0);
	if (!initialized) {
		for (size_t ix = 0; ix < 256; ix++) {
			size_t xx = 0, yy = 0;

			for (size_t i = 0; i < 32; i++) {
				xx |= ((ix >> (64 - 2 * (i + 1) + 1)) & 1) << (32 - (i + 1));
				yy |= ((ix >> (64 - 2 * (i + 1) + 0)) & 1) << (32 - (i + 1));
			}

			decodex[ix] = xx;
			decodey[ix] = yy;
		}

		initialized = 1;
	}

	*wx = *wy = 0;

	for (size_t i = 0; i < 8; i++) {
		*wx |= ((unsigned) decodex[(index >> (8 * i)) & 0xFF]) << (4 * i);
		*wy |= ((unsigned) decodey[(index >> (8 * i)) & 0xFF]) << (4 * i);
	}
}

void set_projection_or_exit(const char *optarg) {
	struct projection *p;
	for (p = projections; p->name != NULL; p++) {
		if (strcmp(p->name, optarg) == 0) {
			projection = p;
			break;
		}
		if (strcmp(p->alias, optarg) == 0) {
			projection = p;
			break;
		}
	}
	if (p->name == NULL) {
		fprintf(stderr, "Unknown projection (-s): %s\n", optarg);
		exit(EXIT_ARGS);
	}
}

unsigned long long encode_vertex(unsigned int wx, unsigned int wy) {
	return (((unsigned long long) wx) << 32) | wy;
}

unsigned long long encode_hilbert2(uint32_t x, uint32_t y) {
	uint8_t order = 32;
	// Calculate effective bits needed
	const uint32_t coord_bits = 32;
	const uint32_t useless_bits = (((x | y) == 0) ? 32 : __builtin_clz(x | y)) & ~1;
	const uint8_t lowest_order = (coord_bits - useless_bits) + (order & 1);

	uint64_t result = 0;
	uint8_t state = 0;
	int8_t shift_factor = lowest_order - 3;

	// Process 3 bits at a time
	while (shift_factor > 0) {
		uint8_t x_in = ((x >> shift_factor) & 7) << 3;
		uint8_t y_in = (y >> shift_factor) & 7;
		uint8_t index = x_in | y_in | state;

		uint8_t r = LUT_XY2H[index];
		state = r & 0xC0;  // Top 2 bits for next state
		uint64_t hhh = r & 0x3F;  // Bottom 6 bits for Hilbert value

		result |= (hhh << (shift_factor * 2));
		shift_factor -= 3;
	}

	// Handle remaining bits
	shift_factor *= -1;
	uint8_t x_in = ((x << shift_factor) & 7) << 3;
	uint8_t y_in = (y << shift_factor) & 7;
	uint8_t index = x_in | y_in | state;

	uint8_t r = LUT_XY2H[index];
	uint64_t hhh = r & 0x3F;
	result |= (hhh >> (shift_factor * 2));

	return result;
}

void decode_hilbert2(unsigned long long h, uint32_t *x, uint32_t *y) {
	uint8_t order = 32;
    const uint32_t coord_bits = 32;
    const uint32_t useless_bits = ((h ? __builtin_clzll(h) : 64) >> 1) & ~1;
    const uint8_t lowest_order = coord_bits - useless_bits + (order & 1);

    *x = 0, *y = 0;
    uint8_t state = 0;
    int8_t shift_factor = lowest_order - 3;

    // Process 3 bits at a time
    while (shift_factor > 0) {
        uint8_t h_in = (h >> (shift_factor << 1)) & 0x3F;
        uint8_t r = LUT_H2XY[state | h_in];

        state = r & 0xC0;
        uint32_t xxx = (r >> 3) & 7;
        uint32_t yyy = r & 7;

        *x |= xxx << shift_factor;
        *y |= yyy << shift_factor;
        shift_factor -= 3;
    }

    // Handle remaining bits
    shift_factor *= -1;
    uint8_t h_in = (h << (shift_factor * 2)) & 0x3F;
    uint8_t r = LUT_H2XY[state | h_in];

    uint32_t xxx = (r >> 3) & 7;
    uint32_t yyy = r & 7;

    *x = (xxx >> shift_factor) | *x;
    *y = (yyy >> shift_factor) | *y;
}

void euclidean2tile(double x, double y, int zoom, long long *ox, long long *oy) {
	assert (x >= 0 && y >= 0);
	// should accept extra parameters for the bounds
	double dmax = (double) std::numeric_limits<uint32_t>::max();

    // Place infinite and NaN coordinates at maximum bounds
    int x_class = std::fpclassify(x);
    int y_class = std::fpclassify(y);
    if (x_class == FP_INFINITE || x_class == FP_NAN) {
        x = dmax;
    }
    if (y_class == FP_INFINITE || y_class == FP_NAN) {
        y = dmax;
    }

    long long n = 1LL << zoom;
    *ox = std::round(n * (x/dmax));
    *oy = std::round(n * (y/dmax));

    // Ensure coordinates are within bounds
    if (*ox < 0) *ox = 0;
    if (*oy < 0) *oy = 0;
    if (*ox >= n) *ox = n - 1;
    if (*oy >= n) *oy = n - 1;
}

void tile2euclidean(long long x, long long y, int zoom, double *ox, double *oy) {
    long long n = 1LL << zoom;
	double dmax = (double) std::numeric_limits<uint32_t>::max();
    *ox = 1. * x / n * dmax;
    *oy = 1. * y / n * dmax;
}
