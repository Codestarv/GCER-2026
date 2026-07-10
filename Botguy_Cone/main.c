#include <kipr/wombat.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// ===========================================================================
// MACRO DEFINITIONS
// ===========================================================================

// --- Ports ---
#define CLAW 2
#define ARM 3
#define MOTORl 3
#define MOTORr 0
#define SENSORl 5
#define SENSORr 2
#define LIGHT_SENSOR_PORT 5

// --- Sensor Thresholds & Multipliers ---
#define BLACK 3000
#define WHITE 2000
#define RIGHT_MOTOR_CORRECTION 1.002
#define CORRECTION 0.3

// --- Motor Speeds ---
#define MOTOR_SPEED_250 250
#define MOTOR_SPEED_500 500
#define MOTOR_SPEED_1000 1000
#define MOTOR_SPEED_1500 1500

// --- Claw Positions ---
#define CLAW_UPPER_OPEN 450
#define CLAW_OPEN 600
#define CLAW_CONE 560
#define CLAW_BOTGUY 874    
#define CLAW_DOOR 1080
#define CLAW_CLOSE 1130

// --- Arm Positions ---
#define ARM_START 300
#define ARM_BOTGUY_CARRY 402
#define ARM_CONE_UP 500
#define ARM_UP 820
#define ARM_BOTGUY 1500
#define ARM_DOOR 1630
#define ARM_DOWN_BOTGUY 1750
#define ARM_CONE 1940

// --- Navigation & Timings ---
#define DEGREES 5
#define SHUT_DOWN_SECS 11
#define CONE1_LINE_FOLLOW 6000
#define CONE2_LINE_FOLLOW 2300
#define RETURN_LINE_FOLLOW 11000
#define RAMP_LINE_FOLLOW 15300
#define TO_BOTGUY_LINE_FOLLOW 4100


// ===========================================================================
// SERVO MOVEMENT HELPERS
// ===========================================================================

// Generic function to move any servo gradually to prevent jerking
void move_servo_gradual(int port, int end_pos, int step_size, int delay_ms, const char* name) {
    enable_servos();
    int current_pos = get_servo_position(port);
    printf("move_%s(START): current_pos= %d, end_pos= %d\n", name, current_pos, end_pos);
    
    while (current_pos != end_pos) {
        if (current_pos > end_pos) {
            current_pos -= step_size;
            if (current_pos < end_pos) current_pos = end_pos;   
        } else {
            current_pos += step_size;
            if (current_pos > end_pos) current_pos = end_pos;   
        }
        set_servo_position(port, current_pos);
        msleep(delay_ms);
    }
    disable_servos();
    printf("move_%s(END): current_pos= %d, end_pos= %d\n", name, current_pos, end_pos);
}

void move_arm(int end_pos) {
    move_servo_gradual(ARM, end_pos, 10, 15, "arm");
}

void move_claw(int end_pos) {
    move_servo_gradual(CLAW, end_pos, 30, 60, "claw");
}

void move_arm_fast_slow_down(int position) {        
    enable_servos();
    set_servo_position(ARM, position - 500);
    disable_servos();
    enable_servos();
    msleep(100);
    move_arm(position);
    msleep(100);
}  

// ===========================================================================
// BASIC DRIVING
// ===========================================================================

void drive(int speed1, int speed2) {
    mav(MOTORl, speed1);
    mav(MOTORr, (int)(speed2 * RIGHT_MOTOR_CORRECTION));
}

// ===========================================================================
// SENSOR-BASED DRIVING
// ===========================================================================

void drive_till_blk() {
    printf("DRIVE_TILL_BLK STARTED\n");
    while (analog(SENSORl) <= BLACK || analog(SENSORr) <= BLACK) {
        drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    }
    printf("DRIVE_TILL_BLK ENDED\n");
}    

void drive_till_blk_speed(int speed) {
    while (analog(SENSORl) <= BLACK || analog(SENSORr) <= BLACK) {
        drive(speed, speed);
    }
}

void drive_till_blk_time(int timeout_secs) {
    double start_time = seconds();
    printf("drive_till_blk(START): start_time= %d, timeout_secs= %d\n", (int)start_time, timeout_secs);
    while (analog(SENSORl) <= BLACK || analog(SENSORr) <= BLACK) {
        if (seconds() - start_time >= timeout_secs) {
            printf("drive_till_blk(): TIMEOUT after %d secs\n", timeout_secs);
            break;
        }
        drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    }
    printf("drive_till_blk(END): seconds= %d\n", (int)seconds());
    ao();
}

void drive_till_white(int timeout_secs) {
    double start_time = seconds();
    printf("drive_till_white(START): start_time= %d, timeout_secs= %d\n", (int)start_time, timeout_secs);
    while (analog(SENSORl) >= WHITE || analog(SENSORr) >= WHITE) {
        if (seconds() - start_time >= timeout_secs) {
            printf("drive_till_white(): TIMEOUT after %d secs\n", timeout_secs);
            break;
        }
        drive(MOTOR_SPEED_1000, MOTOR_SPEED_1000);
    }
    printf("drive_till_white(END): seconds= %d\n", (int)seconds());
    ao();
}

void back_till_blk() {
    while (analog(SENSORl) <= BLACK && analog(SENSORr) <= BLACK) {
        drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    }
}

// ===========================================================================
// TURNING FUNCTIONS
// ===========================================================================

void right_degrees(int degrees) {
    printf("RIGHT TURN DEGREES\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = (int)(12.7 * degrees);
    
    while(gmpc(MOTORl) < ticks || gmpc(MOTORr) > -ticks) {
        drive(MOTOR_SPEED_500, -MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}

void right_degrees_speed(int degrees) {
    printf("RIGHT TURN DEGREES SPEED\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = (int)(12.7 * degrees);
    
    while(gmpc(MOTORl) < ticks || gmpc(MOTORr) > -ticks) {
        drive(MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    }
    ao();
    msleep(50);
}

void right_degrees_less(int degrees) {
    printf("RIGHT TURN LESS\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 2 * degrees;
    while(gmpc(MOTORl) < ticks || gmpc(MOTORr) > -ticks) {
        drive(MOTOR_SPEED_500, -MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}

void left_degrees(int degrees) {
    printf("LEFT TURN START\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = (int)(12.7 * degrees);
    while(gmpc(MOTORl) > -ticks || gmpc(MOTORr) < ticks) {
        drive(-MOTOR_SPEED_500, MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
    printf("LEFT TURN END: Motor_L= %d, Motor_R= %d\n", gmpc(MOTORl), gmpc(MOTORr));
}

void left_degrees_less(int degrees) {
    printf("LEFT TURN LESS START\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 2 * degrees;
    while(gmpc(MOTORl) > -ticks || gmpc(MOTORr) < ticks) {
        drive(-MOTOR_SPEED_500, MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
    printf("LEFT TURN LESS END: Motor_L= %d, Motor_R= %d\n", gmpc(MOTORl), gmpc(MOTORr));
}

// ===========================================================================
// LINE FOLLOWING & ALIGNMENT
// ===========================================================================

// Two-sensor line follower with proportional steering
void line_follow_2(int distance, int speed) {
    cmpc(MOTORl);
    cmpc(MOTORr);
    msleep(50);

    // Loop until the average travelled distance reaches the target
    while ((abs(gmpc(MOTORl)) + abs(gmpc(MOTORr))) / 2.0 < distance) {
        int left_val  = analog(SENSORl);
        int right_val = analog(SENSORr);

        // Error = left - right
        // Positive => left sees more black => turn left
        // Negative => right sees more black => turn right
        int error = left_val - right_val;
        float speed_modifier = error * CORRECTION;

        mav(MOTORr, (int)(speed + speed_modifier));
        mav(MOTORl, (int)(speed - speed_modifier));

        msleep(5);  // small delay to keep CPU responsive
    }
    ao();
    msleep(50);
}

// ===========================================================================
// MISSION ROUTINES
// ===========================================================================

void move_to_botguy() {
    printf("starting to go to cone1\n");
    enable_servos();
    move_arm(ARM_START);
    move_claw(CLAW_DOOR);
    
    drive_till_blk();
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(1300);
    
    right_degrees(46);
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(2700);
    
    left_degrees(43);
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(400);
    
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(500);
    ao();
    
    right_degrees(65);
    line_follow_2(TO_BOTGUY_LINE_FOLLOW, MOTOR_SPEED_1500);
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(600);
    
    left_degrees(50);
    drive(MOTOR_SPEED_500, MOTOR_SPEED_500);
    msleep(500);
    
    left_degrees(55);
    right_degrees(21);
    ao();
    
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1500);
    ao();
}

void open_doors() {
    move_arm_fast_slow_down(ARM_DOOR);
    drive_till_blk();
    line_follow_2(630, MOTOR_SPEED_1000);
    right_degrees_speed(35);
    
    move_arm(ARM_DOWN_BOTGUY);
    msleep(50);
    
    left_degrees(60);
    ao();
    msleep(100);
    
    right_degrees(35);
    left_degrees(5);
    msleep(100);
    
    disable_servos();
    msleep(300);
    ao();
    
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(300);
    ao();
    
    move_arm(ARM_DOWN_BOTGUY);  
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(250);
    ao();
    
    enable_servos();
    msleep(300);
    
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(30);
    
    set_servo_position(CLAW, CLAW_OPEN);
    msleep(200);
    
    right_degrees_less(1);
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(260);
    ao();
    msleep(300);
    
    set_servo_position(CLAW, CLAW_CLOSE);
    move_arm(ARM_BOTGUY);
    
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(300);
    ao();
    
    left_degrees_less(1);
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1450);
    
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1000);
    ao();
    
    move_arm(ARM_BOTGUY_CARRY);
}

void deliver_botguy() {
    drive_till_blk();
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(300);
    
    right_degrees(84);
    drive_till_blk_time(1);
    
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(300);
    
    drive_till_blk();
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(3100);
    
    left_degrees(90);
    drive(MOTOR_SPEED_1000, MOTOR_SPEED_1000);
    msleep(1000);
    
    move_arm(ARM_UP);
    left_degrees(190);
    
    drive_till_blk_speed(MOTOR_SPEED_1000);
    drive(MOTOR_SPEED_500, MOTOR_SPEED_500);
    msleep(400);
    ao();
    
    enable_servos();
    set_servo_position(CLAW, CLAW_OPEN);
    msleep(200);
    
    left_degrees(5);
    set_servo_position(CLAW, CLAW_UPPER_OPEN);
    msleep(300);
    
    right_degrees(5);
    disable_servos();
}

void get_cones() {
    move_arm(ARM_UP);
    move_claw(CLAW_OPEN);
    enable_servos();
    
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(600);
    
    back_till_blk();
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(900);
    
    right_degrees(95);
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1200);
    ao();
    
    right_degrees(5);
    move_arm(ARM_CONE);
    line_follow_2(450, MOTOR_SPEED_1500);
    
    printf("FINISHED CONE1 GOING TO CONE2\n");
    line_follow_2(3300, MOTOR_SPEED_1500);
    right_degrees(DEGREES);
    
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(1200);
    ao();
    
    enable_servos();
    set_servo_position(CLAW, CLAW_CLOSE);
    msleep(1000);
    msleep(200);
    printf("Closed Claw, Ended Cone2\n");
    
    move_arm(ARM_UP);
    left_degrees(DEGREES);
    msleep(200);
    
    line_follow_2(RETURN_LINE_FOLLOW, MOTOR_SPEED_1500);
    right_degrees(90);
    
    back_till_blk();
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(400);
    
    back_till_blk();
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1700);
    
    drive(MOTOR_SPEED_1000, MOTOR_SPEED_1000);
    msleep(200);
    right_degrees(90);
    
    drive_till_blk_time(0.25);
    line_follow_2(15100, MOTOR_SPEED_1500);
    drive_till_white(0.5);
}

void parking() {
    right_degrees(5);
    ao();
    
    move_arm_fast_slow_down(ARM_CONE);
    enable_servos();
    
    set_servo_position(CLAW, CLAW_UPPER_OPEN);
    msleep(200);
    
    set_servo_position(ARM, ARM_UP);
    msleep(500);
    
    set_servo_position(CLAW, CLAW_CLOSE);
    msleep(500);
    disable_servos();
    
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(870);
    
    back_till_blk();
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1000);
    
    left_degrees(33);
    drive_till_blk_time(0.5);
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(1470);
    
    back_till_blk();
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(1330);
    
    left_degrees(155);
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(3200);
    
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(300);
    
    right_degrees(100);
    drive(-MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    msleep(200);
    
    drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    msleep(300);
    right_degrees(10);
}

// ===========================================================================
// MAIN
// ===========================================================================

int main() {
    move_to_botguy();
    open_doors();
    deliver_botguy();
    get_cones();
    parking();
    
    return 0;
}
