#include <EEPROM.h>

const int ADDR_MODE=0;              //EEPROMに書き込むモード用変数のアドレス
const byte DUTY_DEFAULT=255;        //PWMのDuty比。0-255の間で設定
const unsigned int FLASH_INTERVAL=500;      //点滅間隔(ミリ秒)
const unsigned int GRADATION_CYCLE=4000;    //グラデーションの周期
const byte DUTY_MIN_GRADATION=50;   //グラデーションで一番暗くなる時のDuty比
const byte OUT_MIN=2;               //出力ピンの一番下
const byte OUT_MAX=13;              //出力ピンの一番上
const byte SW_MODE=15;              //モード切替スイッチのピン。15? 16?
const byte SW_MYST=16;              //TODO もう一つの謎入力。要調査

const byte PATTERN_XMAS=3;              //Pattern:パターン数
const byte PATTERN_MOCHI=3;
const byte PATTERN_ONI=3;

const bool GRADATION_FLAG=true;

//出力ピンのテーブルたち。定数だがポインタ参照がなぜかできないので、危ないけど変数
//ここから下、お触り厳禁！！！！！
bool XMAS[PATTERN_XMAS][12]={
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW}
};

bool MOCHI[PATTERN_MOCHI][12]={
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW}
};

bool ONI[PATTERN_ONI][12]={
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
    {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW}
};
//お触り厳禁ここまで

typedef enum{   //モードの列挙型。intで管理するより契約的
    xmas=0,     //Fu*k X'mas
    mochi=1,    //お正月
    oni=2       //節分
}Mode;          //typedefでclassみたいに使えて便利

void resetSoftware(){       //処理を強制終了して再起動
    asm volatile("jmp 0");  //よくわからんけどインラインアセンブラ
    //多分メモリの先頭に飛ぶ
}

class Light{                    //点灯関係のclass
    private:                    //直接触れないようにprivateに
        Mode mode;              //Mode型。現在のモード
        bool *flash_pins;    //点滅パターンでのピン光らせ方テーブル(ポインタ配列)
        bool *out_status;       //出力データ。flash_pinsをそのままポインタ参照
        byte pattern_num;            //パターン数

    public:                     //外から触れるpublicメンバ
        Light(byte read_mode){      //コンストラクタ。モードを設定
            mode=(Mode)read_mode;   //この辺くさい。byteからModeがどうcastされるか...
            switch(read_mode){
                case xmas:
                    flash_pins=&XMAS[0][0];
                    pattern_num=PATTERN_XMAS;
                    break;
                case mochi:
                    flash_pins=&MOCHI[0][0];
                    pattern_num=PATTERN_MOCHI;
                    break;
                case oni:
                    flash_pins=&ONI[0][0];
                    pattern_num=PATTERN_ONI;
                    break;
                default:            //Modeの内容に合致しなかったらxmasにして再起動
                    EEPROM.write(ADDR_MODE,(byte)xmas);
                    resetSoftware();
            }
        }

        Mode getMode(){     //modeのgetter。カプセル化
            return mode;
        }

        byte getPatternNum(){
            return pattern_num;
        }

        void flash(byte flash_status){
            //flash_status<PATTERN_XXX
            out_status=&flash_pins[flash_status];
            //out_statusの参照をflash_statusのtrue/falseの先頭ポインタにする
        }

        /*  手動でout_statusを設定する関数。後からテーブルを思いついたのと
            その結果out_statusを配列じゃなくてポインタ変数にしたので、
            領域が確保されている保証はない。使わないほうがいいと思う

            void setStatus(byte set_pin, boolean hl){  //hl...High Low
            out_status[set_pin-2]=hl;
            }
         */

        void pwmUpdate(byte pwm_cnt){   //PWM用の関数。やってることは見たまんま
            if(pwm_cnt>DUTY_DEFAULT){
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, LOW);
                }
            }else{
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, out_status[pin-OUT_MIN]);
                }
            }
        }

        void pwmUpdate(byte pwm_cnt,byte duty){   //上の関数のポリモーフィズム。duty比を任意の値で実行できる
            if(pwm_cnt>duty){
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, LOW);
                }
            }else{
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, out_status[pin-OUT_MIN]);
                }
            }
        }
};

void cycleUpdate(byte *now,byte *latest,byte *millis_latest,byte patterns){
    byte millis_now=(byte)(millis()%patterns);
    if(*millis_latest>millis_now){
        if(*now==0){
            *now++;
            *latest--;
        }else if(*now==patterns-1){
            *now--;
            *latest++;
        }else if(*now>*latest){
            *now++;
            *latest++;
        }else if(*now<*latest){
            *now--;
            *latest--;
        }
    }
    *millis_latest=millis_now;
}

byte getGradationDuty(){
    //PIは標準で定義済み
    byte duty_middle=(DUTY_DEFAULT+DUTY_MIN_GRADATION)/2;
    float phase=2*PI*(float)(millis()%GRADATION_CYCLE+1)/(float)GRADATION_CYCLE;
    return (byte)(duty_middle+(sin(phase)*(DUTY_DEFAULT-duty_middle)));
}

void setup(){
    for(int cnt=OUT_MIN; cnt<=OUT_MAX; cnt++){
        pinMode(cnt,OUTPUT);    //出力ピンをまとめて設定
    }
    pinMode(SW_MODE,INPUT);          //謎のピン達の設定。役目と定数名がわかったら
    pinMode(SW_MYST,INPUT);          //マジックナンバーじゃなくてちゃんとつける
}

void loop(){
    //グローバルで作ったLightのオブジェクトをsetupで初期化しようとしたら
    //エラー吐いたので、泣く泣くこっちに引っ越し
    //loop直後一回読んだ後はwhile(1)内をグルグルするので、実質的に
    //setupをloopスコープ内に実装できる。
    //契約的な問題で、ピンの設定等はsetup、それ以外はloopに書いた。

    byte cycle_now=0;
    byte cycle_latest=1;

    Light light(EEPROM.read(ADDR_MODE));    //Lightのコンストラクタ呼び出し
    light.flash(cycle_now);                     //出力ピンの初期状態をセット
    byte pattern=light.getPatternNum();

    if(pattern<2){
        resetSoftware();    //パターン数が2より小さいと成り立たない(そもそも点滅じゃない)のでリセット
    }

    byte millis_latest;
    millis_latest=(byte)(millis()%pattern);

    while(1){  //実質的なloop 
        if(digitalRead(SW_MODE)==HIGH){     //モード切替のハンドラ
            while(digitalRead(SW_MODE)==HIGH){  //押しっぱ防止
                if(digitalRead(SW_MODE)==LOW){  //手を離したらモード更新
                    EEPROM.write(ADDR_MODE,light.getMode()+1);  //モードを一個進める
                    resetSoftware();    //再起動
                }
            }
        }

        cycleUpdate(&cycle_now,&cycle_latest,&millis_latest,light.getPatternNum());

        for(byte pwm_count=0;pwm_count<=255;pwm_count++){   //PWM用ループ
            if(GRADATION_FLAG){
                light.pwmUpdate(pwm_count,getGradationDuty());
            }else{
            light.pwmUpdate(pwm_count);                     //pwm用に更新
            }
        }
    }
}
