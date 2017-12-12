#include <SdFat.h>
#include "SdFat.h"
SdFat SD;
//librerias para ethernet shield
#include <SPI.h>
#include <Ethernet.h>
//#include <SD.h>
//Librerias para uso del reloj
#include <Wire.h>
#include "RTClib.h"


 
// Configuración de direccion MAC e IP.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,50);
//IPAddress gateway(148,213,41,129);
//IPAddress subnet(255,255,255,192);
 
// Inicia la libreria Ethernet server con el puerto 80 (por defecto el puerto HTTP).
EthernetServer server(80);
char daysOfTheWeek[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};

//Uso del reloj
RTC_DS3231 rtc;

//Salidas digitales que se usaran
byte activarPiston=2;
byte desactivarPiston=3;
byte sensor=5;
byte buzzer=6;
bool banPuerta;

//Mensajes
String CERROJO, CERROJOACTIVADO;
String horaInicio, horaFin, minutoInicio, minutoFin;
String horaInicio2, horaFin2, minutoInicio2, minutoFin2;
String reporte;
bool ban;
byte cont=0;
bool ban3;

//Hora y minutos
String hora, minutos, segundos;
 
void setup() {
// Inicia el puerto serie.
Serial.begin(9600);
//Iniciar la tarjeta SD
Serial.print("Iniciando tarjeta SD...");
pinMode(53, OUTPUT);
delay(2000);
if(!SD.begin(4)){
  Serial.println("Tarjeta no reconocida, asegurese que esta insertada y el puerto digital 4 este libre, seguido reinicie el arduino");
  return;  
}else{
  Serial.println("Tarjeta SD reconocida");  
} 


// Inicia la conexión Ethernet y el servidor.
Ethernet.begin(mac, ip);
server.begin();
Serial.print("IP local del servidor ");
Serial.println(Ethernet.localIP());

//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

// Inicializar los pines digiales
pinMode(activarPiston, OUTPUT);
pinMode(desactivarPiston, OUTPUT);
pinMode(sensor, INPUT);
pinMode(buzzer, OUTPUT);


if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

ban=true;
ban3=true;
cont=0;
horaInicio="", horaFin="", minutoInicio, minutoFin="";
banPuerta=true;

hora="";
minutos="";
segundos="";
}

void ftp(String mensaje){
    DateTime now=rtc.now(); //Declaramos la variable del reloj
    File dataFile=SD.open("reporte.txt", FILE_WRITE);
    if(String(now.hour(), DEC).length()==1){
      hora="0"+String(now.hour(), DEC);  
    }else{
      hora=String(now.hour(), DEC);  
    }
    if(String(now.minute(), DEC).length()==1){
      minutos="0"+String(now.minute(), DEC);  
    }else{
      minutos=String(now.minute(), DEC);  
    }
    if(String(now.second(), DEC).length()==1){
      segundos="0"+String(now.second(), DEC);  
    }else{
      segundos=String(now.second(), DEC);  
    }
    String reporte=(String(daysOfTheWeek[now.dayOfTheWeek()])+"\t"+String(now.day(), DEC)+"/"+String(now.month(), DEC)+"/"+String(now.year(), DEC)+"\t"+hora+":"+minutos+":"+segundos+"  \t"+mensaje+"\n");
    //Serial.print(reporte);
    dataFile.print(reporte);
    dataFile.close();
}
 
void loop() {
    String comando;
    int posicion;
    DateTime now=rtc.now(); //Declaramos la variable del reloj

    //Funcion para detectar si se abrio la puerta
    if(digitalRead(sensor)==HIGH){
      digitalWrite(buzzer, HIGH);
      delay(500);
      digitalWrite(buzzer, LOW);
      delay(500);
      if(banPuerta){
        banPuerta=false;
        ftp("La puerta a sido abierta");
      }
    }else if(digitalRead(sensor)==LOW && banPuerta==false){
      banPuerta=true;  
      ftp("La puerta a sido cerrada");
    }

    //Funcion para activar la puerta por horas;
    if(String(now.hour(), DEC).length()==1){
      hora="0"+String(now.hour(), DEC);  
    }else{
      hora=String(now.hour(), DEC);  
    }
    if(String(now.minute(), DEC).length()==1){
      minutos="0"+String(now.minute(), DEC);  
    }else{
      minutos=String(now.minute(), DEC);  
    }

    if(ban3 && horaInicio2==hora && minutoInicio2==minutos){
        digitalWrite(activarPiston, HIGH);
        delay(100);
        digitalWrite(activarPiston, LOW);
        CERROJO="activado";
        ftp("Se activo el cerrojo automatico");
        ban3=false;
    }
    if(ban3==false && horaFin2==hora && minutoFin2==minutos){
        digitalWrite(desactivarPiston, HIGH);
        delay(100);
        digitalWrite(desactivarPiston, LOW);
        ban=true;
        CERROJO="desactivado";
        ftp("Se desactivo el cerrojo automatico");
        ban3=true;
    }
  
    EthernetClient client = server.available(); // Escucha a los clientes entrantes.
    
    if (client) { // Si un cliente se conecta al servidor:
      //Serial.println("GET /?horaInicio=horaInicio");
      
      Serial.println("Nuevo cliente\nHoy es "+String(now.day(), DEC)+"/"+String(now.month(), DEC)+"/"+String(now.year(), DEC));
      Serial.println(String(now.hour(), DEC)+":"+String(now.minute(), DEC)+":"+String(now.second(), DEC));
      boolean currentLineIsBlank = true; // Marcador para enviar la respuesta desde el servidor.
      String cadena="";   
          while (client.connected()) { // Repite mientas existe clientes conectados:
              if (client.available()) {
              char c = client.read();
              Serial.write(c); // Imprime por el puerto serie la petición del cliente (caracter a caracter)
              //Fecha y hora

              if(cadena.length()<60){
                cadena.concat(c);

                int posicion=cadena.indexOf("data");
                comando=cadena.substring(posicion);

                
                 if(comando=="data1=0" && ban){
                    digitalWrite(activarPiston, HIGH);
                    delay(100);
                    digitalWrite(activarPiston, LOW);
                    ban=false;
                    CERROJO="activado";
                    ftp("Se activo el cerrojo");
                  }else if(comando=="data1=1" && ban==false){
                    digitalWrite(desactivarPiston, HIGH);
                    delay(100);
                     digitalWrite(desactivarPiston, LOW);
                     ban=true;
                     CERROJO="desactivado";
                     ftp("Se desactivo el cerrojo");
                 }else if(comando=="data1=2"){
                    ftp("Se desactivo el cerrojo por horas");
                    horaInicio2="";
                    minutoInicio2="";
                    horaFin2="";
                    minutoFin2="";
                    CERROJOACTIVADO="El cerrojo por horas esta desactivado";
                  }else if(comando=="data2=0"){
                    reporte="";
                    File archivo=SD.open("reporte.txt");
                    while(archivo.available()){
                        reporte+=((char)archivo.read());
                    }
                    //Serial.println(reporte);
                    archivo.close();
                  }else if(comando=="data2=1"){
                      SD.remove("reporte.txt");
                      File file=SD.open("reporte.txt",FILE_WRITE);
                      file.close();
                  }else{
                    //Hora en que se activara el seguro
                    posicion=cadena.indexOf("horaInicio");
                    comando=cadena.substring(posicion);
                    horaInicio=comando.substring(11,13);
                    posicion=cadena.indexOf("minutoInicio");
                    comando=cadena.substring(posicion);
                    minutoInicio=comando.substring(13,15);

                    //Hora en que se desactivara el seguro
                    posicion=cadena.indexOf("horaFin");
                    comando=cadena.substring(posicion);
                    horaFin=comando.substring(8,10);
                    posicion=cadena.indexOf("minutoFin");
                    comando=cadena.substring(posicion);
                    minutoFin=comando.substring(10,12);
                    
                  }
              }
              
              
              if(isDigit(horaInicio.charAt(0)) && isDigit(horaInicio.charAt(1)) && isDigit(minutoInicio.charAt(0)) && isDigit(minutoInicio.charAt(1)) && isDigit(horaFin.charAt(0))&& isDigit(horaFin.charAt(1)) && isDigit(minutoFin.charAt(0))&& isDigit(minutoFin.charAt(1))){
                 
                 horaInicio2=horaInicio;
                 minutoInicio2=minutoInicio;
                 horaFin2=horaFin;
                 minutoFin2=minutoFin;
                 CERROJOACTIVADO="Se activo el cerrojo de "+horaInicio2+":"+minutoInicio2+" hasta las "+horaFin+":"+minutoFin; 
                 if(cont==1){
                  ftp("Se activo el cerrojo por horas"); 
                  cont=0;
                 }else{
                 cont++;
                 }
              }
                
                 horaInicio="";
                 minutoInicio="";
                 horaFin="";
                 minutoFin="";
      
            
            if (c == '\n' && currentLineIsBlank) { // Se envia la respuesta a una petición de un cliente cuando a finalizado la petición:
                
                // Respuesta:
                client.println("HTTP/1.1 200 OK"); // Enviar un encabezado de respuesta HTTP estándar
                client.println("Content-Type: text/html");
                client.println("Connection: close"); // Se cerrará la conexiós despues de enviar la respuesta.
                //client.println("Refresh: 5"); // Refrescar automáticamente la página después de 5 segundos.
                client.println();
                client.println("<!DOCTYPE HTML>"); // Tipo de documento.
                client.println("<html>"); // Etiqueta html inicio del documento. 
                client.print("<html>\n<head>\n<title>Door Pet</title>\n</head>\n<body>");
                client.print("<div style='text-align:center;'>");
                client.println("<h2>Activar cerrojo manualmente</h2>");
                client.println("<button onClick=location.href='./?data1=0'>Activar cerrojo</button>");
                client.println("<button onClick=location.href='./?data1=1'>Desactivar cerrojo</button>");
                client.println("<br/><h4>El cerrojo esta "+CERROJO+"</h4>"); 
                client.println("<h2>Activar cerrojo por horas</h2>");
                client.println("<h4><form method=get>Activar a las: <input type='text' name='horaInicio' maxlength=2 size=3/>:<input type='text' name='minutoInicio' maxlength=2 size=3/></h4>");
                client.println("<h4>Hasta las: <input type='text' name='horaFin' maxlength=2 size=3/>:<input type='text' name='minutoFin' maxlength=2 size=3/></h4>");
                client.println("<input type=submit value=Activar>");
                client.println("</form>");
                client.println("<button onClick=location.href='./?data1=2'>Desactivar</button>");
                client.println("</br><h4>"+CERROJOACTIVADO+"</h4>");
                client.println("<h2>Reporte</h2>");
                client.print("<button onClick=location.href='./?data2=0'>Ver reporte</button>");
                client.println("<button onClick=location.href='./?data2=1'>Borrar reporte</button>");
                client.println("</br>");
                client.println("<textarea name=txtReporte rows=40 cols=70>"+reporte+"</textarea>");
                //client.println("</br><a href="">Descargar reporte</a>");
                client.println("<p>Hoy es "+String(daysOfTheWeek[now.dayOfTheWeek()])+" "+String(now.day(), DEC)+"/"+String(now.month(), DEC)+"/"+String(now.year(), DEC)+"</p>");
                client.println("</html>"); // Etiqueta html fin del documento.
                break;
            }
            if (c == '\n') { // Si el caracter es un salto de linea:
                currentLineIsBlank = true; // La petición a terminado, se respondera a dicha peticón en el sigueitne ciclo.
            } 
            else if (c != '\r') { // Si el caracter no es un retorno de carro (la petición no a terminado).
                currentLineIsBlank = false; // Seguimos escuchando la petición.
            } 
          }
       }
       delay(1); // Espera para dar tiempo al navegador a recivir los datos.
       client.stop(); // Cierra la conexión.
       Serial.println("Cliente desconectado");
       Serial.println(horaInicio2+":"+minutoInicio2+" - "+horaFin2+":"+minutoFin2);
       reporte="";
       Serial.println();
    }
}
