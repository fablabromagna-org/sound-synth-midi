# Semplice audio delay con Raspberry Pi Pico 
### per la comprensione dei concetti fondamentali dell'elaborazione audio real-time

Questo semplice dispositivo, realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un delay stereo a 12bit, 40Ksps (samples per second) assai semplficato, perché privo di comandi/regolazioni come anche di filtro anti aliasing in ingresso.

Al RP Pi Pico è demandato il ruolo di effettuare la lettura dei due canali (L, R) in ingresso utilizzando il convertitore ADC 12bit interno alla board, quindi effettuare le elaborazioni necessarie (inserire i sample letti in due code FIFO, leggere le code FIFO, effettuare una somma), infine inviare i due flussi audio digitali ai due DAC con connessione/protocollo I2S.