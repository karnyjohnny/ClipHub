#include "gui/Theme.h"

namespace cliphub {

const Theme& Theme::dark() {
    static Theme s_darkTheme;
    return s_darkTheme;
}

} // namespace cliphub
