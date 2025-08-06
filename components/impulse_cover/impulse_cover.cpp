#include "impulse_cover.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/core/application.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#include <cmath>

namespace esphome {
namespace impulse_cover {

static const char *const TAG = "impulse_cover";

using namespace esphome::cover;

void ImpulseCover::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Impulse Cover...");
  
  if (this->output_ == nullptr) {
    ESP_LOGE(TAG, "Output is required!");
    this->mark_failed();
    return;
  }
  
  // Initialize state
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  } else {
    this->position = 0.5f;  // Default to half open if no restore state
  }
  
  this->current_operation = COVER_OPERATION_IDLE;
  this->current_trigger_operation_ = COVER_OPERATION_IDLE;

#ifdef USE_BINARY_SENSOR
  // Initialize position from sensors if available
  ESP_LOGV(TAG, "Initializing position from sensors...");
  this->update_position_from_sensors_(true);
#endif
  
  this->start_dir_time_ = this->last_recompute_time_ = millis();
#ifdef USE_BINARY_SENSOR
  this->last_sensor_check_time_ = millis();
#endif
  ESP_LOGCONFIG(TAG, "Impulse Cover setup complete");
}

void ImpulseCover::loop() {
  const uint32_t now = millis();
  
  if (this->current_operation == COVER_OPERATION_IDLE) {
#ifdef USE_BINARY_SENSOR
    // Check sensor alignment periodically when idle
    if (now - this->last_sensor_check_time_ > this->safety_timeout_) {
      this->check_sensor_alignment_();
      this->last_sensor_check_time_ = now;
    }
#endif
    return;
  }
    
  // Recompute position every loop cycle
  this->recompute_position_();
  
  // Check safety conditions
  this->check_safety_();
  
  // If we initiated the move, check if we reached target or safety limits
  if (this->current_trigger_operation_ != COVER_OPERATION_IDLE) {
    if (this->is_at_target_()) {
      ESP_LOGI(TAG, "Target position reached, stopping movement");
      
      // In impulse mode, only send stop pulse for intermediate positions
      // Final positions (fully open/closed) will stop automatically at endstops
      bool is_intermediate_target = (this->target_position_ > COVER_CLOSED + 0.00f && 
                                   this->target_position_ < COVER_OPEN - 0.00f);
      
      if (is_intermediate_target) {
        ESP_LOGD(TAG, "Intermediate target - sending stop pulse");
        this->start_direction_(COVER_OPERATION_IDLE);
      } else {
        ESP_LOGV(TAG, "Final position target - no stop pulse needed");
        this->set_current_operation_(COVER_OPERATION_IDLE, false);
      }
    } else if (now - this->start_dir_time_ > this->safety_timeout_) {
      ESP_LOGW(TAG, "Safety timeout reached, stopping movement");
      this->set_current_operation_(COVER_OPERATION_IDLE, false);
    }
  }
  
  // Publish state at regular intervals
  if (now - this->last_publish_time_ > 1000) {  // Every 1 second
    this->publish_state(false);
    this->last_publish_time_ = now;
  }
}

void ImpulseCover::dump_config() {
  ESP_LOGCONFIG(TAG, "Impulse Cover:");
  ESP_LOGCONFIG(TAG, "  Open Duration: %ums", this->open_duration_);
  ESP_LOGCONFIG(TAG, "  Close Duration: %ums", this->close_duration_);
  ESP_LOGCONFIG(TAG, "  Pulse Delay: %ums", this->pulse_delay_);
  ESP_LOGCONFIG(TAG, "  Safety Timeout: %ums", this->safety_timeout_);
  ESP_LOGCONFIG(TAG, "  Safety Max Cycles: %u", this->safety_max_cycles_);
  
#ifdef USE_BINARY_SENSOR
  if (this->open_sensor_) {
    ESP_LOGCONFIG(TAG, "  Open Sensor: %s", this->open_sensor_->get_name().c_str());
    ESP_LOGCONFIG(TAG, "  Open Sensor Inverted: %s", this->open_sensor_inverted_ ? "YES" : "NO");
  }
  if (this->close_sensor_) {
    ESP_LOGCONFIG(TAG, "  Close Sensor: %s", this->close_sensor_->get_name().c_str());
    ESP_LOGCONFIG(TAG, "  Close Sensor Inverted: %s", this->close_sensor_inverted_ ? "YES" : "NO");
  }
#endif
}

cover::CoverTraits ImpulseCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_supports_position(true);
  traits.set_supports_tilt(false);
  traits.set_supports_stop(true);
#ifdef USE_BINARY_SENSOR
  traits.set_is_assumed_state(this->open_sensor_ == nullptr && this->close_sensor_ == nullptr);
#else
  traits.set_is_assumed_state(true);
#endif
  return traits;
}

void ImpulseCover::control(const cover::CoverCall &call) {
  ESP_LOGV(TAG, "control() called - stop: %s, toggle: %s, position: %s", 
           call.get_stop() ? "true" : "false",
           call.get_toggle().has_value() ? "true" : "false",
           call.get_position().has_value() ? "true" : "false");
  
  if (call.get_position().has_value()) {
    ESP_LOGV(TAG, "Position command: %.3f (current: %.3f)", 
             *call.get_position(), this->position);
  }
  
  if (this->safety_triggered_) {
    ESP_LOGW(TAG, "Cover is in safety mode, ignoring command");
    return;
  }

  // Stop action logic
  if (call.get_stop()) {
    ESP_LOGI(TAG, "Stop command received");
    this->start_direction_(COVER_OPERATION_IDLE);
    return;
  }
  
  // Toggle action logic
  if (call.get_toggle().has_value()) {
    if (this->current_trigger_operation_ != COVER_OPERATION_IDLE) {
      this->start_direction_(COVER_OPERATION_IDLE);
    } else {
      if (this->position == COVER_CLOSED || this->last_operation_ == COVER_OPERATION_CLOSING) {
        this->target_position_ = COVER_OPEN;
        this->start_direction_(COVER_OPERATION_OPENING);
      } else {
        this->target_position_ = COVER_CLOSED;
        this->start_direction_(COVER_OPERATION_CLOSING);
      }
    }
    return;
  }

  // Position command
  if (call.get_position().has_value()) {
    auto pos = *call.get_position();
    if (pos == this->position) {
      // Already at target
      if (this->current_operation != COVER_OPERATION_IDLE || 
          this->current_trigger_operation_ != COVER_OPERATION_IDLE) {
        this->start_direction_(COVER_OPERATION_IDLE);
      }
    } else {
      this->target_position_ = pos;
      this->start_direction_(pos < this->position ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING);
    }
    return;
  }
}

// Main control methods based on impulse cover logic
void ImpulseCover::start_direction_(cover::CoverOperation dir) {
  ESP_LOGV(TAG, "start_direction_ called with dir=%d, safety_triggered_=%s", 
           static_cast<int>(dir), this->safety_triggered_ ? "true" : "false");
  
  if (this->safety_triggered_) {
    ESP_LOGW(TAG, "Cannot start movement: safety triggered");
    return;
  }

  // Determine what type of pulse to send based on current state and desired operation
  bool send_pulse = false;
  bool send_double_pulse = false;
  
  ESP_LOGV(TAG, "Current position: %.3f, target: %.3f, current_operation: %d, last_operation_: %d", 
           this->position, this->target_position_, 
           static_cast<int>(this->current_operation), static_cast<int>(this->last_operation_));
  
  if (dir == COVER_OPERATION_IDLE) {
    // Stop command
    if (this->current_operation != COVER_OPERATION_IDLE) {
      ESP_LOGD(TAG, "Stopping movement - sending single pulse");
      send_pulse = true;
    }
  } else if (dir == COVER_OPERATION_OPENING) {
    if (this->position >= COVER_OPEN - 0.00f) {
      // Already fully open - nothing to do
      ESP_LOGV(TAG, "Already fully open - no pulse needed");
    } else if (this->position <= COVER_CLOSED + 0.00f) {
      // Fully closed, want to open - single pulse
      ESP_LOGD(TAG, "Closed to open - sending single pulse");
      send_pulse = true;
    } else {
      // Partially open
      if (this->last_operation_ != COVER_OPERATION_OPENING) {
        // Different direction from previous - single pulse
        ESP_LOGD(TAG, "Partial position, direction change - sending single pulse");
        send_pulse = true;
      } else {
        // Same direction as before - double pulse
        ESP_LOGD(TAG, "Partial position, same direction - sending double pulse");
        send_double_pulse = true;
      }
    }
  } else if (dir == COVER_OPERATION_CLOSING) {
    if (this->position <= COVER_CLOSED + 0.00f) {
      // Already fully closed - nothing to do
      ESP_LOGV(TAG, "Already fully closed - no pulse needed");
    } else if (this->position >= COVER_OPEN - 0.00f) {
      // Fully open, want to close - single pulse
      ESP_LOGD(TAG, "Open to close - sending single pulse");
      send_pulse = true;
    } else {
      // Partially open
      if (this->last_operation_ != COVER_OPERATION_CLOSING) {
        // Different direction from previous - single pulse
        ESP_LOGD(TAG, "Partial position, direction change - sending single pulse");
        send_pulse = true;
      } else {
        // Same direction as before - double pulse
        ESP_LOGD(TAG, "Partial position, same direction - sending double pulse");
        send_double_pulse = true;
      }
    }
  }

  // Execute the appropriate pulse sequence
  ESP_LOGV(TAG, "Pulse decision: send_pulse=%s, send_double_pulse=%s", 
           send_pulse ? "true" : "false", send_double_pulse ? "true" : "false");
  
  if (send_double_pulse) {
    this->send_double_pulse_();
    this->safety_cycle_count_ += 2;  // Double pulse counts as 2 cycles
  } else if (send_pulse) {
    this->send_pulse_();
    this->safety_cycle_count_++;
  }

  // Update operation state
  this->set_current_operation_(dir, true);
  
  // Log and fire triggers
  if (dir != COVER_OPERATION_IDLE) {
    ESP_LOGI(TAG, "Starting %s operation to %.2f (cycle %u/%u)", 
             dir == COVER_OPERATION_OPENING ? "OPEN" : "CLOSE",
             this->target_position_,
             this->safety_cycle_count_, this->safety_max_cycles_);
             
    // Fire appropriate triggers
    if (dir == COVER_OPERATION_OPENING) {
      this->fire_on_open_triggers_();
    } else if (dir == COVER_OPERATION_CLOSING) {
      this->fire_on_close_triggers_();
    }
  } else {
    ESP_LOGI(TAG, "Stopping movement");
    this->fire_on_idle_triggers_();
  }
}

void ImpulseCover::set_current_operation_(cover::CoverOperation operation, bool is_triggered) {
  if (is_triggered) {
    this->current_trigger_operation_ = operation;
  }
  
  auto now = millis();
  this->current_operation = operation;
  this->start_dir_time_ = this->last_recompute_time_ = now;
  this->pulse_sent_ = false;
  
  if (operation != COVER_OPERATION_IDLE) {
    this->last_operation_ = operation;
  }
  
  this->publish_state();
  this->last_publish_time_ = now;
}

void ImpulseCover::recompute_position_() {
  if (this->current_operation == COVER_OPERATION_IDLE)
    return;

  const uint32_t now = millis();
  const uint32_t elapsed = now - this->last_recompute_time_;
  
  float dir;
  float action_dur;
  
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      dir = 1.0f;
      action_dur = this->open_duration_;
      break;
    case COVER_OPERATION_CLOSING:
      dir = -1.0f;
      action_dur = this->close_duration_;
      break;
    default:
      return;
  }
  
  // Calculate position based on time - using captured start position
  float progress = std::min(1.0f, static_cast<float>(elapsed) / action_dur);
  this->position = (this->current_operation == COVER_OPERATION_OPENING) ?
                   (this->position + progress) : (this->position - progress);
  
  
  // Clamp position
  if (this->position < COVER_CLOSED) this->position = COVER_CLOSED;
  if (this->position > COVER_OPEN) this->position = COVER_OPEN;
  
  this->last_recompute_time_ = now;
}

bool ImpulseCover::is_at_target_() const {
  const float tolerance = 0.00f;  // 1% tolerance
  
  switch (this->current_trigger_operation_) {
    case COVER_OPERATION_OPENING:
      return this->position >= this->target_position_ - tolerance;
    case COVER_OPERATION_CLOSING:
      return this->position <= this->target_position_ + tolerance;
    case COVER_OPERATION_IDLE:
      return this->current_operation == COVER_OPERATION_IDLE;
    default:
      return true;
  }
}

// Private helper methods
void ImpulseCover::send_pulse_() {
  this->send_pulse_internal_(false);
}

void ImpulseCover::send_double_pulse_() {
  this->send_pulse_internal_(true);
}

void ImpulseCover::send_pulse_internal_(bool double_pulse) {
  ESP_LOGV(TAG, "send_pulse_internal_ called with double_pulse=%s", double_pulse ? "true" : "false");
  
  if (this->output_ == nullptr) {
    ESP_LOGE(TAG, "Output is null! Cannot send pulse");
    return;
  }
  
  if (this->pulse_sent_) {
    ESP_LOGV(TAG, "Pulse already sent, skipping");
    return;
  }
  
  const uint32_t now = millis();
  ESP_LOGV(TAG, "Current time: %u, last_pulse_time_: %u, pulse_delay_: %u", 
           now, this->last_pulse_time_, this->pulse_delay_);
  
  // Check if enough time has passed since last pulse
  if ((now - this->last_pulse_time_) < this->pulse_delay_) {
    const char* pulse_type = double_pulse ? "double" : "single";
    ESP_LOGV(TAG, "Pulse too rapid, delaying %s pulse", pulse_type);
    
    std::string timeout_name = double_pulse ? "double_pulse_delay" : "single_pulse_delay";
    this->set_timeout(timeout_name, this->pulse_delay_ - (now - this->last_pulse_time_), [this, double_pulse]() {
      this->send_pulse_internal_(double_pulse);
    });
    return;
  }
  
  if (double_pulse) {
    ESP_LOGD(TAG, "Sending double control pulse");
    
    // First pulse
    ESP_LOGV(TAG, "Turning output ON (first pulse)");
    this->output_->turn_on();
    this->set_timeout("double_pulse_first_off", this->pulse_delay_ , [this]() { 
      ESP_LOGV(TAG, "Turning output OFF (after first pulse)");
      this->output_->turn_off();
      // Second pulse after a short delay
      this->set_timeout("double_pulse_second_on", this->pulse_delay_ , [this]() {
        ESP_LOGV(TAG, "Turning output ON (second pulse)");
        this->output_->turn_on();
        this->set_timeout("double_pulse_second_off", this->pulse_delay_ , [this]() { 
          ESP_LOGV(TAG, "Turning output OFF (after second pulse)");
          this->output_->turn_off(); 
        });
      });
    });
  } else {
    ESP_LOGD(TAG, "Sending single control pulse");
    
    ESP_LOGV(TAG, "Turning output ON");
    this->output_->turn_on();
    ESP_LOGV(TAG, "Setting timeout for output OFF in %ums", this->pulse_delay_);
    this->set_timeout("single_pulse_off", this->pulse_delay_, [this]() {
      ESP_LOGV(TAG, "Turning output OFF");
      this->output_->turn_off(); 
    });
  }
  
  this->last_pulse_time_ = millis();
  this->pulse_sent_ = true;
  ESP_LOGV(TAG, "Pulse sequence initiated, pulse_sent_ set to true");
}

void ImpulseCover::check_safety_() {
  if (this->current_operation == COVER_OPERATION_IDLE) {
    return;
  }
  
  // Check cycle count safety
  if (this->safety_cycle_count_ >= this->safety_max_cycles_) {
    ESP_LOGW(TAG, "Safety max cycles triggered (%u cycles)", this->safety_cycle_count_);
    this->safety_triggered_ = true;
    this->fire_on_safety_triggers_();
    this->start_direction_(COVER_OPERATION_IDLE);
    return;
  }
  
  // Auto-reset safety cycle count after period of inactivity
  const uint32_t now = millis();
  if (this->current_operation == COVER_OPERATION_IDLE && 
      this->safety_cycle_count_ > 0 && 
      (now - this->start_dir_time_) > 30000) {
    ESP_LOGV(TAG, "Auto-resetting safety cycle count after inactivity");
    this->safety_cycle_count_ = 0;
  }
}

#ifdef USE_BINARY_SENSOR
void ImpulseCover::update_position_from_sensors_(bool is_initialization) {
  bool has_open_sensor = (this->open_sensor_ != nullptr);
  bool has_close_sensor = (this->close_sensor_ != nullptr);
  bool open_sensor_active = has_open_sensor && this->get_sensor_state_(this->open_sensor_, this->open_sensor_inverted_);
  bool close_sensor_active = has_close_sensor && this->get_sensor_state_(this->close_sensor_, this->close_sensor_inverted_);
  
  ESP_LOGV(TAG, "Sensors: open=%s (active=%s), close=%s (active=%s)", 
           has_open_sensor ? "YES" : "NO", open_sensor_active ? "YES" : "NO",
           has_close_sensor ? "YES" : "NO", close_sensor_active ? "YES" : "NO");
  
  float old_position = this->position;
  bool position_updated = false;
  
  if (has_open_sensor && has_close_sensor) {
    // Both sensors configured - use precise logic
    if (open_sensor_active && !close_sensor_active) {
      if (is_initialization || this->position != COVER_OPEN) {
        this->position = COVER_OPEN;
        position_updated = true;
        if (is_initialization) {
          this->has_initial_state_ = true;
          ESP_LOGI(TAG, "Initial state: OPEN (open sensor active, close sensor inactive)");
        } else {
          ESP_LOGW(TAG, "Position misalignment detected: position=%.3f but open sensor active - correcting to OPEN", old_position);
        }
      }
    } else if (close_sensor_active && !open_sensor_active) {
      if (is_initialization || this->position != COVER_CLOSED) {
        this->position = COVER_CLOSED;
        position_updated = true;
        if (is_initialization) {
          this->has_initial_state_ = true;
          ESP_LOGI(TAG, "Initial state: CLOSED (close sensor active, open sensor inactive)");
        } else {
          ESP_LOGW(TAG, "Position misalignment detected: position=%.3f but close sensor active - correcting to CLOSED", old_position);
        }
      }
    } else if (!open_sensor_active && !close_sensor_active) {
      if (is_initialization) {
        this->position = 0.5f;  // Unknown position - intermediate
        this->has_initial_state_ = false;
        ESP_LOGD(TAG, "Initial state: UNKNOWN (neither sensor active) - position set to 50%%");
      } else if (this->position == COVER_OPEN || this->position == COVER_CLOSED) {
        // Position indicates endpoint but no sensor active - misalignment
        ESP_LOGW(TAG, "Position misalignment detected: position=%s but no sensor active - correcting to intermediate", 
                 this->position == COVER_OPEN ? "OPEN" : "CLOSED");
        this->position = 0.5f;
        position_updated = true;
      }
    } else {
      // Both sensors active - should not happen, probably misconfiguration
      if (is_initialization) {
        ESP_LOGW(TAG, "Both sensors active simultaneously - possible misconfiguration!");
        this->position = 0.5f;  // Unknown position
        this->has_initial_state_ = false;
        ESP_LOGD(TAG, "Initial state: CONFLICT (both sensors active) - position set to 50%%");
      } else {
        ESP_LOGW(TAG, "Sensor conflict detected: both sensors active simultaneously!");
      }
    }
  } else if (has_open_sensor && !has_close_sensor) {
    // Only open sensor configured
    if (open_sensor_active) {
      if (is_initialization || this->position != COVER_OPEN) {
        this->position = COVER_OPEN;
        position_updated = true;
        if (is_initialization) {
          this->has_initial_state_ = true;
          ESP_LOGI(TAG, "Initial state: OPEN (open sensor active)");
        } else {
          ESP_LOGW(TAG, "Position misalignment detected: position=%.3f but open sensor active - correcting to OPEN", old_position);
        }
      }
    } else {
      if (is_initialization) {
        this->position = COVER_CLOSED;  // Default to closed when open sensor inactive
        this->has_initial_state_ = false;
        ESP_LOGD(TAG, "Initial state: CLOSED (open sensor inactive, assuming closed)");
      } else if (this->position == COVER_OPEN) {
        // Position indicates open but open sensor inactive - misalignment
        ESP_LOGW(TAG, "Position misalignment detected: position=OPEN but open sensor inactive - correcting to CLOSED");
        this->position = COVER_CLOSED;
        position_updated = true;
      }
    }
  } else if (!has_open_sensor && has_close_sensor) {
    // Only close sensor configured
    if (close_sensor_active) {
      if (is_initialization || this->position != COVER_CLOSED) {
        this->position = COVER_CLOSED;
        position_updated = true;
        if (is_initialization) {
          this->has_initial_state_ = true;
          ESP_LOGI(TAG, "Initial state: CLOSED (close sensor active)");
        } else {
          ESP_LOGW(TAG, "Position misalignment detected: position=%.3f but close sensor active - correcting to CLOSED", old_position);
        }
      }
    } else {
      if (is_initialization) {
        this->position = COVER_OPEN;  // Default to open when close sensor inactive
        this->has_initial_state_ = false;
        ESP_LOGD(TAG, "Initial state: OPEN (close sensor inactive, assuming open)");
      } else if (this->position == COVER_CLOSED) {
        // Position indicates closed but close sensor inactive - misalignment
        ESP_LOGW(TAG, "Position misalignment detected: position=CLOSED but close sensor inactive - correcting to OPEN");
        this->position = COVER_OPEN;
        position_updated = true;
      }
    }
  } else if (is_initialization) {
    // No sensors configured - keep restore state or default
    ESP_LOGV(TAG, "No sensors configured - keeping current position: %.2f", this->position);
  }
  
  if (!is_initialization) {
    if (position_updated) {
      ESP_LOGI(TAG, "Position corrected based on sensor feedback");
      this->publish_state();
    } else {
      ESP_LOGV(TAG, "Sensor alignment check passed - no correction needed");
    }
  }
}

void ImpulseCover::check_sensor_alignment_() {
  ESP_LOGV(TAG, "Checking sensor alignment with current position...");
  ESP_LOGV(TAG, "Current position: %.3f", this->position);
  this->update_position_from_sensors_(false);
}

void ImpulseCover::endstop_reached_(bool open_endstop) {
  const uint32_t now = millis();
  
  ESP_LOGV(TAG, "endstop_reached_ called - open_endstop=%s", open_endstop ? "true" : "false");
  ESP_LOGV(TAG, "Current state: position=%.3f, current_operation=%d, current_trigger_operation_=%d", 
           this->position, static_cast<int>(this->current_operation), static_cast<int>(this->current_trigger_operation_));
  ESP_LOGV(TAG, "Expected operation for this endstop: %d", 
           static_cast<int>(open_endstop ? COVER_OPERATION_OPENING : COVER_OPERATION_CLOSING));
  
  // Set position based on endstop
  float old_position = this->position;
  this->position = open_endstop ? COVER_OPEN : COVER_CLOSED;
  ESP_LOGD(TAG, "Position updated from %.3f to %.3f (%s endstop)", 
           old_position, this->position, open_endstop ? "OPEN" : "CLOSE");
  
  // Only act if endstop activated while moving in the right direction
  bool is_correct_direction = (this->current_trigger_operation_ == (open_endstop ? COVER_OPERATION_OPENING : COVER_OPERATION_CLOSING));
  ESP_LOGV(TAG, "Direction check: is_correct_direction=%s", is_correct_direction ? "true" : "false");
  
  if (is_correct_direction) {
    float dur = (now - this->start_dir_time_) / 1e3f;
    ESP_LOGI(TAG, "'%s' - %s endstop reached. Took %.1fs.",
             this->get_name().c_str(), open_endstop ? "Open" : "Close", dur);
  } else {
    ESP_LOGD(TAG, "Ignoring %s endstop - not moving in expected direction (current_trigger=%d, expected=%d)", 
             open_endstop ? "OPEN" : "CLOSE", 
             static_cast<int>(this->current_trigger_operation_),
             static_cast<int>(open_endstop ? COVER_OPERATION_OPENING : COVER_OPERATION_CLOSING));
  }
  
  ESP_LOGV(TAG, "Stopping operation and setting to IDLE");
  this->set_current_operation_(COVER_OPERATION_IDLE, false);
  ESP_LOGV(TAG, "endstop_reached_ completed");
}

bool ImpulseCover::get_sensor_state_(binary_sensor::BinarySensor *sensor, bool inverted) {
  return sensor ? (inverted ? !sensor->state : sensor->state) : false;
}

void ImpulseCover::set_open_sensor(binary_sensor::BinarySensor *sensor) {
  this->open_sensor_ = sensor;
  if (sensor) {
    sensor->add_on_state_callback([this](bool state) {
      if (this->get_sensor_state_(this->open_sensor_, this->open_sensor_inverted_)) {
         this->endstop_reached_(true);
      }
    });
  }
}

void ImpulseCover::set_close_sensor(binary_sensor::BinarySensor *sensor) {
  this->close_sensor_ = sensor;
  if (sensor) {
    sensor->add_on_state_callback([this](bool state) {
      if (this->get_sensor_state_(this->close_sensor_, this->close_sensor_inverted_)) {
        this->endstop_reached_(false);
      } 
    });
  }
}
#endif

// Automation trigger methods
void ImpulseCover::add_on_open_trigger(Trigger<> *trigger) {
  this->on_open_triggers_.push_back(trigger);
}

void ImpulseCover::add_on_close_trigger(Trigger<> *trigger) {
  this->on_close_triggers_.push_back(trigger);
}

void ImpulseCover::add_on_idle_trigger(Trigger<> *trigger) {
  this->on_idle_triggers_.push_back(trigger);
}

void ImpulseCover::add_on_safety_trigger(SafetyTrigger *trigger) {
  this->on_safety_triggers_.push_back(trigger);
}

// Protected helper methods for firing triggers
void ImpulseCover::fire_on_open_triggers_() {
  for (auto *trigger : this->on_open_triggers_) {
    trigger->trigger();
  }
}

void ImpulseCover::fire_on_close_triggers_() {
  for (auto *trigger : this->on_close_triggers_) {
    trigger->trigger();
  }
}

void ImpulseCover::fire_on_idle_triggers_() {
  for (auto *trigger : this->on_idle_triggers_) {
    trigger->trigger();
  }
}

void ImpulseCover::fire_on_safety_triggers_() {
  for (auto *trigger : this->on_safety_triggers_) {
    trigger->trigger();
  }
}

}  // namespace impulse_cover
}  // namespace esphome
