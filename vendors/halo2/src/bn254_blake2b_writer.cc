#include "vendors/halo2/include/bn254_blake2b_writer.h"

#include "tachyon/math/elliptic_curves/bn/bn254/g1.h"
#include "tachyon/rs/base/container_util.h"
#include "vendors/halo2/src/blake2b_writer_impl.h"

namespace tachyon::halo2_api::bn254 {

class Blake2bWriter::Impl
    : public Blake2bWriterImpl<math::bn254::G1AffinePoint> {};

Blake2bWriter::Blake2bWriter() : impl_(new Impl()) {}

void Blake2bWriter::update(rust::Slice<const uint8_t> data) {
  return impl_->Update(data);
}

void Blake2bWriter::finalize(
    std::array<uint8_t, BLAKE2B512_DIGEST_LENGTH>& result) {
  return impl_->Finalize(result);
}

rust::Vec<uint8_t> Blake2bWriter::state() const {
  return rs::ConvertCppContainerToRustVec(impl_->GetState());
}

std::unique_ptr<Blake2bWriter> new_blake2b_writer() {
  return std::make_unique<Blake2bWriter>();
}

}  // namespace tachyon::halo2_api::bn254
