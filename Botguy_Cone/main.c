#include <kipr/wombat.h>
#include <math.h>
#include <stdlib.h>
#include<stdint.h>
#include<stdbool.h>


#define CLAW 2
#define ARM 3
#define MOTORl 3
#define MOTORr 0
#define SENSORl 5
#define SENSORr 2
#define LIGHT_SENSOR_PORT 5
#define BLACK 3000
#define RIGHT_MOTOR_CORRECTION 1.002
#define MOTOR_SPEED_250 250
#define MOTOR_SPEED_500 500
#define MOTOR_SPEED_1000 1000
#define MOTOR_SPEED_1500 1500
#define WHITE 2000
#define CLAW_UPPER_OPEN 450
#define CLAW_OPEN 600
#define CLAW_CLOSE 1130
#define CLAW_DOOR 1080
#define CLAW_BOTGUY 874	
#define CLAW_CONE 560
#define ARM_BOTGUY 1500
#define ARM_CONE 1940
#define DEGREES 5
#define ARM_BOTGUY_CARRY 402
#define ARM_UP 820
#define ARM_START 160
#define ARM_DOWN_BOTGUY 1750
#define ARM_DOOR 1630
#define ARM_CONE_UP 500
#define CONE1_LINE_FOLLOW 6000
#define CONE2_LINE_FOLLOW 2300
#define RETURN_LINE_FOLLOW 11000
#define RAMP_LINE_FOLLOW 15300
#define SHUT_DOWN_SECS 11

/* Line Follow Definitions */

#define TO_BOTGUY_LINE_FOLLOW 4100

void move_arm(int end_pos) {
    enable_servos();
    int current_pos = get_servo_position(ARM);
    printf("move_arm(S): current_pos= %d, end_pos= %d\n", current_pos, end_pos);
    while (current_pos != end_pos) {
        if (current_pos > end_pos) {
            current_pos -= 10;
            if(current_pos < end_pos) {
                current_pos = end_pos;
            }   
        } else {
            current_pos += 10;
            if(current_pos > end_pos) {
                current_pos = end_pos;
            }   
        }
        // printf("Updated: current_pos: %d\n", current_pos);
        set_servo_position(ARM, current_pos);
        msleep(15);
    }
    disable_servos();
    printf("move_arm(End): current_pos= %d, end_pos= %d\n", current_pos, end_pos);
}
void move_claw(int end_pos) {
    enable_servos();
    int current_pos = get_servo_position(CLAW);
    printf("move_claw(S): current_pos= %d, end_pos= %d\n", current_pos, end_pos);
    while (current_pos != end_pos) {
        if (current_pos > end_pos) {
            current_pos -= 30;
            if(current_pos < end_pos) {
                current_pos = end_pos;
            }   
        } else {
            current_pos += 30;
            if(current_pos > end_pos) {
                current_pos = end_pos;
            }   
        }
        // printf("Updated: current_pos: %d\n", current_pos);
        set_servo_position(CLAW, current_pos);
        msleep(60);
    }
    disable_servos();
    printf("move_claw(End): current_pos= %d, end_pos= %d\n", current_pos, end_pos);
}

void move_arm_fast_slow_down(int position) {        
    enable_servos();
    set_servo_position(ARM,position-500);
    disable_servos();
    enable_servos();
    msleep(100);
    move_arm(position);
    msleep(100);
}  

void drive(int speed1,int speed2) {
    mav(MOTORl, speed1);
    mav(MOTORr,speed2* RIGHT_MOTOR_CORRECTION);
}
void drive_till_blk_time(int timeout_secs) {
    int start_time =  seconds();
    printf("drive_till_white(START): start_time= %d, timeout_secs= %d\n", start_time, timeout_secs);
    while (analog(SENSORl) <= BLACK || analog(SENSORr) <= BLACK) {
        if ( seconds() - start_time >= timeout_secs) {
            printf("drive_till_white(): TIMEOUT after %d secs, start_time= %d\n", timeout_secs, start_time);
            break;
        }
        drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    }
    printf("drive_till_white(END): start_time= %d, seconds= %d\n", start_time, (int) seconds());
    ao();
}
void drive_till_blk(){
    printf("DRIVE_TILL_BLK STARTED");
    while (analog(SENSORl) <= BLACK || analog(SENSORr) <= BLACK) {
        drive(MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    }
    printf("DRIVE_TILL_BLK ENDED");
}	
void drive_till_blk_speed(int speed){
    while (analog(SENSORl) <= BLACK || analog(SENSORr) <= BLACK) {
        drive(speed, speed);
    }
}
void drive_till_white(int timeout_secs) {
    int start_time =  seconds();
    printf("drive_till_white(START): start_time= %d, timeout_secs= %d\n", start_time, timeout_secs);
    while (analog(SENSORl) >= WHITE || analog(SENSORr) >= WHITE) {
        if ( seconds() - start_time >= timeout_secs) {
            printf("drive_till_white(): TIMEOUT after %d secs, start_time= %d\n", timeout_secs, start_time);
            break;
        }
        drive(MOTOR_SPEED_1000, MOTOR_SPEED_1000);
    }
    printf("drive_till_white(END): start_time= %d, seconds= %d\n", start_time, (int) seconds());
    ao();
}

void right_turn() {
    printf("RIGHT TURN");
    cmpc(MOTORl);
    cmpc(MOTORr);
    while(gmpc(MOTORl) > -1000 && gmpc(MOTORr) > -1000){
        drive(MOTOR_SPEED_500, -MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}
void right_10() {
    printf("RIGHT TURN");
    cmpc(MOTORl);
    cmpc(MOTORr);
    while(gmpc(MOTORl) > -100 && gmpc(MOTORr) > -100){
        drive(MOTOR_SPEED_500, -MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}
void right_degrees(int degrees) {
    printf("RIGHT TURN");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 12.7 * degrees;
    
    while(gmpc(MOTORl) < ticks || gmpc(MOTORr) > -ticks){
        drive(MOTOR_SPEED_500, -MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}
void right_degrees_speed(int degrees) {
    printf("RIGHT TURN");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 12.7 * degrees;
    
    while(gmpc(MOTORl) < ticks || gmpc(MOTORr) > -ticks){
        drive(MOTOR_SPEED_1500, -MOTOR_SPEED_1500);
    }
    ao();
    msleep(50);
}

void right_degrees_less(int degrees) {
    printf("RIGHT TURN");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 2 * degrees;
    while(gmpc(MOTORl) < ticks || gmpc(MOTORr) > -ticks){
        drive(MOTOR_SPEED_500, -MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}
void left_degrees_fast(int degrees) {
    printf("LEFT TURN START\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 12.7 * degrees;
    while(gmpc(MOTORl) > -ticks || gmpc(MOTORr) < ticks){
        drive(-MOTOR_SPEED_1500, MOTOR_SPEED_1500);
    }
    ao();
    msleep(50);
    printf("LEFT TURN END: Motor_L= %d, Motor_R= %d\n", gmpc(MOTORl),  gmpc(MOTORr));
}
void left_degrees(int degrees) {
    printf("LEFT TURN START\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 12.7 * degrees;
    while(gmpc(MOTORl) > -ticks || gmpc(MOTORr) < ticks){
        drive(-MOTOR_SPEED_500, MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
    printf("LEFT TURN END: Motor_L= %d, Motor_R= %d\n", gmpc(MOTORl),  gmpc(MOTORr));
}
void left_degrees_less(int degrees) {
    printf("LEFT TURN START\n");
    cmpc(MOTORl);
    cmpc(MOTORr);
    int ticks = 2 * degrees;
    while(gmpc(MOTORl) > -ticks || gmpc(MOTORr) < ticks){
        drive(-MOTOR_SPEED_500, MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
    printf("LEFT TURN END: Motor_L= %d, Motor_R= %d\n", gmpc(MOTORl),  gmpc(MOTORr));
}

void back_till_blk(){
    while (analog(SENSORl) <= BLACK && analog(SENSORr) <= BLACK) {
        drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    }
}
void reverse_line_follow(int distance, int speed) {
    int high = -speed;
    int low = (int)(-0.6 * speed);

    cmpc(MOTORl);
    cmpc(MOTORr);
    msleep(100);
    printf("reverse_line_follow(): distance=%d, speed=%d\n", distance, speed);

    while (abs(gmpc(MOTORl)) < distance || abs(gmpc(MOTORr)) < distance) {
        int sl = analog(SENSORl);
        int sr = analog(SENSORr);

        if (sl >= BLACK && sr >= BLACK) {
            drive(high, high);   // both on black, go straight
        } else if (sl >= BLACK) {
            drive(low, high);    // flipped from before
        } else if (sr >= BLACK) {
            drive(high, low);    // flipped from before
        } else {
            drive(high, high);   // lost line, go straight
        }
        msleep(1);
    }

    ao();
    msleep(50);
}

void left_turn() {
    cmpc(MOTORl);
    cmpc(MOTORr);
    while(gmpc(MOTORl) < 1000 && gmpc(MOTORr) < 1000) {
        drive(-MOTOR_SPEED_500, MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}
void left_60() {
    cmpc(MOTORl);
    cmpc(MOTORr);
    while(gmpc(MOTORl) < 700 && gmpc(MOTORr) < 700) {
        drive(-MOTOR_SPEED_500, MOTOR_SPEED_500);
    }
    ao();
    msleep(50);
}
void line_follow(int distance, int speed) {
    int high = speed;
    int low = 0.6 * speed;

    cmpc(MOTORl);
    cmpc(MOTORr);
    msleep(100);
    printf("line_follow(): distance=%d, speed=%d\n", distance, speed);

    while (abs(gmpc(MOTORl)) < distance || abs(gmpc(MOTORr)) < distance) {
        int sl = analog(SENSORl);
        int sr = analog(SENSORr);

        if (sl >= BLACK && sr >= BLACK) {
            drive(high, high);   // both on black, go straight
        } else if (sl >= BLACK) {
            drive(low, high);    // flipped from before
        } else if (sr >= BLACK) {
            drive(high, low);    // flipped from before
        } else {
            drive(high, high);   // lost line, go straight
        }
        msleep(1);
    }

    ao();
    msleep(50);
}

void line_follow_old(int distance, int speed) {
    //follow black line for set distance.
    int high = speed;
    int low = 0.6 * speed;
    cmpc(MOTORl);
    cmpc(MOTORr);
    printf("line_follow(): distance= %d, speed= %d\n", distance, speed);
    while (gmpc(MOTORl) < distance && gmpc(MOTORr) < distance) {
        msleep(1);
        // if both sensors do not detect black, drive forward
        // if the left sensor detects black, turn left
        // if the right sensor detects black, turn right
        if (analog(SENSORl) <= BLACK && analog(SENSORr) <= BLACK) {  
            drive(high, high);   
        } else if (analog(SENSORl) >= BLACK) {
            drive(low, high);
        } else if(analog(SENSORr) >= BLACK) {
            drive(high, low);        
        }
    }
    drive(0, -10);    
    msleep(50);
    ao();
    msleep(50);
}
void drive_distance(int distance, int speed) {
    cmpc(MOTORl);
    cmpc(MOTORr);
    while (gmpc(MOTORl) < distance && gmpc(MOTORr) < distance) {
        drive(speed,speed);
    }
}

void drive_negative_distance(int distance, int speed) {
    cmpc(MOTORl);
    cmpc(MOTORr);
    while (gmpc(MOTORl) > distance && gmpc(MOTORr) > distance) {
        drive(speed,speed);
    }
}
void square_up_smooth(int tick_guess) {
    cmpc(MOTORl);
    cmpc(MOTORr);
    int speed = MOTOR_SPEED_1500;
    speed = abs(speed);
    const int direction = tick_guess > 0 ? 1 : -1;
    int dynspeed = speed;
    bool tape_detected = false;

    while(!(analog(SENSORl) >= BLACK && analog(SENSORr) >= BLACK)) {
        int cur_ticks = (abs(gmpc(MOTORl)) + abs(gmpc(MOTORr))) / 2;

        // Slow start
        if (abs(cur_ticks) < 400) {
            speed = fmax(cur_ticks * 10, 100);
            speed = fmin(speed, 1200);
        }
        dynspeed = speed;

        if (!tape_detected && tick_guess < cur_ticks) {
            dynspeed = speed / 3;
        }
        if (analog(SENSORl) >= BLACK || analog(SENSORr) >= BLACK) {
            dynspeed = speed / 5;
            tape_detected = true;
        }
        dynspeed *= direction;

        if (analog(SENSORl) >= BLACK && analog(SENSORr) < BLACK) {
            mav(MOTORl, -dynspeed);
            mav(MOTORr, +dynspeed);
            tape_detected = true;
        } else if (analog(SENSORl) < BLACK && analog(SENSORr) >= BLACK) {
            mav(MOTORl, +dynspeed);
            mav(MOTORr, -dynspeed);
            tape_detected = true;
        } else {
            mav(MOTORl, +dynspeed);
            mav(MOTORr, +dynspeed);
        }
        msleep(10);
    }
    ao();
}

#define CORRECTION 0.3
// Twobsensor line follower with proportional steering
// Uses global MOTORl, MOTORr, SENSORl, SENSORr
// distance : target encoder ticks (average of both wheels)
// speed    : base motor speed (0b1000)
void line_follow_2(int distance, int speed) {
    // Reset encoders
    cmpc(MOTORl);
    cmpc(MOTORr);
    msleep(50);

    // Loop until the average travelled distance reaches the target
    while ((abs(gmpc(MOTORl)) + abs(gmpc(MOTORr))) / 2.0 < distance) {
        int left_val  = analog(SENSORl);
        int right_val = analog(SENSORr);

        // Error = left - right
        // Positive => left sees more black (lower value) => line is to the left => turn left
        // Negative => right sees more black => turn right
        int error = left_val - right_val;

        float speed_modifier = error * CORRECTION;

        // Differential steering
        mav(MOTORr, speed + speed_modifier);
        mav(MOTORl, speed - speed_modifier);

        msleep(5);  // small delay to keep CPU responsive
    }

    // Stop both motors
    ao();
    msleep(50);
}

void go_up_ramp() {
    line_follow(11000, MOTOR_SPEED_1500);
    //drive_till_white(3);
    drive_till_blk();
}

void move_to_botguy() {
    printf("starting to go to cone1");
    enable_servos();
    move_arm(ARM_START);
    move_claw(CLAW_DOOR);
    drive_till_blk();
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);

    msleep(1300);
    right_degrees(46);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(2700);
    left_degrees(40);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(400);
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(500);
    ao();
    right_degrees(65);

    line_follow(TO_BOTGUY_LINE_FOLLOW, MOTOR_SPEED_1500);
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(600);
    left_degrees(50);
    drive(MOTOR_SPEED_500,MOTOR_SPEED_500);
    msleep(500);
    left_degrees(55);
    right_degrees(23);
    ao();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(670);
    ao();

}

void open_doors() {
    move_arm_fast_slow_down(ARM_DOOR);
    line_follow(150,MOTOR_SPEED_500);
    right_degrees_speed(35);
    move_arm(ARM_DOWN_BOTGUY);
    msleep(50);
    left_degrees(40);
    ao();
    msleep(100);
    left_degrees(20);
    right_degrees(35);
    left_degrees(5);
    msleep(100);
    disable_servos();
    msleep(300);
    ao();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(300);
    ao();
    move_arm(ARM_DOWN_BOTGUY);  
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(250);
    ao();
    enable_servos();
    msleep(300);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(40);
    set_servo_position(CLAW,CLAW_OPEN);
    msleep(200);
    right_degrees_less(1);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(250);
    ao();
    msleep(300);
    set_servo_position(CLAW,CLAW_CLOSE);
    move_arm(ARM_BOTGUY);
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(300);
    ao();
    left_degrees_less(1);
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(1450);
    drive(-MOTOR_SPEED_1000,-MOTOR_SPEED_1000);
    msleep(1000);
    ao();
    move_arm(ARM_BOTGUY_CARRY);


}

void deliver_botguy() {
    drive_till_blk();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(300);
    //line_follow(1000,MOTOR_SPEED_1500);
    right_degrees(88);
    drive_till_blk_time(1);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(300);
    drive_till_blk();
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(3100);
    left_degrees(90);
    drive(MOTOR_SPEED_1000,MOTOR_SPEED_1000);
    msleep(1000);
    move_arm(ARM_UP);
    left_degrees(190);
    drive_till_blk_speed(MOTOR_SPEED_1000);
    drive(MOTOR_SPEED_500,MOTOR_SPEED_500);
    msleep(400);
    ao();
    enable_servos();
    set_servo_position(CLAW,CLAW_OPEN);
    msleep(200);
    left_degrees(5);
    set_servo_position(CLAW,CLAW_UPPER_OPEN);
    msleep(300);
    right_degrees(5);
    disable_servos();

}

void get_cones() {
    move_arm(ARM_UP);
    move_claw(CLAW_OPEN);
    enable_servos();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(600);
    back_till_blk();
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(900);
    right_degrees(95);
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(1200);
    ao();
    right_degrees(5);
    move_arm(ARM_CONE);
    line_follow(450, MOTOR_SPEED_1500);
    /*ao();
    right_degrees(DEGREES);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(500);
    ao();
    left_degrees(DEGREES);
    */
    printf("FINISHED CONE1 GOING TO CONE2\n");
    line_follow(3300, MOTOR_SPEED_1500);
    right_degrees(DEGREES);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(1200);
    ao();
    enable_servos();
    set_servo_position(CLAW, CLAW_CLOSE);
    msleep(1000);
    msleep(200);
    printf("Closed Claw, Ended Cone2");
    move_arm(ARM_UP);
    left_degrees(DEGREES);
    msleep(200);
    line_follow_2(RETURN_LINE_FOLLOW,MOTOR_SPEED_1500);
    right_degrees(90);
    back_till_blk();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(400);
    back_till_blk();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(1700);
    drive(MOTOR_SPEED_1000,MOTOR_SPEED_1000);
    msleep(200);
    right_degrees(90);
    drive_till_blk_time(0.25);
    line_follow_2(15100,MOTOR_SPEED_1500);
    drive_till_white(0.5);
}
void parking() {
    right_degrees(5);
    ao();
    move_arm_fast_slow_down(ARM_CONE);
    enable_servos();
    set_servo_position(CLAW,CLAW_UPPER_OPEN);
    msleep(200);
    set_servo_position(ARM,ARM_UP);
    msleep(500);
    set_servo_position(CLAW,CLAW_CLOSE);
    msleep(500);
    disable_servos();
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(870);
    back_till_blk();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(1000);
    left_degrees(33);
    drive_till_blk_time(0.5);
    drive(MOTOR_SPEED_1500,MOTOR_SPEED_1500);
    msleep(1470);
    back_till_blk();
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
    msleep(1330);
    left_degrees(145);
    drive(-MOTOR_SPEED_1500,-MOTOR_SPEED_1500);
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

int main() {

    move_to_botguy();
    open_doors();
    deliver_botguy();
    get_cones();
    
    parking();
   

}
