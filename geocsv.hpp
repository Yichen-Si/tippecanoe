#ifndef GEOCSV_HPP
#define GEOCSV_HPP

#include <stdio.h>
#include <set>
#include <map>
#include <string>
#include "mbtiles.hpp"
#include "serial.hpp"

void parse_geocsv(std::vector<struct serialization_state> &sst, std::string fname, int layer, std::string layername);

struct csv_columns {
    ssize_t latcol;
    ssize_t loncol;
    std::vector<std::string> header;
    char delimiter = '\0';
};

bool parse_header(const std::string& header_line, csv_columns& cols);
void process_csv_line(const std::string& line, size_t seq, const csv_columns& cols, struct serialization_state& sst, int layer, const std::string& layername);
void parse_geocsv2(std::vector<struct serialization_state> &sst, std::string fname, int layer, std::string layername);

#endif
