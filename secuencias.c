#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>   

int check_teclado(int *);
void enviar_velocidad(int );

void autoFantastico(const int *leds, int *vel) {
    int i;
    while (1) { 
        // ida
        for (i = 0; i < 8; i++) {
            digitalWrite(leds[i], HIGH);
            delay(*vel);        
            if (check_teclado(vel)) return;
           //  enviar_velocidad( vel);
            digitalWrite(leds[i], LOW);
        }
        // vuelta
        for (i = 6; i > 0; i--) {
            digitalWrite(leds[i], HIGH);
            delay(*vel);
            if (check_teclado(vel)) return;
           // enviar_velocidad( vel);
            digitalWrite(leds[i], LOW);
        }
    }
}

void elChoque(const int *leds, int *vel) {
    int i;
    while (1) {
        for (i = 0; i < 8; i++) {
            digitalWrite(leds[i], HIGH);
            digitalWrite(leds[7-i], HIGH);
            
            delay(*vel);
            if (check_teclado(vel)) return;

            digitalWrite(leds[i], LOW);
            digitalWrite(leds[7-i], LOW);
        }
    }
}

void laApilada(const int *leds, int *vel) {
    int i, pila;
    while (1) {

        for (pila = 7; pila >= 0; pila--) {
            for (i = 0; i < pila; i++) {
                digitalWrite(leds[i], HIGH);
                delay(*vel / 2); 
                if (check_teclado(vel)) return;
                digitalWrite(leds[i], LOW);
            }
            digitalWrite(leds[pila], LOW);  
            delay(*vel); 
            digitalWrite(leds[pila], HIGH); 
            delay(*vel); 
            digitalWrite(leds[pila], LOW); 
            delay(*vel);
            digitalWrite(leds[pila], HIGH); 
            if(check_teclado(vel)) return;
        }
        delay(1000); 
    }
}

void laCarrera(const int *leds, int *vel) {
    int i, bit;
    const unsigned char secuencia[] = {
        0x01, 0x02, 0x04, 0x08, 0x11, 0x12, 0x24, 0x28, 
        0x50, 0x60, 0xC0, 0x80, 0x00
    };
    int num_pasos = 13;
    
    while (1) {
        for (i = 0; i < num_pasos; i++) {
            unsigned char aux = secuencia[i];
            for (bit = 0; bit < 8; bit++) {
                if ((aux >> bit) & 1) digitalWrite(leds[bit], HIGH);
                else digitalWrite(leds[bit], LOW);
            }

            if (i < 4){
                delay(*vel);
            }
            else{
                delay(*vel / 2);
            } 

            if (check_teclado(vel)) return;
        }
        delay(500);
    }
}
