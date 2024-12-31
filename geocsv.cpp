#include <stdlib.h>
#include <algorithm>
#include "geocsv.hpp"
#include "mvt.hpp"
#include "serial.hpp"
#include "projection.hpp"
#include "main.hpp"
#include "text.hpp"
#include "csv.hpp"
#include "milo/dtoa_milo.h"
#include "options.hpp"
#include "errors.hpp"

// read & process one line from csv as a point
// 1 project lat/lon to world x/y and get spatial index (hilbert/quadkey)
// 2 create a mvt geometry and serialize to temporary files
bool parse_header(const std::string& header_line, csv_columns& cols) {
    cols.latcol = -1;
    cols.loncol = -1;
	int32_t find_x = 0, find_y = 0;
	if (header_line.size() <= 0) {
		return false;
	}
	// temporary delimiter detection
	if (cols.delimiter == '\0') {
		int32_t ncomma = 0, ntab = 0;
		for (size_t i = 0; i < header_line.size(); i++) {
			if (header_line[i] == ',') {
				ncomma++;
			} else if (header_line[i] == '\t') {
				ntab++;
			}
		}
		cols.delimiter = ncomma > ntab ? ',' : '\t';
	}
	std::string err = check_utf8(header_line);
	if (err != "") {
		fprintf(stderr, "Header UTF-8 error: %s\n", err.c_str());
		exit(EXIT_UTF8);
	}
	cols.header = csv_split(header_line.c_str(), cols.delimiter);
	for (size_t i = 0; i < cols.header.size(); i++) {
		cols.header[i] = csv_dequote(cols.header[i]);
		std::string lower(cols.header[i]);
		std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
		if (lower == "y" || lower == "lat" || (lower.find("latitude") != std::string::npos)) {
			cols.latcol = i;
			find_y++;
		}
		if (lower == "x" || lower == "lon" || lower == "lng" || lower == "long" || (lower.find("longitude") != std::string::npos)) {
			cols.loncol = i;
			find_x++;
		}
	}
    return find_x == 1 && find_y == 1;
}

void process_csv_line(const std::string& line, size_t seq, const csv_columns& cols, struct serialization_state& sst, int layer, const std::string& layername) {
    if (line.empty()) {
        return;
    }

    std::string err = check_utf8(line);
    if (err != "") {
        fprintf(stderr, "%s: %s\n", sst.fname, err.c_str());
        exit(EXIT_UTF8);
    }

    std::vector<std::string> fields = csv_split(line.c_str(), cols.delimiter);

    if (fields.size() != cols.header.size()) {
        fprintf(stderr, "%s:%zu: Mismatched column count: %zu in line, %zu in header\n", sst.fname, seq + 1, fields.size(), cols.header.size());
        exit(EXIT_CSV);
    }

    if (fields[cols.loncol].empty() || fields[cols.latcol].empty()) {
        static int warned = 0;
        if (!warned) {
            fprintf(stderr, "%s:%zu: null geometry (additional not reported)\n", sst.fname, seq + 1);
            warned = 1;
        }
        return;
    }
    double lon = atof(fields[cols.loncol].c_str());
    double lat = atof(fields[cols.latcol].c_str());
	long long x, y;
	if (global_bbox.initialized) {
		global_bbox.project2tile32(lon, lat, &x, &y);
	} else {
		projection->project(lon, lat, 32, &x, &y);
	}

    drawvec dv;
    dv.push_back(draw(VT_MOVETO, x, y));

    std::vector<std::shared_ptr<std::string>> full_keys;
    std::vector<serial_val> full_values;
    key_pool key_pool;

    // Process all columns except lat/lon
    for (size_t i = 0; i < fields.size(); i++) {
        if (i != (size_t)cols.latcol && i != (size_t)cols.loncol) {
            fields[i] = csv_dequote(fields[i]);

            serial_val sv;
            if (is_number(fields[i])) {
                sv.type = mvt_double;
            } else if (fields[i].size() == 0 && prevent[P_EMPTY_CSV_COLUMNS]) {
                sv.type = mvt_null;
                fields[i] = "null";
            } else {
                sv.type = mvt_string;
            }
            sv.s = fields[i];

            full_keys.push_back(key_pool.pool(cols.header[i]));
            full_values.push_back(sv);
        }
    }

    serial_feature sf;
    sf.layer = layer;
    sf.segment = sst.segment;
    sf.has_id = false;
    sf.id = 0;
    sf.tippecanoe_minzoom = -1;
    sf.tippecanoe_maxzoom = -1;
    sf.feature_minzoom = false;
    sf.seq = *(sst.layer_seq);
    sf.geometry = dv;
    sf.t = 1;  // POINT
    sf.full_keys = full_keys;
    sf.full_values = full_values;

    serialize_feature(&sst, sf, layername);
}

void parse_geocsv2(std::vector<struct serialization_state> &sst, std::string fname, int layer, std::string layername) {

    if (fname.size() == 0) {
        FILE* f = stdin;
        fprintf(stderr, "Warning: Parallel CSV processing not supported for stdin\n");
        // Read header
        std::string header_line = csv_getline(f);
        csv_columns cols;
		if (!parse_header(header_line, cols)) {
			fprintf(stderr, "Can't parse header\n");
			exit(EXIT_CSV);
		}
        if (cols.latcol < 0 || cols.loncol < 0) {
            fprintf(stderr, "Can't find \"lat\" and \"lon\" columns\n");
            exit(EXIT_CSV);
        }
        // Process lines serially for stdin
        size_t seq = 0;
        std::string line;
        while ((line = csv_getline(f)).size() > 0) {
            process_csv_line(line, seq++, cols, sst[0], layer, layername);
        }
        return;
    }

    FILE* f = fopen(fname.c_str(), "r");
    if (f == NULL) {
        perror(fname.c_str());
        exit(EXIT_OPEN);
    }

    // Read and parse header first
    std::string header_line = csv_getline(f);
    csv_columns cols;
	if (!parse_header(header_line, cols)) {
		fprintf(stderr, "Can't parse header\n");
		exit(EXIT_CSV);
	}
    if (cols.latcol < 0 || cols.loncol < 0) {
        fprintf(stderr, "%s: Can't find coordinate columns\n", fname.c_str());
        exit(EXIT_CSV);
    }

    // Get file size for chunking
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
	long header_end = header_line.length() + 1;
	long data_size = file_size - header_end;
	if (data_size <= 0) {
		fclose(f);
		return;
	}
    size_t num_threads = sst.size();
    long chunk_size = data_size / num_threads;

    // Structure to hold thread arguments
    struct thread_arg {
        long start_pos;
        long end_pos;
        const csv_columns* cols;
        struct serialization_state* sst;
        int layer;
        std::string layername;
        std::string filename;
    };

    // Thread function
    static auto thread_process_chunk = [](void* arg) -> void* {
        std::unique_ptr<thread_arg> targ(static_cast<thread_arg*>(arg));
        FILE* thread_file = fopen(targ->filename.c_str(), "r");
        if (thread_file == NULL) {
            perror("Thread fopen failed");
            return nullptr;
        }
        fseek(thread_file, targ->start_pos, SEEK_SET);
        size_t seq = targ->start_pos;

        while (ftell(thread_file) <= targ->end_pos) {
            std::string line = csv_getline(thread_file);
            if (line.empty()) {
                break;
            }
            process_csv_line(line, seq, *targ->cols, *targ->sst, targ->layer, targ->layername);
            seq++;
        }
        fclose(thread_file);
        return nullptr;
    };

    // Create and launch threads
    std::vector<pthread_t> threads(num_threads);
    std::vector<bool> thread_created(num_threads, false);

    fseek(f, header_end, SEEK_SET);
    for (size_t i = 0; i < num_threads; i++) {
        long start_pos;
        if (i == 0) {
            start_pos = header_end;
        } else {
            long chunk_start = header_end + i * chunk_size;
            fseek(f, chunk_start, SEEK_SET);
            std::string partial = csv_getline(f);  // partial line
            start_pos = ftell(f);
        }
        long end_pos = (i == num_threads - 1) ? file_size : header_end + (i + 1) * chunk_size;
        if (start_pos > end_pos) {
            continue;
        }

        auto arg = new thread_arg{start_pos, end_pos, &cols, &sst[i],
            layer, layername, fname};
        int ret = pthread_create(&threads[i], nullptr, thread_process_chunk, arg);
        if (ret != 0) {
            perror("pthread_create error");
            delete arg;
            exit(EXIT_PTHREAD);
        }
        thread_created[i] = true;
    }
	fclose(f);
	if (!quiet) {
		fprintf(stderr, "Created %zu threads for processing %s\n", num_threads, fname.c_str());
	}

    for (size_t i = 0; i < num_threads; i++) {
        if (thread_created[i]) {
            void* retval;
            if (pthread_join(threads[i], &retval) != 0) {
                perror("pthread_join");
                exit(EXIT_PTHREAD);
            }
        }
    }
	if (!quiet) {
		fprintf(stderr, "Finished processing %s\n", fname.c_str());
	}
}


// not parallelized
void parse_geocsv(std::vector<struct serialization_state> &sst, std::string fname, int layer, std::string layername) {
	FILE *f;

	if (fname.size() == 0) {
		f = stdin;
	} else {
		f = fopen(fname.c_str(), "r");
		if (f == NULL) {
			perror(fname.c_str());
			exit(EXIT_OPEN);
		}
	}

	// process header, find column index for lat/lon (y=lat, x=lon)
	std::string s;
	std::vector<std::string> header;
	ssize_t latcol = -1, loncol = -1;

	if ((s = csv_getline(f)).size() > 0) {
		std::string err = check_utf8(s);
		if (err != "") {
			fprintf(stderr, "%s: %s\n", fname.c_str(), err.c_str());
			exit(EXIT_UTF8);
		}

		header = csv_split(s.c_str());

		for (size_t i = 0; i < header.size(); i++) {
			header[i] = csv_dequote(header[i]);

			std::string lower(header[i]);
			std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

			if (lower == "y" || lower == "lat" || (lower.find("latitude") != std::string::npos)) {
				latcol = i;
			}
			if (lower == "x" || lower == "lon" || lower == "lng" || lower == "long" || (lower.find("longitude") != std::string::npos)) {
				loncol = i;
			}
		}
	}

	if (latcol < 0 || loncol < 0) {
		fprintf(stderr, "%s: Can't find \"lat\" and \"lon\" columns\n", fname.c_str());
		exit(EXIT_CSV);
	}

	// read & process each line from csv as a point
	// 1 project lat/lon to world x/y and get spatial index (hilbert/quadkey)
	// 2 create a mvt geometry and serialize to temporary files
	size_t seq = 0;
	key_pool key_pool;
	while ((s = csv_getline(f)).size() > 0) {
		std::string err = check_utf8(s);
		if (err != "") {
			fprintf(stderr, "%s: %s\n", fname.c_str(), err.c_str());
			exit(EXIT_UTF8);
		}

		seq++;
		std::vector<std::string> line = csv_split(s.c_str());

		if (line.size() != header.size()) {
			fprintf(stderr, "%s:%zu: Mismatched column count: %zu in line, %zu in header\n", fname.c_str(), seq + 1, line.size(), header.size());
			exit(EXIT_CSV);
		}

		if (line[loncol].empty() || line[latcol].empty()) {
			static int warned = 0;
			if (!warned) {
				fprintf(stderr, "%s:%zu: null geometry (additional not reported)\n", fname.c_str(), seq + 1);
				warned = 1;
			}
			continue;
		}
		double lon = atof(line[loncol].c_str());
		double lat = atof(line[latcol].c_str());

		long long x, y;
		projection->project(lon, lat, 32, &x, &y); // project to tile coord
		drawvec dv;
		dv.push_back(draw(VT_MOVETO, x, y)); // mvt geometry command: MoveTo

		std::vector<std::shared_ptr<std::string>> full_keys; // keys (hreaders\{lat, lon})
		std::vector<serial_val> full_values; // attribute values

		// read all other columns as double/string attributes
		// problems: 1 every value has a separate "type" even if all values from one column would normally have the same type
		// 2 integers are stored as doubles? (from mvt spec values chould be integer)
		for (size_t i = 0; i < line.size(); i++) {
			if (i != (size_t) latcol && i != (size_t) loncol) {
				line[i] = csv_dequote(line[i]);

				serial_val sv;
				if (is_number(line[i])) {
					sv.type = mvt_double;
				} else if (line[i].size() == 0 && prevent[P_EMPTY_CSV_COLUMNS]) {
					sv.type = mvt_null;
					line[i] = "null";
				} else {
					sv.type = mvt_string;
				}
				sv.s = line[i];

				full_keys.push_back(key_pool.pool(header[i])); // add key
				full_values.push_back(sv); // add value (not unique)
			}
		}

		serial_feature sf;

		sf.layer = layer;
		sf.segment = sst[0].segment; // 0
		sf.has_id = false;
		sf.id = 0;
		sf.tippecanoe_minzoom = -1;
		sf.tippecanoe_maxzoom = -1;
		sf.feature_minzoom = false;
		sf.seq = *(sst[0].layer_seq);
		sf.geometry = dv;
		sf.t = 1;  // POINT
		sf.full_keys = full_keys;
		sf.full_values = full_values;

		// process the geometry feature, write to temporary files
		serialize_feature(&sst[0], sf, layername);
	}

	if (fname.size() != 0) {
		if (fclose(f) != 0) {
			perror("fclose");
			exit(EXIT_CLOSE);
		}
	}
}
