#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <termios.h>  
#include <unistd.h>   
#include <wiringPi.h>
#include <pcf8591.h>
#include <wiringSerial.h>

const int leds[8] = {23, 24, 25, 12, 16, 20, 21, 26};
const int num_leds = 8;
int fd;        
int modo_remoto = 0;
const char CLAVE_CORRECTA[] = "12345";

#define BASE 64      
#define ADRESS 0x48 
#define A0 BASE + 0
#define RX_BUFFER_SIZE 128

//////////////////////// prototipos///////////////////////////////////
void autoFantastico(const int *leds, int *velocidad);      
void elChoque(const int *leds, int *velocidad);            
void laApilada(const int *leds, int *velocidad);           
void laCarrera(const int *leds, int *velocidad);           
void secuenciaPropia1_Algoritmo(const int *leds, int *velocidad); 
void secuenciaPropia2_Algoritmo(const int *leds, int *velocidad); 
void secuenciaPropia3_Tabla(const int *leds, int *velocidad);     
void secuenciaPropia4_Tabla(const int *leds, int *velocidad);  
extern int calculoLectura(int );//le paso analogRead(A0)
void enviar_velocidad(int );

//////////////////////enviar vel//////////////////////

void enviar_velocidad(int vel){
     if (modo_remoto) {
        serialPrintf(fd, " %d\n", vel);
    }
    
}

////////////////////modo remoto//////////////////////
int serial_readline(int fd, char *buffer, size_t maxlen) {
    size_t idx = 0;
    char c;

    while (idx < maxlen - 1) {
        int n = read(fd, &c, 1);
        if (n <= 0) continue;

        if (c == '\n') break;
        buffer[idx++] = c;
    }

    buffer[idx] = '\0';
    return idx;
}

void modoRemoto(int fd){
    
    char rx_buffer[RX_BUFFER_SIZE];
    system("clear");
    printf("modo remoto inicializado");
    char *msg="rem_mod_en";
        
        if(modo_remoto){
                
                write(fd, msg,strlen(msg));
                write(fd, "\n",1);
                
                
                while(strcmp(msg,"pass_ok")){
                    serial_readline(fd,rx_buffer,sizeof(rx_buffer));
                      if(strcmp(CLAVE_CORRECTA, rx_buffer) == 0){
                    msg="pass_ok";
                    }else{
                        msg="pass_fail";
                        }
                        write(fd, msg,strlen(msg));
                        write(fd, "\n",1);
                    }
              
        
            }
    
}

//funcion para leer tecla en local o remoto
int leer_tecla(char *c) {
    if (modo_remoto == 1) {
        // MODO REMOTO
        if (serialDataAvail(fd)) {
           *c = serialGetchar(fd);
            //printf ("\n\n%c\n\n", *c);
            return 1; 
        }
    } else {
        // MODO LOCAL
        if (read(0, c, 1) > 0) {
            return 1; 
        }
    }
    return 0; 
}

// Devuelve 1 si hay que salir, y 0 si no hay que salir
int check_teclado(int *ms) {
    char c;
       if (leer_tecla(&c)) {  // devuelve 1 si hay tecla 
        if (c == 83) 
        {
            return 1;//enter 
        }
        if (c == 27) { // flecha
            char buf[2];
                if (leer_tecla(&buf[0])) {
                delay(1);
                leer_tecla(&buf[1]);
                
                if (buf[1] == 'A') *ms -= 200; 
                if (buf[1] == 'B') *ms += 200; 
                if (*ms < 10) *ms = 10;
                if (*ms > 2000) *ms = 2000;
                
                printf("\r Velocidad: %d ms   ", *ms);
                fflush(stdout);
            }
        }
    }
    return 0;
}   

//////////////////////////configuracion terminal///////////////////////
struct termios old, new;	
void config0(void){
	tcgetattr(0,&old);
	new = old;
	new.c_lflag &= ~(ECHO | ICANON);	//elimina eco y configura modo no canonico
	new.c_cc[VMIN]=0;			//setea el minimo numero de caracteres que espera read()
	new.c_cc[VTIME] = 0;			//setea tiempo maximo de espera de caracteres que lee read()
	tcsetattr(0,TCSANOW,&new);
}

void restaurarTerminal() {
    tcsetattr(0, TCSANOW, &old);
}


///////////////////////////////validar password/////////////////////////////////
void validarPassword() {
    char entrada[6]; 
    int clave_ok = 0; 

    printf("\n=== CONTROL DE ACCESO ===\n");

    while (clave_ok == 0) {
        printf("Ingrese password (5 digitos): ");
        fflush(stdout);

        for (int i = 0; i < 5; i++) {//lee los 5 caracteres
            while(read(0, &entrada[i], 1) <= 0) delay(10); //solo lee cuando se escribe algo
            printf("*"); 
            fflush(stdout);
        }
        entrada[5] = '\0'; 

        printf("\n"); 

        if (strcmp(entrada, CLAVE_CORRECTA) == 0) {// compara hasta leer un /0
            printf(" Bienvenido.\n");
            clave_ok = 1; 
        } else {
            printf("Clave Incorrecta.\n\n");
        }
    }
}
////////////////////////velocidad ajuste //////////////////////////////////////7 
void ajusteVelocidadADC(int *velocidad) {
    char c = 0;
    system("clear");
    printf("\n--- AJUSTE DE VELOCIDAD INICIAL ---\n");
    printf("Gire el potenciometro para ajustar. Presione S para aceptar.\n"); // 

    while (c != 83) { 
        *velocidad = calculoLectura(analogRead(A0));
        printf("Velocidad actual: %3d ms \r", *velocidad);
        fflush(stdout);
        
        leer_tecla(&c);
        delay(50); 
    }
}

////////////////////mostrar secuenica en ejecucion////////////////////////////////
void mostrarSecuencia(const char* nombre, int *velocidad) {
   system("clear"); // Limpia la consola
    printf(" EJECUTANDO: %s\n", nombre);
    printf(" Velocidad Inicial: %d ms\n", *velocidad); 
    printf(" Presione [S] para volver al menu.\n");
    printf(" Use [FLECHAS] para variar velocidad.\n");
}
/////////////////////////main/////////////////////////////////////////////////////
int main() {
    int velocidad; 
    char c;

    config0();
    if (wiringPiSetupGpio() == -1)printf("Error wiringpi");
    
    if (pcf8591Setup(BASE, ADRESS) <= 0) printf("Error adc");
    velocidad = calculoLectura(analogRead(A0)); 

    if ((fd = serialOpen("/dev/serial0", 9600)) < 0) printf("Error puerto");
    

    for (int i = 0; i < num_leds; i++) {
        pinMode(leds[i], OUTPUT);
        digitalWrite(leds[i], LOW); 
    }

    validarPassword(); 

    while (1) {
        for (int i = 0; i < num_leds; i++)digitalWrite(leds[i], LOW); 
        system("clear");
        printf("=== MENU (Vel: %d ms) ===\n", velocidad);
        printf("1. Auto Fantastico\n");
        printf("2. El Choque\n");
        printf("3. La Apilada\n");
        printf("4. La Carrera\n");
        printf("5-8. Propias\n");
        printf("V. Ajustar Velocidad Inicial\n");
        printf("A. Cambiar modo:LOCAL/REMOTO\n");
        printf("0. Salir\n");
        printf("---------------------\n");
        printf("[FLECHAS]: Velocidad\n");
        printf("[S]  : Volver\n");
        printf("Opcion: ");
        fflush(stdout);

        while (!leer_tecla(&c)) delay(100); 
        
        if (modo_remoto && (c >= '1' && c <= '8')) {
             enviar_velocidad(velocidad);
        }      
    

        if (c == '0') break;
      switch (c) {
            case '1': 
                mostrarSecuencia("AUTO FANTASTICO", &velocidad);
                autoFantastico(leds, &velocidad); 
                break;
            case '2': 
                mostrarSecuencia("EL CHOQUE", &velocidad);
                elChoque(leds, &velocidad); 
                break;
            case '3': 
                mostrarSecuencia("LA APILADA", &velocidad);
                laApilada(leds, &velocidad); 
                break;
            case '4': 
                mostrarSecuencia("LA CARRERA", &velocidad);
                laCarrera(leds, &velocidad); 
                break;
            case '5': 
                mostrarSecuencia("PROPIA 1 ", &velocidad);
                secuenciaPropia1_Algoritmo(leds, &velocidad); 
                break;
            case '6': 
                mostrarSecuencia("PROPIA 2 ", &velocidad);
                secuenciaPropia2_Algoritmo(leds, &velocidad); 
                break;
            case '7': 
                mostrarSecuencia("PROPIA 3 ", &velocidad);
                secuenciaPropia3_Tabla(leds, &velocidad); 
                break;
            case '8': 
                mostrarSecuencia("PROPIA 4 ", &velocidad);
                secuenciaPropia4_Tabla(leds, &velocidad); 
                break;
            case 'V': 
                ajusteVelocidadADC(&velocidad); 
                break;
            case 'A':
                modo_remoto = !modo_remoto;
                modoRemoto(fd);
                break;
         
        }
    
    }

    restaurarTerminal();
    for(int i=0; i<8; i++) digitalWrite(leds[i], LOW);
    return 0;
}
