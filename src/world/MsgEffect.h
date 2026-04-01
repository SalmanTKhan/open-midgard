#pragma once

#include "GameActor.h"

bool QueueQueuedMsgEffectsQuads();
bool GetQueuedMsgEffectsBounds(RECT* outRect);
void ClearQueuedMsgEffects();
bool HasQueuedMsgEffects();
bool HasActiveMsgEffects();
