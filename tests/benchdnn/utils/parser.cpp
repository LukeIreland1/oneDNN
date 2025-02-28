/*******************************************************************************
* Copyright 2019-2021 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "utils/parser.hpp"

#include "dnnl_common.hpp"

namespace parser {

bool last_parsed_is_problem = false;

// vector types
bool parse_dir(std::vector<dir_t> &dir, const std::vector<dir_t> &def_dir,
        const char *str, const std::string &option_name /* = "dir"*/) {
    return parse_vector_option(dir, def_dir, str2dir, str, option_name);
}

bool parse_dt(std::vector<dnnl_data_type_t> &dt,
        const std::vector<dnnl_data_type_t> &def_dt, const char *str,
        const std::string &option_name /* = "dt"*/) {
    return parse_vector_option(dt, def_dt, str2dt, str, option_name);
}

bool parse_multi_dt(std::vector<std::vector<dnnl_data_type_t>> &dt,
        const std::vector<std::vector<dnnl_data_type_t>> &def_dt,
        const char *str, const std::string &option_name /* = "sdt"*/) {
    return parse_multivector_option(dt, def_dt, str2dt, str, option_name);
}

bool parse_tag(std::vector<std::string> &tag,
        const std::vector<std::string> &def_tag, const char *str,
        const std::string &option_name /* = "tag"*/) {
    auto ret_string = [](const char *str) { return std::string(str); };
    bool ok = parse_vector_option(tag, def_tag, ret_string, str, option_name);
    if (!ok) return false;

    for (size_t i = 0; i < tag.size(); i++) {
        if (check_tag(tag[i], allow_enum_tags_only) != OK) {
            if (allow_enum_tags_only && check_tag(tag[i]) == OK) {
                fprintf(stderr,
                        "ERROR: tag `%s` is valid but not found in "
                        "`dnnl::memory::format_tag`. To force the testing with "
                        "this tag, please specify `--allow-enum-tags-only=0` "
                        "prior to any tag option.\n",
                        tag[i].c_str());
            } else {
                fprintf(stderr,
                        "ERROR: unknown or invalid tag: `%s`, exiting...\n",
                        tag[i].c_str());
            }
            exit(2);
        }
    }
    return true;
}

bool parse_multi_tag(std::vector<std::vector<std::string>> &tag,
        const std::vector<std::vector<std::string>> &def_tag, const char *str,
        const std::string &option_name /* = "stag"*/) {
    auto ret_string = [](const char *str) { return std::string(str); };
    return parse_multivector_option(tag, def_tag, ret_string, str, option_name);
}

bool parse_mb(std::vector<int64_t> &mb, const std::vector<int64_t> &def_mb,
        const char *str, const std::string &option_name /* = "mb"*/) {
    return parse_vector_option(mb, def_mb, atoi, str, option_name);
}

bool parse_attr(attr_t &attr, const char *str,
        const std::string &option_name /* = "attr"*/) {
    const std::string pattern = get_pattern(option_name);
    if (pattern.find(str, 0, pattern.size()) == eol) return false;
    static bool notice_printed = false;
    if (!notice_printed) {
        BENCHDNN_PRINT(0, "%s\n",
                "WARNING (DEPRECATION NOTICE): `--attr` option is deprecated. "
                "Please use one of `--attr-oscale`, `--attr-post-ops`, "
                "`--attr-scales` or `--attr-zero-points` to specify attributes "
                "specific part for a problem. New options support mixing and "
                "will iterate over all possible combinations.");
        notice_printed = true;
    }
    SAFE(str2attr(&attr, str + pattern.size()), CRIT);
    return true;
}

bool parse_attr_oscale(std::vector<attr_t::scale_t> &oscale, const char *str,
        const std::string &option_name /* = "attr-oscale"*/) {
    return parse_subattr(oscale, str, option_name);
}

bool parse_attr_post_ops(std::vector<attr_t::post_ops_t> &po, const char *str,
        const std::string &option_name /* = "attr-post-ops"*/) {
    return parse_subattr(po, str, option_name);
}

bool parse_attr_scales(std::vector<attr_t::arg_scales_t> &scales,
        const char *str, const std::string &option_name /* = "attr-scales"*/) {
    return parse_subattr(scales, str, option_name);
}

bool parse_attr_zero_points(std::vector<attr_t::zero_points_t> &zp,
        const char *str,
        const std::string &option_name /* = "attr-zero-points"*/) {
    return parse_subattr(zp, str, option_name);
}

bool parse_attr_scratchpad_mode(
        std::vector<dnnl_scratchpad_mode_t> &scratchpad_mode,
        const std::vector<dnnl_scratchpad_mode_t> &def_scratchpad_mode,
        const char *str,
        const std::string &option_name /* = "attr-scratchpad"*/) {
    return parse_vector_option(scratchpad_mode, def_scratchpad_mode,
            str2scratchpad_mode, str, option_name);
}

bool parse_axis(std::vector<int> &axis, const std::vector<int> &def_axis,
        const char *str, const std::string &option_name /* = "axis"*/) {
    return parse_vector_option(axis, def_axis, atoi, str, option_name);
}

bool parse_test_pattern_match(const char *&match, const char *str,
        const std::string &option_name /* = "match"*/) {
    const std::string pattern = get_pattern(option_name);
    if (pattern.find(str, 0, pattern.size()) != eol) {
        match = str + pattern.size();
        return true;
    }
    return false;
}

bool parse_inplace(std::vector<bool> &inplace,
        const std::vector<bool> &def_inplace, const char *str,
        const std::string &option_name /* = "inplace"*/) {
    return parse_vector_option(
            inplace, def_inplace, str2bool, str, option_name);
}

bool parse_skip_nonlinear(std::vector<bool> &skip,
        const std::vector<bool> &def_skip, const char *str,
        const std::string &option_name /* = "skip-nonlinear"*/) {
    return parse_vector_option(skip, def_skip, str2bool, str, option_name);
}

bool parse_strides(std::vector<vdims_t> &strides,
        const std::vector<vdims_t> &def_strides, const char *str,
        const std::string &option_name /* = "strides"*/) {
    auto str2strides = [&](const char *str) -> vdims_t {
        vdims_t strides(STRIDES_SIZE);
        parse_multivector_str(strides, vdims_t(), atoi, str, ':', 'x');
        return strides;
    };
    return parse_vector_option(
            strides, def_strides, str2strides, str, option_name);
}

bool parse_trivial_strides(std::vector<bool> &ts,
        const std::vector<bool> &def_ts, const char *str,
        const std::string &option_name /* = "trivial-strides"*/) {
    return parse_vector_option(ts, def_ts, str2bool, str, option_name);
}

bool parse_scale_policy(std::vector<policy_t> &policy,
        const std::vector<policy_t> &def_policy, const char *str,
        const std::string &option_name /*= "scaling"*/) {
    return parse_vector_option(
            policy, def_policy, attr_t::str2policy, str, option_name);
}

// plain types
bool parse_perf_template(const char *&pt, const char *pt_def,
        const char *pt_csv, const char *str,
        const std::string &option_name /* = "perf-template"*/) {
    const std::string pattern = get_pattern(option_name);
    if (pattern.find(str, 0, pattern.size()) != eol) {
        const std::string csv_pattern = "csv";
        const std::string def_pattern = "def";
        str += pattern.size();
        if (csv_pattern.find(str, 0, csv_pattern.size()) != eol)
            pt = pt_csv;
        else if (def_pattern.find(str, 0, def_pattern.size()) != eol)
            pt = pt_def;
        else
            pt = str;
        return true;
    }
    return false;
}

bool parse_batch(const bench_f bench, const char *str,
        const std::string &option_name /* = "batch"*/) {
    const std::string pattern = get_pattern(option_name);
    if (pattern.find(str, 0, pattern.size()) != eol) {
        SAFE(batch(str + pattern.size(), bench), CRIT);
        return true;
    }
    return false;
}

// prb_dims_t type
void parse_prb_vdims(prb_vdims_t &prb_vdims, const std::string &str) {
    size_t start_pos = 0;
    // `n` is an indicator for a name supplied with dims_t object.
    std::string vdims_str = get_substr(str, start_pos, 'n');
    parse_multivector_str(
            prb_vdims.vdims, {dims_t()}, atoi, vdims_str, ':', 'x');

    prb_vdims.ndims = static_cast<int>(prb_vdims.vdims[0].size());
    // If second and consecutive inputs are provided with less dimensions
    // (ndims0 > ndims1), then fill these tensors with ones to match ndims,
    // e.g., 8x3x5:8 -> 8x3x5:8x1x1
    // We put this implicit broadcast feature on parser since `ndims` would be
    // set next and can't be updated by driver, but `ndims` mismatch triggers
    // the library `invalid_arguments` error.
    for (int i = 1; i < prb_vdims.n_inputs(); i++)
        if (prb_vdims.ndims > static_cast<int>(prb_vdims.vdims[i].size()))
            prb_vdims.vdims[i].resize(prb_vdims.ndims, 1);

    prb_vdims.dst_dims = prb_vdims.vdims[0];
    for (int i = 1; i < prb_vdims.n_inputs(); i++)
        for (int d = 0; d < prb_vdims.ndims; ++d)
            prb_vdims.dst_dims[d]
                    = std::max(prb_vdims.dst_dims[d], prb_vdims.vdims[i][d]);

    if (start_pos != eol) prb_vdims.name = str.substr(start_pos);
}

void parse_prb_dims(prb_dims_t &prb_dims, const std::string &str) {
    size_t start_pos = 0;
    // `n` is an indicator for a name supplied with dims_t object.
    std::string dims_str = get_substr(str, start_pos, 'n');
    parse_vector_str(prb_dims.dims, dims_t(), atoi, dims_str, 'x');

    prb_dims.ndims = static_cast<int>(prb_dims.dims.size());

    if (start_pos != eol) prb_dims.name = str.substr(start_pos);
}

// service functions
static bool parse_bench_mode(
        const char *str, const std::string &option_name = "mode") {
    const auto str2bench_mode = [](const std::string &_str) {
        bench_mode_t mode;
        for (size_t i = 0; i < _str.size(); i++) {
            if (_str[i] == 'r' || _str[i] == 'R') mode |= RUN;
            if (_str[i] == 'c' || _str[i] == 'C') mode |= CORR;
            if (_str[i] == 'p' || _str[i] == 'P') mode |= PERF;
            if (_str[i] == 'l' || _str[i] == 'L') mode |= LIST;
        }
        if (!(mode & LIST).none() && mode.count() > 1) {
            fprintf(stderr,
                    "ERROR: LIST mode is incompatible with any other modes. "
                    "Please use just `--mode=L` instead.\n");
            exit(2);
        }
        if (mode.none()) {
            fprintf(stderr, "ERROR: empty mode is not allowed.\n");
            exit(2);
        }

        return mode;
    };

    return parse_single_value_option(
            bench_mode, CORR, str2bench_mode, str, option_name);
}

static bool parse_max_ms_per_prb(
        const char *str, const std::string &option_name = "max-ms-per-prb") {
    if (parse_single_value_option(max_ms_per_prb, 3e3, atof, str, option_name))
        return max_ms_per_prb = MAX2(100, MIN2(max_ms_per_prb, 60e3)), true;
    return false;
}

static bool parse_fix_times_per_prb(
        const char *str, const std::string &option_name = "fix-times-per-prb") {
    if (parse_single_value_option(fix_times_per_prb, 0, atoi, str, option_name))
        return fix_times_per_prb = MAX2(0, fix_times_per_prb), true;
    return false;
}

static bool parse_verbose(
        const char *str, const std::string &option_name = "verbose") {
    const std::string pattern("-v"); // check short option first
    if (pattern.find(str, 0, pattern.size()) != eol) {
        verbose = atoi(str + pattern.size());
        return true;
    }
    return parse_single_value_option(verbose, 0, atoi, str, option_name);
}

static bool parse_engine(
        const char *str, const std::string &option_name = "engine") {
    if (!parse_single_value_option(
                engine_tgt_kind, dnnl_cpu, str2engine_kind, str, option_name))
        return false;
    // Parse engine index if present
    std::string s(str);
    auto start_pos = s.find_first_of(':');
    if (start_pos != eol) engine_index = stoi(s.substr(start_pos + 1));

    auto n_devices = dnnl_engine_get_count(engine_tgt_kind);
    if (engine_index >= n_devices) {
        fprintf(stderr,
                "ERROR: requested engine with index %ld is not registered in "
                "the system. Number of devices registered is %ld.\n",
                (long)engine_index, (long)n_devices);
        exit(2);
    }
    return true;
}

static bool parse_fast_ref_gpu(
        const char *str, const std::string &option_name = "fast-ref-gpu") {
    bool parsed = parse_single_value_option(
            fast_ref_gpu, true, str2bool, str, option_name);
#if DNNL_CPU_RUNTIME == DNNL_RUNTIME_NONE
    if (parsed && fast_ref_gpu) {
        fast_ref_gpu = false;
        fprintf(stderr,
                "%s driver: WARNING: option `fast_ref_gpu` is not supported "
                "for GPU only configurations.\n",
                driver_name);
    }
#endif
    return parsed;
}

static bool parse_canonical(
        const char *str, const std::string &option_name = "canonical") {
    return parse_single_value_option(
            canonical, false, str2bool, str, option_name);
}

static bool parse_mem_check(
        const char *str, const std::string &option_name = "mem-check") {
    return parse_single_value_option(
            mem_check, true, str2bool, str, option_name);
}

static bool parse_skip_impl(
        const char *str, const std::string &option_name = "skip-impl") {
    const std::string pattern = get_pattern(option_name);
    if (pattern.find(str, 0, pattern.size()) == eol) return false;

    skip_impl = std::string(str + pattern.size());
    // Remove all quotes from input string since they affect the search.
    for (auto c : {'"', '\''}) {
        size_t start_pos = 0;
        while (start_pos != eol) {
            start_pos = skip_impl.find_first_of(c, start_pos);
            if (start_pos != eol) skip_impl.erase(start_pos, 1);
        }
    }
    return true;
}

static bool parse_allow_enum_tags_only(const char *str,
        const std::string &option_name = "allow-enum-tags-only") {
    return parse_single_value_option(
            allow_enum_tags_only, true, str2bool, str, option_name);
}

static bool parse_cpu_isa_hints(
        const char *str, const std::string &option_name = "cpu-isa-hints") {
    const bool parsed
            = parse_single_value_option(hints, isa_hints_t {isa_hints_t::none},
                    isa_hints_t::str2hints, str, option_name);
    if (parsed) init_isa_settings();
    return parsed;
}

static bool parse_memory_kind(
        const char *str, const std::string &option_name = "memory-kind") {
    const bool parsed = parse_single_value_option(memory_kind,
            default_memory_kind, str2memory_kind, str, option_name);

    if (!parsed) {
        const bool parsed_old_style = parse_single_value_option(memory_kind,
                default_memory_kind, str2memory_kind, str, "sycl-memory-kind");
        if (!parsed_old_style) return false;
    }

#if !defined(DNNL_WITH_SYCL) && DNNL_GPU_RUNTIME != DNNL_RUNTIME_OCL
    fprintf(stderr,
            "ERROR: option `%s` is supported with DPC++ and OpenCL builds "
            "only, "
            "exiting...\n",
            option_name.c_str());
    exit(2);
#endif
    return true;
}

static bool parse_test_start(
        const char *str, const std::string &option_name = "start") {
    return parse_single_value_option(
            test_start, 0, [](const std::string &s) { return std::stoi(s); },
            str, option_name);
}

static bool parse_attr_same_pd_check(const char *str,
        const std::string &option_name = "attr-same-pd-check") {
    return parse_single_value_option(
            attr_same_pd_check, false, str2bool, str, option_name);
}

bool parse_bench_settings(const char *str) {
    last_parsed_is_problem = false; // if start parsing, expect an option

    return parse_bench_mode(str) || parse_max_ms_per_prb(str)
            || parse_fix_times_per_prb(str) || parse_verbose(str)
            || parse_engine(str) || parse_fast_ref_gpu(str)
            || parse_canonical(str) || parse_mem_check(str)
            || parse_skip_impl(str) || parse_allow_enum_tags_only(str)
            || parse_cpu_isa_hints(str) || parse_memory_kind(str)
            || parse_test_start(str) || parse_attr_same_pd_check(str);
}

void catch_unknown_options(const char *str) {
    last_parsed_is_problem = true; // if reached, means problem parsing

    const std::string pattern = "--";
    if (pattern.find(str, 0, pattern.size()) != eol) {
        fprintf(stderr, "%s driver: ERROR: unknown option: `%s`, exiting...\n",
                driver_name, str);
        exit(2);
    }
}

int parse_last_argument() {
    if (!last_parsed_is_problem)
        fprintf(stderr,
                "%s driver: WARNING: No problem found for a given option!\n",
                driver_name);
    return OK;
}

std::string get_substr(const std::string &s, size_t &start_pos, char delim) {
    auto end_pos = s.find_first_of(delim, start_pos);
    auto sub = s.substr(start_pos, end_pos - start_pos);
    start_pos = end_pos + (end_pos != eol);
    return sub;
}

} // namespace parser
