#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUDRATE B9600

#define RX_BUFFER_SIZE 128
#define PASS_BUFFER_SIZE 64


// ----------------------------------------------------
// configuracion terminal
// ----------------------------------------------------
struct termios old, new;
void config0(void){
    tcgetattr(0, &old);
    new = old;
    new.c_lflag &= ~(ECHO | ICANON);    //elimina eco y configura modo no canonico
    new.c_cc[VMIN]=0;           //setea el minimo numero de caracteres que espera read()
    new.c_cc[VTIME] = 0;            //setea tiempo maximo de espera de caracteres que lee read()
    tcsetattr(0, TCSANOW, &new);
}
void restaurarTerminal() {
    tcsetattr(0, TCSANOW, &old);
}


// ----------------------------------------------------
// Configuración del puerto serie
// ----------------------------------------------------
int serial_open(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error abriendo el puerto serie");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, BAUDRATE);
    cfsetispeed(&tty, BAUDRATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 bits
    tty.c_cflag |= (CLOCAL | CREAD);           // habilitar RX
    tty.c_cflag &= ~(PARENB | PARODD);         // sin paridad
    tty.c_cflag &= ~CSTOPB;                    // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                   // sin flow control

    tty.c_lflag = 0;   // modo raw
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

// ----------------------------------------------------
// Lee una línea del puerto serie 
// ----------------------------------------------------
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

// ----------------------------------------------------
// Envía string por puerto serie
// ----------------------------------------------------
void serial_send(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
}

///////////////////////////////////////////
int leer_tecla(char *c) {
   if (read(0, c, 1) > 0) {
        return 1; 
    }
    return 0;
}
// ----------------------------------------------------
// Menú de autenticación
// ----------------------------------------------------
void password_menu(int fd) {
    char password[PASS_BUFFER_SIZE];
    char rx_buffer[RX_BUFFER_SIZE];
    
	//restaurarTerminal();
    config0();   // activar modo RAW al salir para el menu principal

    while (1) {
        printf("\n=== INGRESO DE CONTRASEÑA ===\n");
        printf("Ingrese la contraseña: ");
		//	printf("contraseña %s",password);
        for (int i = 0; i < 5; i++) {//lee los 5 caracteres
            while(read(0, &password[i], 1) <= 0) usleep(10); //solo lee cuando se escribe algo
            printf("*"); 
            fflush(stdout);
        }
        password[5] = '\0'; 
            

        // eliminar salto de línea
        password[strcspn(password, "\n")] = '\0';

        serial_send(fd, password);

        // esperar respuesta
        serial_readline(fd, rx_buffer, sizeof(rx_buffer));
     
        if (strcmp(rx_buffer, "pass_ok") == 0) {
            printf("\n✔ Contraseña correcta\n");
            break;
        } else if (strcmp(rx_buffer, "pass_fail") == 0) {
            printf("\n✖ Contraseña incorrecta, vuelva a digitar la contraseña\n");
        } else {
            printf("\nRespuesta desconocida: %s\n", rx_buffer);
        }
    }


    // Menú siguiente (placeholder)
}

void menu_principal(){
		system("clear");
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
}

void mostrarSecuencia(const char* nombre) {
   system("clear"); // Limpia la consola
    printf(" EJECUTANDO: %s\n", nombre);
    printf(" Presione [S] para volver al menu.\n");
    printf(" Use [FLECHAS] para variar velocidad.\n");
}
// ----------------------------------------------------
// MAIN
// ----------------------------------------------------
int main(void) {
    char rx_buffer[RX_BUFFER_SIZE];
    char c = ' ';
	tcgetattr(0, &old);//guardar configuracion original
	
    int serial_fd = serial_open(SERIAL_PORT);
    if (serial_fd < 0) return 1;

	while(1){
	system("clear");
    printf("Esperando comando 'rem_mod_en'...\n");

    while (1) {
        serial_readline(serial_fd, rx_buffer, sizeof(rx_buffer));

        if (strcmp(rx_buffer, "rem_mod_en") == 0 ) {
            printf("Modo remoto habilitado\n");
            password_menu(serial_fd);
            menu_principal();
            break;
        }
	}
	    
	while(1){
		int n = read(serial_fd, rx_buffer, sizeof(rx_buffer) - 1);
        if (n > 0) {
            rx_buffer[n] = 0;
            rx_buffer[strcspn(rx_buffer, "\r\n")] = 0; // Limpiamos saltos de linea

            
            if(rx_buffer[0] >= '0' && rx_buffer[0] <= '9') {//para mostrar solo numeros
                printf("\rVelocidad Actual: %s ms    ", rx_buffer);
                fflush(stdout);
            }
        }

        // LEER TECLADO 
        if (leer_tecla(&c)) {
            write(serial_fd, &c, 1);
            
            if(c == 27) { // Manejo de flechas
                if(leer_tecla(&c)) write(serial_fd, &c, 1); // [
                if(leer_tecla(&c)) write(serial_fd, &c, 1); // A o B
                continue;
            }

            if(c=='0' || c=='A') break;
		
      switch (c) {
            case '1': 
                mostrarSecuencia("AUTO FANTASTICO");
                break;
            case '2': 
                mostrarSecuencia("EL CHOQUE");
                break;
            case '3': 
                mostrarSecuencia("LA APILADA" );
                break;
            case '4': 
                mostrarSecuencia("LA CARRERA");
                break;
            case '5': 
                mostrarSecuencia("PROPIA 1 ");
                break;
            case '6': 
                mostrarSecuencia("PROPIA 2 ");
                break;
            case '7': 
                mostrarSecuencia("PROPIA 3 ");
                break;
            case '8': 
                mostrarSecuencia("PROPIA 4 ");
                break;
            case 'S':
                system("clear");   
                menu_principal();          
				break;
			case 'V':
				system("clear"); 
				printf(" Gire potenciometro para variar la velocidad \n");
				printf(" Presione [S] para volver al menu.\n");
				printf("Velocidad actual: %s ms \r", rx_buffer );
				fflush(stdout);
			break;
		
            
       }
	}
	usleep(10000);
		}
		
		if (c == '0') break; 
        
        
        if (c == 'A') {
             printf("\nModo Local activado\n");
             sleep(1);
        }
		}

	restaurarTerminal();
    close(serial_fd);
    return 0;
}
