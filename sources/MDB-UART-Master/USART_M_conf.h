#define  EXT0_MDB1
	
#ifdef EXT0_MDB1

#define MDB_UBRRH      UBRR1H
#define MDB_UBRRL      UBRR1L
#define MDB_UCSR_A     UCSR1A
#define MDB_UCSR_B     UCSR1B
#define MDB_UCSR_C     UCSR1C
#define MDB_UDR        UDR1
#define MDB_TXB8       TXB81
#define MDB_RXC        RXC1
#define MDB_UDRE       UDRE1
#define MDB_RXEN	   RXEN1
#define MDB_UMSEL_1    UMSEL11

#define EXT_UBRRH      UBRR0H
#define EXT_UBRRL      UBRR0L
#define EXT_UCSR_A     UCSR0A
#define EXT_UCSR_B     UCSR0B
#define EXT_UCSR_C     UCSR0C
#define EXT_UDR        UDR0
#define EXT_TXEN       TXEN0
#define EXT_RXEN       RXEN0
#define EXT_RXCIE      RXCIE0
#define EXT_UDRE       UDRE0
#define EXT_RXC        RXC0
#define EXT_USART_RX_vect  USART0_RX_vect

//bity
#define UMSEL0   UMSEL10
#define UMSEL1   UMSEL11
#define UPM0     UPM10
#define UPM1     UPM11
#define USBS     USBS1
#define UCSZ0    UCSZ10
#define UCSZ1    UCSZ11
#define UCSZ2    UCSZ12
#define RXEN     RXEN1
#define TXEN     TXEN1

#define EXT_UMSEL0   UMSEL00
#define EXT_UMSEL1   UMSEL01
#define EXT_UPM0     UPM00
#define EXT_UPM1     UPM01
#define EXT_USBS     USBS0
#define EXT_UCSZ0    UCSZ00
#define EXT_UCSZ1    UCSZ01
#define EXT_UCSZ2    UCSZ02
#define EXT_RXEN     RXEN0
#define EXT_TXEN     TXEN0



#endif

#ifdef EXT1_MDB0

#define MDB_UBRRH      UBRR0H
#define MDB_UBRRL      UBRR0L
#define MDB_UCSR_A     UCSR0A
#define MDB_UCSR_B     UCSR0B
#define MDB_UCSR_C     UCSR0C
#define MDB_UDR        UDR0
#define MDB_TXB8       TXB80
#define MDB_RXC        RXC0
#define MDB_UDRE       UDRE0
#define MDB_RXEN	   RXEN0
#define MDB_UMSEL_1    UMSEL01

#define EXT_UBRRH      UBRR1H
#define EXT_UBRRL      UBRR1L
#define EXT_UCSR_A     UCSR1A
#define EXT_UCSR_B     UCSR1B
#define EXT_UCSR_C     UCSR1C
#define EXT_UDR        UDR1
#define EXT_TXEN       TXEN1
#define EXT_RXEN       RXEN1
#define EXT_RXCIE      RXCIE1
#define EXT_UDRE       UDRE1
#define EXT_RXC        RXC1
#define EXT_USART_RX_vect  USART1_RX_vect

//bity
#define UMSEL0   UMSEL00
#define UMSEL1   UMSEL01
#define UPM0     UPM00
#define UPM1     UPM01
#define USBS     USBS0
#define UCSZ0    UCSZ00
#define UCSZ1    UCSZ01
#define UCSZ2    UCSZ02
#define RXEN     RXEN0
#define TXEN     TXEN0


#define EXT_UMSEL0   UMSEL10
#define EXT_UMSEL1   UMSEL11
#define EXT_UPM0     UPM10
#define EXT_UPM1     UPM11
#define EXT_USBS     USBS1
#define EXT_UCSZ0    UCSZ10
#define EXT_UCSZ1    UCSZ11
#define EXT_UCSZ2    UCSZ12
#define EXT_RXEN     RXEN1
#define EXT_TXEN     TXEN1

#endif
