#include <LiquidCrystal_I2C.h>
#define INTENSIDAD_RELAJACION 40
#define INTENSIDAD_LECTURA   150


// Clase padre
class Componente {
  public:
    virtual void iniciar() {}
    virtual int leer() { return 0; }
    virtual void escribir(int valor = 0) {}
};

// Clase de los leds
class LED : public Componente {
  private:
    int pinLed;
    int pinSwitch;

  public:
    LED(int led, int sw) {
      pinLed = led;
      pinSwitch = sw;
    }

    void iniciar() override {
      pinMode(pinLed, OUTPUT);
      pinMode(pinSwitch, INPUT_PULLUP);
    }

    int leer() override {
      return digitalRead(pinSwitch);
    }

    void escribir(int valor) override {
      analogWrite(pinLed, valor);
    }

    bool normal(int ldr, int umbral) {
      if (leer() == LOW && ldr <= umbral) {
        digitalWrite(pinLed, HIGH);
        return true;
      }
      digitalWrite(pinLed, LOW);
      return false;
    }

    bool relajacion(int ldr, int umbral) {
      if (leer() == LOW && ldr <= umbral) {
        escribir(INTENSIDAD_RELAJACION);
        return true;
      }
      escribir(0);
      return false;
    }

    bool lectura() {
      if (leer() == LOW) {
        escribir(INTENSIDAD_LECTURA);
        return true;
      }
      escribir(0);
      return false;
    }

    bool fiesta(int ldr, int umbral, bool estado) {
      if (leer() == LOW && ldr <= umbral && estado) {
        digitalWrite(pinLed, HIGH);
        return true;
      }
      digitalWrite(pinLed, LOW);
      return false;
    }

    void noche () {
      digitalWrite(pinLed, LOW);
    }
};

// Clase del LDR
class Sensor : public Componente {
  private:
    int pin;
    int umbral;

  public:
    Sensor(int p, int u) {
      pin = p;
      umbral = u;
    }

    void iniciar() override {
      pinMode(pin, INPUT);
    }

    int leer() override {
      return analogRead(pin);
    }

    int getUmbral() {
      return umbral;
    }
};

// Clase de los botones
class Boton : public Componente {
  private:
    int pin;

  public:
    Boton(int p) {
      pin = p;
    }

    void iniciar() override {
      pinMode(pin, INPUT_PULLUP);
    }

    int leer() override {
      return digitalRead(pin);
    }
};

// Clase de la LCD
class Pantalla : public Componente {
  private:
    LiquidCrystal_I2C lcd;

  public:
    Pantalla() : lcd(0x3F, 16, 4) {}

    void iniciar() override {
      lcd.init();
      lcd.backlight();
    }

    void escribir(String modo, int leds, int ldr) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Modo:");
      lcd.setCursor(0, 1);
      lcd.print(modo);
      lcd.setCursor(0, 2);
      lcd.print("LEDs ON:");
      lcd.print(leds);
      lcd.setCursor(0, 3);
      lcd.print("LDR:");
      lcd.print(ldr);
    }
};

// Objetos
Sensor ldr(A0, 25); // 
Pantalla pantalla;

LED leds[] = { 
  LED(12,13),
  LED(7,13), 
  LED(10,52), 
  LED(8,51), 
  LED(6,51),
  LED(2,28), 
  LED(3,32), 
  LED(45,32), 
  LED(4,11), 
  LED(5,50), 
  LED(9,48)
};
int cantidad = 11;

Boton btnNoche(24);
Boton btnRelaj(35);
Boton btnFiesta(33);
Boton btnLectura(29);

// Modos
enum Modo { NORMAL, RELAJACION, LECTURA, FIESTA };
Modo modo = NORMAL;

bool noche = false;
bool lastNoche = HIGH, lastRelaj = HIGH, lastFiesta = HIGH, lastLectura = HIGH;

// Fiesta
unsigned long tiempoPrevio = 0;
bool estadoFiesta = false;

// Cambio de modo

void cambiarModo(Modo nuevoModo) {
  if (modo == nuevoModo) {
    modo = NORMAL;
  } else {
    modo = nuevoModo;
  }
  noche = false;
}

// SETUP
void setup() {
  Serial.begin(9600);

  ldr.iniciar();
  pantalla.iniciar();

  btnNoche.iniciar();
  btnRelaj.iniciar();
  btnFiesta.iniciar();
  btnLectura.iniciar();

  for (int i = 0; i < cantidad; i++) {
    leds[i].iniciar();
  }

  pantalla.escribir("INICIANDO", 0, 0);
  delay(1000);
}

// LOOP
void loop() {
  int valorLDR = ldr.leer();
  int encendidos = 0;

  // Boton noche
  if (lastNoche == HIGH && btnNoche.leer() == LOW) {
    noche = !noche;
    delay(200);
  }
  lastNoche = btnNoche.leer();

  // Boton relajación
  if (lastRelaj == HIGH && btnRelaj.leer() == LOW) {
    cambiarModo(RELAJACION);
    delay(200);
  }
  lastRelaj = btnRelaj.leer();
  // boton lectura
  if (lastLectura == HIGH && btnLectura.leer() == LOW) {
    cambiarModo(LECTURA);
    delay(200);
  }
  lastLectura = btnLectura.leer();
  // Boton fiesta
  if (lastFiesta == HIGH && btnFiesta.leer() == LOW) {
    cambiarModo(FIESTA);
    delay(200);
  }
  lastFiesta = btnFiesta.leer();

  // intervalo de luz modo fiesta 
  if (millis() - tiempoPrevio >= 100) {
    tiempoPrevio = millis();
    estadoFiesta = !estadoFiesta;
  }

  if (noche) {
    for (int i = 0; i < cantidad; i++) leds[i].noche();
    pantalla.escribir("NOCHE", 0, valorLDR);
    delay(150);
    return;
  }

  String nombreModo;
  switch (modo) {
  case NORMAL:
  nombreModo = "NORMAL";
  break;
  case RELAJACION:
  nombreModo = "RELAJACION";
  break;
  case LECTURA:
  nombreModo = "LECTURA";
  break;
  case FIESTA:
  nombreModo = "FIESTA";
  break;

}
  for (int i = 0; i < cantidad; i++) {
    if (modo == NORMAL)
      encendidos += leds[i].normal(valorLDR, ldr.getUmbral());
    else if (modo == RELAJACION)
      encendidos += leds[i].relajacion(valorLDR, ldr.getUmbral());
    else if (modo == LECTURA)
      encendidos += leds[i].lectura();
    else
      encendidos += leds[i].fiesta(valorLDR, ldr.getUmbral(), estadoFiesta);
  }

  pantalla.escribir(nombreModo, encendidos, valorLDR);
  delay(150);
}
