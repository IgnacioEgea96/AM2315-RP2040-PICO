#include <am2315.h>

int main(){
    float t, h;

    stdio_init_all();
    sleep_ms(2000);

    if(!init_am2315()){
        printf("Sensor not found\n");
        while(1);
    }else
        printf("Sensor found\n");
    
    sleep_ms(2000);

    while(true){
        if(readTemperatureAndHumidity(&t,&h)){
            printf("Temperature:%6.2f C\n", t);
            printf("Humidity:%6.2f %%\n",h);
        }else
            printf("Lecture error");
        sleep_ms(2000);
    }
}