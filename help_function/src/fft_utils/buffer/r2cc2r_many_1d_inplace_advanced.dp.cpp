// ===--- r2cc2r_many_1d_inplace_advanced.dp.cpp --------------*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===---------------------------------------------------------------------===//

#define DPCT_USM_LEVEL_NONE
#include <sycl/sycl.hpp>
#include <dpct/dpct.hpp>
#include <oneapi/mkl.hpp>
#include <dpct/fft_utils.hpp>
#include "common.h"
#include <cstring>
#include <iostream>


// forward
// input
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | r | 0 | r | 0 | r | 0 | r | 0 | r | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | r | 0 | r | 0 | r | 0 | r | 0 | r | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |___________________n___________________|                               |___________________n___________________|                               |
// |_________________nembed________________|                               |_________________nembed________________|                               |
// |___________________________________batch0______________________________|___________________________________batch1______________________________|
// output
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |   c   |   0   |   c   |   0   |   c   |   0   |   0   |   0   |   0   |   c   |   0   |   c   |   0   |   c   |   0   |   0   |   0   |   0   |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |________________________n______________________|               |       |________________________n______________________|               |       |
// |_____________________________nembed____________________________|       |_____________________________nembed____________________________|       |
// |___________________________________batch0______________________________|___________________________________batch1______________________________|
bool r2cc2r_many_1d_inplace_advanced() {
  dpct::device_ext &dev_ct1 = dpct::get_current_device();
  sycl::queue &q_ct1 = dev_ct1.default_queue();
  dpct::fft::fft_engine *plan_fwd;
  plan_fwd = dpct::fft::fft_engine::create();
  float forward_idata_h[36];
  std::memset(forward_idata_h, 0, sizeof(float) * 36);
  forward_idata_h[0] = 0;
  forward_idata_h[2] = 1;
  forward_idata_h[4] = 2;
  forward_idata_h[6] = 3;
  forward_idata_h[8] = 4;
  forward_idata_h[18] = 0;
  forward_idata_h[20] = 1;
  forward_idata_h[22] = 2;
  forward_idata_h[24] = 3;
  forward_idata_h[26] = 4;

  float* data_d;
  data_d = (float *)dpct::dpct_malloc(sizeof(float) * 36);
  dpct::dpct_memcpy(data_d, forward_idata_h, sizeof(float) * 36,
                    dpct::host_to_device);

  size_t workSize;
  long long int n[1] = {5};
  long long int inembed[1] = {5};
  long long int onembed[1] = {4};
  plan_fwd->commit(&q_ct1, 1, n, inembed, 2, 18, onembed, 2, 9,
                   dpct::fft::fft_type::real_float_to_complex_float, 2,
                   nullptr);
  plan_fwd->compute<float, sycl::float2>(data_d, (sycl::float2 *)data_d,
                                         dpct::fft::fft_direction::forward);
  dev_ct1.queues_wait_and_throw();
  sycl::float2 forward_odata_h[18];
  dpct::dpct_memcpy(forward_odata_h, data_d, sizeof(float) * 36,
                    dpct::device_to_host);

  sycl::float2 forward_odata_ref[18];
  forward_odata_ref[0] = sycl::float2{10, 0};
  forward_odata_ref[1] = sycl::float2{2, 3};
  forward_odata_ref[2] = sycl::float2{-2.5, 3.44095};
  forward_odata_ref[3] = sycl::float2{1, 2};
  forward_odata_ref[4] = sycl::float2{-2.5, 0.812299};
  forward_odata_ref[5] = sycl::float2{0, 0};
  forward_odata_ref[6] = sycl::float2{0, 0};
  forward_odata_ref[7] = sycl::float2{0, 0};
  forward_odata_ref[8] = sycl::float2{0, 0};
  forward_odata_ref[9] = sycl::float2{10, 0};
  forward_odata_ref[10] = sycl::float2{0, 0};
  forward_odata_ref[11] = sycl::float2{-2.5, 3.44095};
  forward_odata_ref[12] = sycl::float2{0, 0};
  forward_odata_ref[13] = sycl::float2{-2.5, 0.812299};
  forward_odata_ref[14] = sycl::float2{0, 0};
  forward_odata_ref[15] = sycl::float2{0, 0};
  forward_odata_ref[16] = sycl::float2{0, 0};
  forward_odata_ref[17] = sycl::float2{0, 0};

  dpct::fft::fft_engine::destroy(plan_fwd);

  std::vector<int> indices = {0, 2, 4,
                              9, 11, 13};
  if (!compare(forward_odata_ref, forward_odata_h, indices)) {
    std::cout << "forward_odata_h:" << std::endl;
    print_values(forward_odata_h, indices);
    std::cout << "forward_odata_ref:" << std::endl;
    print_values(forward_odata_ref, indices);

    dpct::dpct_free(data_d);

    return false;
  }

  dpct::fft::fft_engine *plan_bwd;
  plan_bwd = dpct::fft::fft_engine::create();
  plan_bwd->commit(&q_ct1, 1, n, onembed, 2, 9, inembed, 2, 18,
                   dpct::fft::fft_type::complex_float_to_real_float, 2,
                   nullptr);
  plan_bwd->compute<sycl::float2, float>((sycl::float2 *)data_d, data_d,
                                         dpct::fft::fft_direction::backward);
  dev_ct1.queues_wait_and_throw();
  float backward_odata_h[36];
  dpct::dpct_memcpy(backward_odata_h, data_d, sizeof(float) * 36,
                    dpct::device_to_host);

  float backward_odata_ref[36];
  backward_odata_ref[0] = 0;
  backward_odata_ref[2] = 5;
  backward_odata_ref[4] = 10;
  backward_odata_ref[6] = 15;
  backward_odata_ref[8] = 20;
  backward_odata_ref[18] = 0;
  backward_odata_ref[20] = 5;
  backward_odata_ref[22] = 10;
  backward_odata_ref[24] = 15;
  backward_odata_ref[26] = 20;

  dpct::dpct_free(data_d);
  dpct::fft::fft_engine::destroy(plan_bwd);

  std::vector<int> indices_bwd = {0, 2, 4, 6, 8,
                                  18, 20, 22, 24, 26};
  if (!compare(backward_odata_ref, backward_odata_h, indices_bwd)) {
    std::cout << "backward_odata_h:" << std::endl;
    print_values(backward_odata_h, indices_bwd);
    std::cout << "backward_odata_ref:" << std::endl;
    print_values(backward_odata_ref, indices_bwd);
    return false;
  }
  return true;
}


#ifdef DEBUG_FFT
int main() {
#define FUNC r2cc2r_many_1d_inplace_advanced
  bool res = FUNC();
  cudaDeviceSynchronize();
  if (!res) {
    std::cout << "Fail" << std::endl;
    return -1;
  }
  std::cout << "Pass" << std::endl;
  return 0;
}
#endif

