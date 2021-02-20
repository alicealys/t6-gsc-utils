#pragma once

#define DLL_EXPORT extern "C" __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <cassert>
#include <mutex>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <regex>
#include <queue>

using namespace std::literals;

#pragma warning(disable: 6011)
#pragma warning(disable: 6054)
#pragma warning(disable: 6387)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 28182)

#include "utils/hook.hpp"
#include "utils/memory.hpp"
#include "utils/string.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc/functions.hpp"
#include "gsc/methods.hpp"

#include "component/http.hpp"
#include "component/scheduler.hpp"
#include "component/chat.hpp"
#include "component/io.hpp"
#include "component/command.hpp"

#include "gsc/gsc.hpp"