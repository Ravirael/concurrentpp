#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main( int argc, char* argv[] ) {
    constexpr auto number_of_runs = 1000u;
    auto result = 0;
    Catch::Session session;

    for (auto i = 0u; i < number_of_runs; ++i) {
        std::cout << i+1 << " / " << number_of_runs << std::endl;
        result = session.run(argc, argv);
        std::cout << std::endl;

        if (result != 0) {
            break;
        }
    }

    return 0;
}