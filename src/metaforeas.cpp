#include "lib_samos.hpp"
#include "metaforeas.hpp"

int main(int argc, char** argv)
{
    samos::log::logger::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    samos::log::logger::set_level(samos::log::logger::LogLevel::Trace);
    samos::user_interface::metaforeas::Metaforeas metaforeas(argc, argv);

    metaforeas.run();
    return 0;
}
