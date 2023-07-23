# Semplice audio delay con Raspberry Pi Pico 
### per la comprensione dei concetti fondamentali dell'elaborazione audio real-time


Questo semplice dispositivo, realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un delay stereo a 12bit - 40Ksps (samples per second) assai semplificato, perché privo di comandi/regolazioni come anche di filtri anti aliasing agli ingressi.

Al RP Pi Pico è demandato il ruolo di effettuare la lettura dei due canali (L, R) in ingresso utilizzando il convertitore ADC 12bit interno alla board, quindi effettuare le elaborazioni necessarie (scrivere i sample letti nelle code FIFO, leggere sample dalle code FIFO, effettuare moltiplicazioni e somme), infine inviare due flussi audio digitali ai due DAC utilizzando connessioni fisiche a 3 fili con protocollo I2S.