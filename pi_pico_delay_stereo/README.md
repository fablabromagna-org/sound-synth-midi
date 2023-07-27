# Audio delay con Raspberry Pi Pico 
### Un esercizio per illustrare alcuni concetti dell'elaborazione audio real-time


Questo semplice dispositivo, realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un effetto echo (comunemente detto delay perchè basato su un delay) stereo, con campionamento 12bit - 40Ksps (samples per second) assai semplificato, perché privo di comandi/regolazioni come anche di filtri anti aliasing agli ingressi.

Al Pi Pico è demandato il ruolo di effettuare la lettura/campionamento dei segnali in ingresso (Left - Right) utilizzando il convertitore ADC 12bit interno alla board, quindi eseguire le elaborazioni necessarie (scrivere nelle code FIFO i sample letti, leggere sample dalle code FIFO, effettuare moltiplicazioni e somme), infine inviare un flusso audio digitale 16bit - 40Ksps a ciascuno dei due DAC, utilizzando connessioni fisiche a 3 fili con protocollo I2S.

A loro volta i DAC effettuano una conversione da digitale a PWM, con frequenza 300Khz, utilizzando in uscita un circuito a bassa impedenza d'uscita ed elevata corrente, in grado quindi di pilotare direttamente gli altoparlanti 4-8Ω; le caratteristiche di impedenza e risposta elettromeccanica degli altoparlanti abbattono e rendono inudibili le componenti audio del flusso PWM superiori alle frequenze udibili.

Nel video seguente una breve registrazione dell'effetto; all'ingresso del delay è stato collegato un generatore di funzioni con sweep regolato manualmente:

https://github.com/fablabromagna-org/sound-synth-midi/assets/41198542/2fde0d05-7ad2-4d5e-9d04-8c904a834ee6




#### Schema a blocchi

Lo schema ingresso-uscita seguente illustra l'articolazione del delay nei suoi componenti (sommatore, moltiplicatori per costanti, delay), l'interconnessione tra gli stessi e la direzione del flusso di elaborazione.

<p align="left">
<img width="600" src="/pi_pico_delay_stereo/media/delay_0.jpg")
</p>

Come nasce questo schema? Da una semplice intuizione: per esperienza comune un suono con eco è dato dalla somma (in aria) del suono origine con le sue riflessioni (che giungono in ritardo all'origine) e con le riflessioni delle riflessioni.

Nel passaggio alla "macchina", questo schema potrà essere realizzato in tutto/in parte tenendo conto delle modalità operative di un algoritmo digitale per applicazioni audio, che presenta questi limiti/caratteristiche:
- valori discreti: la quantizzazione numerica opera per valori discreti, non continui
- lunghezza di parola finita: range numerico (dinamica) limitata
- tempo discreto (quantizzato): l'elaborazione avviene mandatoriamente su base temporale discreta
- numero di operazioni finito: ogni "giro di calcolo" deve avvenire in un tempo finito

Soffermiamoci sulla caretteristica temporale del tempo discreto: in ingresso si riceve una successione di valori (sample) misurati dell'ingresso, poi si esegue una sequenza finita di passi computazionali, infine si fornisce in uscita una sequenza di valori.

Il nostro schema a blocchi è quindi da interpretare nel tempo discreto, eseguendo una sequenza finita di operazioni; in termini di codice lo schema si traduce in una singola funzione lineare. Se:
- D è il valore del ritardo introdotto dal blocco delay
- x(t) il sample in ingresso al tempo t
- y(t) il sample in uscita al tempo t

il calcolo è il seguente:
Y(t) = C*X(t) + K*Y(t-D)

La presenza dei parametri C e K è prima di tutto dovuta al fatto che la lunghezza di parola è finita, ed essi aiutano a prevenire fenomeni di saturazione numerica. Il parametro K ha una ulteriore importante ricaduta, perchè da lui dipende la "stabilità" dell'algoritmo: per valori >= 1 il calcolo diverge rapidamente in presenza del più piccolo e breve segnale in ingresso. Tratteremo il tema della stabilità nelle sezioni successive introducendo ulteriori semplici strumenti di analisi.