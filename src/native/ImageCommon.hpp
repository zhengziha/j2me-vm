#pragma once

#include <map>
#include <set>
#include <SDL2/SDL.h>

namespace j2me {
namespace natives {

extern std::map<int32_t, SDL_Surface*> imageMap;
extern std::set<int32_t> mutableImages;
extern int32_t nextImageId;

} // namespace natives
} // namespace j2me
