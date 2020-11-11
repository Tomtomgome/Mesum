#ifndef M_INPUTCOMMON
#define M_INPUTCOMMON
#pragma once

#include <Logger/Logger.hpp>
#include <Kernel/Callbacks.hpp>
#include <Kernel/Types.hpp>
#include <Math/MathTypes.hpp>

namespace m
{
	extern const logging::ChannelID INPUT_LOG_ID;

	namespace input
	{
		using KeyActionCallback = Callback<void>;
		using KeyActionSignal = Signal<>;

		using MouseActionCallback = Callback<void, const math::DVec2&>;
		using MouseActionSignal = Signal<const math::DVec2&>;

		using MouseStateCallback = Callback<void, mBool>;
		using MouseStateSignal = Signal<mBool>;

		using ScrollCallback = Callback<void, const math::DVec2&>;
		using ScrollSignal = Signal<const math::DVec2&>;

		using MouseMoveCallback = Callback<void, const math::DVec2&>;
		using MouseMoveSignal = Signal<const math::DVec2&>;
	};
};

#endif //M_INPUTCOMMON