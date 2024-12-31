#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#define A_DEBUG_POLYGON ((int) '@')
#define A_CLUSTER_DENSEST_AS_NEEDED ((int) 'C')
#define A_COALESCE_FRACTION_AS_NEEDED ((int) 'D')
#define A_INCREASE_GAMMA_AS_NEEDED ((int) 'G')
#define A_CONVERT_NUMERIC_IDS ((int) 'I')
#define A_GRID_LOW_ZOOMS ((int) 'L')
#define A_COALESCE_SMALLEST_AS_NEEDED ((int) 'N')
#define A_GENERATE_POLYGON_LABEL_POINTS ((int) 'P')
#define A_PREFER_RADIX_SORT ((int) 'R')
#define A_COALESCE_DENSEST_AS_NEEDED ((int) 'S')
#define A_CALCULATE_INDEX ((int) 'X')
#define A_DETECT_SHARED_BORDERS ((int) 'b')
#define A_COALESCE ((int) 'c')
#define A_DROP_FRACTION_AS_NEEDED ((int) 'd')
#define A_EXTEND_ZOOMS ((int) 'e')
#define A_CALCULATE_FEATURE_DENSITY ((int) 'g')
#define A_HILBERT ((int) 'h')
#define A_HILBERT_LUT ((int) 'H')
#define A_GENERATE_IDS ((int) 'i')
#define A_LINE_DROP ((int) 'l')
#define A_DROP_SMALLEST_AS_NEEDED ((int) 'n')
#define A_REORDER ((int) 'o')
#define A_POLYGON_DROP ((int) 'p')
#define A_REVERSE ((int) 'r')
#define A_DROP_DENSEST_AS_NEEDED ((int) 's')
#define A_VARIABLE_DEPTH_PYRAMID ((int) 't')
#define A_VISVALINGAM ((int) 'v')
#define A_DETECT_WRAPAROUND ((int) 'w')

#define P_TILE_COMPRESSION ((int) 'C')
#define P_DUPLICATION ((int) 'D')
#define P_SINGLE_PRECISION ((int) 'N')
#define P_SIMPLIFY_LOW ((int) 'S')
#define P_TINY_POLYGON_REDUCTION_AT_MAXZOOM ((int) 'T')
#define P_REVERSE_SOURCE_POLYGON_WINDING ((int) 'W')
#define P_BASEZOOM_ABOVE_MAXZOOM ((int) 'b')
#define P_CLIPPING ((int) 'c')
#define P_DYNAMIC_DROP ((int) 'd')
#define P_EMPTY_CSV_COLUMNS ((int) 'e')
#define P_FEATURE_LIMIT ((int) 'f')
#define P_TILE_STATS ((int) 'g')
#define P_INPUT_ORDER ((int) 'i')
#define P_KILOBYTE_LIMIT ((int) 'k')
#define P_SIMPLIFY_SHARED_NODES ((int) 'n')
#define P_POLYGON_SPLIT ((int) 'p')
#define P_SIMPLIFY ((int) 's')
#define P_TINY_POLYGON_REDUCTION ((int) 't')
#define P_USE_SOURCE_POLYGON_WINDING ((int) 'w')

extern int prevent[256];
extern int additional[256];

// #include <string>
// #include <vector>
// #include <memory>
// #include <map>
// #include <getopt.h>
// #include "text.hpp"

// static struct option long_options_orig[] = {
//     {"Output tileset", 0, 0, 0},
//     {"output", required_argument, 0, 'o'},
//     {"output-to-directory", required_argument, 0, 'e'},
//     {"force", no_argument, 0, 'f'},
//     {"allow-existing", no_argument, 0, 'F'},

//     {"Tileset description and attribution", 0, 0, 0},
//     {"name", required_argument, 0, 'n'},
//     {"attribution", required_argument, 0, 'A'},
//     {"description", required_argument, 0, 'N'},

//     {"Input files and layer names", 0, 0, 0},
//     {"layer", required_argument, 0, 'l'},
//     {"named-layer", required_argument, 0, 'L'},

//     {"Parallel processing of input", 0, 0, 0},
//     {"read-parallel", no_argument, 0, 'P'},

//     {"Projection of input", 0, 0, 0},
//     {"projection", required_argument, 0, 's'},

//     {"Zoom levels", 0, 0, 0},
//     {"maximum-zoom", required_argument, 0, 'z'},
//     {"minimum-zoom", required_argument, 0, 'Z'},
//     {"smallest-maximum-zoom-guess", required_argument, 0, '~'},
//     {"extend-zooms-if-still-dropping", no_argument, &additional[A_EXTEND_ZOOMS], 1},
//     {"extend-zooms-if-still-dropping-maximum", required_argument, 0, '~'},
//     {"generate-variable-depth-tile-pyramid", no_argument, &additional[A_VARIABLE_DEPTH_PYRAMID], 1},
//     {"one-tile", required_argument, 0, 'R'},

//     {"Tile resolution", 0, 0, 0},
//     {"full-detail", required_argument, 0, 'd'},
//     {"low-detail", required_argument, 0, 'D'},
//     {"minimum-detail", required_argument, 0, 'm'},
//     {"extra-detail", required_argument, 0, '~'},

//     {"Filtering feature attributes", 0, 0, 0},
//     {"exclude", required_argument, 0, 'x'},
//     {"include", required_argument, 0, 'y'},
//     {"exclude-all", no_argument, 0, 'X'},

//     {"Modifying feature attributes", 0, 0, 0},
//     {"attribute-type", required_argument, 0, 'T'},
//     {"attribute-description", required_argument, 0, 'Y'},
//     {"accumulate-attribute", required_argument, 0, 'E'},
//     {"accumulate-numeric-attributes", required_argument, 0, '~'},
//     {"empty-csv-columns-are-null", no_argument, &prevent[P_EMPTY_CSV_COLUMNS], 1},
//     {"convert-stringified-ids-to-numbers", no_argument, &additional[A_CONVERT_NUMERIC_IDS], 1},
//     {"use-attribute-for-id", required_argument, 0, '~'},
//     {"single-precision", no_argument, &prevent[P_SINGLE_PRECISION], 1},
//     {"set-attribute", required_argument, 0, '~'},
//     {"maximum-string-attribute-length", required_argument, 0, '~'},

//     {"Filtering features by attributes", 0, 0, 0},
//     {"feature-filter-file", required_argument, 0, 'J'},
//     {"feature-filter", required_argument, 0, 'j'},
//     {"unidecode-data", required_argument, 0, '~'},

//     {"Dropping a fixed fraction of features by zoom level", 0, 0, 0},
//     {"drop-rate", required_argument, 0, 'r'},
//     {"retain-points-multiplier", required_argument, 0, '~'},
//     {"base-zoom", required_argument, 0, 'B'},
//     {"drop-denser", required_argument, 0, '~'},
//     {"limit-base-zoom-to-maximum-zoom", no_argument, &prevent[P_BASEZOOM_ABOVE_MAXZOOM], 1},
//     {"drop-lines", no_argument, &additional[A_LINE_DROP], 1},
//     {"drop-polygons", no_argument, &additional[A_POLYGON_DROP], 1},
//     {"cluster-distance", required_argument, 0, 'K'},
//     {"cluster-maxzoom", required_argument, 0, 'k'},
//     {"preserve-point-density-threshold", required_argument, 0, '~'},
//     {"preserve-multiplier-density-threshold", required_argument, 0, '~'},

//     {"Dropping or merging a fraction of features to keep under tile size limits", 0, 0, 0},
//     {"drop-densest-as-needed", no_argument, &additional[A_DROP_DENSEST_AS_NEEDED], 1},
//     {"drop-fraction-as-needed", no_argument, &additional[A_DROP_FRACTION_AS_NEEDED], 1},
//     {"drop-smallest-as-needed", no_argument, &additional[A_DROP_SMALLEST_AS_NEEDED], 1},
//     {"coalesce-densest-as-needed", no_argument, &additional[A_COALESCE_DENSEST_AS_NEEDED], 1},
//     {"coalesce-fraction-as-needed", no_argument, &additional[A_COALESCE_FRACTION_AS_NEEDED], 1},
//     {"coalesce-smallest-as-needed", no_argument, &additional[A_COALESCE_SMALLEST_AS_NEEDED], 1},
//     {"force-feature-limit", no_argument, &prevent[P_DYNAMIC_DROP], 1},
//     {"cluster-densest-as-needed", no_argument, &additional[A_CLUSTER_DENSEST_AS_NEEDED], 1},

//     {"Dropping tightly overlapping features", 0, 0, 0},
//     {"gamma", required_argument, 0, 'g'},
//     {"increase-gamma-as-needed", no_argument, &additional[A_INCREASE_GAMMA_AS_NEEDED], 1},

//     {"Line and polygon simplification", 0, 0, 0},
//     {"simplification", required_argument, 0, 'S'},
//     {"no-line-simplification", no_argument, &prevent[P_SIMPLIFY], 1},
//     {"simplify-only-low-zooms", no_argument, &prevent[P_SIMPLIFY_LOW], 1},
//     {"simplification-at-maximum-zoom", required_argument, 0, '~'},
//     {"no-tiny-polygon-reduction", no_argument, &prevent[P_TINY_POLYGON_REDUCTION], 1},
//     {"no-tiny-polygon-reduction-at-maximum-zoom", no_argument, &prevent[P_TINY_POLYGON_REDUCTION_AT_MAXZOOM], 1},
//     {"tiny-polygon-size", required_argument, 0, '~'},
//     {"no-simplification-of-shared-nodes", no_argument, &prevent[P_SIMPLIFY_SHARED_NODES], 1},
//     {"visvalingam", no_argument, &additional[A_VISVALINGAM], 1},

//     {"Attempts to improve shared polygon boundaries", 0, 0, 0},
//     {"detect-shared-borders", no_argument, &additional[A_DETECT_SHARED_BORDERS], 1},
//     {"grid-low-zooms", no_argument, &additional[A_GRID_LOW_ZOOMS], 1},

//     {"Controlling clipping to tile boundaries", 0, 0, 0},
//     {"buffer", required_argument, 0, 'b'},
//     {"no-clipping", no_argument, &prevent[P_CLIPPING], 1},
//     {"no-duplication", no_argument, &prevent[P_DUPLICATION], 1},

//     {"Reordering features within each tile", 0, 0, 0},
//     {"preserve-input-order", no_argument, &prevent[P_INPUT_ORDER], 1},
//     {"reorder", no_argument, &additional[A_REORDER], 1},
//     {"coalesce", no_argument, &additional[A_COALESCE], 1},
//     {"reverse", no_argument, &additional[A_REVERSE], 1},
//     {"hilbert", no_argument, &additional[A_HILBERT], 1},
//     {"order-by", required_argument, 0, '~'},
//     {"order-descending-by", required_argument, 0, '~'},
//     {"order-smallest-first", no_argument, 0, '~'},
//     {"order-largest-first", no_argument, 0, '~'},

//     {"Adding calculated attributes", 0, 0, 0},
//     {"calculate-feature-density", no_argument, &additional[A_CALCULATE_FEATURE_DENSITY], 1},
//     {"generate-ids", no_argument, &additional[A_GENERATE_IDS], 1},
//     {"calculate-feature-index", no_argument, &additional[A_CALCULATE_INDEX], 1},

//     {"Trying to correct bad source geometry", 0, 0, 0},
//     {"detect-longitude-wraparound", no_argument, &additional[A_DETECT_WRAPAROUND], 1},
//     {"use-source-polygon-winding", no_argument, &prevent[P_USE_SOURCE_POLYGON_WINDING], 1},
//     {"reverse-source-polygon-winding", no_argument, &prevent[P_REVERSE_SOURCE_POLYGON_WINDING], 1},
//     {"clip-bounding-box", required_argument, 0, '~'},
//     {"convert-polygons-to-label-points", no_argument, &additional[A_GENERATE_POLYGON_LABEL_POINTS], 1},

//     {"Filtering tile contents", 0, 0, 0},
//     {"prefilter", required_argument, 0, 'C'},
//     {"postfilter", required_argument, 0, 'c'},

//     {"Setting or disabling tile size limits", 0, 0, 0},
//     {"maximum-tile-bytes", required_argument, 0, 'M'},
//     {"maximum-tile-features", required_argument, 0, 'O'},
//     {"limit-tile-feature-count", required_argument, 0, '~'},
//     {"limit-tile-feature-count-at-maximum-zoom", required_argument, 0, '~'},
//     {"no-feature-limit", no_argument, &prevent[P_FEATURE_LIMIT], 1},
//     {"no-tile-size-limit", no_argument, &prevent[P_KILOBYTE_LIMIT], 1},
//     {"no-tile-compression", no_argument, &prevent[P_TILE_COMPRESSION], 1},
//     {"no-tile-stats", no_argument, &prevent[P_TILE_STATS], 1},
//     {"tile-stats-attributes-limit", required_argument, 0, '~'},
//     {"tile-stats-sample-values-limit", required_argument, 0, '~'},
//     {"tile-stats-values-limit", required_argument, 0, '~'},

//     {"Temporary storage", 0, 0, 0},
//     {"temporary-directory", required_argument, 0, 't'},

//     {"Progress indicator", 0, 0, 0},
//     {"quiet", no_argument, 0, 'q'},
//     {"no-progress-indicator", no_argument, 0, 'Q'},
//     {"progress-interval", required_argument, 0, 'U'},
//     {"json-progress", no_argument, 0, 'u'},
//     {"version", no_argument, 0, 'v'},

//     {"", 0, 0, 0},
//     {"prevent", required_argument, 0, 'p'},
//     {"additional", required_argument, 0, 'a'},
//     {"check-polygons", no_argument, &additional[A_DEBUG_POLYGON], 1},
//     {"no-polygon-splitting", no_argument, &prevent[P_POLYGON_SPLIT], 1},
//     {"prefer-radix-sort", no_argument, &additional[A_PREFER_RADIX_SORT], 1},
//     {"help", no_argument, 0, 'H'},

//     {0, 0, 0, 0},
// };

// static struct option long_options[sizeof(long_options_orig) / sizeof(long_options_orig[0])];
// static char getopt_str[sizeof(long_options_orig) / sizeof(long_options_orig[0]) * 2 + 1];

// struct ProgramConfig {

// };

// class OptionParser {
// public:
//     ProgramConfig parse(int argc, char** argv);
// private:

// };

#endif
