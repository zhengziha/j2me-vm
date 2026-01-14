#include "ImageCommon.hpp"

namespace j2me {
namespace natives {

std::map<int32_t, SDL_Surface*> imageMap;
int32_t nextImageId = 1;

} // namespace natives
} // namespace j2me
