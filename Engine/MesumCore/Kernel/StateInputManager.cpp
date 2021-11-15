#include "StateInputManager.hpp"

namespace m::input
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mStateInputManager::mStateInputManager()
{
    m_keyState.fill(mInputType::released);  // Useful?
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mStateInputManager::~mStateInputManager() {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mStateInputManager::process_keyEvent(mKey a_key, mInputType a_action)
{
    m_keyState[a_key] = a_action;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mInputType mStateInputManager::get_keyState(mKey a_key) const
{
    return m_keyState[a_key];
}

}  // namespace m::input