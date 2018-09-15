 #include <LiquidCrystal.h>
#include "SoftwareSerial.h"
#include "FPS_GT511C1R.h"
#include <Servo.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7); // --> (rs, enable, d4, d5, d6, d7) pinos LCD conectados ao arduino
FPS_GT511C1R fps(8, 9); // --> (tx,rx) pinos scanner digital conectadas ao arduino
Servo servo1,servo2;
int id=0, b1=0, b2=0, b3=0, b4=0;
struct digital {int id, pv;};
struct digital* tabelaHash[20];

void setup()
{
  fechado(servo1,10,90); // configura posição do servo
  fechado(servo2,11,90);
  lcd.begin(16, 2); // --> (coluna, linha) do LCD
  fps.Open();
  fps.SetLED(true);
  fps.DeleteAll();
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
}


void loop()
{
  if (fps.IsPressFinger()) verificarAcesso();
  
  b1 = digitalRead(A1);
  if (b1==1) cadastrar();
  
  b2 = digitalRead(A2);
  if (b2==1) apagar();
  
  b3 = digitalRead(A3);
  b4 = digitalRead(A4);  
  if (b3!=b4) verificarBotoes();
  
  printLCD("   Bem Vindo!","");
  
}

void printLCD(char *msg1, char *msg2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(msg1);
    lcd.setCursor(0, 1);
    lcd.print(msg2);
}

void cadastrar()
{  
        printLCD("      Novo      ","Cadastro Digital");
        delay(2000);
        
        while(fps.CheckEnrolled(id)) id++; // verifica ID disponível para armazenamento
        
        int ret = fps.EnrollStart(id);
        if (ret==0) // se retorno for 0, cadastro pode ser efetuado
        {
          printLCD("Pressione o dedo"," sobre o leitor ");
          
          // Lê a digtal por 3 vezes.
          for (int x=0;x<=2;x++) 
          {
            while(!fps.IsPressFinger()) delay(100); // aguarda o posicionamento do dedo
            printLCD("Aguarde leitura!","");
            if (!existeNoBD()) // verifica se digital já consta no banco de dados
            {                    
                  if (fps.CaptureFinger(true))
                  {
                      if (x==0) ret = fps.Enroll1(); 
                      if (x==1 && ret==0) ret = fps.Enroll2(); 
                      if (x==2 && ret==0) 
                      {
                        ret = fps.Enroll3();
                        if (fps.CheckEnrolled(id))
                        {
                          printLCD("   Digital Ok   ","     ID: ");
                          lcd.print(id);
                          setPrivilegio(id);
                          break;
                        }
                        else
                        {
                          printLCD("Erro no cadastro","    Codigo ");
                          lcd.print(ret);
                          break;
                        }
                      }
                    printLCD(" Retire o dedo ","   do leitor!   ");
                    while(fps.IsPressFinger()) delay(100); // aguarda a remoção do dedo
                    printLCD("Pressione o dedo","   novamente!   ");  
                  }
                  else
                  {
                      printLCD("Falha na captura","   da digital   ");
                      break;
                  }                  
          }
          else 
          {
             printLCD("   Digital ja   ","cadastrada ID ");
             lcd.print(id);
             break;
          }
       }
    }
    else if(ret==1) printLCD("Memoria","  Insuficiente  ");
    delay(2000);
}

void apagar()
{ 
  int s,n;
  struct digital* temp;
    
  printLCD(" Apagar somente ","  um registro?  ");
  delay(2000);
  printLCD("  Sim: botao 1  ","  Nao: botao 2  ");
  
  do{
    b1 = digitalRead(A1);
    b2 = digitalRead(A2);
  }while (b1==b2);
    
  if(b1==1) // Efetua deleção de um ID
  {
      printLCD("Pressione o dedo"," sobre o leitor ");
      while(!fps.IsPressFinger()) delay(100);
      printLCD("Aguarde leitura!","");
      if (existeNoBD())
      {          
          printLCD("Confirma delecao","   do ID: ");
          lcd.print(id);
          lcd.print("?");
          delay(2000);
          printLCD("  Sim: botao 1  ","  Nao: botao 2  ");
          
          do{
            s = digitalRead(A1);
            n = digitalRead(A2);
          }while (s==n);
          
          // apaga id da tabela e scanner
          if (s==1)
          {
            tabelaHash[id] = temp;
            fps.DeleteID(id);
            if(!fps.CheckEnrolled(id)) printLCD("Delecao Efetuada","");
            else printLCD("Falha na delecao","");
          }
          else printLCD("    Exclusao    ","   Cancelada!   ");
      }
      else printLCD("Falha: Digital","nao reconhecida!");
  }
  else if (b2==1) // Apaga todos os registros do banco
  {
      printLCD("Pressione o dedo"," sobre o leitor ");
      while(!fps.IsPressFinger()) delay(100);
      printLCD("Aguarde leitura!","");
      if (existeNoBD())
      {
          if(getPrivilegio(id)==3)
          {
              printLCD("Confirma delecao","da base de dados?");
              delay(2000);
              printLCD("  Sim: botao 1  ","  Nao: botao 2  ");
              
              do{
                s = digitalRead(A1);
                n = digitalRead(A2);
              }while (s==n);
              
              if (s==1)
              {
                while(tabelaHash[id] !=NULL)
                {   
                    tabelaHash[id] = temp;
                    ++id;
                }
                fps.DeleteAll();
                if(!fps.CheckEnrolled(id)) printLCD("Delecao Efetuada","");
                else printLCD("Falha na delecao","");
              }
              else printLCD("    Exclusao   ","   Cancelada!   ");
          }
          printLCD(" Acesso Negado! ","");
      }
      else printLCD("Falha: Digital","nao reconhecida!");
  }
  delay(2000);  
}

boolean existeNoBD()
{ 
    fps.CaptureFinger(false);
    int id2 = fps.Identify1_N();
    if (id2 < 200)
    {
        id=id2;
        return true;
    }
    else return false;
}

void verificarAcesso()
{
    if (!fps.IsPressFinger()) printLCD("Pressione o dedo"," sobre o leitor ");   
    while(!fps.IsPressFinger()) delay(100);
    printLCD("Aguarde leitura!","");    
 
    if(existeNoBD())
    {
        int pv = getPrivilegio(id); 
        if (pv==1)
        {
            if(b4!=1) // verifica se não foi solicitado acesso indevido a porta 2
            {
                if(servo1.read()==0)
                {
                    printLCD("Acesso permitido","    porta 1:    ");
                    aberto (servo1,10,0);
                }
            }
            printLCD(" Acesso Negado! ","");
        }
        else if (pv==2)
        {
            if(b3!=1) // verifica se não foi solicitado acesso indevido ao porta 1
            {
                if (servo2.read()==0){
                    printLCD("Acesso permitido","    porta 2:    ");  
                    aberto (servo2,11,0);
                }
            }
            printLCD(" Acesso Negado! ",""); 
        }
        else if (pv==3)
        {
            if(b3==b4)
            {
                printLCD("Aperte botao da","  porta 1 ou 2  ");
                   
                do{
                    b3 = digitalRead(A3);
                    b4 = digitalRead(A4);
                }while (b3==b4);
            }
               
            printLCD("Acesso Permitido","    porta: ");
            if(b3==1)
            {
               lcd.print("1");
               if (servo1.read()==0) aberto (servo1,10,0);
            }
            else
            {
                lcd.print("2");
                if (servo2.read()==0) aberto (servo2,11,0);
            }
        }
    } 
    else printLCD("Falha: Digital","nao reconhecida!");
    delay(2000);        
}

void verificarBotoes()
{
    if(b3==1)
    {
        if(servo1.read()==0) verificarAcesso();
        else fechado(servo1,10,90); 
    }
    else if(b4==1)
    {
        if(servo2.read()==0) verificarAcesso();
        else fechado(servo2,11,90);
    }
}

void setPrivilegio(int id)
{
    struct digital *item = (struct digital*) malloc(sizeof(struct digital));
    item->id = id;
        
    delay(2000);
    printLCD("    Definindo","   Permissoes   ");
    delay(2000);
    printLCD("Permissao local:"," Aperte botao 1 ");
    delay(2000);
    printLCD("Permissao total:"," Aperte botao 2 ");

    do{
        b1 = digitalRead(A1);
        b2 = digitalRead(A2);
    }while (b1==b2);

    if (b1==1)
    {
      printLCD("  Para porta 1  "," Aperte botao 1 ");
      delay(2000);
      printLCD("  Para porta 2  "," Aperte botao 2 ");

      do{
        b1 = digitalRead(A1);
        b2 = digitalRead(A2);
      }while (b1==b2);

      if(b1==1) item->pv = 1;
      else item->pv = 2;     
    }
    else item->pv = 3;

    tabelaHash[id] = item;

    printLCD("    Cadastro    ","   Finalizado   ");
}
int getPrivilegio(int id)
{
    return tabelaHash[id]->pv;
}

void aberto (Servo myservo, int pino,int ang)
{
    myservo.attach(pino);  
    for (ang; ang <= 90; ang += 1)
    { 
      myservo.write(ang);
      delay(63);
    }
    myservo.detach();
}

void fechado (Servo myservo, int pino,int ang)
{
    myservo.attach(pino);
    for (ang; ang >= 0; ang -= 1)
    {
      myservo.write(ang);
      delay(63);
    }
    myservo.detach();
}

