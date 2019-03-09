#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//LEITOR BIOMETRICO
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;

//BOTAO
const int botaoVerde = 8;  
const int botaoVermelho = 9;  
int botaoEstadoVerde = 0; 
int botaoEstadoVermelho = 0;
const int botao1 = 13;  
const int botao2 = 12; 
const int botao3 = 11; 
int botaoEstado1 = 0; 
int botaoEstado2 = 0; 
int botaoEstado3 = 0; 

//FAIXA DE NIVEIS DE ACESSO
int contnivel1 = 0;
int contnivel2 = 30;
int contnivel3 = 60;
int contnivel4 = 90;
int adm = 0;
//LED
int led_verde_deposito_1 = 4;
int led_vermelho_deposito_1 = 14 ;
  
int led_verde_deposito_2 = 5;
int led_vermelho_deposito_2 = 15;

int led_verde_deposito_3 = 6;
int led_vermelho_deposito_3 = 16;

//LCD
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() 
{
  // LEDs
  pinMode(led_verde_deposito_1, OUTPUT);
  pinMode(led_vermelho_deposito_1, OUTPUT);
  pinMode(led_verde_deposito_2, OUTPUT);
  pinMode(led_vermelho_deposito_2, OUTPUT);  
  pinMode(led_verde_deposito_3, OUTPUT);
  pinMode(led_vermelho_deposito_3, OUTPUT);
    
  // CONFIGURACAO DO LEITOR BIOMETRICO
  Serial.begin(9600);
  

  // LCD
  lcd.begin (16, 2);
  lcd.setBacklight(HIGH);
  
  //Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  if (finger.verifyPassword())
  {
    Serial.println("Encontrado fingerprint sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("*");
    delay(1000);
    lcd.clear();
    finger.emptyDatabase();
  } 
  else 
  {  
    Serial.println("Nao foi encontrado o fingerprint sensor :(");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("* *");
    delay(500);
    lcd.clear();
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contem "); Serial.print(finger.templateCount); Serial.println(" modelos");
  Serial.println("Waiting for valid finger...");
  
  //BOTOES
  pinMode(botaoVerde, INPUT_PULLUP);  
  pinMode(botaoVermelho, INPUT_PULLUP); 
  pinMode(botao1, INPUT_PULLUP); 
  pinMode(botao2, INPUT_PULLUP); 
  pinMode(botao3, INPUT_PULLUP);

}

int getFingerprintIDez();
int autenticaADM();
uint8_t deleteFingerprint(uint8_t id);
uint8_t getFingerprintID();
uint8_t getFingerprintEnroll();
void acendeLed(int nivelSeguranca);
void verificaNivelDeAcesso(int id,int nivelSeguranca); 
void acendeLedVermelho(int nivelSeguranca);
void autentica(int nivelSeguranca);
boolean verificaSeExisteAlgumCadastrado();
void cadastrarDedo();
void lerDedo();
void excluiDigitalUnica();
void excluiTudo();
void excluir();
void printLcd(long tempo, String texto1, String texto2,boolean limpa);
void acendeLedNegado(int nivelSeguranca);
void acendeLedLiberado(int nivelSeguranca);

void loop() {
  
printLcd(100,"1 Cadastrar ","2 Ler 3 Excluir ", true);

  botaoEstado1 = digitalRead(botao1);
  if (botaoEstado1 == 0) {    
    cadastrarDedo();
  }
  
  botaoEstado2 = digitalRead(botao2);
  if(botaoEstado2 == 0){
    lerDedo();
  }

  botaoEstado3 = digitalRead(botao3);
  if(botaoEstado3 == 0){
    excluir();
  }

}

// FUNCOES DO SISTEMA 
void cadastrarDedo(){
     
     if(!(verificaSeExisteAlgumCadastrado())){
      printLcd(3000, "Cadastre um ADM","para continuar",true); 
      cadastrarADM();
      return;
     }   
  
      printLcd(4000,"Autenticar ADM", "",true);
       int admId = 0;
      admId = autenticaADM();
     if (admId <= 0 ){
      printLcd(1000, "Acesso Negado!","",false); 
      acendeLedVermelho();
      return;
     }
     
     printLcd(2000, "Escolha o nivel","-1- -2- -3-",false); 
     
     while(true){ 
      botaoEstado1 = digitalRead(botao1);
      botaoEstado2 = digitalRead(botao2);
      botaoEstado3 = digitalRead(botao3);
      botaoEstadoVermelho = digitalRead(botaoVermelho);
      botaoEstadoVerde = digitalRead(botaoVerde);
      if (botaoEstado1 == 0 && contnivel1 < contnivel2){
        contnivel1++;
        id = contnivel1;
        break;
        }
        else if (botaoEstado2 == 0 && contnivel2 < contnivel3){
        contnivel2++;
        id = contnivel2;
        break;
        }
        else if (botaoEstado3 == 0 && contnivel3 < contnivel4){
        contnivel3++;
        id = contnivel3;
        break;
        }
        else if (botaoEstado1 == 0 && botaoEstado2 == 0 && botaoEstado3 == 0 && contnivel4 < 120){
          contnivel4++;
          id = contnivel4;
          printLcd(2000, "Cadastrando ADM","",false); 
          break; 
        }

      if(botaoEstadoVerde == 0){
          contnivel4++;
          id = contnivel4;
          printLcd(2000, "Cadastrando ADM","",false); 
          break; 
      }
     
      if(botaoEstadoVermelho == 0){
       return;
      }
        
     }
     
     if (id == 0) 
     {
      return;
     }
     Serial.print("Enrolling ID #");
     Serial.println(id);
  
     while (!  getFingerprintEnroll() );
}

void cadastrarADM(){
    printLcd(2000,"Cadastrando..", "",true);
    if (contnivel4 < 120)
    {
      contnivel4++;
      id = contnivel4;
    }else{
      printLcd(2000,"Excedeu n de", "ADMs",true);
    }
    if (id == 0) 
    {
    return;
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);
    while (!  getFingerprintEnroll() );  
}
uint8_t getFingerprintEnroll() 
{
int p = -1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Coloque o dedo");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      lcd.setCursor(0,0);
//      lcd.print("*");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remova o dedo");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
    lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Coloque o dedo");
  delay(1000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Erro");
    delay(2000);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digital salva!");
  delay(3000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

void acendeLed(int nivelSeguranca){
  printLcd(2000,"Liberado!", "",false);
     
  switch (nivelSeguranca) {
      case 1:
        digitalWrite(led_verde_deposito_1, HIGH);
        digitalWrite(led_verde_deposito_2, LOW);
        digitalWrite(led_verde_deposito_3, LOW);
        digitalWrite(led_vermelho_deposito_2, HIGH);
        digitalWrite(led_vermelho_deposito_3, HIGH);
        delay(5000);
        digitalWrite(led_verde_deposito_1, LOW);
        digitalWrite(led_vermelho_deposito_2, LOW);
        digitalWrite(led_vermelho_deposito_3, LOW);
        break;
      case 2:
        digitalWrite(led_verde_deposito_1, HIGH);
        digitalWrite(led_verde_deposito_2, HIGH);
        digitalWrite(led_verde_deposito_3, LOW);
        digitalWrite(led_vermelho_deposito_3, HIGH);
        delay(5000);
        digitalWrite(led_verde_deposito_1, LOW);
        digitalWrite(led_verde_deposito_2, LOW);
        digitalWrite(led_vermelho_deposito_3, LOW);        
        break;
      case 3:
        digitalWrite(led_verde_deposito_1, HIGH);
        digitalWrite(led_verde_deposito_2, HIGH);
        digitalWrite(led_verde_deposito_3, HIGH);
        delay(5000);
        digitalWrite(led_verde_deposito_1, LOW);
        digitalWrite(led_verde_deposito_2, LOW);
        digitalWrite(led_verde_deposito_3, LOW);
        break;
  }  
}
void acendeLedLiberado(int nivelSeguranca){
  printLcd(2000,"Liberado!", "",false);
     
  switch (nivelSeguranca) {
      case 1:
        digitalWrite(led_verde_deposito_1, HIGH);
        delay(5000);
        digitalWrite(led_verde_deposito_1, LOW);
        break;
      case 2:
        digitalWrite(led_verde_deposito_2, HIGH);
        delay(5000);
        digitalWrite(led_verde_deposito_2, LOW);
        break;
      case 3:
        digitalWrite(led_verde_deposito_3, HIGH);
        delay(5000);
        digitalWrite(led_verde_deposito_3, LOW);
        break;
  }  
}
void acendeLedNegado(int nivelSeguranca){
     
  switch (nivelSeguranca) {
      case 1:
        digitalWrite(led_vermelho_deposito_1, HIGH);
        delay(5000);
        digitalWrite(led_vermelho_deposito_1, LOW);
        break;
      case 2:
        digitalWrite(led_vermelho_deposito_2, HIGH);
        delay(5000);
        digitalWrite(led_vermelho_deposito_2, LOW);
        break;
      case 3:
        digitalWrite(led_vermelho_deposito_3, HIGH);
        delay(5000);
        digitalWrite(led_vermelho_deposito_3, LOW);
        break;
  }  
}
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}

void lerDedo(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite o numero");
  lcd.setCursor(0, 1);
  lcd.print("do armazem");
  delay(2000);
  botaoEstado1 = digitalRead(botao1);
  botaoEstado2 = digitalRead(botao2);
  botaoEstado3 = digitalRead(botao3);
  botaoEstadoVermelho = digitalRead(botaoVermelho);
  while (true){
      botaoEstado1 = digitalRead(botao1);
      if (botaoEstado1 == 0) {
        Serial.println("Apertado bt 1");   
         printLcd(2500, "Acessando A1","",false); 
         autentica(1);
         break;
      }
      
       botaoEstado2 = digitalRead(botao2);
      if (botaoEstado2 == 0) {  
        Serial.println("Apertado bt 2"); 
         printLcd(2500, "Acessando A2","",false); 
         autentica(2);
          break ;
      }
    
       botaoEstado3 = digitalRead(botao3);
      if (botaoEstado3 == 0) {  
        Serial.println("Apertado bt 3");
        printLcd(2500, "Acessando A3","",false); 
          autentica(3);
          break;
      }
     botaoEstadoVermelho = digitalRead(botaoVermelho);
     if(botaoEstadoVermelho == 0){
       return;
      }
      
  }
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    printLcd(2500, "Digital apagada","",false); 
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    printLcd(1000, "Erro","",false); 
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    printLcd(1000, "Erro","",false); 
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    printLcd(1000, "Erro","",false); 
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    printLcd(1000, "Erro","",false); 
    return p;
  }   
}

void verificaNivelDeAcesso(int id,int nivelSeguranca){
  printLcd(2000, "Validando","acesso",false); 
      
  Serial.println(nivelSeguranca);
  Serial.println(id);
  switch (nivelSeguranca) {
      case 1:
        if ((id > 0 && id < 30) || (id > 30 && id < 60) || (id > 60 && id <90) ){
          acendeLedLiberado(nivelSeguranca);
        }else{
          printLcd(3000, "Acesso negado! ","",false); 
          acendeLedNegado(nivelSeguranca);
        }
        break;
      case 2:
         if ((id > 30 && id < 60)|| (id > 60 && id <90)){
          acendeLedLiberado(nivelSeguranca);
        }else{
         printLcd(3000, "Acesso negado! ","",false); 
         acendeLedNegado(nivelSeguranca);
        }
        break;
      case 3:
         if (id > 60 && id < 90){
         acendeLedLiberado(nivelSeguranca);
        }else{
         printLcd(3000, "Acesso negado!","",false); 
         acendeLedNegado(nivelSeguranca);
        }
        break;
      default:
        printLcd(3000, "Falha na","autenticacao!",false); 
  }  
}

void acendeLedVermelho(){
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Acesso negado!");
  digitalWrite(led_vermelho_deposito_1, HIGH);
  digitalWrite(led_vermelho_deposito_2, HIGH);
  digitalWrite(led_vermelho_deposito_3, HIGH);
  delay(5000);
  digitalWrite(led_vermelho_deposito_1, LOW);
  digitalWrite(led_vermelho_deposito_2, LOW);
  digitalWrite(led_vermelho_deposito_3, LOW);  
}

void autentica(int nivelSeguranca){
      printLcd(1000, "Coloque o dedo!","",false); 
      int id = -2;
      long intervalo = 10000;
      long tempoAtual = millis();
      while(id < 0){
        
            id = getFingerprintIDez();
            if(id > 0){
            verificaNivelDeAcesso(id,nivelSeguranca);
             break;
            }  
            if(millis() - tempoAtual > intervalo){
               printLcd(2500, "Falha na ","autenticacao!",false); 
               acendeLedNegado(nivelSeguranca);
               Serial.print("Falha na autenticacao!");
               break;
            }    
      }
}
int autenticaADM(){
      printLcd(1000, "Coloque o dedo","",false); 
      int id = -2;
      long intervalo = 10000;
      long tempoAtual = millis();
      while(id < 0){
        
            id = getFingerprintIDez();
            if(id > 90){
              Serial.print(id);
              return 1;
            }  
            if(millis() - tempoAtual > intervalo){
               printLcd(2500, "Falha na","autenticacao",false); 
               Serial.print("Falha na autenticacao!");
               return -1;
            }    
      }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}

boolean verificaSeExisteAlgumCadastrado(){
 finger.getTemplateCount();
  Serial.print("Sensor contem "); Serial.print(finger.templateCount); Serial.println(" modelos");
  Serial.println("Waiting for valid finger...");

  if(finger.templateCount > 0){
    return true;
  }
  return false;
}

void excluiDigitalUnica(){
   if(!(verificaSeExisteAlgumCadastrado())){
      printLcd(3000, "Nenhuma digital","cadastrada",true); 
      return;
     }   
  
  printLcd(1000, "Coloque o dedo","",false); 

  long intervalo = 10000;
  long tempoAtual = millis();
  int id = -2;
  while(id < 0){    
   id = getFingerprintIDez();
   if(id > 0){
   deleteFingerprint(id);
   break;
   } 
   if(millis() - tempoAtual > intervalo){
      printLcd(2500, "Falha na","autenticacao!",false); 
      break;
   }       
 }
}

void excluiTudo(){
  if(!(verificaSeExisteAlgumCadastrado())){
      printLcd(3000, "Nenhuma digital","cadastrada",true); 
      return;
     }   
  
  adm = autenticaADM();
  if (adm < 0 ){
  printLcd(2500, "Falha na","autenticacao!",false); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Acesso Negado!");
  acendeLedVermelho();
  return;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Apagar todas?");
  lcd.setCursor(0,1);
  lcd.print("Verde-S/Vermel-N");
  delay(4000);
  while(true){
    botaoEstadoVerde = digitalRead(botaoVerde);
    botaoEstadoVermelho = digitalRead(botaoVermelho);
    if(botaoEstadoVerde == 0){
        finger.emptyDatabase();
        contnivel1 = 0;
        contnivel2 = 30;
        contnivel3 = 60;
        contnivel4 = 90;
        printLcd(2500, "Registros","apagados!",false); 
        break;
      }
    if(botaoEstadoVermelho == 0){
        break;
      }
  }
}

void excluir(){ 
  printLcd(3000, "1-Uma digital","2-Todas",false); 
  botaoEstado1 = digitalRead(botao1);
  if (botaoEstado1 == 0) {    
    excluiDigitalUnica();
  }
  
  botaoEstado2 = digitalRead(botao2);
  if(botaoEstado2 == 0){
    excluiTudo();
  }
 }
void printLcd(long tempo, String texto1, String texto2, boolean limpa){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(texto1);
    if(texto2 != ""){
      lcd.setCursor(0,1);
      lcd.print(texto2);
    }
    delay(tempo);
   if(limpa){
     lcd.clear();
   }
   
}
 
