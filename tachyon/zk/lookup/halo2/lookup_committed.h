// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_LOOKUP_HALO2_LOOKUP_COMMITTED_H_
#define TACHYON_ZK_LOOKUP_HALO2_LOOKUP_COMMITTED_H_

#include <utility>

#include "tachyon/zk/base/blinded_polynomial.h"

namespace tachyon::zk::lookup::halo2 {

template <typename Poly>
class LookupCommitted {
 public:
  using F = typename Poly::Field;

  LookupCommitted(BlindedPolynomial<Poly>&& permuted_input_poly,
                  BlindedPolynomial<Poly>&& permuted_table_poly,
                  BlindedPolynomial<Poly>&& product_poly)
      : permuted_input_poly_(std::move(permuted_input_poly)),
        permuted_table_poly_(std::move(permuted_table_poly)),
        product_poly_(std::move(product_poly)) {}

  const BlindedPolynomial<Poly>& permuted_input_poly() const {
    return permuted_input_poly_;
  }
  const BlindedPolynomial<Poly>& permuted_table_poly() const {
    return permuted_table_poly_;
  }
  const BlindedPolynomial<Poly>& product_poly() const { return product_poly_; }

  BlindedPolynomial<Poly>&& TakePermutedInputPoly() && {
    return std::move(permuted_input_poly_);
  }
  BlindedPolynomial<Poly>&& TakePermutedTablePoly() && {
    return std::move(permuted_table_poly_);
  }
  BlindedPolynomial<Poly>&& TakeProductPoly() && {
    return std::move(product_poly_);
  }

 private:
  BlindedPolynomial<Poly> permuted_input_poly_;
  BlindedPolynomial<Poly> permuted_table_poly_;
  BlindedPolynomial<Poly> product_poly_;
};

}  // namespace tachyon::zk::lookup::halo2

#endif  // TACHYON_ZK_LOOKUP_HALO2_LOOKUP_COMMITTED_H_
