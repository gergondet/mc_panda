#pragma once

#include <mc_panda/devices/api.h>

#include <mc_rbdyn/Device.h>

#include <mc_rtc/Configuration.h>
#include <mc_rtc/log/Logger.h>

#ifndef MC_PANDA_WITHOUT_FRANKA
#  include <franka/vacuum_gripper.h>
#  include <franka/vacuum_gripper_state.h>
#endif

#include <chrono>
#include <thread>
#include <vector>

namespace mc_panda
{

/** This device is an asynchronous wrapper around the synchronous VacuumGripper
 * interface provided by libfranka */
struct MC_PANDA_DEVICES_DLLAPI Pump : public mc_rbdyn::Device
{
  static constexpr auto name = "Pump";

#ifndef MC_PANDA_WITHOUT_FRANKA
  using ProductionSetupProfile = franka::VacuumGripper::ProductionSetupProfile;

  using StatusInt = std::underlying_type<franka::VacuumGripperDeviceStatus>::type;

  enum class Status : StatusInt{kGreen = static_cast<int>(franka::VacuumGripperDeviceStatus::kGreen),
                                kYellow = static_cast<int>(franka::VacuumGripperDeviceStatus::kYellow),
                                kOrange = static_cast<int>(franka::VacuumGripperDeviceStatus::kOrange),
                                kRed = static_cast<int>(franka::VacuumGripperDeviceStatus::kRed),
                                disconnected = static_cast<int>(kRed + 1)};
#else
  enum class ProductionSetupProfile : uint8_t
  {
    kP0,
    kP1,
    kP2,
    kP3
  };

  enum class Status : uint8_t
  {
    kGreen,
    kYellow,
    kOrange,
    kRed,
    disconnected = static_cast<uint8_t>(kRed + 1)
  };
#endif

  /** Get the pump associated to the provided robot
   *
   * \returns nullptr if the device does not exist in this robot
   */
  static Pump * get(mc_rbdyn::Robot & robot);

  /** Constructor
   *
   * @param name Name of the pump
   *
   */
  Pump(const std::string & parent, const sva::PTransformd & X_p_d);

  ~Pump() override;

  /** Connect the pump device to an actual pump, the pump operations are then done in a background thread*/
  bool connect(const std::string & ip);

  /** Disconnect from the actual pump */
  void disconnect();

#ifndef MC_PANDA_WITHOUT_FRANKA
  /** Access the vacuum gripper state */
  const franka::VacuumGripperState & state() const;
#endif

  /** Get the pump status */
  Status status() const;

  /** True if the pump is currently busy */
  bool busy() const;

  /** True if the last command succeeded, false otherwise */
  bool success() const;

  /** Message describing the latest error */
  const std::string & error() const;

  /**
   * Vacuums an object.
   *
   * @param[in] vacuum Setpoint for control mode. Unit: \f$[10*mbar]\f$.
   * @param[in] timeout Vacuum timeout. Unit: \f$[ms]\f$.
   * @param[in] profile Production setup profile P0 to P3. Default: P0.
   *
   * @return True if the command was requested, false otherwise
   */
  bool vacuum(uint8_t vacuum,
              std::chrono::milliseconds timeout,
              ProductionSetupProfile profile = ProductionSetupProfile::kP0);

  /**
   * Drops the grasped object off.
   *
   * @param[in] timeout Dropoff timeout. Unit: \f$[ms]\f$.
   *
   * @return True if the command was requested, false otherwise
   */
  bool dropOff(std::chrono::milliseconds timeout);

  /**
   * Stops a currently running vacuum gripper vacuum or drop off operation.
   *
   * @return True if the command was requested, false otherwise
   */
  bool stop();

  void addToLogger(mc_rtc::Logger & logger, const std::string & prefix);

  void removeFromLogger(mc_rtc::Logger & logger, const std::string & prefix);

  mc_rbdyn::DevicePtr clone() const override;

private:
  // Status of the pump
  Status status_ = Status::disconnected;
#ifndef MC_PANDA_WITHOUT_FRANKA
  // Only non-null if the pump is connected
  std::unique_ptr<franka::VacuumGripper> gripper_;
#endif
  // Thread for reading the gripper state
  std::thread stateThread_;
  // Mutex for protecting the gripper state
  mutable std::mutex stateMutex_;
#ifndef MC_PANDA_WITHOUT_FRANKA
  // Current state
  franka::VacuumGripperState state_;
#endif
  // Thread for sending commands
  std::thread commandThread_;
  // Thread for interrupting commands
  std::thread interruptThread_;
  // Only true while the gripper is connected
  std::atomic<bool> connected_{false};
  // Only true while a command is being executed
  std::atomic<bool> busy_{false};
  // Only true if a command has been interrupted
  std::atomic<bool> interrupted_{false};
  struct Command
  {
    std::string name;
    std::function<bool()> callback;
  };
  // Only valid while a command is being executed
  Command command_;
  // Represent the last command executed
  uint8_t last_command_id_ = 0;
  // Store the last command success
  bool success_ = false;
  // Store the last command error (if any)
  std::string error_ = "";
};

typedef std::vector<Pump, Eigen::aligned_allocator<Pump>> PumpVector;

} // namespace mc_panda

#ifndef MC_PANDA_WITHOUT_FRANKA
static_assert(std::is_same<typename std::underlying_type<mc_panda::Pump::Status>::type,
                           typename std::underlying_type<franka::VacuumGripperDeviceStatus>::type>::value,
              "Something is wrong");

inline bool operator==(const mc_panda::Pump::Status & lhs, const franka::VacuumGripperDeviceStatus & rhs)
{
  return static_cast<int>(lhs) == static_cast<int>(rhs);
}

inline bool operator==(const franka::VacuumGripperDeviceStatus & lhs, const mc_panda::Pump::Status & rhs)
{
  return rhs == lhs;
}

inline bool operator!=(const mc_panda::Pump::Status & lhs, const franka::VacuumGripperDeviceStatus & rhs)
{
  return !(lhs == rhs);
}

inline bool operator!=(const franka::VacuumGripperDeviceStatus & lhs, const mc_panda::Pump::Status & rhs)
{
  return rhs != lhs;
}
#endif

namespace mc_rtc
{

template<>
struct MC_RBDYN_DLLAPI ConfigurationLoader<mc_panda::Pump::ProductionSetupProfile>
{
  using PSP = mc_panda::Pump::ProductionSetupProfile;
  static PSP load(const Configuration & c)
  {
    std::string p = c;
    if(p == "kP0")
    {
      return PSP::kP0;
    }
    if(p == "kP1")
    {
      return PSP::kP1;
    }
    if(p == "kP2")
    {
      return PSP::kP2;
    }
    if(p == "kP3")
    {
      return PSP::kP3;
    }
    auto msg = fmt::format(
        "Could not convert stored configuration into a ProductionSetupProfile, got {}, expected kP[0-3]", p);
    mc_rtc::log::critical(msg);
    throw Configuration::Exception(msg, c);
  }

  static Configuration save(const PSP & profile)
  {
    Configuration out;
    switch(profile)
    {
      case PSP::kP0:
        out.add("profile", "kP0");
        break;
      case PSP::kP1:
        out.add("profile", "kP1");
        break;
      case PSP::kP2:
        out.add("profile", "kP2");
        break;
      case PSP::kP3:
        out.add("profile", "kP3");
        break;
    };
    return out("profile");
  }
};

} // namespace mc_rtc
