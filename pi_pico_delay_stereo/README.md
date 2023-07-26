# Audio delay con Raspberry Pi Pico 
### Un esercizio per illustrare alcuni concetti dell'elaborazione audio real-time


Questo semplice dispositivo, realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un delay stereo con campionamento 12bit - 40Ksps (samples per second) assai semplificato, perché privo di comandi/regolazioni come anche di filtri anti aliasing agli ingressi.

Al Pi Pico è demandato il ruolo di effettuare la lettura/campionamento dei segnali in ingresso (Left - Right) utilizzando il convertitore ADC 12bit interno alla board, quindi eseguire le elaborazioni necessarie (scrivere nelle code FIFO i sample letti, leggere sample dalle code FIFO, effettuare moltiplicazioni e somme), infine inviare un flusso audio digitale 16bit - 40Ksps a ciascuno dei due DAC, utilizzando connessioni fisiche a 3 fili con protocollo I2S.

A loro volta i DAC effettuano una conversione da digitale a PWM, con frequenza 300Khz, utilizzando in uscita un circuito a bassa impedenza d'uscita ed elevata corrente, in grado quindi di pilotare direttamente gli altoparlanti 4-8Ohm; le caratteristiche di impedenza e risposta elettromeccanica degli altoparlanti abbattono e rendono inudibili le componenti audio del flusso PWM superiori alle frequenze udibili.

Nel video seguente all'ingresso del delay è stato collegato un generatore di funzioni:

https://github.com/fablabromagna-org/sound-synth-midi/assets/41198542/2fde0d05-7ad2-4d5e-9d04-8c904a834ee6
