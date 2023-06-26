#pragma once

#include <variant>

#include "path.h"
#include "rect.h"
#include "circle.h"

using Shape = std::variant<Path, Rect, Circle>;