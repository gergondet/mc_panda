#pragma once

#include <mc_rbdyn/RobotModuleMacros.h>
#include <mc_rtc/logging.h>

#include <mc_robots/api.h>

namespace mc_robots
{

struct ROBOT_MODULE_API PandaRobotModule : public mc_rbdyn::RobotModule
{
public:
  PandaRobotModule(bool pump, bool foot, bool hand);
};

} // namespace mc_robots

#ifndef MC_RTC_BUILD_STATIC

extern "C"
{
  ROBOT_MODULE_API void MC_RTC_ROBOT_MODULE(std::vector<std::string> & names)
  {
    names = {"Panda", "PandaDefault", "PandaHand", "PandaPump", "PandaFoot"};
  }
  ROBOT_MODULE_API void destroy(mc_rbdyn::RobotModule * ptr)
  {
    delete ptr;
  }
  ROBOT_MODULE_API mc_rbdyn::RobotModule * create(const std::string & n)
  {
    ROBOT_MODULE_CHECK_VERSION("Panda")
    if(n == "Panda" || n == "PandaDefault")
    {
      return new mc_robots::PandaRobotModule(false, false, false);
    }
    else if(n == "PandaPump")
    {
      return new mc_robots::PandaRobotModule(true, false, false);
    }
    else if(n == "PandaFoot")
    {
      return new mc_robots::PandaRobotModule(false, true, false);
    }
    else if(n == "PandaHand")
    {
      return new mc_robots::PandaRobotModule(false, false, true);
    }
    else
    {
      mc_rtc::log::error("Panda module cannot create an object of type {}", n);
      return nullptr;
    }
  }
}

#else

#  include <mc_rbdyn/RobotLoader.h>

namespace
{
static auto registered = []() {
  using TYPE = mc_robots::PandaRobotModule;
  using fn_t = std::function<TYPE *()>;
  mc_rbdyn::RobotLoader::register_object("Panda", fn_t([]() { return new TYPE(false, false, false); }));
  mc_rbdyn::RobotLoader::register_object("PandaDefault", fn_t([]() { return new TYPE(false, false, false); }));
  mc_rbdyn::RobotLoader::register_object("PandaPump", fn_t([]() { return new TYPE(true, false, false); }));
  mc_rbdyn::RobotLoader::register_object("PandaFoot", fn_t([]() { return new TYPE(false, true, false); }));
  mc_rbdyn::RobotLoader::register_object("PandaHand", fn_t([]() { return new TYPE(false, false, true); }));
  return true;
}();
}

#endif
