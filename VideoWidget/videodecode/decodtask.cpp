#include "decodetaskmanagerimpl.h"

DecodeTaskManager* DecodeTaskManager::Instance()
{
    static DecodeTaskManagerImpl obj;
    return &obj;
}
