#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>   

int check_teclado(int *);

void secuenciaPropia1_Algoritmo(const int *leds, int *vel) {
    while (1) {
        for (int i=0; i<8; i+=2) digitalWrite(leds[i], HIGH);
        for (int i=1; i<8; i+=2) digitalWrite(leds[i], LOW);
        
        delay(*vel);
        if (check_teclado(vel)) return;

        for (int i=0; i<8; i+=2) digitalWrite(leds[i], LOW);
        for (int i=1; i<8; i+=2) digitalWrite(leds[i], HIGH);
        
        delay(*vel);
        if (check_teclado(vel)) return;
    }
}

void secuenciaPropia2_Algoritmo(const int *leds, int *vel) {
    while (1) {
        for (int i=0; i<8; i++) digitalWrite(leds[i], LOW); 
        
        for (int i=0; i<8; i++) {
            digitalWrite(leds[i], HIGH);
            delay(*vel);
            if (check_teclado(vel)) return;
        }
    }
}


void secuenciaPropia3_Tabla(const int *leds, int *vel) {
    int i, bit;
    
    const unsigned char tabla[] = {
        0x07, // 00000111
        0x0E, // 00001110
        0x1C, // 00011100
        0x38, // 00111000
        0x70, // 01110000
        0xE0  // 11100000
    }; 
    int pasos = 6;

    while(1) {
        for(i = 0; i < pasos; i++) {
            unsigned char aux = tabla[i];
            
            for(bit = 0; bit < 8; bit++) {
                 if((aux >> bit) & 1) digitalWrite(leds[bit], HIGH);
                 else digitalWrite(leds[bit], LOW);
            }
            
            delay(*vel);
            if (check_teclado(vel)) return;
        }
    }
}

void secuenciaPropia4_Tabla(const int *leds, int *vel) {
    int i, bit;
    
    const unsigned char tabla[] = {0x0F, 0xF0};
    int pasos = 2;

    while(1) {
        for(i = 0; i < pasos; i++) {
            unsigned char aux = tabla[i];
            
            for(bit = 0; bit < 8; bit++) {
                if((aux >> bit) & 1) digitalWrite(leds[bit], HIGH);
                else digitalWrite(leds[bit], LOW);
            }

            delay(*vel);
            if (check_teclado(vel)) return;
        }
    }
}
