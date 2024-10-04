// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstring>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

#define FOUNTAIN_WHITESPACE " \t\n\r\f\v"

std::string &ltrim_inplace(std::string &s, char const *t = FOUNTAIN_WHITESPACE);
std::string &rtrim_inplace(std::string &s, char const *t = FOUNTAIN_WHITESPACE);
std::string &trim_inplace(std::string &s, char const *t = FOUNTAIN_WHITESPACE);

std::string &replace_all_inplace(std::string &subject,
                                 const std::string &search,
                                 const std::string &replace);

std::string ws_ltrim(std::string s);
std::string ws_rtrim(std::string s);
std::string ws_trim(std::string s);

std::string replace_all(std::string subject, const std::string &search,
                        const std::string &replace);

bool begins_with(std::string const &input, std::string const &match);

std::vector<std::string> split_string(std::string const &str,
                                      std::string const &delimiter = " ");

std::vector<std::string> split_lines(std::string const &s);

std::string &to_upper_inplace(std::string &s);
std::string &to_lower_inplace(std::string &s);

std::string to_upper(std::string s);
std::string to_lower(std::string s);

bool is_upper(std::string const &s);

std::string &encode_entities_inplace(std::string &input,
                                     bool const bProcessAllEntities = false);
std::string encode_entities(std::string input,
                            bool const bProcessAllEntities = false);

std::string &decode_entities_inplace(std::string &input);
std::string decode_entities(std::string input);

std::string cstr_assign(char *input);

std::vector<std::string> cstrv_assign(char **input);
std::vector<std::string> cstrv_copy(char const *const *input);
std::vector<char *> cstrv_get(std::vector<std::string> const input);

std::string file_get_contents(std::string const &filename);

bool file_set_contents(std::string const &filename,
                       std::string const &contents);

void print_regex_error(std::regex_error &e, char const *file, int const line);
