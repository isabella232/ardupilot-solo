// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/// @file	AP_MotorsHeli_Single.h
/// @brief	Motor control class for traditional heli

#ifndef __AP_MOTORS_HELI_SINGLE_H__
#define __AP_MOTORS_HELI_SINGLE_H__

#include <inttypes.h>
#include <AP_Common.h>
#include <AP_Math.h>
#include <RC_Channel.h>

#include "AP_MotorsHeli.h"
#include "AP_MotorsHeli_RSC.h"

// rsc and aux function output channels
#define AP_MOTORS_HELI_SINGLE_RSC                              CH_8
#define AP_MOTORS_HELI_SINGLE_AUX                              CH_7

// servo position defaults
#define AP_MOTORS_HELI_SINGLE_SERVO1_POS                       -60
#define AP_MOTORS_HELI_SINGLE_SERVO2_POS                       60
#define AP_MOTORS_HELI_SINGLE_SERVO3_POS                       180

// swash type definitions
#define AP_MOTORS_HELI_SINGLE_SWASH_CCPM                       0
#define AP_MOTORS_HELI_SINGLE_SWASH_H1                         1

// tail types
#define AP_MOTORS_HELI_SINGLE_TAILTYPE_SERVO                   0
#define AP_MOTORS_HELI_SINGLE_TAILTYPE_SERVO_EXTGYRO           1
#define AP_MOTORS_HELI_SINGLE_TAILTYPE_DIRECTDRIVE_VARPITCH    2
#define AP_MOTORS_HELI_SINGLE_TAILTYPE_DIRECTDRIVE_FIXEDPITCH  3

// default direct-drive variable pitch speed
#define AP_MOTOR_HELI_SINGLE_DDTAIL_DEFAULT                    500

// default external gyro gain
#define AP_MOTORS_HELI_SINGLE_EXT_GYRO_GAIN                    350

// COLYAW parameter min and max values
#define AP_MOTORS_HELI_SINGLE_COLYAW_RANGE             10.0f

#define AP_MOTORS_HELI_SINGLE_TAIL_RAMP_INCREMENT              5       // 5 is 2 seconds for direct drive tail rotor to reach to full speed (5 = (2sec*100hz)/1000)

/// @class      AP_MotorsHeli_Single
class AP_MotorsHeli_Single : public AP_MotorsHeli {
public:
    // constructor
    AP_MotorsHeli_Single(RC_Channel&    servo_aux,
                         RC_Channel&    servo_rsc,
                         RC_Channel&    servo_1,
                         RC_Channel&    servo_2,
                         RC_Channel&    servo_3,
                         RC_Channel&    servo_4,
                         uint16_t       loop_rate,
                         uint16_t       speed_hz = AP_MOTORS_HELI_SPEED_DEFAULT) :
        AP_MotorsHeli(loop_rate, speed_hz),
        _servo_aux(servo_aux),
        _servo_1(servo_1),
        _servo_2(servo_2),
        _servo_3(servo_3),
        _servo_4(servo_4),
        _main_rotor(servo_rsc, AP_MOTORS_HELI_SINGLE_RSC, loop_rate),
        _tail_rotor(servo_aux, AP_MOTORS_HELI_SINGLE_AUX, loop_rate)
    {
        AP_Param::setup_object_defaults(this, var_info);
    };

    // init
    void Init();

    // set update rate to motors - a value in hertz
    // you must have setup_motors before calling this
    void set_update_rate(uint16_t speed_hz);

    // enable - starts allowing signals to be sent to motors
    void enable();

    // output_test - spin a motor at the pwm value specified
    //  motor_seq is the motor's sequence number from 1 to the number of motors on the frame
    //  pwm value is an actual pwm value that will be output, normally in the range of 1000 ~ 2000
    void output_test(uint8_t motor_seq, int16_t pwm);

    // allow_arming - returns true if main rotor is spinning and it is ok to arm
    bool allow_arming() const;

    // set_desired_rotor_speed - sets target rotor speed as a number from 0 ~ 1000
    void set_desired_rotor_speed(int16_t desired_speed);

    // get_estimated_rotor_speed - gets estimated rotor speed as a number from 0 ~ 1000
    int16_t get_estimated_rotor_speed() const { return _main_rotor.get_estimated_speed(); }

    // get_desired_rotor_speed - gets target rotor speed as a number from 0 ~ 1000
    int16_t get_desired_rotor_speed() const { return _main_rotor.get_desired_speed(); }

    // rotor_speed_above_critical - return true if rotor speed is above that critical for flight
    bool rotor_speed_above_critical() const { return _main_rotor.get_estimated_speed() > _main_rotor.get_critical_speed(); }

    // recalc_scalers - recalculates various scalers used.  Should be called at about 1hz to allow users to see effect of changing parameters
    void recalc_scalers();

    // get_motor_mask - returns a bitmask of which outputs are being used for motors or servos (1 means being used)
    //  this can be used to ensure other pwm outputs (i.e. for servos) do not conflict
    uint16_t get_motor_mask();

    // _tail_type - returns the tail type (servo, servo with ext gyro, direct drive var pitch, direct drive fixed pitch)
    int16_t tail_type() const { return _tail_type; }

    // ext_gyro_gain - gets and sets external gyro gain as a pwm (1000~2000)
    int16_t ext_gyro_gain() const { return _ext_gyro_gain; }
    void ext_gyro_gain(int16_t pwm) { _ext_gyro_gain = pwm; }

    // has_flybar - returns true if we have a mechical flybar
    bool has_flybar() const { return _flybar_mode; }

    // get_phase_angle - returns phase angle
    int16_t get_phase_angle() const { return _phase_angle; }

    // supports_yaw_passthrought - returns true if we support yaw passthrough
    bool supports_yaw_passthrough() const { return _tail_type == AP_MOTORS_HELI_SINGLE_TAILTYPE_SERVO_EXTGYRO; }
    
    // var_info
    static const struct AP_Param::GroupInfo var_info[];

protected:

    // output - sends commands to the motors
    void output_armed_stabilizing();
    void output_disarmed();

    // reset_servos - free up the swash servos for maximum movement
    void reset_servos();

    // init_servos - initialize the servos
    void init_servos();

    // calculate_roll_pitch_collective_factors - calculate factors based on swash type and servo position
    void calculate_roll_pitch_collective_factors();

    // heli_move_swash - moves swash plate to attitude of parameters passed in
    void move_swash(int16_t roll_out, int16_t pitch_out, int16_t coll_in, int16_t yaw_out);

    // move_yaw - moves the yaw servo
    void move_yaw(int16_t yaw_out);

    // write_aux - outputs pwm onto output aux channel (ch7). servo_out parameter is of the range 0 ~ 1000
    void write_aux(int16_t servo_out);

    // external objects we depend upon
    RC_Channel&     _servo_aux;                 // output to ext gyro gain and tail direct drive esc (ch7)
    RC_Channel&     _servo_1;                   // swash plate servo #1
    RC_Channel&     _servo_2;                   // swash plate servo #2
    RC_Channel&     _servo_3;                   // swash plate servo #3
    RC_Channel&     _servo_4;                   // tail servo

    AP_MotorsHeli_RSC   _main_rotor;            // main rotor
    AP_MotorsHeli_RSC   _tail_rotor;            // tail rotor

    // parameters
    AP_Int16        _servo1_pos;                // Angular location of swash servo #1
    AP_Int16        _servo2_pos;                // Angular location of swash servo #2
    AP_Int16        _servo3_pos;                // Angular location of swash servo #3    
    AP_Int16        _tail_type;                 // Tail type used: Servo, Servo with external gyro, direct drive variable pitch or direct drive fixed pitch
    AP_Int8         _swash_type;                // Swash Type Setting - either 3-servo CCPM or H1 Mechanical Mixing
    AP_Int16        _ext_gyro_gain;             // PWM sent to external gyro on ch7 when tail type is Servo w/ ExtGyro
    AP_Int16        _phase_angle;               // Phase angle correction for rotor head.  If pitching the swash forward induces a roll, this can be correct the problem
    AP_Float        _collective_yaw_effect;     // Feed-forward compensation to automatically add rudder input when collective pitch is increased. Can be positive or negative depending on mechanics.
    AP_Int8         _flybar_mode;               // Flybar present or not.  Affects attitude controller used during ACRO flight mode
    AP_Int16        _direct_drive_tailspeed;    // Direct Drive VarPitch Tail ESC speed (0 ~ 1000)

};

#endif  // __AP_MOTORS_HELI_SINGLE_H__