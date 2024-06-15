#include "Mustard/Env/Memory/internal/SingletonBase.h++"
#include "Mustard/Env/Memory/internal/SingletonDeleter.h++"
#include "Mustard/Env/Memory/internal/SingletonPool.h++"

namespace Mustard::Env::Memory::internal {

SingletonDeleter::~SingletonDeleter() {
    for (auto&& singletonBase : SingletonPool::Instance().GetUndeletedInReverseInsertionOrder()) {
        delete singletonBase;
    }
}

} // namespace Mustard::Env::Memory::internal
