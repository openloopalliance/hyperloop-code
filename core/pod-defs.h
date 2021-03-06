/*****************************************************************************
 * Copyright (c) Paradigm Hyperloop, 2017
 *
 * This material is proprietary intellectual property of Paradigm Hyperloop.
 * All rights reserved.
 *
 * The methods and techniques described herein are considered proprietary
 * information. Reproduction or distribution, in whole or in part, is
 * forbidden without the express written permission of Paradigm Hyperloop.
 *
 * Please send requests and inquiries to:
 *
 *  Software Engineering Lead - Eddie Hurtig <hurtige@ccs.neu.edu>
 *
 * Source that is published publicly is for demonstration purposes only and
 * shall not be utilized to any extent without express written permission of
 * Paradigm Hyperloop.
 *
 * Please see http://www.paradigm.team for additional information.
 *
 * THIS SOFTWARE IS PROVIDED BY PARADIGM HYPERLOOP ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL PARADIGM HYPERLOOP BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
#include "pod.h"

#ifndef PARADIGM_POD_DEFS_H
#define PARADIGM_POD_DEFS_H

#define MAX_STATE_REASON_MSG 255

typedef struct pod_value {
  union {
    int32_t int32;
    float fl;
  } value;
  pthread_rwlock_t lock;
} pod_value_t;

#define POD_VALUE_INITIALIZER_FL                                               \
  { {.fl = 0.0}, PTHREAD_RWLOCK_INITIALIZER }
#define POD_VALUE_INITIALIZER_INT32                                            \
  { {.int32 = 0}, PTHREAD_RWLOCK_INITIALIZER }

typedef enum clamp_brake_state {
  kClampBrakeClosed,
  kClampBrakeEngaged,
  kClampBrakeReleased
} clamp_brake_state_t;

typedef uint32_t thermocouple_raw_t;
typedef uint32_t transducer_raw_t;
typedef uint32_t photodiode_raw_t;
typedef uint32_t aux_raw_t;
typedef uint32_t distance_raw_t;
typedef uint32_t spare_raw_t;

/**
 * Bundles information for analog sensor reading
 */
typedef struct {
  // The internal id number for the sensor
  int sensor_id;
  // The Human readable name of the sensor
  char name[MAX_NAME];
  int adc_num;
  int input;
  // The last calibrated sensor value
  pod_value_t value;
  // The most recent raw ADC reading (-1 if no recent value)
  pod_value_t raw;
  // quadratic calibration coefficients ax^2 + bx + c where x is the raw value
  double cal_a;
  double cal_b;
  double cal_c;
  // low pass filter alpha
  double alpha;
  // Mannual offset
  double offset;
} sensor_t;

#define SENSOR_INITIALIZER                                                     \
  {                                                                            \
    .sensor_id = 0, .name = {0}, .value = POD_VALUE_INITIALIZER_INT32,         \
    .cal_a = 0.0, .cal_b = 0.0, .cal_c = 0.0, .alpha = 0.0, .offset = 0.0      \
  }

/**
 * Information from the battery control boards
 */
typedef struct {
  sensor_t voltage;
  sensor_t current;
  sensor_t temperature;
  sensor_t charge;
  sensor_t remaining_time;
} pod_battery_t;

typedef enum pod_caution {
  PodCautionNone = 0x00,
  LPThermocoupleLow = 0x1,
  LPThermocoupleHigh = 0x2,
  SkatePressureLow = 0x4,
  SkatePressureHigh = 0x8,
  BatteryChargeLow = 0xF,
  BatteryChargeHigh = 0x10,
  PodCautionAll = 0xFF
} pod_caution_t;

typedef enum pod_warning {
  PodWarningNone = 0x00,
  BatteryTempHigh = 0x1,
  BatteryTempLow = 0x2,
  PodWarningAll = 0xFF,
} pod_warning_t;

typedef enum pod_shutdown {
  Halt = 0,
  WarmReboot = 1,
  ColdReboot = 2,
} pod_shutdown_t;

/**
 * Defines the state of valves that a manual remote controller desires
 */
typedef struct manual_config {
  clamp_brake_state_t front_brake;
  clamp_brake_state_t rear_brake;
  solenoid_state_t vent;
  solenoid_state_t fill;
  solenoid_state_t battery_a;
  solenoid_state_t battery_b;
  solenoid_state_t skate_a;
  solenoid_state_t skate_b;
  solenoid_state_t skate_c;
  solenoid_state_t skate_d;
  mpye_value_t mpye_a;
  mpye_value_t mpye_b;
  mpye_value_t mpye_c;
  mpye_value_t mpye_d;
} manual_config_t;

// Used to configure flight profiles
typedef struct flight_profile {
  useconds_t watchdog_timer; // Main braking timeout initiated by pushing state
                             // 5MPH 4300000
  useconds_t emergency_hold; // time held in the emergency state
  useconds_t braking_wait;   // Time before engaging secondary brake, if needed
  useconds_t braking_hold;   // min time to hold brakes before vent

  useconds_t pusher_timeout;    // Timeout for the pusher plate debounce
  float pusher_state_accel_min; // m/s/s Threshold for transitioning into the
                                // pushing state
  useconds_t pusher_state_min_timer; // Minimium time in the pushing state
  float pusher_distance_min; // mm Distance to register the pusher as present
  float primary_braking_accel_min; // m/s/s min acceptable acceleration while
                                   // braking
  pthread_rwlock_t lock;
} flight_profile_t;

typedef uint16_t relay_mask_t;

/**
 * Defines the master state of the pod
 */
typedef struct pod {
  int version;
  char *name;
  pod_value_t accel_x;
  pod_value_t accel_y;
  pod_value_t accel_z;

  pod_value_t velocity_x;
  pod_value_t velocity_z;
  pod_value_t velocity_y;

  pod_value_t position_x;
  pod_value_t position_y;
  pod_value_t position_z;

  pod_value_t rotvel_x;
  pod_value_t rotvel_y;
  pod_value_t rotvel_z;

  pod_value_t variance_x;
  pod_value_t variance_y;
  pod_value_t variance_z;

  pod_value_t quaternion_real;
  pod_value_t quaternion_i;
  pod_value_t quaternion_j;
  pod_value_t quaternion_k;

  pod_value_t imu_calibration_x;
  pod_value_t imu_calibration_y;
  pod_value_t imu_calibration_z;

  // Solenoids
  solenoid_t skate_solenoids[N_SKATE_SOLENOIDS];
  solenoid_t clamp_engage_solenoids[N_CLAMP_ENGAGE_SOLENOIDS];
  solenoid_t clamp_release_solenoids[N_CLAMP_RELEASE_SOLENOIDS];
  solenoid_t vent_solenoid;
  solenoid_t hp_fill_valve;
  solenoid_t battery_pack_relays[N_BATTERY_PACK_RELAYS];

  // HP Fill Valve Limit Switches
  sensor_t hp_fill_valve_open;
  sensor_t hp_fill_valve_close;

  // MPYEs
  mpye_t mpye[N_MPYES];

  // Pressure Transducers
  sensor_t hp_pressure;
  sensor_t reg_pressure[N_REG_PRESSURE];
  sensor_t clamp_pressure[N_CLAMP_PRESSURE];
  sensor_t brake_tank_pressure[N_BRAKE_TANK_PRESSURE];

  // Thermocouples
  sensor_t hp_thermo;
  sensor_t power_thermo[N_POWER_THERMO];
  sensor_t reg_thermo[N_REG_THERMO];
  sensor_t reg_surf_thermo[N_REG_THERMO];
  sensor_t frame_thermo;
  sensor_t clamp_thermo[N_CLAMP_PAD_THERMO];

  // Distance Sensors
  sensor_t pusher_plate_distance[N_PUSHER_DISTANCE];
  sensor_t levitation_distance[N_LEVITATION_DISTANCE];

  manual_config_t manual;
  // Pusher Plate
  pod_value_t pusher_plate;
  uint64_t last_pusher_seen;
  uint64_t launch_time;

  // Batteries
  pod_battery_t battery[N_BATTERIES];

  // Thread Tracking
  pthread_t core_thread;
  pthread_t logging_thread;
  pthread_t cmd_thread;

  // Current Overall Pod Mode (Goal of the System)
  pod_mode_t mode;
  pthread_rwlock_t mode_mutex;

  // Flight profile
  flight_profile_t flight_profile;

  // Ports
  pod_value_t logging_port;
  pod_value_t command_port;

  // Holds the pod in a boot state until set to 1 by an operator
  pod_value_t ready;

  // Pointers to all the solenoids that are connected to the relays
  // (Don't think too much about this one, it is really just a convienience)
  solenoid_t *relays[N_RELAY_CHANNELS];
  sensor_t *sensors[N_ADC_CHANNELS * N_ADCS];

  bus_t i2c[N_I2C_BUSSES];

  // fd of the IMU
  int imu;

  // socket for the logging UDP socket
  int logging_socket;

  // log file
  int logging_fd;

  // log file name
  char logging_filename[PATH_MAX];

  char state_reason[MAX_STATE_REASON_MSG];

  uint64_t last_ping;
  uint64_t last_transition;
  uint64_t engaged_brakes;
  pod_value_t core_speed;
  enum pod_caution cautions;
  enum pod_warning warnings;

  bool calibrated;
  bool func_test;
  bool return_to_standby;
  bool manual_emergency;
  sem_t *boot_sem;
  uint64_t last_imu_reading;
  uint64_t start;
  uint64_t overrides;
  pthread_rwlock_t overrides_mutex;

  pod_shutdown_t shutdown;
  bool initialized;
} pod_t;

/**
 * Sets the target flow through the skates on a scale of 0% to 100%
 */
int set_skate_target(int no, mpye_value_t val, bool override);

/**
 * Sets the clamp brake state
 */
int ensure_clamp_brakes(int no, clamp_brake_state_t val, bool override);

/**
 * Gets a bitmask representing the state of all the relays on the SSR board(s)
 */
relay_mask_t get_relay_mask(pod_t *pod);

/**
 * Performs a software self test on the pod's systems
 */
int self_tests(pod_t *pod);

/**
 * Sends the given message to all logging destinations
 */
int pod_log(char *fmt, ...);

/**
 * Dump entire pod_t to the network logging buffer
 */
void log_dump(pod_t *pod);

/**
 * Create a human understandable text description of the current pod status
 *
 * @param pod A pod with data that you want a report of
 * @param buf The buffer to put the report in
 * @param len The length of buf
 *
 * @return The length of the report in bytes, or -1 on failure
 */
int status_dump(pod_t *pod, char *buf, size_t len);

/**
 * Initiates a halt of all threads
 */
int pod_shutdown(pod_t *pod);

/**
 * Starts all threads for the pod
 */
int pod_start(pod_t *pod);

/**
 * Initializes the GPIO pins on the BBB for use with the pod
 */
void setup_pins(pod_t *state);

/**
 * Calibrate sensors based on currently read values (zero out)
 */
void pod_calibrate(void);

/**
 * Reset positional and sensor data to blank slate
 */
bool pod_reset(void);

/**
 * Shuts down the pod safely and performs any required cleanup actions
 */
void pod_exit(int code);

///////////////////

/**
 * @brief Gets a pointer to the pod state structure
 *
 * The pod state struct contains all the current state information for the pod
 * as well as mutexes for locking the values for reading and writing
 *
 * @return the current pod state as of calling
 */
pod_t *get_pod(void);

/**
 * Intiializes the pod's pod_t returned by get_pod()
 */
int init_pod(void);

static inline int32_t get_value(pod_value_t *pod_field) {
  pthread_rwlock_rdlock(&(pod_field->lock));
  int32_t value = pod_field->value.int32;
  pthread_rwlock_unlock(&(pod_field->lock));
  return value;
}

static inline float get_value_f(pod_value_t *pod_field) {
  pthread_rwlock_rdlock(&(pod_field->lock));
  float value = pod_field->value.fl;
  pthread_rwlock_unlock(&(pod_field->lock));
  return value;
}

static inline void set_value(pod_value_t *pod_field, int32_t newValue) {
  pthread_rwlock_wrlock(&(pod_field->lock));
  pod_field->value.int32 = newValue;
  pthread_rwlock_unlock(&(pod_field->lock));
}

static inline void set_value_f(pod_value_t *pod_field, float newValue) {
#ifdef DEBUG
  if (newValue != newValue) {
    warn("Attempted to set NaN");
    return;
  }
#endif

  pthread_rwlock_wrlock(&(pod_field->lock));
  pod_field->value.fl = newValue;
  pthread_rwlock_unlock(&(pod_field->lock));
}

// Setters for flight_profile
static inline void set_watchdog_timer(flight_profile_t *profile,
                                      useconds_t newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->watchdog_timer = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_emergency_hold(flight_profile_t *profile,
                                      useconds_t newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->emergency_hold = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_braking_wait(flight_profile_t *profile,
                                    useconds_t newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->braking_wait = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_braking_hold(flight_profile_t *profile,
                                    useconds_t newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->braking_hold = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_pusher_timeout(flight_profile_t *profile,
                                      useconds_t newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->pusher_timeout = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_pusher_state_accel_min(flight_profile_t *profile,
                                              float newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->pusher_state_accel_min = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_pusher_state_min_timer(flight_profile_t *profile,
                                              useconds_t newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->pusher_state_min_timer = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_pusher_distance_min(flight_profile_t *profile,
                                           float newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->pusher_distance_min = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

static inline void set_primary_braking_accel_min(flight_profile_t *profile,
                                                 float newValue) {
  pthread_rwlock_wrlock(&profile->lock);
  profile->primary_braking_accel_min = newValue;
  pthread_rwlock_unlock(&profile->lock);
}

// Getters for flight_profile
static inline useconds_t get_watchdog_timer(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  useconds_t output = profile->watchdog_timer;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline useconds_t get_emergency_hold(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  useconds_t output = profile->emergency_hold;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline useconds_t get_braking_wait(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  useconds_t output = profile->braking_wait;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline useconds_t get_braking_hold(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  useconds_t output = profile->braking_hold;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline useconds_t get_pusher_timeout(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  useconds_t output = profile->pusher_timeout;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline float get_pusher_state_accel_min(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  float output = profile->pusher_state_accel_min;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline useconds_t get_pusher_state_min_timer(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  useconds_t output = profile->pusher_state_min_timer;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline float get_pusher_distance_min(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  float output = profile->pusher_distance_min;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

static inline float get_primary_braking_accel_min(flight_profile_t *profile) {
  pthread_rwlock_wrlock(&profile->lock);
  float output = profile->primary_braking_accel_min;
  pthread_rwlock_unlock(&profile->lock);
  return output;
}

void queue_sensor(sensor_t *sensor, int32_t new_value);
float get_sensor(sensor_t *sensor);
void set_sensor(sensor_t *sensor, float val);
float update_sensor(sensor_t *sensor);

/**
 * Manual override handlers
 */
void override_surface(uint64_t surfaces, bool override);
bool is_surface_overriden(uint64_t surface);

////////////////

#endif /* PARADIGM_POD_H */
