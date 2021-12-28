#include "profiles.h"
#include <gearbox_config.h>

// REMEMBER
// Higher number = LESS pressure

// LOAD -10% (NEG), 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
pressure_map map_1_2 = {530, 520, 510, 500, 490, 480, 470, 460, 450, 420, 400};
pressure_map map_2_3 = {500, 490, 480, 470, 460, 450, 440, 430, 420, 410, 400};
pressure_map map_3_4 = {500, 485, 475, 462, 452, 336, 431, 420, 399, 378, 357};
pressure_map map_4_5 = {490, 470, 450, 430, 410, 390, 370, 350, 330, 310, 300};

pressure_map map_2_1 = {540, 530, 520, 510, 490, 480, 470, 460, 450, 420, 400};
pressure_map map_3_2 = {415, 415, 411, 410, 405, 400, 390, 380, 370, 360, 350}; // BEEFY
pressure_map map_4_3 = {470, 462, 430, 420, 410, 400, 380, 360, 340, 320, 300};
pressure_map map_5_4 = {430, 431, 395, 385, 385, 370, 360, 360, 350, 340, 325};

const float pressure_temp_normalizer[17] = {
    0.8, 0.8, 0.8, 0.8, 0.8, // -40-0C (0-40)
    0.8, 0.82, 0.84, 0.86, 0.91, // 10-50C (50-90)
    0.97, 1.0, 1.01, 1.01, 1.02, 1.02, 1.02 //60C+ (100-160)
};

// 0, 1k, 2k, 3k, 4k, 5k, 6k, 7k, 8k RPM
const float rpm_normalizer[9] = {1.04, 1.02, 1.00, 0.98, 0.94, 0.91, 0.88, 0.85, 0.82};

float find_temp_multiplier(int temp_raw) {
    if (temp_raw < 0) { return pressure_temp_normalizer[0]; }
    else if (temp_raw > 160) { return pressure_temp_normalizer[16]; }
    int min = temp_raw/10;
    int max = min+1;
    float dy = pressure_temp_normalizer[max] - pressure_temp_normalizer[min];
    float dx = (max-min)*10;
    return (pressure_temp_normalizer[min] + ((dy/dx)) * (temp_raw-(min*10)));
}

float find_rpm_multiplier(int engine_rpm) {
    if (engine_rpm < 0) { return rpm_normalizer[0]; }
    else if (engine_rpm > 8000) { return rpm_normalizer[8]; }
    int min = engine_rpm/1000;
    int max = min+1;
    float dy = rpm_normalizer[max] - rpm_normalizer[min];
    float dx = (max-min)*1000;
    return (rpm_normalizer[min] + ((dy/dx)) * (engine_rpm-(min*1000)));
}

inline uint16_t locate_pressure_map_value(pressure_map map, int percent) {
    if (percent <= 0) { return map[0]; }
    else if (percent >= 100) { return map[10]; }
    else {
        int min = percent/10;
        int max = min+1;
        float dy = map[max] - map[min];
        float dx = (max-min)*10;
        return (map[min] + ((dy/dx)) * (percent-(min*10)));
    }
}

uint16_t find_spc_pressure(pressure_map map, SensorData* sensors, float shift_speed = 1.0) {
    if (shift_speed > 1.1) {
        shift_speed = 1.1;
    } else if (shift_speed < 0.9) {
        shift_speed = 0.9;
    }
    // SPC reacts to throttle position (Pedal) (Firmness dictates how aggressively to traverse this model)
    int load = (sensors->pedal_pos*100/250);
    return locate_pressure_map_value(map, load) * find_temp_multiplier(sensors->atf_temp) * find_rpm_multiplier(sensors->engine_rpm) * shift_speed;
}

uint16_t find_mpc_pressure(pressure_map map, SensorData* sensors, float shift_firmness) {
    if (shift_firmness > 1.1) {
        shift_firmness = 1.1;
    } else if (shift_firmness < 0.9) {
        shift_firmness = 0.9;
    }
    // MPC reacts to Torque (Also sets pressure for SPC. Shift firmness can be increased)
    int load = sensors->static_torque*100/MAX_TORQUE_RATING_NM;
    if (load < 0) { load = 0; } // Pulling engine
    return locate_pressure_map_value(map, load) * find_temp_multiplier(sensors->atf_temp) * find_rpm_multiplier(sensors->engine_rpm) * shift_firmness;
}

char AgilityProfile::get_display_gear(GearboxGear target, GearboxGear actual) {
   switch (actual) {
        case GearboxGear::Park:
            return 'P';
        case GearboxGear::Reverse_First:
        case GearboxGear::Reverse_Second:
            return 'R';
        case GearboxGear::Neutral:
            return 'N';
        case GearboxGear::First:
            return '1';
        case GearboxGear::Second:
            return '2';
        case GearboxGear::Third:
            return '3';
        case GearboxGear::Fourth:
            return '4';
        case GearboxGear::Fifth:
            return '5';
        case GearboxGear::Sixth:
            return '6';
        case GearboxGear::Seventh:
            return '7';
        case GearboxGear::SignalNotAvaliable:
            return 0xFF;
        default:
            return ' ';
    }
}


ShiftData AgilityProfile::get_shift_data(ProfileGearChange requested, SensorData* sensors, float shift_speed, float shift_firmness) {
    return standard->get_shift_data(requested, sensors, 0.9, 0.9);
}

bool AgilityProfile::should_upshift(GearboxGear current_gear, SensorData* sensors) {
    return standard->should_upshift(current_gear, sensors);
}

bool AgilityProfile::should_downshift(GearboxGear current_gear, SensorData* sensors) {
    return standard->should_downshift(current_gear, sensors);
}


ShiftData ComfortProfile::get_shift_data(ProfileGearChange requested, SensorData* sensors, float shift_speed, float shift_firmness) {
    return standard->get_shift_data(requested, sensors, 1.1, 1.1);
}

char ComfortProfile::get_display_gear(GearboxGear target, GearboxGear actual) {
    switch (actual) {
        case GearboxGear::Park:
            return 'P';
        case GearboxGear::Reverse_First:
        case GearboxGear::Reverse_Second:
            return 'R';
        case GearboxGear::Neutral:
            return 'N';
        case GearboxGear::First:
            return '1';
        case GearboxGear::Second:
            return '2';
        case GearboxGear::Third:
            return '3';
        case GearboxGear::Fourth:
            return '4';
        case GearboxGear::Fifth:
            return '5';
        case GearboxGear::Sixth:
            return '6';
        case GearboxGear::Seventh:
            return '7';
        case GearboxGear::SignalNotAvaliable:
            return 0xFF;
        default:
            return ' ';
    }
}

bool ComfortProfile::should_upshift(GearboxGear current_gear, SensorData* sensors) {
    return standard->should_upshift(current_gear, sensors);
}

bool ComfortProfile::should_downshift(GearboxGear current_gear, SensorData* sensors) {
    if (current_gear == GearboxGear::Second || current_gear == GearboxGear::First) {
        return false;
    }
    return standard->should_downshift(current_gear, sensors);
}

ShiftData WinterProfile::get_shift_data(ProfileGearChange requested, SensorData* sensors, float shift_speed, float shift_firmness) {
    return comfort->get_shift_data(requested, sensors, 1.0); // Same shift quality as C
}

char WinterProfile::get_display_gear(GearboxGear target, GearboxGear actual) {
    switch (target) {
        case GearboxGear::Park:
            return 'P';
        case GearboxGear::Reverse_First:
        case GearboxGear::Reverse_Second:
            return 'R';
        case GearboxGear::Neutral:
            return 'N';
        case GearboxGear::First:
            return '1';
        case GearboxGear::Second:
            return '2';
        case GearboxGear::Third:
            return '3';
        case GearboxGear::Fourth:
            return '4';
        case GearboxGear::Fifth:
            return '5';
        case GearboxGear::Sixth:
            return '6';
        case GearboxGear::Seventh:
            return '7';
        case GearboxGear::SignalNotAvaliable:
            return 0xFF;
        default:
            return ' ';
    }
}

bool WinterProfile::should_upshift(GearboxGear current_gear, SensorData* sensors) {
    return false;
}

bool WinterProfile::should_downshift(GearboxGear current_gear, SensorData* sensors) {
    return false;
}

void StandardProfile::on_upshift_complete(ShiftResponse resp, uint8_t from_gear, SensorData* sensors) {
    ESP_LOGI("ADAPT", "Adaptation called. Shifted? %d, Measured shift? %d, Shift time %d ms, Shift d_rpm avg: %d", 
        resp.shifted, resp.valid_measurement, resp.time_ms, resp.avg_d_rpm
    );
    /*
    float* target;
    switch(from_gear) {
        case 1:
            target = &adaptation_1_2;
            break;
        case 2:
            target = &adaptation_2_3;
            break;
        case 3:
            target = &adaptation_3_4;
            break;
        case 4:
            target = &adaptation_4_5;
            break;
        default:
            return;
    }
    int min_time = 500;
    int max_time = 600;
    if (from_gear == 1 || from_gear == 4) {
        min_time = 400;
        max_time = 500;
    }
    if (resp.shifted && resp.valid_measurement) {
        if (resp.time_ms < min_time) {
            *target += 0.01;
        } else if (resp.time_ms > max_time) {
            *target -= 0.01;
        }
    }
    ESP_LOGI("ADAPT", "Adaptation block is now %.2f %.2f %.2f %.2f", adaptation_1_2, adaptation_2_3, adaptation_3_4, adaptation_4_5);
    */
}

ShiftData StandardProfile::get_shift_data(ProfileGearChange requested, SensorData* sensors, float shift_speed, float shift_firmness) {
    if (requested == ProfileGearChange::ONE_TWO) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_1_2, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_1_2, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else if (requested == ProfileGearChange::TWO_THREE) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_2_3, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_2_3, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else if (requested == ProfileGearChange::THREE_FOUR) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_3_4, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_3_4, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else if (requested == ProfileGearChange::FOUR_FIVE) {
       return ShiftData {
            .spc_pwm = find_spc_pressure(map_4_5, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_4_5, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } 
    // Downshifts
    else if (requested == ProfileGearChange::TWO_ONE) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_2_1, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_2_1, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else if (requested == ProfileGearChange::THREE_TWO) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_3_2, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_3_2, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else if (requested == ProfileGearChange::FOUR_THREE) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_4_3, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_4_3, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else if (requested == ProfileGearChange::FIVE_FOUR) {
        return ShiftData {
            .spc_pwm = find_spc_pressure(map_5_4, sensors, shift_speed),
            .mpc_pwm = find_mpc_pressure(map_5_4, sensors, shift_firmness),
            .targ_ms = 500 
        };
    } else {
        return DEFAULT_SHIFT_DATA;
    }
}

char StandardProfile::get_display_gear(GearboxGear target, GearboxGear actual) {
    switch (target) {
        case GearboxGear::Park:
            return 'P';
        case GearboxGear::Reverse_First:
        case GearboxGear::Reverse_Second:
            return 'R';
        case GearboxGear::Neutral:
            return 'N';
        case GearboxGear::First:
            return '1';
        case GearboxGear::Second:
            return '2';
        case GearboxGear::Third:
            return '3';
        case GearboxGear::Fourth:
            return '4';
        case GearboxGear::Fifth:
            return '5';
        case GearboxGear::Sixth:
            return '6';
        case GearboxGear::Seventh:
            return '7';
        case GearboxGear::SignalNotAvaliable:
            return 0xFF;
        default:
            return ' ';
    }
}



bool StandardProfile::should_upshift(GearboxGear current_gear, SensorData* sensors) {
    if (current_gear == GearboxGear::Fifth) { return false; }
    int curr_rpm = sensors->input_rpm;
    if (curr_rpm > 4000) { // Protect da engine
        return true;
    }
    float pedal_perc = ((float)sensors->pedal_pos*100)/250.0;
    float rpm_percent = (float)(sensors->input_rpm-1000)*100.0 / (float)(4500-1000);
    int rpm_threshold = 0;
    // Load (idx) vs pedal
    if (current_gear == GearboxGear::First) {
        rpm_threshold = 1500;
    } else if (current_gear == GearboxGear::Second) {
        rpm_threshold = 1500;
    } else if (current_gear == GearboxGear::Third) {
        rpm_threshold = 1500;
    } else if (current_gear == GearboxGear::Fourth) {
        rpm_threshold = 1500;
    }
    unsigned long t =  esp_timer_get_time()/1000;

    if (curr_rpm > rpm_threshold && pedal_perc <= rpm_percent && t-sensors->last_shift_time > 2000) {
        return true;
    }
    return false;
}

bool StandardProfile::should_downshift(GearboxGear current_gear, SensorData* sensors) {
    if (current_gear == GearboxGear::First) { return false; }
    float pedal_perc = ((float)sensors->pedal_pos*100)/250.0;
    float rpm_percent = (float)(sensors->input_rpm-1000)*100.0/(float)(4500-1000);
    unsigned long t =  esp_timer_get_time()/1000;
    if (sensors->input_rpm < 1000 && current_gear != GearboxGear::Third && t-sensors->last_shift_time > 2000) {
        return true;
    } else if (sensors->output_rpm < 500 && current_gear == GearboxGear::Third && t-sensors->last_shift_time > 2000) { // 3-2 downshift only at low speeds if idle
        return true;
    }
    else if (sensors->input_rpm < 2000 && pedal_perc > 60 && pedal_perc >= rpm_percent*2 && t-sensors->last_shift_time > 2000) {
        if (current_gear == GearboxGear::Second) { // Prevent 2-1 downshift (Too twitchy)
            return false;
        }
        return true;
    } else {
        return false;
    }
}

char ManualProfile::get_display_gear(GearboxGear target, GearboxGear actual) {
    switch (target) {
        case GearboxGear::Park:
            return 'P';
        case GearboxGear::Reverse_First:
        case GearboxGear::Reverse_Second:
            return 'R';
        case GearboxGear::Neutral:
            return 'N';
        case GearboxGear::First:
            return '1';
        case GearboxGear::Second:
            return '2';
        case GearboxGear::Third:
            return '3';
        case GearboxGear::Fourth:
            return '4';
        case GearboxGear::Fifth:
            return '5';
        case GearboxGear::Sixth:
            return '6';
        case GearboxGear::Seventh:
            return '7';
        case GearboxGear::SignalNotAvaliable:
            return 0xFF;
        default:
            return ' ';
    }
}

bool ManualProfile::should_upshift(GearboxGear current_gear, SensorData* sensors) {
    return false;
}

bool ManualProfile::should_downshift(GearboxGear current_gear, SensorData* sensors) {
    return false;
}


AgilityProfile* agility = new AgilityProfile();
ComfortProfile* comfort = new ComfortProfile();
WinterProfile* winter = new WinterProfile();
ManualProfile* manual = new ManualProfile();
StandardProfile* standard = new StandardProfile();
