#ifndef TACHYON_MATH_BASE_FIELD_H_
#define TACHYON_MATH_BASE_FIELD_H_

#include <ostream>
#include <utility>

#include "tachyon/math/base/ring.h"

namespace tachyon::math {

// Field is any set of elements that satisfies the field axioms for both
// addition and multiplication and is commutative division algebra.
// Simply put, a field is a ring in which multiplicative commutativity exists,
// and every non-zero element has a multiplicative inverse.
// See https://mathworld.wolfram.com/Field.html

// The Field supports SumOfProducts and BatchInverse, inheriting the properties
// of both AdditiveGroup and MultiplicativeGroup.
template <typename F>
class Field : public AdditiveGroup<F>, public MultiplicativeGroup<F> {
 public:
  // Sum of products: a₁ * b₁ + a₂ * b₂ + ... + aₙ * bₙ
  template <typename ContainerA, typename ContainerB>
  constexpr static F SumOfProducts(const ContainerA& a, const ContainerB& b) {
    return Ring<F>::SumOfProducts(a, b);
  }

  // Sum of products: a₁ * b₁ + a₂ * b₂ + ... + aₙ * bₙ
  template <typename ContainerA, typename ContainerB>
  constexpr static F SumOfProductsSerial(const ContainerA& a,
                                         const ContainerB& b) {
    return Ring<F>::SumOfProductsSerial(a, b);
  }
};

template <typename F>
std::ostream& operator<<(std::ostream& os, const Field<F>& f) {
  const F& derived = static_cast<const F&>(f);
  return os << derived.ToString();
}

}  // namespace tachyon::math

#endif  // TACHYON_MATH_BASE_FIELD_H_
