#include <EEPROM.h>

const int ADDR_MODE=0;              //EEPROMに書き込むモード用変数のアドレス
const byte DUTY_RATIO=255;          //PWMのDuty比。0-255の間で設定
const unsigned int FLASH_CYCLE=2000;//点滅周期(ミリ秒)
const byte OUT_MIN=2;               //出力ピンの一番下
const byte OUT_MAX=13;              //出力ピンの一番上
const byte SW_MODE=15;              //TODO モード切替スイッチのピン。15? 16?
const byte SW_MYST=16;              //TODO もう一つの謎入力。要調査

//出力ピンのテーブルたち。定数だがポインタ参照がなぜかできないので、危ないけど変数
//ここから下、お触り厳禁！！！！！
const bool XMAS_FIRST[12]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
const bool XMAS_LATTER[12]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
const bool MOCHI_FIRST[12]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
const bool MOCHI_LATTER[12]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
const bool ONI_FIRST[12]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
const bool ONI_LATTER[12]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
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
        const bool *flash_pins[2];    //点滅パターンでのピン光らせ方テーブル(ポインタ配列)
        const bool *out_status;       //出力データ。flash_pinsをそのままポインタ参照

    public:                     //外から触れるpublicメンバ
        Light(byte read_mode){      //コンストラクタ。モードを設定
            mode=(Mode)read_mode;   //この辺くさい。byteからModeがどうcastされるか...
            switch(read_mode){
                case xmas:
                    flash_pins[false]=XMAS_FIRST;
                    flash_pins[true]=XMAS_LATTER;
                    break;
                case mochi:
                    flash_pins[false]=MOCHI_FIRST;
                    flash_pins[true]=MOCHI_LATTER;
                    break;
                case oni:
                    flash_pins[false]=ONI_FIRST;
                    flash_pins[true]=ONI_LATTER;
                    break;
                default:            //Modeの内容に合致しなかったらxmasにして再起動
                    EEPROM.write(ADDR_MODE,(byte)xmas);
                    resetSoftware();
            }
        }


        Mode getMode(){     //modeのgetter。カプセル化
            return mode;
        }

        void flash(bool flash_status){
            //出力データをテーブルから引っ張る。点滅は２パターンなので、
            //安全性を考えてboolで指定させることにした(flash_status)
            //C/C++のboolは内部的にはfalseが0、trueはそれ以外だが、ほとんどの
            //処理系では1なので、そのまま配列要素に突っ込む

            out_status=flash_pins[flash_status];
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
            if(pwm_cnt>DUTY_RATIO){
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, LOW);
                }
            }else{
                for(byte pin=OUT_MIN; pin<=OUT_MAX; pin++){
                    digitalWrite(pin, out_status[pin]);
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
    //グローバルで作ったLightのオブジェクトをsetupで初期化しようとしたら
    //エラー吐いたので、泣く泣くこっちに引っ越し
    //loop直後一回読んだ後はwhile(1)内をグルグルするので、実質的に
    //setupをloopスコープ内に実装できる。
    //契約的な問題で、ピンの設定等はsetup、それ以外はloopに書いた。
    bool cycle_now,cycle_latest;
    cycle_latest=cycle_now=false;               //初期状態はfalse(前半)
    Light light(EEPROM.read(ADDR_MODE));    //Lightのコンストラクタ呼び出し
    light.flash(cycle_now);                     //出力ピンの初期状態をセット

    while(1){  //実質的なloop 
        if(digitalRead(SW_MODE)==HIGH){     //モード切替のハンドラ
            while(digitalRead(SW_MODE)==HIGH){  //押しっぱ防止
                if(digitalRead(SW_MODE)==LOW){  //手を離したらモード更新
                    EEPROM.write(ADDR_MODE,light.getMode()+1);  //モードを一個進める
                    resetSoftware();    //再起動
                }
            }
        }

        if(millis()%FLASH_CYCLE<=FLASH_CYCLE/2){//ループを見る。周期の半分以下なら
            cycle_now=false;                    //現在はfalse
        }else{
            cycle_now=true;                     //もう半分はtrue
        }
        if(cycle_now!=cycle_latest){            //現在の状態が前回の切替と違ったら
            light.flash(cycle_now);             //出力ピンを現在の状態に更新
            cycle_latest=cycle_now;             //直前の変更を現在にする
        }

        for(byte pwm_count=0;pwm_count<=255;pwm_count++){   //PWM用ループ
            light.pwmUpdate(pwm_count);                     //pwm用に更新
        }
    }
}
