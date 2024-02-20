#define SAMPLE_STORE 512
#define BAUD 115200
#define BUFFER_LAG 32 // Minimum permissible difference between head and tail of the ring buffer

char last = 0;

// Circular buffer structure used; array of chars with functions to push and pop elements
struct ring_buffer {
    
  int index_head = 0; // This variable should only be changed inside the ADC interrupt code to avoid conflicts
  int index_tail = 0;

  char data[SAMPLE_STORE];
  
} buf;

// buffer_push and buffer_pop return 1 if failed, 0 by default, 2 if buffer overflow occurs

// Add an element to the head of the buffer
int buffer_push(struct ring_buffer * target, char element)
{
  target->index_head = (target->index_head + 1) % SAMPLE_STORE;
  target->data[target->index_head] = element;

  return 0;
}

// Read an element from the tail of the buffer into pointer provided by argument 'target' and remove that element from the buffer 
int buffer_pop(struct ring_buffer * source, char * target)
{  
   if (source->index_head == source->index_tail)
     return 1;

   source->index_tail = (source->index_tail + 1) % SAMPLE_STORE;
   *target = source->data[source->index_tail];

    return 0;
}

void ADC_setup(int sample_rate)
{
  cli();//disable interrupts
  
  //Set up continuous sampling of analog pin 0
  
  //Clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //Set reference voltage to internal 1.1v reference
  ADMUX |= (1 << REFS1);
  ADMUX |= (1 << ADLAR); //Left align the ADC value- so we can read highest 8 bits from ADCH register only

  //Set ADC clock with 32 prescaler- 16mHz/32=500kHz
  // The ADC needs 13 clock cycles, so this corresponds to sampling freq. of 38462 Hz
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
  ADCSRA |= (1 << ADATE); //Enabble auto trigger
  ADCSRA |= (1 << ADIE); //Enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //Enable ADC
  ADCSRA |= (1 << ADSC); //Start ADC measurements
  
  sei();//Enable interrupts
}

void setup()
{
  Serial.begin(BAUD);
  
  ADC_setup(38462);
}

ISR(ADC_vect)
{
  last = ADCH;
  buffer_push(&buf, ADCH);
}

void loop()
{
  char t;
  if(!buffer_pop(&buf, &t))
    Serial.write(t); 
}
