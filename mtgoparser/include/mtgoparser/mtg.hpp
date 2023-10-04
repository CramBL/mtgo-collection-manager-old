// For general Magic: The Gathering info/utility

#include <glaze/glaze.hpp>

namespace mtg {

enum class Rarity { Common, Uncommon, Rare, Mythic };


}// namespace mtg


template<> struct glz::meta<mtg::Rarity>
{
  using enum mtg::Rarity;
  static constexpr auto value = enumerate("C", Common, "U", Uncommon, "R", Rare, "M", Mythic);
};