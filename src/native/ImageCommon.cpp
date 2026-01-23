#include "ImageCommon.hpp"

namespace j2me {
namespace natives {

std::map<int32_t, SDL_Surface*> imageMap;
std::set<int32_t> mutableImages;
int32_t nextImageId = 1;

} // namespace natives
} // namespace j2me
