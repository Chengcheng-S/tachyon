// Copyright (c) 2022 The Plonky3 Authors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.plonky3 and the LICENCE-APACHE.plonky3
// file.

#ifndef TACHYON_CRYPTO_COMMITMENTS_MERKLE_TREE_FIELD_MERKLE_TREE_FIELD_MERKLE_TREE_MMCS_H_
#define TACHYON_CRYPTO_COMMITMENTS_MERKLE_TREE_FIELD_MERKLE_TREE_FIELD_MERKLE_TREE_MMCS_H_

#include <type_traits>
#include <utility>
#include <vector>

#include "absl/numeric/bits.h"
#include "third_party/pdqsort/include/pdqsort.h"

#include "tachyon/base/bits.h"
#include "tachyon/base/containers/container_util.h"
#include "tachyon/crypto/commitments/merkle_tree/field_merkle_tree/field_merkle_tree.h"
#include "tachyon/crypto/commitments/mixed_matrix_commitment_scheme.h"
#include "tachyon/math/finite_fields/extension_field_traits_forward.h"
#include "tachyon/math/finite_fields/finite_field_traits.h"

namespace tachyon::crypto {

template <typename F, typename Hasher, typename PackedHasher,
          typename Compressor, typename PackedCompressor, size_t N>
class FieldMerkleTreeMMCS final
    : public MixedMatrixCommitmentScheme<FieldMerkleTreeMMCS<
          F, Hasher, PackedHasher, Compressor, PackedCompressor, N>> {
 public:
  using PrimeField =
      std::conditional_t<math::FiniteFieldTraits<F>::kIsExtensionField,
                         typename math::ExtensionFieldTraits<F>::BasePrimeField,
                         F>;
  using Commitment = std::array<PrimeField, N>;
  using Digest = Commitment;
  using Proof = std::vector<Digest>;

  FieldMerkleTreeMMCS(const Hasher& hasher, const PackedHasher& packed_hasher,
                      const Compressor& compressor,
                      const PackedCompressor& packed_compressor)
      : hasher_(hasher),
        packed_hasher_(packed_hasher),
        compressor_(compressor),
        packed_compressor_(packed_compressor) {}
  FieldMerkleTreeMMCS(Hasher&& hasher, PackedHasher&& packed_hasher,
                      Compressor&& compressor,
                      PackedCompressor&& packed_compressor)
      : hasher_(std::move(hasher)),
        packed_hasher_(std::move(packed_hasher)),
        compressor_(std::move(compressor)),
        packed_compressor_(std::move(packed_compressor)) {}

  const FieldMerkleTree<F, N>& field_merkle_tree() const {
    return field_merkle_tree_;
  }
  const Hasher& hasher() const { return hasher_; }
  const PackedHasher& packed_hasher() const { return packed_hasher_; }
  const Compressor& compressor() const { return compressor_; }
  const PackedCompressor& packed_compressor() const {
    return packed_compressor_;
  }

 private:
  friend class MixedMatrixCommitmentScheme<FieldMerkleTreeMMCS<
      F, Hasher, PackedHasher, Compressor, PackedCompressor, N>>;

  [[nodiscard]] bool DoCommit(std::vector<math::RowMajorMatrix<F>>&& matrices,
                              Commitment* result) {
    field_merkle_tree_ =
        FieldMerkleTree<F, N>::Build(hasher_, packed_hasher_, compressor_,
                                     packed_compressor_, std::move(matrices));
    *result = field_merkle_tree_.GetRoot();
    return true;
  }

  const std::vector<math::RowMajorMatrix<F>>& DoGetMatrices() const {
    return field_merkle_tree_.leaves();
  }

  [[nodiscard]] bool DoCreateOpeningProof(size_t index,
                                          std::vector<std::vector<F>>* openings,
                                          Proof* proof) const {
    size_t max_row_size = this->GetMaxRowSize();
    size_t log_max_row_size = base::bits::Log2Ceiling(max_row_size);

    // TODO(chokobole): Is it able to be parallelized?
    *openings = base::Map(
        field_merkle_tree_.leaves(),
        [log_max_row_size, index](const math::RowMajorMatrix<F>& matrix) {
          size_t log_row_size =
              base::bits::Log2Ceiling(static_cast<size_t>(matrix.rows()));
          size_t bits_reduced = log_max_row_size - log_row_size;
          size_t reduced_index = index >> bits_reduced;
          return base::CreateVector(matrix.cols(),
                                    [reduced_index, &matrix](size_t col) {
                                      return matrix(reduced_index, col);
                                    });
        });

    *proof = base::CreateVector(log_max_row_size, [this, index](size_t i) {
      // NOTE(chokobole): Let v be |index >> i|. If v is even, v ^ 1 is v + 1.
      // Otherwise, v ^ 1 is v - 1.
      return field_merkle_tree_.digest_layers()[i][(index >> i) ^ 1];
    });

    return true;
  }

  [[nodiscard]] bool DoVerifyOpeningProof(
      const Commitment& commitment,
      absl::Span<const math::Dimensions> dimensions_list, size_t index,
      absl::Span<const std::vector<F>> opened_values,
      const Proof& proof) const {
    std::vector<math::Dimensions> sorted_dimensions_list(
        dimensions_list.begin(), dimensions_list.end());

    pdqsort(sorted_dimensions_list.begin(), sorted_dimensions_list.end(),
            [](math::Dimensions a, math::Dimensions b) {
              return a.height > b.height;
            });
    absl::Span<const math::Dimensions> remaining_dimensions_list =
        absl::MakeConstSpan(sorted_dimensions_list);
    absl::Span<const std::vector<F>> remaining_opened_values =
        absl::MakeConstSpan(opened_values);

    size_t next_layer =
        absl::bit_ceil(remaining_dimensions_list.front().height);
    size_t next_layer_size = CountLayers(next_layer, remaining_dimensions_list);
    remaining_dimensions_list.remove_prefix(next_layer_size);
    Digest root = hasher_.Hash(GetOpenedValuesAsPrimeFieldVectors(
        remaining_opened_values, next_layer_size));
    remaining_opened_values.remove_prefix(next_layer_size);

    for (const Digest& sibling : proof) {
      Digest inputs[2];
      inputs[0] = (index & 1) == 0 ? root : sibling;
      inputs[1] = (index & 1) == 0 ? sibling : root;
      root = compressor_.Compress(inputs);

      index >>= 1;
      next_layer >>= 1;
      next_layer_size = CountLayers(next_layer, remaining_dimensions_list);
      if (next_layer_size > 0) {
        remaining_dimensions_list.remove_prefix(next_layer_size);

        inputs[0] = std::move(root);
        inputs[1] = hasher_.Hash(GetOpenedValuesAsPrimeFieldVectors(
            remaining_opened_values, next_layer_size));
        remaining_opened_values.remove_prefix(next_layer_size);

        root = compressor_.Compress(inputs);
      }
    }

    return root == commitment;
  }

  constexpr static size_t CountLayers(
      size_t target_height,
      absl::Span<const math::Dimensions> dimensions_list) {
    size_t ret = 0;
    for (size_t i = 0; i < dimensions_list.size(); ++i) {
      if (target_height ==
          absl::bit_ceil(static_cast<size_t>(dimensions_list[i].height))) {
        ++ret;
      } else {
        break;
      }
    }
    return ret;
  }

  static std::vector<PrimeField> GetOpenedValuesAsPrimeFieldVectors(
      absl::Span<const std::vector<F>> opened_values, size_t next_layer_size) {
    if constexpr (math::FiniteFieldTraits<F>::kIsExtensionField) {
      static_assert(math::ExtensionFieldTraits<F>::kDegreeOverBasePrimeField ==
                    math::ExtensionFieldTraits<F>::kDegreeOverBaseField);
      absl::Span<const std::vector<F>> sub_opened_values =
          opened_values.subspan(0, next_layer_size);
      size_t size =
          std::accumulate(sub_opened_values.begin(), sub_opened_values.end(), 0,
                          [](size_t acc, const std::vector<F>& fields) {
                            return acc + fields.size();
                          });
      std::vector<PrimeField> ret;
      ret.reserve(size *
                  math::ExtensionFieldTraits<F>::kDegreeOverBasePrimeField);
      for (size_t i = 0; i < sub_opened_values.size(); ++i) {
        const std::vector<F>& elements = sub_opened_values[i];
        for (size_t j = 0; j < elements.size(); ++j) {
          const F& element = elements[j];
          for (size_t k = 0;
               k < math::ExtensionFieldTraits<F>::kDegreeOverBasePrimeField;
               ++k) {
            ret.push_back(element[k]);
          }
        }
      }
      return ret;
    } else {
      return base::FlatMap(opened_values.subspan(0, next_layer_size),
                           [](const std::vector<F>& fields) { return fields; });
    }
  }

  FieldMerkleTree<F, N> field_merkle_tree_;
  Hasher hasher_;
  PackedHasher packed_hasher_;
  Compressor compressor_;
  PackedCompressor packed_compressor_;
};

template <typename F, typename Hasher, typename PackedHasher,
          typename Compressor, typename PackedCompressor, size_t N>
struct MixedMatrixCommitmentSchemeTraits<FieldMerkleTreeMMCS<
    F, Hasher, PackedHasher, Compressor, PackedCompressor, N>> {
 public:
  using Field = F;
  using PrimeField =
      std::conditional_t<math::FiniteFieldTraits<F>::kIsExtensionField,
                         typename math::ExtensionFieldTraits<F>::BasePrimeField,
                         F>;
  using Commitment = std::array<PrimeField, N>;
  using Proof = std::vector<std::array<PrimeField, N>>;
};

}  // namespace tachyon::crypto

#endif  // TACHYON_CRYPTO_COMMITMENTS_MERKLE_TREE_FIELD_MERKLE_TREE_FIELD_MERKLE_TREE_MMCS_H_
