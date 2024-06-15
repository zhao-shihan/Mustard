#include "Mustard/Concept/MerelyMoveable.h++"
#include "Mustard/Concept/NonCopyable.h++"
#include "Mustard/Concept/NonMoveable.h++"
#include "Mustard/Utility/MerelyMoveableBase.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include <iostream>

using namespace Mustard;

class TrivialClass {};

static_assert(not Concept::NonCopyable<TrivialClass>);
static_assert(not Concept::MerelyMoveable<TrivialClass>);
static_assert(not Concept::NonMoveable<TrivialClass>);

class MerelyMoveableClass : public MerelyMoveableBase {};

static_assert(Concept::NonCopyable<MerelyMoveableClass>);
static_assert(Concept::MerelyMoveable<MerelyMoveableClass>);
static_assert(not Concept::NonMoveable<MerelyMoveableClass>);

class NonMoveableClass : public NonMoveableBase {};

static_assert(Concept::NonCopyable<NonMoveableClass>);
static_assert(not Concept::MerelyMoveable<NonMoveableClass>);
static_assert(Concept::NonMoveable<NonMoveableClass>);

int main() {
    std::cout << "I'm compiled, I'm passed." << std::endl;
    return EXIT_SUCCESS;
}
