#ifndef M_INPUTCOMMON
#define M_INPUTCOMMON
#pragma once

#include <Logger/Logger.hpp>
#include <Kernel/Callbacks.hpp>
#include <Kernel/Types.hpp>
#include <Math/MathTypes.hpp>

static const logging::ChannelID INPUT_LOG_ID = LOG_GET_ID();

namespace m
{
	namespace input
	{
		using KeyActionCallback = Callback<void>;
		using KeyActionSignal = Signal<>;

		using MouseActionCallback = Callback<void, const math::DVec2&>;
		using MouseActionSignal = Signal<const math::DVec2&>;

		using MouseStateCallback = Callback<void, Bool>;
		using MouseStateSignal = Signal<Bool>;

		using ScrollCallback = Callback<void, const math::DVec2&>;
		using ScrollSignal = Signal<const math::DVec2&>;

		using MouseMoveCallback = Callback<void, const math::DVec2&>;
		using MouseMoveSignal = Signal<const math::DVec2&>;
	};
};

#endif //M_INPUTCOMMON