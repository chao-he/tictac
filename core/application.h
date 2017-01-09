#pragma once
#include <string>
#include <vector>
#include <map>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>

class Application {
  private:
    boost::program_options::options_description opts_;
    boost::program_options::positional_options_description pos_opts_;
    boost::optional<boost::program_options::variables_map> configuration_;

  public:
    struct positional_option {
      const char* name;
      const boost::program_options::value_semantic* value_semantic;
      const char* help;
      int max_count;
    };
    boost::program_options::options_description_easy_init add_options();
    void add_positional_options(std::initializer_list<positional_option> options);
    boost::program_options::variables_map& configuration();

  public:
    Application();
    virtual ~Application() {}

    void PrintUsage();

    virtual void Init(int argc, char** argv);
    virtual int Run() = 0;
    static Application& Instance();
};

class SimpleServer: public Application {
  public:
    SimpleServer();
    int Run() override;
};
