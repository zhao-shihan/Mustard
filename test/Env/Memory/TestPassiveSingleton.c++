#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"

class NullPassiveSingleton : public Mustard::Env::Memory::PassiveSingleton<NullPassiveSingleton> {};

int main(int /* argc */, char* /* argv */[]) {
    // Mustard::Env::BasicEnv environment(argc, argv, {});
    NullPassiveSingleton::Instance();
    return EXIT_SUCCESS;
}
