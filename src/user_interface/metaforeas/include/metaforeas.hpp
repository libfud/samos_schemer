#ifndef SAMOS_METAFOREAS_HPP
#define SAMOS_METAFOREAS_HPP

#include "result.hpp"
#include "scheme.hpp"
#include "option_parser.hpp"

#include <string>
#include <vector>

namespace samos::user_interface::metaforeas
{

class Metaforeas
{
public:
    Metaforeas(int argc, char** argv);

    void run();

private:
    option_parser::OptionParser option_parser;

    scheme::Schemer schemer;

    std::vector<std::string> filenames;
};

} // namespace samos::user_interface::metaforeas

#endif // SAMOS_METAFOREAS_HPP
