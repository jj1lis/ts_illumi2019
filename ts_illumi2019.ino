#include <EEPROM.h>

const int ADDR_MODE=0;              //EEPROMに書き込むモード用変数のアドレス
const byte DUTY_RATIO=255;          //PWMのDuty比。0-255の間で設定
const unsigned int FLASH_CYCLE=2000;//点滅周期(ミリ秒)
const byte OUT_MIN=2;               //出力ピンの一番下
const byte OUT_MAX=13;              //出力ピンの一番上
const byte SW_MODE=15;              //モード切替スイッチのピン。15? 16?
const byte SW_MYST=16;              //もう一つの謎入力。要調査

typedef enum{
    xmas=0,     //Fu*k X'mas
    mochi=1,    //お正月
    oni=2       //節分
}Mode;

void resetSoftware(){
    asm volatile("jmp 0");
}

namespace Common{
    float pin0(float phase){
        return 1.;
    }
    float pin1(float phase){
        return sin(phase);
    }
    float pin2(float phase){
        return -sin(phase);
    }
    float pin3(float phase){
        return sin(phase);
    }
    float pin4(float phase){
        return -sin(phase);
    }
}

namespace Xmas{
    float pin5(float phase){
        return 1.;
    }
    float pin6(float phase){
        return 1.;
    }
    float pin7(float phase){
        return 0.;
    }
    float pin8(float phase){
        return 0.;
    }
    float pin9(float phase){
        return 0.;
    }
    float pin10(float phase){
        return 0.;
    }
    float pin11(float phase){
        return 0.;
    }
}

namespace Mochi{
    float pin5(float phase){
        return 0.;
    }
    float pin6(float phase){
        return 0.;
    }
    float pin7(float phase){
        return 0.;
    }
    float pin8(float phase){
        return 1.;
    }
    float pin9(float phase){
        return 0.;
    }
    float pin10(float phase){
        return 0.;
    }
    float pin11(float phase){
        return 0.;
    }
}

namespace Oni{
    float pin5(float phase){
        return 1.;
    }
    float pin6(float phase){
        return 0.;
    }
    float pin7(float phase){
        return 0.;
    }
    float pin8(float phase){
        return 0.;
    }
    float pin9(float phase){
        return 0.;
    }
    float pin10(float phase){
        return 0.;
    }
    float pin11(float phase){
        return 0.;
    }
}

namespace Xmas{
    using namespace Common;
    float (*pinFunc[12])(float)={pin0,pin1,pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11};
}

namespace Mochi{
    using namespace Common;
    float (*pinFunc[12])(float)={pin0,pin1,pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11};
}

namespace Oni{
    using namespace Common;
    float (*pinFunc[12])(float)={pin0,pin1,pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11};
}

float (*getFunc(Mode mode,byte pin_num))(float){
    switch(mode){
        case xmas:
            return Xmas::pinFunc[pin_num];
        case mochi:
            return Mochi::pinFunc[pin_num];
        case oni:
            return Oni::pinFunc[pin_num];
        default:
            return NULL;
    }
}

struct Pin{
        float (*ratio)(float);
};

class Light{
    private:
        Mode mode;
        Pin pins[12];

    public:
        Light(Mode m){
            mode=m;
            switch(mode){
                case xmas:
                case mochi:
                case oni:
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=Pin();
                        pins[cnt].ratio=getFunc(mode,cnt);
                    }
                    break;
                default:
                    resetSoftware();
            }
        }

        void pwmUpdate(byte pwm_cnt,float phase){
            float duty_pin=0.;
            if(pwm_cnt>DUTY_RATIO){
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, LOW);
                }
            }else{
                for(byte cnt=0;cnt<12;cnt++){
                    duty_pin=pins[cnt].ratio(phase);
                    if(pwm_cnt>(byte)DUTY_RATIO*duty_pin){
                        digitalWrite(cnt+OUT_MIN,LOW);
                    }else{
                        digitalWrite(cnt+OUT_MIN,HIGH);
                    }
                }
            }
        }

        Mode getMode(){
            return mode;
        }
};

void setup(){
    for(int cnt=OUT_MIN; cnt<=OUT_MAX; cnt++){
        pinMode(cnt,OUTPUT);
    }
    pinMode(SW_MODE,INPUT);
    pinMode(SW_MYST,INPUT);
}

void loop(){
    Light light((Mode)EEPROM.read(ADDR_MODE));
    float phase_now;

    while(1){
        if(digitalRead(SW_MODE)==HIGH){
            while(digitalRead(SW_MODE)==HIGH){
                if(digitalRead(SW_MODE)==LOW){
                    EEPROM.write(ADDR_MODE,light.getMode()+1);
                    resetSoftware();
                }
            }
        }

        phase_now=(millis()%FLASH_CYCLE)/FLASH_CYCLE*2*PI;

        for(byte pwm_count=0;pwm_count<=255;pwm_count++){
            light.pwmUpdate(pwm_count,phase_now);
        }
    }
}
