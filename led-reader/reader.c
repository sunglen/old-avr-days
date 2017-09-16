/*
reader.c :

Mini LED message reader using Sixteen-Segments Display --
Type 5101BH (Common Anode) and ATmega48.

In order to reduce the current, a 20 Ohm resistor was put
between VCC and PIN 11 (common anode) of 5101BH display.

See #define macros for pin connection and hardware details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

/* !! F_CPU must be defined BEFORE #include delay.h */

/* use mega48 interal RC 8MHz/8=1MHz, default*/
#define F_CPU 1000000UL

/* use mega48 interal RC 8MHz/1=8MHz lfuse=0xe2*/
//#define F_CPU 8000000UL

#include <util/delay.h>

/* setting time for delay between displaying two characters */
#define DELAY_TIME_ms 0


/* define the MCU connection for each segment of LED */

/* PORT for displaying a, b, f, g, h, k, m, u */
#define SEG_PORT_1 PORTD
#define SEG_DDR_1 DDRD

/* PORT for displaying c, d, e, n, p, r, s, t */
#define SEG_PORT_2 PORTB
#define SEG_DDR_2 DDRB

/* PORT for displaying dp */
#define SEG_PORT_3 PORTC
#define SEG_DDR_3 DDRC

/* pin connection to AVR */
#define SEG_a PD0 
#define SEG_b PD7 
#define SEG_c PB5 
#define SEG_d PB2
#define SEG_e PB1
#define SEG_f PD6
#define SEG_g PD5
#define SEG_h PD3
#define SEG_k PD2
#define SEG_m PD1
#define SEG_n PB0
#define SEG_p PB4
#define SEG_r PB3
#define SEG_s PB6
#define SEG_t PB7
#define SEG_u PD4
#define SEG_dp PC0

/* set macros for toggle '.' (dot) */
#define DOT_ON 1
#define DOT_OFF 0

/* If Common Anode is not connected to VCC, it should be set below */
/*
#define COMMON_ANODE_DDR DDRC
#define COMMON_ANODE_PIN PC1
*/

/* setting time=100*N(ms) for displaying one character */
#define DELAY_CHAR 5
#define DELAY_DOT 3

/* delay routine for character display*/
void delay_n_100ms(n_100ms)
{
  int i;
  for(i=0;i<n_100ms;i++)
    _delay_ms(100);
}

/* initialize the display */
void init_disp(void)
{
  //set DDR for output.
  SEG_DDR_1 = 0xff;
  SEG_DDR_2 = 0xff;
  SEG_DDR_3 |= (1<<SEG_dp);

  //COMMON_ANODE_DDR |= (1<<COMMON_ANODE_PIN);

  //set led off.
  SEG_PORT_1 = 0xff;
  SEG_PORT_2 = 0xff;
  SEG_PORT_3 |= (1<<SEG_dp);
}

/*put one character to 16-segment display */
void led_put_char(unsigned char c)
{
  switch (c){

  case 'A':
  case 'a':
    /* display 'A' = ghkru */
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h)|(1<<SEG_k)|(1<<SEG_u));
    SEG_PORT_2 = ~(1<<SEG_r);
    break;

  case 'B':
  case 'b':
    /* display 'B' = abcdefmps */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p)|(1<<SEG_s));
    break;

  case 'C':
  case 'c':
    /* display 'C' = abefgh */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_e));
    break;

  case 'D':
  case 'd':
    /* display 'D' = abcdefms */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_s));
    break;

  case 'E':
  case 'e':
    /* display 'E' = abefghu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_e));
    break;

  case 'F':
  case 'f':
    /* display 'F' = abghpu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_p));
    break;

  case 'G':
  case 'g':
    /* display 'G' = abdefghp*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case 'H':
  case 'h':
    /* display 'H' = cdghpu*/
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_p));
    break;

  case 'I':
  case 'i':
    /* display 'I' = abefms*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_e)|(1<<SEG_s));
    break;

  case 'J':
  case 'j':
    /* display 'J' = cdefg */
    SEG_PORT_1 = ~((1<<SEG_f)|(1<<SEG_g));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e));
    break;

  case 'K':
  case 'k':
    /* display 'K' = ghnru*/
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_r));
    break;

  case 'L':
  case 'l':
    /* display 'L' = efgh*/
    SEG_PORT_1 = ~((1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_e));
    break;

  case 'M':
  case 'm':
    /* display 'M' = cdghkn */
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h)|(1<<SEG_k));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_n));
    break;

  case 'N':
  case 'n':
    /* display 'N' = cdghkr*/
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h)|(1<<SEG_k));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_r));
    break;

  case 'O':
  case 'o':
    /* display 'O' = abcdefgh*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e));
    break;

  case 'P':
  case 'p':
    /* display 'P' = abcghpu*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_p));
    break;

  case 'Q':
  case 'q':
    /* display 'Q' = abcdefghr*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_r));
    break;

  case 'R':
  case 'r':
    /* display 'R' = abcghpru*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_p)|(1<<SEG_r));
    break;

  case 'S':
  case 's':
    /* display 'S' = abdefhpu*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case 'T':
  case 't':
    /* display 'T' = abms*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_s));
    break;

  case 'U':
  case 'u':
    /* display 'U' = cdghef*/
    SEG_PORT_1 = ~((1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e));
    break;

  case 'V':
  case 'v':
    /* display 'V' = ghnt*/
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_t));
    break;

  case 'W':
  case 'w':
    /* display 'W' = cdghrt*/
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_r)|(1<<SEG_t));
    break;

  case 'X':
  case 'x':
    /* display 'X' = knrt */
    SEG_PORT_1 = ~((1<<SEG_k));
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_r)|(1<<SEG_t));
    break;

  case 'Y':
  case 'y':
    /* display 'Y' = kns */
    SEG_PORT_1 = ~((1<<SEG_k));
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_s));
    break;

  case 'Z':
  case 'z':
    /* display 'Z' = abefnt*/
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f));
    SEG_PORT_2 = ~((1<<SEG_e)|(1<<SEG_n)|(1<<SEG_t));
    break;
  
  case '0':
    /*display '0' = abcdefghnt */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_n)|(1<<SEG_t));
    break;

  case '1':
    /*display '1' = cd */
    SEG_PORT_1 = 0xff;
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d));
    break;

  case '2':
    /*display '2' = abcefgpu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case '3':
    /*display '3' = abcdefpu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case '4':
    /*display '4' = cdhpu */
    SEG_PORT_1 = ~((1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_p));
    break;

  case '5':
    /*display '5' = abefhru */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_e)|(1<<SEG_r));
    break;

  case '6':
    /*display '6' = abdefghpu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case '7':
    /*display '7' = abcd */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d));
    break;

  case '8':
    /*display '8' = abcdefghpu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case '9':
    /*display '9' = abcdefhpu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_h)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p));
    break;

  case '?':
    /*display '?' = abfn */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f));
    SEG_PORT_2 = ~((1<<SEG_n));
    break;

  case '!':
    /*display '!' = fm */
    SEG_PORT_1 = ~((1<<SEG_f)|(1<<SEG_m));
    SEG_PORT_2 = 0xff;
    break;

  case ',':
    /*display ',' = fs */
    SEG_PORT_1 = ~((1<<SEG_f));
    SEG_PORT_2 = ~((1<<SEG_s));
    break;

  case '\'':
  case '`':
    /*display '\'' = h */
    SEG_PORT_1 = ~((1<<SEG_h));
    SEG_PORT_2 = 0xff;
    break;

  case '\"':
    /*display ',' = hm */
    SEG_PORT_1 = ~((1<<SEG_h)|(1<<SEG_m));
    SEG_PORT_2 = 0xff;
    break;

  case '+':
    /*display ',' = mpsu */
    SEG_PORT_1 = ~((1<<SEG_m)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_p)|(1<<SEG_s));
    break;

  case ':':
  case ';':
    /*display ':' or ';' = ms */
    SEG_PORT_1 = ~((1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_s));
    break;

  case '{':
    /*display '{' = bemsu */
    SEG_PORT_1 = ~((1<<SEG_b)|(1<<SEG_m)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_e)|(1<<SEG_s));
    break;
  
  case '}':
    /*display '}' = afmps */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_f)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_p)|(1<<SEG_s));
    break;
  
  case '[':
  case '(':
    /*display '[' or '(' = bems */
    SEG_PORT_1 = ~((1<<SEG_b)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_e)|(1<<SEG_s));
    break;

  case ']':
  case ')':
    /*display ']' or ')' = afms */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_f)|(1<<SEG_m));
    SEG_PORT_2 = ~((1<<SEG_s));
    break;

  case '$':
    /*display '$' = abdefhmpsu */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_h)|(1<<SEG_m)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p)|(1<<SEG_s));
    break;

  case '<':
    /*display '<' = nr */
    SEG_PORT_1 = 0xff;
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_r));
    break;

  case '>':
    /*display '>' = kt */
    SEG_PORT_1 = ~((1<<SEG_k));
    SEG_PORT_2 = ~((1<<SEG_t));
    break;

  case '=':
    /*display '=' = efpu */
    SEG_PORT_1 = ~((1<<SEG_f)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_e)|(1<<SEG_p));
    break;

  case '_':
    /*display '_' = ef */
    SEG_PORT_1 = ~((1<<SEG_f));
    SEG_PORT_2 = ~((1<<SEG_e));
    break;

  case '^':
  case '~':
    /*display '^' or '~' = rt */
    SEG_PORT_1 = 0xff;
    SEG_PORT_2 = ~((1<<SEG_r)|(1<<SEG_t));
    break;

  case '%':
    /*display '%' = cgmnpstu */
    SEG_PORT_1 = ~((1<<SEG_g)|(1<<SEG_m)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_n)|(1<<SEG_p)|(1<<SEG_s)|(1<<SEG_t));
    break;

  case '@':
    /*display '@' = abcdefghps */
    SEG_PORT_1 = ~((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_h));
    SEG_PORT_2 = ~((1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_p)|(1<<SEG_s));
    break;

  case '/':
    /* display '/' = nt*/
    SEG_PORT_1 = 0xff;
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_t));
    break;

  case '-':
    /* display '-' = up*/
    SEG_PORT_1 = ~((1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_p));
    break;

  case '\\':
    /* display '\' = kr*/
    SEG_PORT_1 = ~((1<<SEG_k));
    SEG_PORT_2 = ~((1<<SEG_r));
    break;

  case ' ':
    /* display ' ' (space) */
    SEG_PORT_1 = 0xff;
    SEG_PORT_2 = 0xff;
    break;

  case '*':
  default:
    /* display '*' (asterisk) for unknown character */
    SEG_PORT_1 = ~((1<<SEG_k)|(1<<SEG_m)|(1<<SEG_u));
    SEG_PORT_2 = ~((1<<SEG_n)|(1<<SEG_p)|(1<<SEG_r)|(1<<SEG_s)|(1<<SEG_t));
    break;
  }
}

/* put 'dot' to display or clear it*/
void led_put_dot(uint8_t dot)
{
  if (dot == DOT_OFF) //clear dot
    SEG_PORT_3 |= (1<<SEG_dp);
  else //show dot
    SEG_PORT_3 &= ~(1<<SEG_dp);
}

/* put a string to display */
void led_put_str(const char *s)
{
  char c;
  
  while ((c = pgm_read_byte(s++))){
    if(c == '.'){ // act "dot" as a special character.
      led_put_dot(DOT_ON);
      delay_n_100ms(DELAY_DOT);
      led_put_dot(DOT_OFF);
    }else{
      led_put_char(c);
      delay_n_100ms(DELAY_CHAR);
    }
  }
}

int main (void)
{
  int i;

  init_disp();
  
  //led_put_str(PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ */-\\-/-"));
  //led_put_str(PSTR(" 0123456789 ,!?80%\"\'$()[]{}_+-*/\\:;<>=^@~, "));

  for(;;){

    //display simple animation.
    for(i=0;i<15;i++){
      led_put_char('-');
      _delay_ms(50);
      led_put_char('\\');
      _delay_ms(50);
      led_put_char(':');
      _delay_ms(50);
      led_put_char('/');
      _delay_ms(50);
    }

    //display your message.
    led_put_str(PSTR(" Best wishes to Olive, May joy and peace around you forever. "));
    
    //display simple animation again.
    for(i=0;i<15;i++){
      led_put_char('-');
      _delay_ms(50);
      led_put_char('\\');
      _delay_ms(50);
      led_put_char(':');
      _delay_ms(50);
      led_put_char('/');
      _delay_ms(50);
    }

    led_put_str(PSTR(" Are you going to Scarborough Fair? Parsley, sage, rosemary and thyme, Remember me to one who lives there, She once was a true love of mine. "));

    led_put_str(PSTR(" Tell her to make me a cambric shirt, Parsley, sage, rosemary and thyme, Without no seam nor needle work, And then she'll be a true love of mine. "));

  }
}

