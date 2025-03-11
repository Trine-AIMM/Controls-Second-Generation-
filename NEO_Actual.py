from machine import Pin, PWM
from time import sleep

# Initialize PWM on a PWM-capable pin
pwm_pin = Pin(18)  # Replace with your PWM-capable GPIO pin
pwm = PWM(pwm_pin)
pwm.freq(200)  # Set frequency to 200Hz (5ms period)

# Set trigger pin as input with pull-down resistor
trigger_pin = Pin(16, Pin.IN, Pin.PULL_DOWN)
prox_sen_pin = Pin(17, Pin.IN, Pin.PULL_DOWN)
exit_pin = Pin(1, Pin.IN, Pin.PULL_DOWN)

def set_duty(pulse_width_us):
    period_us = 5000  # 200 Hz = 1/200 = 5ms period = 5000µs
    duty = int((pulse_width_us / period_us) * 65535)  # Scale to 16-bit duty cycle
    pwm.duty_u16(duty)

def drop():
    print("Executing drop() function...")
    set_duty(1420)  
    sleep(2.7)
    set_duty(1450)
    sleep(1.2)
    print("Drop function completed.")

def hold():
    set_duty(1535)
    print("Holding position.")

def pullUp():
    stop = prox_sen_pin.value()

    set_duty(1600)
    sleep(1.2)
    set_duty(1570)
    sleep(1.8)
    while prox_sen_pin.value() == 0:
        print("waiting for sensor")
        set_duty(1560)
        sleep(0.1)
    print("sensor triggered")
    set_duty(0)


# Main loop
while True:
    t_pin_status = trigger_pin.value()
    
    if exit_pin.value() == 1:  # Exit if Pin 1 goes HIGH
        print("Exit signal received (Pin 1 HIGH), exiting loop.")
        break
    
    if t_pin_status == 1:  # Check if pin is HIGH
        print("Trigger detected! Running drop() and pullUp() functions.")
        drop()
        hold()
        sleep(3)
        pullUp()
        sleep(2)
    else:
        print("Waiting for trigger...")
    
    sleep(0.5)  # Small delay to reduce unnecessary prints
