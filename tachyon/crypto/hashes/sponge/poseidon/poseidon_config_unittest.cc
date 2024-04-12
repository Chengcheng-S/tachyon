// Copyright 2022 arkworks contributors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.arkworks and the LICENCE-APACHE.arkworks
// file.

// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#include "tachyon/crypto/hashes/sponge/poseidon/poseidon_config.h"

#include "gtest/gtest.h"

#include "tachyon/math/elliptic_curves/bls12/bls12_381/fr.h"
#include "tachyon/math/elliptic_curves/bn/bn254/fr.h"

namespace tachyon::crypto {

TEST(PoseidonConfigTest, CreateDefault) {
  using F = math::bls12_381::Fr;
  F::Init();

  // clang-format off
  struct {
    size_t rate;
    bool optimized_for_weights;
    const char* ark00_str;
    const char* mds00_str;
  } tests[] = {
    {2, false, "27117311055620256798560880810000042840428971800021819916023577129547249660720", "26017457457808754696901916760153646963713419596921330311675236858336250747575"},
    {3, false, "11865901593870436687704696210307853465124332568266803587887584059192277437537", "18791275321793747281053101601584820964683215017313972132092847596434094368732"},
    {4, false, "41775194144383840477168997387904574072980173775424253289429546852163474914621", "42906651709148432559075674119637355642263148226238482628104108168707874713729"},
    {5, false, "24877380261526996562448766783081897666376381975344509826094208368479247894723", "30022080821787948421423927053079656488514459012053372877891553084525866347732"},
    {6, false, "37928506567864057383105673253383925733025682403141583234734361541053005808936", "49124738641420159156404016903087065194698370461819821829905285681776084204443"},
    {7, false, "37848764121158464546907147011864524711588624175161409526679215525602690343051", "28113878661515342855868752866874334649815072505130059513989633785080391114646"},
    {8, false, "51456871630395278065627483917901523970718884366549119139144234240744684354360", "12929023787467701044434927689422385731071756681420195282613396560814280256210"},
    {2, true, "25126470399169474618535500283750950727260324358529540538588217772729895991183", "46350838805835525240431215868760423854112287760212339623795708191499274188615"},
    {3, true, "16345358380711600255519479157621098002794924491287389755192263320486827897573", "37432344439659887296708509941462699942272362339508052702346957525719991245918"},
    {4, true, "2997721997773001075802235431463112417440167809433966871891875582435098138600", "43959024692079347032841256941012668338943730711936867712802582656046301966186"},
    {5, true, "28142027771717376151411984909531650866105717069245696861966432993496676054077", "13157425078305676755394500322568002504776463228389342308130514165393397413991"},
    {6, true, "7417004907071346600696060525974582183666365156576759507353305331252133694222", "51393878771453405560681338747290999206747890655420330824736778052231938173954"},
    {7, true, "47093173418416013663709314805327945458844779999893881721688570889452680883650", "51455917624412053400160569105425532358410121118308957353565646758865245830775"},
    {8, true, "16478680729975035007348178961232525927424769683353433314299437589237598655079", "39160448583049384229582837387246752222769278402304070376350288593586064961857"},
  };
  // clang-format on

  for (const auto& test : tests) {
    PoseidonConfig<F> config =
        PoseidonConfig<F>::CreateDefault(test.rate, test.optimized_for_weights);
    ASSERT_TRUE(config.IsValid());
    EXPECT_EQ(config.ark(0, 0), *F::FromDecString(test.ark00_str));
    EXPECT_EQ(config.mds(0, 0), *F::FromDecString(test.mds00_str));
  }
}

TEST(PoseidonConfigTest, CreateCustom) {
  using F = math::bn254::Fr;
  F::Init();

  // clang-format off
  struct {
    size_t rate;
    uint64_t alpha;
    size_t full_rounds;
    size_t partial_rounds;
    size_t skip_matrices;
    const char* mds00_str;
  } tests[] = {
      {2, 5, 8, 57, 0, "7511745149465107256748700652201246547602992235352608707588321460060273774987"},
      {3, 5, 8, 31, 0, "4843064272860702558353681805605581092414485968533095609154162537440763859608"},
      {4, 5, 8, 57, 0, "9390358363320792447057897590391227342305356869000968376798315031785873376651"},
      {5, 5, 4, 31, 0, "10942845101262402626166273431356787436787991939175819278065571096963239049995"},
      {6, 5, 2, 12, 0, "9837954178279989965317992196165220609182866932765981962230951853077616895744"},
      {7, 5, 6, 57, 0, "20096031343166107597256293287487285680099080393005092613845214101013675429510"},
      {8, 5, 8, 63, 0, "708458300293891745856425423607721463509413916954480913172999113933455141974"},
  };
  // clang-format on

  for (const auto& test : tests) {
    PoseidonConfig<F> config = PoseidonConfig<F>::CreateCustom(
        test.rate, test.alpha, test.full_rounds, test.partial_rounds,
        test.skip_matrices);
    ASSERT_TRUE(config.IsValid());
    EXPECT_EQ(config.mds(0, 0), *F::FromDecString(test.mds00_str));
  }
}

}  // namespace tachyon::crypto
