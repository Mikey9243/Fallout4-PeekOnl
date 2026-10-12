#include "pch.h"

F4SE_PLUGIN_PRELOAD(const F4SE::PreLoadInterface* f4se)
{
    F4SE::Init(f4se);
    REX::INFO("PeekOnly loaded (prototype; camera hook not connected)");
    return true;
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* f4se)
{
    F4SE::Init(f4se);
    REX::INFO("PeekOnly runtime initialized (prototype; no in-game peek yet)");
    return true;
}

