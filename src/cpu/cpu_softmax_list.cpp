/*******************************************************************************
* Copyright 2019-2021 Intel Corporation
* Copyright 2021 FUJITSU LIMITED
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

#include "cpu/cpu_engine.hpp"

#include "cpu/ref_softmax.hpp"

#if DNNL_X64
#include "cpu/x64/jit_uni_softmax.hpp"
using namespace dnnl::impl::cpu::x64;
#elif DNNL_AARCH64
#include "cpu/aarch64/jit_uni_softmax.hpp"
using namespace dnnl::impl::cpu::aarch64;
#endif

namespace dnnl {
namespace impl {
namespace cpu {

namespace {
using namespace dnnl::impl::data_type;
using namespace dnnl::impl::prop_kind;

// clang-format off
const std::map<pk_impl_key_t, std::vector<impl_list_item_t>> impl_list_map REG_SOFTMAX_P({
    {{forward}, {
        CPU_INSTANCE_X64(jit_uni_softmax_fwd_t<avx512_common>)
        CPU_INSTANCE_X64(jit_uni_softmax_fwd_t<avx2>)
        CPU_INSTANCE_X64(jit_uni_softmax_fwd_t<sse41>)
        CPU_INSTANCE_AARCH64(jit_uni_softmax_fwd_t<sve_512>)
        CPU_INSTANCE_AARCH64(jit_uni_softmax_bwd_t<sve_512>)
        CPU_INSTANCE(ref_softmax_fwd_t<f32>)
        CPU_INSTANCE(ref_softmax_fwd_t<bf16>)
        nullptr,
    }},
    {{backward}, REG_BWD_PK({
        CPU_INSTANCE_X64(jit_uni_softmax_bwd_t<avx512_common>)
        CPU_INSTANCE(ref_softmax_bwd_t<f32>)
        CPU_INSTANCE(ref_softmax_bwd_t<bf16>)
        nullptr,
    })},
});
// clang-format on
} // namespace

const impl_list_item_t *get_softmax_impl_list(const softmax_desc_t *desc) {
    static const impl_list_item_t empty_list[] = {nullptr};

    const bool is_fwd = utils::one_of(
            desc->prop_kind, forward_training, forward_inference);
    prop_kind_t prop_kind = is_fwd ? forward : backward;

    pk_impl_key_t key {prop_kind};

    const auto impl_list_it = impl_list_map.find(key);
    return impl_list_it != impl_list_map.cend() ? impl_list_it->second.data()
                                                : empty_list;
}

const impl_list_item_t *get_logsoftmax_impl_list(
        const logsoftmax_desc_t *desc) {
    return get_softmax_impl_list(desc);
}

} // namespace cpu
} // namespace impl
} // namespace dnnl
