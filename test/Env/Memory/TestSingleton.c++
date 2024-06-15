#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/Singleton.h++"

class NullSingleton final : public Mustard::Env::Memory::Singleton<NullSingleton> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    NullSingleton() = default;
};

int main(int /* argc */, char* /* argv */[]) {
    // Mustard::Env::BasicEnv environment(argc, argv, {});
    NullSingleton::Instance();
    return EXIT_SUCCESS;
}
