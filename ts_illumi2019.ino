#include <EEPROM.h>
#include <avr/wdt.h>

const int ADDR_MODE=0;              //EEPROMに書き込むモード用変数のアドレス
const byte DUTY_MAX=100;            //PWMの周期。0より大きい範囲
const byte DUTY_RATIO=10;          //PWMのDuty比。0-DUTY_MAXの間で設定
const unsigned int FLASH_CYCLE=2000;//点滅周期(ミリ秒)
const byte OUT_MIN=2;               //出力ピンの一番下
const byte OUT_MAX=13;              //出力ピンの一番上
const char SW_MODE=A1;              //モード切替スイッチのピン。15? 16?
const char SW_MYST=A2;              //もう一つの謎入力。要調査

typedef enum{
    xmas=0,     //Fu*k X'mas
    mochi=1,    //お正月
    oni=2       //節分
}Mode;

namespace Common{   //共通部分
    float pin2(float phase){
        return 1.;
    }
    float pin3(float phase){
        //return pow(sin(phase),2)
        if(0<=phase&&phase<=PI){
            return 1;
        }else{
            return 0;
        }
    }
    float pin4(float phase){
        //return pow(-sin(phase),2)
        if(0<=phase&&phase<=PI){
            return 1;
        }else{
            return 0;
        }
    }
    float pin5(float phase){
        //return pow(sin(phase),2)
        if(0<=phase&&phase<=PI){
            return 0;
        }else{
            return 1;
        }
    }
    float pin6(float phase){
        //return pow(-sin(phase),2)
        if(0<=phase&&phase<=PI){
            return 0;
        }else{
            return 1;
        }
    }
    float pin7(float phase){
        if(0<=phase&&phase<=PI){
            return 0;
        }else{
            return 1;
        }
    }
    float pin8(float phase){
        if(0<=phase&&phase<=PI){
            return 1;
        }else{
            return 0;
        }
    }
}

namespace Xmas{     //クリスマス
    float pin9(float phase){
        return 1.;
    }
    float pin10(float phase){
        return 1.;
    }
    float pin11(float phase){
        return 0.;
    }
    float pin12(float phase){
        return 0.;
    }
    float pin13(float phase){
        return 0.;
    }
}

namespace Mochi{    //正月
    float pin9(float phase){
        return 0.;
    }
    float pin10(float phase){
        return 0.;
    }
    float pin11(float phase){
        return 0.;
    }
    float pin12(float phase){
        return 1.;
    }
    float pin13(float phase){
        return 0.;
    }
}

namespace Oni{      //節分
    float pin9(float phase){
        return 1.;
    }
    float pin10(float phase){
        return 0.;
    }
    float pin11(float phase){
        return 1.;
    }
    float pin12(float phase){
        return 0.;
    }
    float pin13(float phase){
        return 0.;
    }
}

namespace Xmas{
    using namespace Common;
    float (*pinFunc[12])(float)={pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11,pin12,pin13};
}

namespace Mochi{
    using namespace Common;
    float (*pinFunc[12])(float)={pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11,pin12,pin13};
}

namespace Oni{
    using namespace Common;
    float (*pinFunc[12])(float)={pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11,pin12,pin13};
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

void resetSoftware(){
    Serial.println("reset");
    wdt_disable();
    wdt_enable(WDTO_15MS);
    while(1){}
    //asm volatile("jmp 0");
}

class Light{
    private:
        Mode mode;
        float (*pins[12])(float);

    public:
        Light(Mode m){
            mode=m;
            switch(mode){
                case xmas:
                    Serial.println("xmas");
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=getFunc(mode,cnt);
                    }
                    break;
                case mochi:
                    Serial.println("mochi");
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=getFunc(mode,cnt);
                    }
                    break;
                case oni:
                    Serial.println("oni");
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=getFunc(mode,cnt);
                    }
                    break;
                default:
                    EEPROM.write(ADDR_MODE,xmas);
                    resetSoftware();
            }
        }

        void pwmUpdate(byte pwm_cnt,float phase){
            float duty_pin=0.;
            if(pwm_cnt>DUTY_RATIO){
                for(byte pin=OUT_MIN;pin<=OUT_MAX;pin++){
                    digitalWrite(pin,LOW);
                }
            }else{
                for(byte cnt=0;cnt<12;cnt++){
                    if((float)pwm_cnt>=(float)DUTY_RATIO*pins[cnt](phase)){
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
    Serial.begin(9600);
    Serial.println("setup");
    for(int cnt=OUT_MIN; cnt<=OUT_MAX; cnt++){
        pinMode(cnt,OUTPUT);
    }
    pinMode(SW_MODE,INPUT);
    pinMode(SW_MYST,INPUT);
    delay(1000);
}

void loop(){
    Serial.println("loop");
    Light light((Mode)EEPROM.read(ADDR_MODE));
    float phase_now=0;

    while(1){
        if(digitalRead(SW_MODE)==HIGH){
                    EEPROM.write(ADDR_MODE,light.getMode()+1);
                    resetSoftware();
        }

        phase_now=(float)(millis()%FLASH_CYCLE)/(float)FLASH_CYCLE*2.*PI;

        for(byte pwm_count=0;pwm_count<=DUTY_MAX;pwm_count++){
            light.pwmUpdate(pwm_count,phase_now);
        }
    }
}
