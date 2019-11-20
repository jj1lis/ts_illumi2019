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

//XMAS
float xmas0(float phase){
    return 0.0;
}
float xmas1(float phase){
    return 0.0;
}
float xmas2(float phase){
    return 0.0;
}
float xmas3(float phase){
    return 0.0;
}
float xmas4(float phase){
    return 0.0;
}
float xmas5(float phase){
    return 0.0;
}
float xmas6(float phase){
    return 0.0;
}
float xmas7(float phase){
    return 0.0;
}
float xmas8(float phase){
    return 0.0;
}
float xmas9(float phase){
    return 0.0;
}
float xmas10(float phase){
    return 0.0;
}
float xmas11(float phase){
    return 0.0;
}

//Mochi
float mochi0(float phase){
    return 0.0;
}
float mochi1(float phase){
    return 0.0;
}
float mochi2(float phase){
    return 0.0;
}
float mochi3(float phase){
    return 0.0;
}
float mochi4(float phase){
    return 0.0;
}
float mochi5(float phase){
    return 0.0;
}
float mochi6(float phase){
    return 0.0;
}
float mochi7(float phase){
    return 0.0;
}
float mochi8(float phase){
    return 0.0;
}
float mochi9(float phase){
    return 0.0;
}
float mochi10(float phase){
    return 0.0;
}
float mochi11(float phase){
    return 0.0;
}

//Oni
float oni0(float phase){
    return 0.0;
}
float oni1(float phase){
    return 0.0;
}
float oni2(float phase){
    return 0.0;
}
float oni3(float phase){
    return 0.0;
}
float oni4(float phase){
    return 0.0;
}
float oni5(float phase){
    return 0.0;
}
float oni6(float phase){
    return 0.0;
}
float oni7(float phase){
    return 0.0;
}
float oni8(float phase){
    return 0.0;
}
float oni9(float phase){
    return 0.0;
}
float oni10(float phase){
    return 0.0;
}
float oni11(float phase){
    return 0.0;
}

//
float (*xmasFunc[12])(float)={xmas0,xmas1,xmas2,xmas3,xmas4,xmas5,xmas6,xmas7,xmas8,xmas9,xmas10,xmas11};
float (*mochiFunc[12])(float)={mochi0,mochi1,mochi2,mochi3,mochi4,mochi5,mochi6,mochi7,mochi8,mochi9,mochi10,mochi11};
float (*oniFunc[12])(float)={oni0,oni1,oni2,oni3,oni4,oni5,oni6,oni7,oni8,oni9,oni10,oni11};

//float (*getFunc(Mode mode,byte pin_num))(float){
float (*getFunc(Mode mode,byte pin_num))(float){
    switch(mode){
        case xmas:
            return xmasFunc[pin_num];
        case mochi:
            return mochiFunc[pin_num];
        case oni:
            return oniFunc[pin_num];
        default:
            return NULL;
    }
}

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
                case mochi:
                case oni:
                    for(byte cnt=0;cnt<12;cnt++){
                        pins[cnt]=Pin(getFunc(mode,cnt));
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
