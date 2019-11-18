#include <EEPROM.h>

const int ADDR_MODE=0;              //EEPROMに書き込むモード用変数のアドレス
const byte DUTY_RATIO=255;          //PWMのDuty比。0-255の間で設定
const unsigned int FLASH_CYCLE=2000;//点滅周期(ミリ秒)
const byte OUT_MIN=2;               //出力ピンの一番下
const byte OUT_MAX=13;              //出力ピンの一番上
const byte SW_MODE=15;              //TODO モード切替スイッチのピン。15? 16?
const byte SW_MYST=16;              //TODO もう一つの謎入力。要調査

typedef enum{   //モードの列挙型。intで管理するより契約的
    xmas=0,     //Fu*k X'mas
    mochi=1,    //お正月
    oni=2       //節分
}Mode;          //typedefでclassみたいに使えて便利

void resetSoftware(){       //処理を強制終了して再起動
    asm volatile("jmp 0");  //よくわからんけどインラインアセンブラ
    //多分メモリの先頭に飛ぶ
}

//typedef float (*pinFunc)(float);

class Motion{
    float pin0(float);
    float pin1(float);
    float pin2(float);
    float pin3(float);
    float pin4(float);
    float pin5(float);
    float pin6(float);
    float pin7(float);
    float pin8(float);
    float pin9(float);
    float pin10(float);
    float pin11(float);
};

class Xmas:Motion{
public:
        float pin0(float phase){
            return 0.0;
        }
        static float pin1(float phase){
            return 0.0;
        }
        float pin2(float phase){
            return 0.0;
        }
        float pin3(float phase){
            return 0.0;
        }
        float pin4(float phase){
            return 0.0;
        }
        float pin5(float phase){
            return 0.0;
        }
        float pin6(float phase){
            return 0.0;
        }
        float pin7(float phase){
            return 0.0;
        }
        float pin8(float phase){
            return 0.0;
        }
        float pin9(float phase){
            return 0.0;
        }
        float pin10(float phase){
            return 0.0;
        }
        float pin11(float phase){
            return 0.0;
        }
        
        float(*)(float)[] xmasArray{
            return {pin0,pin1,pin2,pin3,pin4,pin5,pin6,pin7,pin8,pin9,pin10,pin11};
        }
};

class Mochi:Motion{
public:
        float pin0(float phase){
            return 0.0;
        }
        float pin1(float phase){
            return 0.0;
        }
        float pin2(float phase){
            return 0.0;
        }
        float pin3(float phase){
            return 0.0;
        }
        float pin4(float phase){
            return 0.0;
        }
        float pin5(float phase){
            return 0.0;
        }
        float pin6(float phase){
            return 0.0;
        }
        float pin7(float phase){
            return 0.0;
        }
        float pin8(float phase){
            return 0.0;
        }
        float pin9(float phase){
            return 0.0;
        }
        float pin10(float phase){
            return 0.0;
        }
        float pin11(float phase){
            return 0.0;
        }
};

class Oni:Motion{
public:
    float pin0(float phase){
        return 0.0;
    }
    float pin1(float phase){
        return 0.0;
    }
    float pin2(float phase){
        return 0.0;
    }
    float pin3(float phase){
        return 0.0;
    }
    float pin4(float phase){
        return 0.0;
    }
    float pin5(float phase){
        return 0.0;
    }
    float pin6(float phase){
        return 0.0;
    }
    float pin7(float phase){
        return 0.0;
    }
    float pin8(float phase){
        return 0.0;
    }
    float pin9(float phase){
        return 0.0;
    }
    float pin10(float phase){
        return 0.0;
    }
    float pin11(float phase){
        return 0.0;
    }
};

/*
float (*getFunc(Mode mode,byte pin_num))(float){
    switch(mode){
        case xmas:
            return Xmas::pin0;
        case mochi:
            return Mochi::pin1;
        case oni:
            return Oni::pin2;
        default:
            return NULL;
    }
}
*/

class Pin{
    private:
        float (*ratio)(float);
    public:
        Pin(float (*f)(float)){
            ratio=f;
        }

        float getRatio(float phase){
            return ratio(phase);
        }
};

class Light{
    private:
        Pin pins[12];

    public:
        Light(Mode mode){
            switch(mode){
                case xmas:
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=new Pin(Xmas::pin1);
                    }
                case mochi:
                case oni:
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=new Pin(getFunc(mode,cnt));
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
                    duty_pin=pins[cnt].getRatio(phase);
                    if(pwm_cnt>(byte)DUTY_RATIO*duty_pin){
                        digitalWrite(pin+OUT_MIN,LOW);
                    }else{
                        digitalWrite(pin+OUT_MIN,HIGH);
                    }
                }
            }
        }
};

void setup(){
    for(int cnt=OUT_MIN; cnt<=OUT_MAX; cnt++){
        pinMode(cnt,OUTPUT);    //出力ピンをまとめて設定
    }
    pinMode(15,INPUT);          //謎のピン達の設定。役目と定数名がわかったら
    pinMode(16,INPUT);          //マジックナンバーじゃなくてちゃんとつける
}

void loop(){
    Light light(EEPROM.read(ADDR_MODE));    //Lightのコンストラクタ呼び出し
    float phase_now;

    while(1){  //実質的なloop 
        if(digitalRead(SW_MODE)==HIGH){     //モード切替のハンドラ
            while(digitalRead(SW_MODE)==HIGH){  //押しっぱ防止
                if(digitalRead(SW_MODE)==LOW){  //手を離したらモード更新
                    EEPROM.write(ADDR_MODE,light.getMode()+1);  //モードを一個進める
                    resetSoftware();    //再起動
                }
            }
        }

        phase_now=(millis()%FLASH_CYCLE)/FLASH_CYCLE*2*PI;

        for(byte pwm_count=0;pwm_count<=255;pwm_count++){
            light.pwmUpdate(pwm_count,phase_now);
        }
    }
}
