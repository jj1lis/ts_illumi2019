#include <EEPROM.h>

const int ADDR_MODE=0;  //Mode memory address in EEPROM
const byte DUTY_RATIO=255;  //ratio of PWM duty. range is 0-255.
const unsigned int FLASH_CYCLE=2000   //flash cycle(milli second)
const byte OUT_MIN=2;
const byte OUT_MAX=13;
const byte SW_MODE=//15? 16?

typedef enum{
    xmas=0,     //Fu*k X'mas
    mochi=1,    //New Year
    oni=2      //Japanese Setsubun
}Mode;

void resetSoftware(){   //end process and reload from first.
    asm volatile("jmp 0");
}

class Light{
    private:
        Mode mode;
        bool flash_pins[2][12];
        bool *out_status; //CAUTION!!!*pin number... 2-13
    public:
        Light(Mode read_mode)//Constractor. set mode by switch-case sentence.
            mode=read_mode;
            switch(read_mode){
                case xmas:
                    flash_pins[false]=//TODO {HIGH,HIGH...};
                    flash_pins[true]=//TODO
                    break;
                case mochi:
                    flash_pins[false]=//TODO
                    flash_pins[true]=//TODO
                    break;
                case oni:
                    flash_pins[false]=//
                    flash_pins[true]=//TODO
                    break;
                default:        //reset and set mode to "xmas"
                    EEPROM.write(ADDR_MODE,(byte)Mode.xmas);
                    resetSoftware();
            }
        }

        Mode getMode(){
            return mode;
        }

        void flash(bool flash_status){
            out_status=flash_pins[flash_status];
        }

        /*  This func is for Pin setting manually.
        void setStatus(byte set_pin, boolean hl){  //hl...High Low
            out_status[set_pin-2]=hl;
        }
        */

        void pwmUpdate(byte pwm_cnt){
            if(pwm_cnt<DUTY_RATIO){
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, LOW);
                }
            }else{
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, out_status[pin]);
                }
            }
        }
}

Light light;

void setup(){
    for(int cnt=OUT_MIN; cnt<=OUT_MAX; cnt++){
        pinMode(cnt,OUTPUT);
    }
    pinMode(15,INPUT);
    pinMode(16,INPUT);
    light=new Light(EEPROM.read(ADDR_MODE));
}

void loop(){
    while(1){
        if(digitalRead(SW_MODE)==HIGH){
            EEPROM.write(ADDR_MODE,light.getMode()+1);
            resetSoftware();
        }

        if(millis()%FLASH_CYCLE<=FLASH_CYCLE/2){
            light.flash(true);
        }else{
            light.flash(false);
        }

        for(byte pwm_count=0;pwm_count<=255;pwm_count++){
            light.pwmUpdate(pwm_count);
        }
    }
}
